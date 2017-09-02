/* http://dm2.sf.net */
#include <windows.h>
#include "../global.h"
#include "ModifySysMenu.h"


/* top-level enum window callback function */
BOOL CALLBACK ModifyWndCaptionMenuCallback(HWND hwnd, LPARAM lParam) {
	ModifyWndCaptionMenu(hwnd, lParam);
	return TRUE;
}

/**
 * Returns system menu of compatible windows.
 */
HMENU getCompatibleSystemMenu(HWND hwnd) {
	HMENU hSysMenu;

	if (GetParent(hwnd) != NULL) {			// skip all child windows (mdi)
		return NULL;
	}

	hSysMenu = GetSystemMenu(hwnd, FALSE);	// get system menu info
	if (hSysMenu == NULL) {
		return NULL;
	}
	if (GetMenuState(hSysMenu, SC_MINIMIZE, MF_BYCOMMAND) == -1) {
		return NULL;						// skip if system menu doesn't have a MINIMIZE menu item
	}
	return hSysMenu;
}


extern BOOL IsNT;
MENUITEMINFO SubMenu = {sizeof(MENUITEMINFO), MIIM_ID | 0x00000040 | MIIM_SUBMENU, 
						MFT_STRING, MFS_ENABLED, IDM_SYS_OPA, 0};
MENUITEMINFO Prio_SubMenu = {sizeof(MENUITEMINFO), MIIM_ID | 0x00000040 | MIIM_SUBMENU, 
						MFT_STRING, MFS_ENABLED, IDM_SYS_PRIO, 0};
// ---------------------------------------------------------------- rclick caption menu
void ModifyWndCaptionMenu(HWND hwnd, BOOL modify) {
	HMENU hSysMenu;
	int i;

	hSysMenu = getCompatibleSystemMenu(hwnd);
	if (hSysMenu == NULL) {					// skip all windows with not-compatible
		return;								// system menus
	}

	if(SendMessage(pshared->DM2wnd, IDM_IS_SYS_SKIP, (WPARAM)hwnd, 0))
		return;
	
	// modify menu
	if (modify == TRUE) {
		// check if menu is already modified
		for(i = 0; i < GetMenuItemCount(hSysMenu) && hSysMenu; i++) {
			if (GetMenuItemID(hSysMenu, i) == IDM_SYS_TRAY) {
				return;
			}
		}
		// modify
		InsertMenu(hSysMenu, GetMenuItemID(hSysMenu, 0), MF_SEPARATOR, IDM_SYS_SEPARATOR, NULL) ;
		SendMessage(pshared->DM2wnd, IDM_NEED_LANG, IDM_SYS_TOP, 0);
		if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) {
			InsertMenu(hSysMenu, GetMenuItemID(hSysMenu, 0), MF_STRING | MF_CHECKED, IDM_SYS_TOP, pshared->szMenu);
		} else {
			InsertMenu(hSysMenu, GetMenuItemID(hSysMenu, 0), MF_STRING, IDM_SYS_TOP, pshared->szMenu);
		}
		SendMessage(pshared->DM2wnd, IDM_NEED_LANG, IDM_SYS_TRAY, 0);
		InsertMenu(hSysMenu, GetMenuItemID(hSysMenu, 0), MF_STRING, IDM_SYS_TRAY, pshared->szMenu);
		SendMessage(pshared->DM2wnd, IDM_NEED_LANG, IDM_SYS_FLICO, 0);
		InsertMenu(hSysMenu, GetMenuItemID(hSysMenu, 0), MF_STRING, IDM_SYS_FLICO, pshared->szMenu);

		if(pshared->IsWinNT)
		{
			int i=1;
			char *buf[4];
			SubMenu.hSubMenu = CreatePopupMenu();
			for(; i<=9; i++)
			{
				wsprintf((char *)buf, "&%d%%", i*10);
				AppendMenu(SubMenu.hSubMenu, MF_STRING, IDM_SYS_OPA+i, (char *)buf);
			}
			SendMessage(pshared->DM2wnd, IDM_NEED_LANG, IDM_SYS_DOPA, 0);
			AppendMenu(SubMenu.hSubMenu, MF_STRING+MF_CHECKED, IDM_SYS_DOPA, pshared->szMenu);
			
			SendMessage(pshared->DM2wnd, IDM_NEED_LANG, IDM_SYS_OPA, 0);
			SubMenu.dwTypeData = pshared->szMenu;
			InsertMenuItem(hSysMenu, IDM_SYS_SEPARATOR, FALSE, &SubMenu);
		}

		//add Priority menu
		Prio_SubMenu.hSubMenu = CreatePopupMenu();
		{
			int i = 0;
			for(; i<6; i++)
			{
				if(!pshared->IsWinNT && (i == IDM_SYS_PRIO_B - IDM_SYS_PRIO_I || 
					i == IDM_SYS_PRIO_A - IDM_SYS_PRIO_I))
					continue;
				
				SendMessage(pshared->DM2wnd, IDM_NEED_LANG, IDM_SYS_PRIO_I+i, 0);
				AppendMenu(Prio_SubMenu.hSubMenu, MF_STRING, IDM_SYS_PRIO_I+i, pshared->szMenu);
			}
		}
		SendMessage(pshared->DM2wnd, IDM_NEED_LANG, IDM_SYS_PRIO, 0);
		Prio_SubMenu.dwTypeData = pshared->szMenu;
		InsertMenuItem(hSysMenu, IDM_SYS_SEPARATOR, FALSE, &Prio_SubMenu);

		*pshared->szMenu = '\0';
	} else {
		// unmodify, i.e. return to default state
		MENUITEMINFO mi = {sizeof(MENUITEMINFO), MIIM_SUBMENU, 0};
		GetMenuItemInfo(hSysMenu, IDM_SYS_OPA, FALSE, &mi);
		if(mi.hSubMenu)
			DestroyMenu(mi.hSubMenu);
		DeleteMenu(hSysMenu, IDM_SYS_OPA, MF_BYCOMMAND);
		
		GetMenuItemInfo(hSysMenu, IDM_SYS_PRIO, FALSE, &mi);
		if(mi.hSubMenu)
			DestroyMenu(mi.hSubMenu);
		DeleteMenu(hSysMenu, IDM_SYS_PRIO, MF_BYCOMMAND);

		DeleteMenu(hSysMenu, IDM_SYS_FLICO, MF_BYCOMMAND);
		DeleteMenu(hSysMenu, IDM_SYS_TRAY, MF_BYCOMMAND);
		DeleteMenu(hSysMenu, IDM_SYS_TOP, MF_BYCOMMAND);
		DeleteMenu(hSysMenu, IDM_SYS_SEPARATOR, MF_BYCOMMAND);
	}
	return;
}
