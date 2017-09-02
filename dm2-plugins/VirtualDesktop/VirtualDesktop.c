#include "VirtualDesktop.h"
#include "../DM2Plugins.h"
#include "resource.h"


#ifdef NDEBUG
#pragma optimize("gsy", on)
#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/IGNORE:4078 /IGNORE:4089")
#pragma comment(linker, "/RELEASE")
#pragma comment(linker, "/merge:.rdata=.data")
//#pragma comment(linker, "/merge:.text=.data")
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

void NeedConfig(HWND hwnd_dm2, char *szini);

HINSTANCE hPlugin;
LPVDOPT VDOpt;
int CVD = 0;
char *Ini;
NOTIFYICONDATA nid;
char *pTip = "VirtualDesktop v0.02 [Activated: %s]";
char *szVDMenuItem = "&%d. %s ";
MENUITEMINFO VDMenu = {sizeof(MENUITEMINFO), MIIM_ID | 0x40| MIIM_STATE, 
						MFT_STRING, MFS_ENABLED, 0};
void SwitchVD()
{
	if(VDOpt->bShowTray)
	{
		nid.hIcon = LoadIcon(hPlugin, (LPCTSTR)CVD+IDI_VD1);
		wsprintf(nid.szTip, pTip, VDOpt->VDName[CVD]);
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
	SwitchVirtualDesktop(CVD);
}

HWND hServer;
extern HWND hOpt;
extern HWND hArrange;
LRESULT WINAPI ServerProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_OPTION: // show option
			NeedConfig(hWnd, Ini);
			break;
		case ID_ARRANGE: // show arrange window
			if(hOpt)
				EndDialog(hOpt, IDCANCEL);
			if(hArrange)
				ShowWindow(hArrange, SW_SHOW);
			else
				DialogBox(hPlugin, (LPCTSTR)IDD_DLG_ARRANGE, hWnd, VDArrangeDlgProc);
			break;
		case ID_HIDETRAY: // hide tray
			VDOpt->bShowTray = FALSE;
			Shell_NotifyIcon(NIM_DELETE, &nid);
			break;
		case ID_DISABLE: // disable vd
			VDOpt->bEnable = FALSE;
			DestroyVirtualDesktop();
			Shell_NotifyIcon(NIM_DELETE, &nid);
			break;
		default:
			if(LOWORD(wParam) >= IDM_VD1 && LOWORD(wParam) <= IDM_VD8)
			{
				int vd = LOWORD(wParam) - IDM_VD1;
				if(vd < VDOpt->VDCount && vd != CVD)
				{
					if(hArrange)
						EndDialog(hArrange, IDCANCEL);
					CVD = vd;
					SwitchVD();
				}
			}
			break;
		}
		break;
	case WM_USER+13:
		//Show Tray Menu
		if(WM_RBUTTONUP == lParam)
		{
			HMENU menu;
			POINT p;
			int i = 0;
			char szMIBuf[MAX_PATH];

			GetCursorPos(&p);
			menu = LoadMenu(hPlugin, (LPCTSTR)IDR_MENU_TRAY);
			menu = GetSubMenu(menu, 0);

			DeleteMenu(menu, 5, MF_BYPOSITION);
			for(; i<VDOpt->VDCount; i++)
			{
				VDMenu.wID = IDM_VD1+i;
				wsprintf(szMIBuf, szVDMenuItem, i+1, VDOpt->VDName[i]);
				
				VDMenu.dwTypeData = szMIBuf;
				if(i == CVD)
					lstrcat(szMIBuf, " [Activated]");
				VDMenu.cch = lstrlen(szMIBuf);
				InsertMenuItem(menu, 5+i, TRUE, &VDMenu);
			}

			SetForegroundWindow(hWnd);
			TrackPopupMenuEx(menu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON
				| TPM_RIGHTBUTTON, p.x, p.y, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
			DestroyMenu(menu);
		}
		else if(WM_LBUTTONDBLCLK == lParam)
		{
			if(++CVD == VDOpt->VDCount)
				CVD = 0;

			SwitchVD();
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

char *pSection = "Plugins_VirtualDesktop_Settings";
char *pKeyEnable = "Enable";
char *pKeyCount = "Count";
char *pKeyTray = "TrayEnable";
char *pKeyName = "Name%d";
char *pDefaultName = "Desk %d";
char *szServer = "DM2_Plugin_VirtualDesktop_Server";
HANDLE hThread = NULL;
HINSTANCE hPlugin;
DWORD WINAPI RunServer(LPVOID lpParameter)
{
	char NameKey[6], DefaultName[7];
	int i = 1;
	MSG msg;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = ServerProc;
	wc.hInstance	 = hPlugin;
	wc.lpszClassName = szServer;
	if (RegisterClassEx(&wc) == FALSE) {
		return FALSE;
	}
	hServer = CreateWindow(szServer, "", 0, -1, -1, 0, 0, 
		NULL, NULL, hPlugin, NULL);

	VDOpt = malloc(sizeof(VDOPT));

	for(; i<=8; i++)
	{
		wsprintf(NameKey, pKeyName, i);
		wsprintf(DefaultName, pDefaultName, i);
		GetPrivateProfileString(pSection, NameKey, DefaultName, 
			VDOpt->VDName[i-1], MAX_PATH, Ini);
	}
	VDOpt->bEnable = (GetPrivateProfileInt(pSection, pKeyEnable, 1, Ini) != 0);
	VDOpt->VDCount = setIntRange(GetPrivateProfileInt(pSection, pKeyCount, 4, Ini), 2, 8);
	VDOpt->bShowTray = (GetPrivateProfileInt(pSection, pKeyTray, 1, Ini) != 0);

	InitVirtualDesktop();

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hWnd = hServer;
	nid.uID = WM_USER+13;
	nid.uCallbackMessage = WM_USER+13;
	wsprintf(nid.szTip, pTip, VDOpt->VDName[0]);
	if(VDOpt->bEnable && VDOpt->bShowTray)
	{
		nid.hIcon = LoadIcon(hPlugin, (LPCTSTR)IDI_VD1);
		Shell_NotifyIcon(NIM_ADD, &nid);
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		hPlugin = hModule;
	}
	else if(ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		DestroyWindow(hServer);
		TerminateThread(hThread, TRUE);
		CloseHandle(hThread);
		DestroyVirtualDesktop();
		free(VDOpt);
		Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	return TRUE;
}

void ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "VirtualDesktop plugins v0.03\r--enable you have 2~8 virtual deaktop and switch it easy.\r\rcode by flyfancy. 2006.08.15", "About",
		MB_OK|MB_ICONINFORMATION);
}

int iLastWidth = 0;
void ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	if(VDOpt->bEnable)
	{
		if(argc == 1)
		{
			int a = atoi(argv[0]);
			//skip param error
			if((a <= VDOpt->VDCount) && (a > 0) && (--a != CVD))
				CVD = a;
			else
				return;
		}
		else if(argc == 0)
		{
			//do switch virtual desktop.
			if(++CVD == VDOpt->VDCount)
				CVD = 0;
		}
		else
		{
			//if param more...
			return;
		}

		SwitchVD();
	}
}

