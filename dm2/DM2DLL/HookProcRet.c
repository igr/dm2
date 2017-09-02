/* http://dm2.sf.net */
#include <windows.h>
#include "../global.h"
#include "../tools.h"
#include "ModifySysMenu.h"

extern HHOOK MenuHook;
LRESULT CALLBACK ProcMenuRetHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	//i think hook ProcRet better than hook cbt.
	if (code >= 0)
	{
		PCWPRETSTRUCT pcs = (PCWPRETSTRUCT)lParam;

		if(pcs->message == WM_CREATE || pcs->message == WM_INITDIALOG)
		{
			DWORD pid;
			GetWindowThreadProcessId(pcs->hwnd, &pid);
			if(pid != pshared->Pid)
				ModifyWndCaptionMenu(pcs->hwnd, TRUE);
		}
		//when pop the system menu
		else if(pcs->message == WM_INITMENUPOPUP && (BOOL)(HIWORD(pcs->lParam)) == TRUE)
		{
			//Get Priority and set menu
			MENUITEMINFO mi = {sizeof(MENUITEMINFO), MIIM_SUBMENU, 0};
			GetMenuItemInfo((HMENU)(pcs->wParam), IDM_SYS_PRIO, FALSE, &mi);
			if(mi.hSubMenu)
			{
				DWORD dwPrio, dwMenuId;
				if(GetPriorityFromhWnd(pcs->hwnd, &dwPrio))
				{
					switch(dwPrio)
					{
					case IDLE_PRIORITY_CLASS:
						dwMenuId = IDM_SYS_PRIO_I;
						break;
					case 16384:		//BELOW_NORMAL_PRIORITY_CLASS
						dwMenuId = IDM_SYS_PRIO_B;
						break;
					case NORMAL_PRIORITY_CLASS:
						dwMenuId = IDM_SYS_PRIO_N;
						break;
					case 32768:		//ABOVE_NORMAL_PRIORITY_CLASS
						dwMenuId = IDM_SYS_PRIO_A;
						break;
					case HIGH_PRIORITY_CLASS:
						dwMenuId = IDM_SYS_PRIO_H;
						break;
					case REALTIME_PRIORITY_CLASS:
						dwMenuId = IDM_SYS_PRIO_R;
						break;
					default:
						break;
					}
					{
						int i = IDM_SYS_PRIO_I;
						for(; i<=IDM_SYS_PRIO_R; i++)
							CheckMenuItem(mi.hSubMenu, i, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(mi.hSubMenu, dwMenuId, 
							MF_BYCOMMAND | MF_CHECKED);
					}
				}
			}
			//Get on top and set menu
			if(GetWindowLong(pcs->hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
				CheckMenuItem((HMENU)(pcs->wParam), IDM_SYS_TOP, MF_BYCOMMAND | MF_CHECKED);
			else
				CheckMenuItem((HMENU)(pcs->wParam), IDM_SYS_TOP, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}

	return CallNextHookEx(MenuHook, code, wParam, lParam);
}
