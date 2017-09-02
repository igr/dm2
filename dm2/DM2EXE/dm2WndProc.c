/* http://dm2.sf.net */
#include <windows.h>
#include <Tlhelp32.h>
#include "../global.h"
#include "dm2.h"
#include "float.h"
#include "util.h"
#include "../DM2DLL/DllMain.h"
#include "SettingDialog/SettingDlg.h"
#include "dm2WndProc.h"

// for Multi-Language
#include "MultiLanguage.h"
extern char *lang_file;
extern BOOL IsWinNT;
PSREC_DATA pRec;

// ---------------------------------------------------------------- handlers

/**
 * Righ-click on DM2 icon.
 */
void ShowDm2RClickMenu(HWND hWnd) {
	HMENU menu;
	HMENU menuPopup;
	POINT p;

	GetCursorPos(&p);
	menu = LoadMenu(processHandle, "DM2_R_POPUP");
	if (menu == NULL) {
		return;
	}
	menuPopup = GetSubMenu(menu, 0);
	TranslateMenuLanguage(lang_file, menuPopup, "MenuDM2");

	SetForegroundWindow(hWnd);
	TrackPopupMenuEx(menuPopup, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, p.x, p.y, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	DestroyMenu(menu);
}


// ---------------------------------------------------------------- dm2 server wnd proc


extern INT_PTR CALLBACK SettingDlgProc(HWND, UINT, WPARAM, LPARAM);
/**
 * DM2 main windows procedure.
 */

HWND rightClickedTayHwnd;
HWND oldSettingWin = NULL;
extern SADV_DATA Adv_data;
extern PSFAV_DATA Fav_data;
extern char szFavApp[];
extern char szFavDef[];
extern PSREC_DATA pRec;
// use _snprintf replace wsprintf at IDM_NEED_FAV message.
extern int __cdecl _snprintf(char *, size_t, const char *, ...);
extern void ResizeWindow(int w, int h, BOOL bIsPixel);
extern INT_PTR WINAPI ResizeDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI DM2WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
		case WM_CREATE:
			pshared->DM2wnd = hWnd;
			ApplyDM2Setting();
			rightClickedTayHwnd = NULL;
			return 0;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;

		case WM_DESTROY:
			RestoreAllWindows(hWnd);
			DeleteTaskBarIcon(hWnd, 0);
			PostQuitMessage(0);
			return 0;
		
		case WM_HOTKEY:
			if(wParam > HK_CMD)
				DoHotKey_Command(wParam - (HK_CMD+1));
			else if(wParam == HK_CMD)
				SendMessage(pshared->DM2wnd, WM_COMMAND, IDM_DM2, 0);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_DM2:										// all menus
					//allow one settings window
					if(oldSettingWin == NULL)
						DialogBox(processHandle, (LPCTSTR)IDD_DM2, 
							NULL, SettingDlgProc);
					SetForegroundWindow(oldSettingWin);
					return 0;
				case IDM_RESTORE_TRAYWND:
					if (rightClickedTayHwnd != NULL) {
						RestoreTrayWindow(hWnd, rightClickedTayHwnd);	// minimized menu
					}
					rightClickedTayHwnd = NULL;
					return 0;
				case IDM_RESTORE_ALL:
					RestoreAllWindows(hWnd);						// dm2 menu
					return 0;
				case IDM_QUIT:
					DestroyWindow(hWnd);							// dm2 menu
					return 0;
				//Window Resize command
				case ID_RESIZE_640480:
					ResizeWindow(640, 480, TRUE);
					return 0;
				case ID_RESIZE_800600:
					ResizeWindow(800, 600, TRUE);
					return 0;
				case ID_RESIZE_1024768:
					ResizeWindow(1024, 768, TRUE);
					return 0;
				case ID_RESIZE_1280960:
					ResizeWindow(1280, 960, TRUE);
					return 0;
				case ID_RESIZE_C100:
					ResizeWindow(100, 100, FALSE);
					return 0;
				case ID_RESIZE_C50:
					ResizeWindow(50, 50, FALSE);
					return 0;
				case ID_RESIZE_C30:
					ResizeWindow(30, 30, FALSE);
					return 0;
				case ID_RESIZE_OTHER:
					DialogBox(processHandle, (LPCTSTR)IDD_DM2_ACTION_RESIZEWIN, 
						NULL, ResizeDlgProc);
					break;
			}
			if(HIWORD(wParam) == 0)
				DM2FavMenu_Command(LOWORD(wParam));
			break;

		case DM2_TRAY_MSG:
			if (wParam != 0) {
				switch (lParam) {
					// v1.18 changed
					//case WM_LBUTTONDOWN:
					case WM_LBUTTONUP:
						RestoreTrayWindow(hWnd, (HWND) wParam);		// tray win: left click on tray icon
						return 0;
					// v1.20 changed
					//case WM_RBUTTONDOWN:
					case WM_RBUTTONUP:
						rightClickedTayHwnd = (HWND) wParam;
						ShowTrayMenu(hWnd, (HWND) wParam);			// tray win: right click on tray icon, will show tray menu
						return 0;
				}
			} else {
				switch (lParam) {
					// v1.20 changed
					//case WM_RBUTTONDOWN:
					case WM_RBUTTONUP:
						ShowDm2RClickMenu(hWnd);					// dm2: tray right click
						return 0;
					case WM_LBUTTONDBLCLK:
						SendMessage(hWnd, WM_COMMAND, IDM_DM2, 0);
						break;
				}
			}
			break;

		case WM_DM2_CMD:
			if(IsException((HWND)lParam, EXCEPT_OTHER))
				break;
			switch (wParam) {
				case IDM_SYS_TRAY:
					MinimizeWndToTray(hWnd, (HWND)lParam);
					break;
				case IDM_SYS_FLICO:
					MinimizeWndToFloatingIcon((HWND)lParam);
					break;
				default:
					//Process Actiom add (v1.20)
					Do_Action_Command((HWND)lParam, wParam);
					break;
			}
			return 0;
		case WM_TIMER:
			switch(wParam)
			{
			case IDC_CHK_DWC:
				DetectCloseWindow();
				break;
			case IDC_CHK_DYNT:
				UpdataToolTip();
				break;
			}
			break;
		// detect file dialog need skip fav menu ?
		// HWND dlg = (HWND)wParam
		case IDM_IS_FAV_SKIP:
			return IsException(GetParent((HWND)wParam), EXCEPT_FAV);
		case IDM_IS_SYS_SKIP:
			return IsException((HWND)wParam, EXCEPT_SYSMENU);
		case IDM_NEED_FAV:
			{
				char szTitlt[MAX_PATH];
				COPYDATASTRUCT cs;
				PSFAV_DATA last = Fav_data;
				HWND hw, hw_last = (HWND)wParam;
				PSFAV_DATA pDef = NULL, pApp = NULL;
				void *pCopy = NULL;
				char *pStr = szTitlt;

				memset(&cs, 0, sizeof(COPYDATASTRUCT));

				pDef = GetFavDataMenuItem(Fav_data, szFavDef);
				pApp = GetFavDataMenuItem(Fav_data, szFavApp);
				
				while(hw = GetParent(hw_last))
				{
					hw_last = hw;
				}

				//GetName;
				if(Adv_data.FavCmpMode)
					GetWindowText(hw_last, szTitlt, MAX_PATH);
				else
				{
					HANDLE hTSP;
					PROCESSENTRY32 pe;
					DWORD pid = 0;

					memset(&pe, 0, sizeof(PROCESSENTRY32));
					pe.dwSize = sizeof(PROCESSENTRY32);

					GetWindowThreadProcessId((HWND)wParam, &pid);
					hTSP = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
					if(hTSP != INVALID_HANDLE_VALUE)
					{
						Process32First(hTSP, &pe);
						do
						{
							if(pe.th32ProcessID == pid)
								break;
						}
						while(Process32Next(hTSP, &pe));
						CloseHandle(hTSP);

						pStr = strrchr(pe.szExeFile, '\\');
						if(pStr == NULL)
							pStr = pe.szExeFile;
						else
							pStr++;
					}
				}
				
				while(last)
				{
					if(lstrcmpi(last->szName, szFavApp) != 0 &&
						lstrcmpi(last->szName, szFavDef) != 0 &&
						regexec_and_free(regcomp(last->szName), pStr))
					{
						break;
					}
					last = last->next;
				}

				if(last)
				{
					cs.cbData = lstrlen(pDef->szMenuItem)+1+lstrlen(pDef->szName)+1
						+lstrlen(pApp->szMenuItem)+1+lstrlen(pApp->szName)+1
						+lstrlen(last->szMenuItem)+1+lstrlen(last->szName)+1;
					pCopy = halloc(cs.cbData);
					_snprintf(pCopy, cs.cbData, "%s=%s\r%s=%s\r%s=%s", pDef->szName, 
						pDef->szMenuItem, pApp->szName, pApp->szMenuItem,
						last->szName, last->szMenuItem);
				}
				else
				{
					cs.cbData = lstrlen(pDef->szMenuItem)+1+lstrlen(pDef->szName)+1
						+lstrlen(pApp->szMenuItem)+1+lstrlen(pApp->szName)+1;
					pCopy = halloc(cs.cbData);
					_snprintf(pCopy, cs.cbData, "%s=%s\r%s=%s", pDef->szName, 
						pDef->szMenuItem, pApp->szName, pApp->szMenuItem);
					lstrlen(pCopy);
				}
				cs.lpData = pCopy;
				cs.dwData = 'dm';
				SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&cs);
				hfree(pCopy);
			}
			break;
		// Get system menu string.
		case IDM_NEED_LANG:
			{
				/*char *pDef;
				switch(wParam)
				{
				case IDM_SYS_TRAY:
					pDef = "Minimize to &tray";
					break;
				case IDM_SYS_TOP:
					pDef = "Always on to&p";
					break;
				case IDM_SYS_FLICO:
					pDef = "To &floating icon";
					break;
				case IDM_SYS_OPA:
					pDef = "Opacity";
					break;
				case IDM_SYS_DOPA:
					pDef = "&Disable";
					break;
				case IDM_SYS_PRIO:
					pDef = "&Priority";
					break;
				case IDM_SYS_PRIO_I:
					pDef = "&Idie";
					break;
				case IDM_SYS_PRIO_B:
					pDef = "&Below Normal";
					break;
				case IDM_SYS_PRIO_N:
					pDef = "&Normal";
					break;
				case IDM_SYS_PRIO_A:
					pDef = "&Above Normal";
					break;
				case IDM_SYS_PRIO_H:
					pDef = "&High";
					break;
				case IDM_SYS_PRIO_R:
					pDef = "&RealTime";
					break;
				default:
					*pshared->szMenu = '\0';
					return FALSE;
				}*/
				LoadLanguageString(lang_file, "MenuSystem", wParam, pshared->szMenu, MAX_MENU_LEN);
			}
			break;
		// change window opacity from system menu.
		case IDM_SYS_OPA:
			{
				int i = wParam - IDM_SYS_OPA;
				HWND hwnd = (HWND)lParam;
				if(!IsWinNT)
					return FALSE;
				if(i >= 1 && i <= 10)
				{
					PWND_DATA pwnd = GetWndData(hwnd);
					i *= 10;
					if (pwnd == NULL)
					{
						pwnd = NewWndDataEx(hwnd);
						pwnd->settings->enableopa = TRUE;
					}
					pwnd->settings->opacity = i;
					
					SetWindowOpacity(hwnd, (pwnd->settings->opacity * 255) / 100);
					if(i == 100)
						RemoveWndData(pwnd);
				}
			}
			break;
		// recent menu add(v1.20)
		case IDM_ADD_RECENT:
			{
				register int i = pRec->iNew;
				if(pRec->szRecentBuf[i][0] == '\0')
					lstrcpy(pRec->szRecentBuf[i], pshared->szRecent);
				else
				{
					if(i == -1)
						i = MAX_REC_ITEM-1;
					else
						i--;
					lstrcpy(pRec->szRecentBuf[i], pshared->szRecent);
					pRec->iNew = i;
				}
				//remove duplicate recent item
				for(i=0; i<MAX_REC_ITEM; i++)
				{
					if(i == pRec->iNew)
						continue;
					else if(lstrcmpi(pshared->szRecent, pRec->szRecentBuf[i]) == 0)
						pRec->szRecentBuf[i][0] = '\0';
				}
			}
			break;
		case IDM_GET_RECENT:
			{	//Create The Recent data and send it.
				COPYDATASTRUCT cs;
				int Num = 1;
				int p = pRec->iNew;
				int i = 0;
				char *pMenuStr = halloc(MAX_PATH2*MAX_REC_ITEM+2);
				char *pMenuTemp = halloc(MAX_PATH2*3);
				char *pMenuShortStr = halloc(MAX_PATH2);
				char *pSlash;
				char *pEnd;
				lstrcpy(pMenuStr, "=");
				pEnd = pMenuStr+1;
				
				if(Adv_data.FavRecNum > 0)
				{
					for(; i<MAX_REC_ITEM; i++)
					{
						if(pRec->szRecentBuf[p][0] != '\0')
						{
							// Make Menu String
							lstrcpy(pMenuTemp, pRec->szRecentBuf[p]);
							lstrcpy(pMenuShortStr, pRec->szRecentBuf[p]);
							if(lstrlen(pMenuTemp) > 3)	//make short name
							{
								pSlash = strrchr(pMenuTemp, '\\');
								if(pSlash && (pSlash-pMenuTemp > 20))
								{
									lstrcpyn(pMenuShortStr, pMenuTemp, 4);
									stradd(stradd(pMenuShortStr+3, "..."), pSlash);
								}
							}
							wsprintf(pMenuTemp, "&%X %s=%s", Num, pMenuShortStr, 
								pRec->szRecentBuf[p]);
							pEnd = stradd(pEnd, pMenuTemp);

							if(Num++ == Adv_data.FavRecNum)
								break;
							pEnd = stradd(pEnd, "|");
						}
						if(p == MAX_REC_ITEM-1)
							p = 0;
						else
							p++;

					}
				}

				cs.cbData = lstrlen(pMenuStr)+1;
				cs.lpData = pMenuStr;
				cs.dwData = 'dm'+1;
				SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&cs);

				hfree(pMenuStr);
				hfree(pMenuTemp);
				hfree(pMenuShortStr);
			}
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

