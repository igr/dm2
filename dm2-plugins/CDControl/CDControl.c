#include <windows.h>
#include "../dm2Plugins.h"
#include "resource.h"

#ifdef NDEBUG
#pragma optimize("gsy", on)
#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/IGNORE:4078 /IGNORE:4089")
#pragma comment(linker, "/RELEASE")
#pragma comment(linker, "/merge:.rdata=.data")
#pragma comment(linker, "/merge:.text=.data")
#pragma comment(linker, "/merge:.reloc=.data")
#if _MSC_VER >= 1000
#pragma comment(linker, "/FILEALIGN:0x200")
#endif
#endif

#ifdef NDEBUG
#pragma comment(lib, "msvcrt.lib")
#pragma comment(linker, "/NODEFAULTLIB:LIBC.lib")
#else
#pragma comment(lib, "msvcrtd.lib")
#pragma comment(linker, "/NODEFAULTLIB:LIBCD.lib")
#endif

#define CD_DRIVE_AUTO	0
#define CD_DRIVE_OPEN	1
#define CD_DRIVE_CLOSE	2

static char *ver = "CDControl v0.04";
static char *cmd = "dm2_ext_cmd_cdcontrol";
static char *cmdcmt = "CD-ROM Control (Plugins)";

HWND hServer;
BOOL AutoClose;
int ACTime;
void CD_OpenCloseDrive(int OpenDrive, char cDrive)
{
	MCI_OPEN_PARMS op;
	MCI_STATUS_PARMS st;
	DWORD flags;

	char szDriveName[4];
	lstrcpy(szDriveName, "X:");

	ZeroMemory(&op, sizeof(MCI_OPEN_PARMS));
	op.lpstrDeviceType = (LPCSTR) MCI_DEVTYPE_CD_AUDIO;
	
	if(cDrive > 1)
	{
		szDriveName[0] = cDrive;
		op.lpstrElementName = szDriveName;
		flags = MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE;
	}
	else flags = MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE;

	if (!mciSendCommand(0, MCI_OPEN, flags, (unsigned long)&op)) 
	{
		st.dwItem = MCI_STATUS_READY;

		if(OpenDrive == CD_DRIVE_OPEN)
		{
			mciSendCommand(op.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, 0);
			//set auto close.
			if(AutoClose) SetTimer(hServer, cDrive, ACTime*1000, NULL);
		}
		else if(OpenDrive == CD_DRIVE_CLOSE)
		{
			//if cd door closed auto close stop. 
			if(AutoClose) KillTimer(hServer, cDrive);
			mciSendCommand(op.wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, 0);
		}
		else if(OpenDrive == CD_DRIVE_AUTO)
		{
			int n, i = GetTickCount();
			mciSendCommand(op.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, 0);
			if(AutoClose) SetTimer(hServer, cDrive, ACTime*1000, NULL);
			n = GetTickCount();
			if(n - i < 200)
			{
				if(AutoClose) KillTimer(hServer, cDrive);
				mciSendCommand(op.wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, 0);
			}
		}

		mciSendCommand(op.wDeviceID, MCI_CLOSE, MCI_WAIT, 0);
	}
}

void CD_OpenCloseAllDrives(int OpenDrives)
{
	// Determine All CD Drives and Open (or Close) each one
    int n = 0;
    UINT nCount = 0;
    char szDrive[4];
    DWORD dwDriveList = GetLogicalDrives();

	lstrcpy(szDrive, "?:\\");

    while (TRUE) {
		szDrive[0] = 'c'+n;
        if (szDrive[0] <= 'z') 
		{
			if(GetDriveType(szDrive) == DRIVE_CDROM)
				CD_OpenCloseDrive(OpenDrives, szDrive[0]);
		}
        dwDriveList >>= 1;
        n++;
    }
}

LRESULT WINAPI CDCProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_TIMER:
		CD_OpenCloseDrive(CD_DRIVE_CLOSE, (char)wParam);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

HINSTANCE hPlugin;
char *CDCServer = "CD Control Server";
DWORD WINAPI RunServer(LPVOID lpParameter)
{
	MSG msg;
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = CDCProc;
	wc.hInstance	 = hPlugin;
	wc.lpszClassName = CDCServer;
	if (RegisterClassEx(&wc) == FALSE) {
		return FALSE;
	}
	hServer = CreateWindow(CDCServer, "", 0, -1, -1, 0, 0, 
		NULL, NULL, hPlugin, NULL);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}

HANDLE hThread;
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				DWORD tid;
				hPlugin = hModule;
				hThread = CreateThread(NULL, 0, RunServer, NULL, 
					NORMAL_PRIORITY_CLASS, &tid);
			}
			break;
		case DLL_PROCESS_DETACH:
			DestroyWindow(hServer);
			TerminateThread(hThread, TRUE);
			CloseHandle(hThread);
			break;
	}
	return TRUE;
}

void ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "CDControl plugins v0.04\r--help you Control CD-ROM.\r\rCode by flyfancy. 2007.3", "About",
		MB_OK|MB_ICONINFORMATION);
}

void ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	int nOpenClose;	// See #define's above
	int nLen;
	char cDrive = 0;
	char *pDrv;

	//get drive control command.
	if(argc == 2)
	{
		if(lstrcmpi(argv[0], "open") == 0)
			nOpenClose = CD_DRIVE_OPEN;
		else if(lstrcmpi(argv[0], "close") == 0)
			nOpenClose = CD_DRIVE_CLOSE;
		else
			return;
		pDrv = argv[1];
	}
	else if(argc == 1)
	{
		nOpenClose = CD_DRIVE_AUTO;
		pDrv = argv[0];
	}
	else
		return;
	
	//get drive.
	if(lstrcmpi(pDrv, "all") == 0)
		cDrive = 0;
	else
	{
		//this api have a error can't get all driver some time
		//DWORD dwDriveList = GetLogicalDrives();
		nLen = lstrlen(pDrv);
		CharLower(pDrv);
		if(nLen>2) return;
		if(nLen == 2 && pDrv[1] != ':') return;
		if(isdigit(pDrv[0]))
		{
			char szRoot[4];
			int j = 0, n = 1, i = atoi(pDrv);
			if(i < 1 || i > 23)
				return;
			lstrcpy(szRoot, "?:\\");
			while(TRUE)
			{
				szRoot[0] = 'c'+n;
				if (szRoot[0] <= 'z') 
				{
					if(GetDriveType(szRoot) == DRIVE_CDROM)
						j++;
					if(j == i)
					{
						cDrive = szRoot[0];
						break;
					}
				}
				n++;
			}
		}
		else if((pDrv[0] <= 'c') || (pDrv[0] > 'z')) return;
		else cDrive = pDrv[0];
	}

	//control drive.
	if(cDrive == 0)
		CD_OpenCloseAllDrives(nOpenClose);
	else
		CD_OpenCloseDrive(nOpenClose, cDrive);


}

char *pIni;
char *pSec = "Plugins_Settings";
char *pKey = "CD_AutoClose";
char *pKey1 = "CD_ACTime";
INT_PTR CALLBACK CfgDlgProc(HWND hwndDlg, UINT uMsg, 
								   WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetWindowText(hwndDlg, ver);

		if(AutoClose)
			CheckDlgButton(hwndDlg, IDC_CHK_AUTOCLOSE, BST_CHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_TIME), AutoClose);
		{
			char szTime[4];
			HWND hCombo = GetDlgItem(hwndDlg, IDC_COMBO_TIME);
			wsprintf(szTime, "%d", ACTime);
			SendMessage(hCombo, WM_SETTEXT, 0, (LPARAM)szTime);
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"5");
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"10");
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"15");
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"20");
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"30");
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"60");
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"90");
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"120");
		}
		return TRUE;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char szTime[4];
				GetDlgItemText(hwndDlg, IDC_COMBO_TIME, szTime, 4);
				ACTime = atoi(szTime);
				if(ACTime < 5) ACTime = 5;
				else if(ACTime > 120) ACTime = 120;

				AutoClose = (IsDlgButtonChecked(hwndDlg, IDC_CHK_AUTOCLOSE) == BST_CHECKED);

				//write ini
				wsprintf(szTime, "%d", AutoClose);
				WritePrivateProfileString(pSec, pKey, szTime, pIni);
				wsprintf(szTime, "%d", ACTime);
				WritePrivateProfileString(pSec, pKey1, szTime, pIni);
			}
			EndDialog(hwndDlg, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		case IDC_CHK_AUTOCLOSE:
			if(IsDlgButtonChecked(hwndDlg, IDC_CHK_AUTOCLOSE) == BST_CHECKED)
				AutoClose = TRUE;
			else
				AutoClose = FALSE;
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_TIME), AutoClose);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return FALSE;
}

void NeedConfig(HWND hwnd_dm2, char *szini)
{
	pIni = szini;
	DialogBox(hPlugin, (LPCTSTR)IDD_CDC, hwnd_dm2, CfgDlgProc);
}

void InitPlugins(char *szini)
{
	AutoClose = (GetPrivateProfileInt(pSec, pKey, 0, szini) != 0);
	ACTime = GetPrivateProfileInt(pSec, pKey1, 15, szini);
	if(ACTime < 5) ACTime = 5;
	else if(ACTime > 120) ACTime = 120;
}

void dm2_get_interface(PFF_PLUGINS ffp)
{
	ffp->About = ShowAbout;
	ffp->Config = NeedConfig;
	ffp->Execute = ExecPlugins;
	ffp->InitPlugins = InitPlugins;
	ffp->pName = ver;
	ffp->pComments = cmdcmt;
	ffp->pCmd = cmd;
	ffp->PluginsCategory = FFPC_HOTKEY;
	ffp->SDKversion = PLUGINSVER;
}
