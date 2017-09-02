#include <windows.h>
#include "../dm2Plugins.h"
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


LRESULT CALLBACK WinEnableMenuHookProc(int code, WPARAM wParam, LPARAM lParam);
void EnableAllMenu(HMENU hMenu);
LRESULT CALLBACK WinEnableMouseHookProc(int code, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
BOOL Tray;

HINSTANCE hPlugins;
HICON hIcon;
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		hPlugins = hModule;
		hIcon = LoadIcon(hModule, (LPCTSTR)IDI_ICON);
	}
	return TRUE;
}

HHOOK hWin_Mouse = NULL;
HHOOK hWin_Menu = NULL;
void ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "WinEnable plugins v0.04 beta\r--Allows the user to enable disabled windows and menu.\r\rCode by flyfancy. 2005", "About",
		MB_OK|MB_ICONINFORMATION);
}

static char *ver = "WinEnable v0.04 beta";
char *pSec = "Plugins_Settings";
char *pKey = "WinEnableTrayIcon";
char *ini;
INT_PTR CALLBACK CfgDlgProc(HWND hwndDlg, UINT uMsg, 
								   WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetWindowText(hwndDlg, ver);
		if(Tray)
			CheckDlgButton(hwndDlg, IDC_CHK_TRAY, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_R1, BST_CHECKED);
		return TRUE;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(IsDlgButtonChecked(hwndDlg, IDC_CHK_TRAY) == BST_CHECKED)
			{
				WritePrivateProfileString(pSec, pKey, "1", ini);
				Tray = TRUE;
			}
			else
			{
				WritePrivateProfileString(pSec, pKey, "0", ini);
				Tray = FALSE;
			}
			EndDialog(hwndDlg, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
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
	ini = szini;
	DialogBox(hPlugins, (LPCTSTR)IDD_DIALOG_CFG, hwnd_dm2, CfgDlgProc);
}

void InitPlugins(char *szini)
{
	Tray = (GetPrivateProfileInt(pSec, pKey, 0, szini) != 0);
}

BOOL Skip = FALSE;
void ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uFlags = NIF_ICON | NIF_TIP;
	nid.hWnd = hwnd_dm2;
	nid.uID = (UINT)hwnd_dm2 - 13;
	nid.hIcon = hIcon;
	if(bUnNeed == TRUE)
	{
		if(hWin_Mouse)
		{
			UnhookWindowsHookEx(hWin_Mouse);
			hWin_Mouse = NULL;
		}
		if(hWin_Menu)
		{
			UnhookWindowsHookEx(hWin_Menu);
			hWin_Menu = NULL;
		}
		if(Tray)
			Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	else
	{
		Skip = TRUE;
		if(hWin_Mouse == NULL)
			hWin_Mouse = SetWindowsHookEx(WH_MOUSE, 
			WinEnableMouseHookProc, hPlugins, 0);
		if(hWin_Menu == NULL)
			hWin_Menu = SetWindowsHookEx(WH_CALLWNDPROCRET, 
			WinEnableMenuHookProc, hPlugins, 0);
		if(Tray)
		{
			lstrcpy(nid.szTip, ver);
			lstrcat(nid.szTip, " - [dm2 plugins]");
			Shell_NotifyIcon(NIM_ADD, &nid);
		}
	}
}

BOOL PluginsState(void)
{
	return (hWin_Mouse || hWin_Menu);
}

static char *cmd = "dm2_ext_cmd_winenable";
static char *cmdcmt = "Toggle WinEnable module (Plugins)";

void dm2_get_interface(PFF_PLUGINS ffp)
{
	ffp->About = ShowAbout;
	ffp->Config = NeedConfig;
	ffp->Execute = ExecPlugins;
	ffp->State = PluginsState;
	ffp->InitPlugins = InitPlugins;
	ffp->pName = ver;
	ffp->pComments = cmdcmt;
	ffp->pCmd = cmd;
	ffp->PluginsCategory = FFPC_HOTKEY;
	ffp->SDKversion = PLUGINSVER;
}

POINT m_pi;
LRESULT CALLBACK WinEnableMenuHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		PCWPRETSTRUCT pcs = (PCWPRETSTRUCT)lParam;
		int msg = pcs->message;

		if((msg == WM_INITMENU || msg == WM_INITMENUPOPUP) &&
			Skip == FALSE)
		{
			//process disable menu.
			EnableAllMenu((HMENU)pcs->wParam);
		}
		else if(msg == WM_MOUSEACTIVATE && HIWORD(lParam) == WM_LBUTTONDOWN)
		{
			GetCursorPos(&m_pi);
			EnumChildWindows(pcs->hwnd, EnumChildProc, FALSE);
		}
	}
	return CallNextHookEx(hWin_Menu, code, wParam, lParam);
}

void EnableAllMenu(HMENU hMenu)
{
	int i = 0;
	int j = GetMenuItemCount(hMenu);
	for(; i<j; i++)
	{
		EnableMenuItem(hMenu, i, MF_BYPOSITION|MF_ENABLED);
	}
}

LRESULT CALLBACK WinEnableMouseHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0 && wParam == WM_LBUTTONDOWN)
	{
		PMOUSEHOOKSTRUCT pm = (PMOUSEHOOKSTRUCT)lParam;

		if(Skip == FALSE)
		{
			m_pi.x = pm->pt.x;
			m_pi.y = pm->pt.y;
			//process disable window or control.
			EnumChildWindows(pm->hwnd, EnumChildProc, FALSE);
		}
	}
	return CallNextHookEx(hWin_Mouse, code, wParam, lParam);
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);
	if(PtInRect(&rc, m_pi))
	{
		EnableWindow(hwnd, TRUE);
	}
	else
		EnumChildWindows(hwnd, EnumChildProc, FALSE);

	return TRUE;
}