void InitPlugins(char *szini)
{
	DWORD tid;
	
	Ini = szini;
	hThread = CreateThread(NULL, 0, RunServer, NULL, 
		NORMAL_PRIORITY_CLASS, &tid);
}

extern INT_PTR CALLBACK VDCfgDlgProc(HWND, UINT, WPARAM, LPARAM);
void NeedConfig(HWND hwnd_dm2, char *szini)
{
	if(hArrange)
		EndDialog(hArrange, IDCANCEL);
	if(hOpt)
		SetForegroundWindow(hOpt);
	else
		DialogBox(hPlugin, (LPCTSTR)IDD_DLG_OPTION, hwnd_dm2, VDCfgDlgProc);
}
//
//BOOL PluginsState(void)
//{
//	return TRUE;
//}

static char *ver = "VirtualDesktop v0.03";
static char *cmd = "dm2_ext_cmd_virtualdesktop";
static char *cmdcmt = "Switch Virtual Desktop (Plugins)";

void __cdecl dm2_get_interface(PFF_PLUGINS ffp)
{
	ffp->About = ShowAbout;
	ffp->InitPlugins = InitPlugins;
	ffp->Execute = ExecPlugins;
	ffp->Config = NeedConfig;
//	ffp->State = PluginsState;
	ffp->pName = ver;
	ffp->pComments = cmdcmt;
	ffp->pCmd = cmd;
	ffp->PluginsCategory = FFPC_HOTKEY;
	ffp->SDKversion = PLUGINSVER;
}
