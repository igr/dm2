/* http://dm2.sf.net */
#include <windows.h>
#include "../global.h"


extern HHOOK msgHook;
int last = -1;
DWORD lasttick = -1;
/* GetMsgHook procedure, scans WM_SYSCOMMAND messages */
LRESULT CALLBACK GetMsgHookProc(int code, WPARAM wParam, LPARAM lParam) {
	int msg;
	HWND hwnd;
	if (code < 0) {
		return CallNextHookEx(msgHook, code, wParam, lParam);
	}
	msg = ((MSG*)lParam)->message;
	hwnd = ((MSG*)lParam)->hwnd;
	if (msg == WM_SYSCOMMAND) {
		int menuCmd = LOWORD(((MSG*)lParam)->wParam);
		lParam = LOWORD(((MSG*)lParam)->lParam);
		if ((menuCmd >= IDM_SYS_TRAY) && (menuCmd <= (IDM_SYS_PRIO_R)) ) {
			BOOL repeated = FALSE;
			int tick = GetTickCount();

			if (last == lParam) {		// mouse position is the same...
				repeated = TRUE;
				if (lParam == 0) {
					if ((tick - lasttick) > 100) {		// when keyboard is used, mouse pos is 0
						repeated = FALSE;				// so time is used to checked duplicates
					}
				}
			}
			if (repeated == FALSE) {
				// from some reasons, some apps duplicate syscommand messages (i.e. WordPad)
				// like this message are not duplicated
				if(menuCmd == IDM_SYS_TOP)
				{
					HMENU hSysMenu = GetSystemMenu(hwnd, FALSE);
					if (hSysMenu) {
						if (GetMenuState(hSysMenu, IDM_SYS_TOP, MF_BYCOMMAND) & MF_CHECKED) {
							SetWindowPos(hwnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
						} else {
							SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
						}
					}
				}
				else if((menuCmd <= (IDM_SYS_OPA+10)) && (menuCmd > IDM_SYS_OPA))
				{
					HMENU hSysMenu = GetSystemMenu(hwnd, FALSE);
					MENUITEMINFO mi = {sizeof(MENUITEMINFO), MIIM_SUBMENU, 0};
					GetMenuItemInfo(hSysMenu, IDM_SYS_OPA, FALSE, &mi);
					if(mi.hSubMenu)
					{
						int i = 1;
						for(; i<=10; i++)
							CheckMenuItem(mi.hSubMenu, i-1, 
								MF_BYPOSITION | MF_UNCHECKED);
						CheckMenuItem(mi.hSubMenu, menuCmd, 
							MF_BYCOMMAND | MF_CHECKED);
					}
					PostMessage(pshared->DM2wnd, IDM_SYS_OPA, menuCmd, (LPARAM) hwnd);
				}
				//add at 1.20 Set Process Priority
				else if((menuCmd <= (IDM_SYS_PRIO_R)) && (menuCmd > IDM_SYS_PRIO))
				{
					HMENU hSysMenu = GetSystemMenu(hwnd, FALSE);
					MENUITEMINFO mi = {sizeof(MENUITEMINFO), MIIM_SUBMENU, 0};
					GetMenuItemInfo(hSysMenu, IDM_SYS_PRIO, FALSE, &mi);
					if(mi.hSubMenu)
					{
						DWORD dwPriorityClass;
						DWORD PID;
						HANDLE hProcess;
						switch(menuCmd)
						{
						case IDM_SYS_PRIO_I:
							dwPriorityClass = IDLE_PRIORITY_CLASS;
							break;
						case IDM_SYS_PRIO_B:
							dwPriorityClass = 16384;	//BELOW_NORMAL_PRIORITY_CLASS
							break;
						case IDM_SYS_PRIO_N:
							dwPriorityClass = NORMAL_PRIORITY_CLASS;
							break;
						case IDM_SYS_PRIO_A:
							dwPriorityClass = 32768;	//ABOVE_NORMAL_PRIORITY_CLASS
							break;
						case IDM_SYS_PRIO_H:
							dwPriorityClass = HIGH_PRIORITY_CLASS;
							break;
						case IDM_SYS_PRIO_R:
							dwPriorityClass = REALTIME_PRIORITY_CLASS;
							break;
						}
						GetWindowThreadProcessId(hwnd, &PID);
						hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);
						if(hProcess)
						{
							SetPriorityClass(hProcess, dwPriorityClass);
							CloseHandle(hProcess);
						}
					}
				}
				else
				{
					PostMessage(pshared->DM2wnd, WM_DM2_CMD, menuCmd, (LPARAM) hwnd);
				}
				last = lParam;
				lasttick = tick;
			}
		}
	}

	return CallNextHookEx(msgHook, code, wParam, lParam);
}