extern PWND_DATA wndData;
//this function use in updata float icon and tray icon tooltip.
void UpdataToolTip()
{
	PWND_DATA pLast = wndData;
	while(pLast)
	{
		// is float icon
		if(pLast->flico)
		{
			char winname[MAX_WIN_TITLE];
			TOOLINFO ti;
			RECT rect;
			GetWinTitle(pLast->hwnd, winname);
			GetClientRect(pLast->hwnd, &rect);
			memset(&ti, 0, sizeof(TOOLINFO));
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_SUBCLASS;
			ti.hwnd = pLast->flico;
			ti.hinst = processHandle;
			ti.uId = 177;
			ti.lpszText = (LPSTR) winname;
			ti.rect.left = rect.left;
			ti.rect.top = rect.top;
			ti.rect.right = rect.right;
			ti.rect.bottom = rect.bottom;
			SendMessage(pLast->tooltip, TTM_UPDATETIPTEXT, 
				0, (LPARAM) (LPTOOLINFO) &ti);
		}
		// is tray icon
		else if(pLast->hwnd && pLast->icon)
		{
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = pshared->DM2wnd;
			nid.uID = (int)pLast->hwnd;
			nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
			nid.uCallbackMessage = DM2_TRAY_MSG;
			nid.hIcon = pLast->icon;
			GetWinTitle(pLast->hwnd, nid.szTip);
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		}
		pLast = pLast->next;
	}
}

void DetectCloseWindow()
{
	register PWND_DATA pLast = wndData;
	while(pLast)
	{
		if(IsWindow(pLast->hwnd))
		{
			pLast = pLast->next;
		}
		else
		{
			PWND_DATA wnd = pLast->next;
			RemoveWindowData(pLast);
			pLast = wnd;
		}
	}
}

void AdjustWindowOpacity(HWND hWnd, BOOL Inc, int step)
{
	PWND_DATA pwnd = GetWndData(hWnd);
	if (pwnd == NULL)
	{
		pwnd = NewWndDataEx(hWnd);
		pwnd->settings->enableopa = TRUE;
		pwnd->settings->opacity = 100;
	}

	if (Inc)
		pwnd->settings->opacity += step;
	else
		pwnd->settings->opacity -= step;
	
	pwnd->settings->opacity = setIntRange(pwnd->settings->opacity, 10, 100);
	
	SetWindowOpacity(hWnd, (pwnd->settings->opacity * 255) / 100);
	
	if(pwnd->settings->opacity == 100)
		RemoveWndData(pwnd);
}
