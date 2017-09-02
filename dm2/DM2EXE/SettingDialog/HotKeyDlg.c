/* http://dm2.sf.net */
#include <Windows.h>
#include "../GetSetting.h"
#include "../util.h"
#include "../../global.h"
#include "SettingDlg.h"
#include "../dm2WndProc.h"
#include "../../DM2DLL/DllMain.h"
#include "../Plugins.h"

extern PSHK_DATA hk_data_temp;
extern PSHK_DATA hk_data;
extern SADV_DATA Adv_data;
HANDLE hListView_hk;
char szHotKey[39];		//MAX: "WIN + CTRL + ALT + SHIFT + Scroll Lock"

char *DlgHKSection = "SettingsDialogHotKey";
extern HFONT hLocalFont;
char HC_hide[MAX_COMMENT];
char HC_tray[MAX_COMMENT];
char HC_icon[MAX_COMMENT];
char HC_inc[MAX_COMMENT];
char HC_dec[MAX_COMMENT];
char HC_reset[MAX_COMMENT];
char HC_top[MAX_COMMENT];
char HC_fav[MAX_COMMENT];
char HC_hides[MAX_COMMENT];

DWORD dwBtnHK[] = {IDC_BTN_HKADD, IDC_BTN_HKEDIT, IDC_BTN_HKDEL};

INT_PTR CALLBACK SettingDlgHotKeyProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			// load local language command
			LoadLanguageString(lang_file, DlgHKSection, 100, HC_hide, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 101, HC_tray, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 102, HC_icon, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 103, HC_inc, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 104, HC_dec, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 105, HC_reset, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 106, HC_top, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 107, HC_fav, MAX_COMMENT);
			LoadLanguageString(lang_file, DlgHKSection, 108, HC_hides, MAX_COMMENT);
		}
		{
			char szBuf[MAX_PATH];
			LVCOLUMN lc;
			LVITEM lv;
			PSHK_DATA last = hk_data_temp;

			memset(&lv, 0, sizeof(lv));
			hListView_hk = GetDlgItem(hwndDlg, IDC_LIST_HOTKEY);
			ListView_SetExtendedListViewStyle(hListView_hk, 
				LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT);
			memset(&lc, 0, sizeof(lc));
			lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lc.fmt = LVCFMT_LEFT;
			lc.cx = 100;
			LoadLanguageString(lang_file, DlgHKSection, 0, szBuf, MAX_PATH);
			lc.pszText = szBuf;
			ListView_InsertColumn(hListView_hk, 0, &lc);
			LoadLanguageString(lang_file, DlgHKSection, 1, szBuf, MAX_PATH);
			lc.cx = 160;
			ListView_InsertColumn(hListView_hk, 1, &lc);
			LoadLanguageString(lang_file, DlgHKSection, 2, szBuf, MAX_PATH);
			lc.cx = 60;
			ListView_InsertColumn(hListView_hk, 2, &lc);
			
			while(last)
			{
				lv.mask = LVIF_TEXT|LVIF_PARAM;
				lv.iItem = ListView_GetItemCount(hListView_hk);
				GetHotKeyText(last->dwKey);
				lv.pszText = szHotKey;
				lv.cchTextMax = lstrlen(lv.pszText);
				lv.iSubItem = 0;
				lv.lParam = (LPARAM)last;
				ListView_InsertItem(hListView_hk, &lv);
				lv.mask = LVIF_TEXT;
				lv.pszText = HK_GetCmtText(last->szCMD);
				lv.cchTextMax = lstrlen(lv.pszText);
				lv.iSubItem = 1;
				ListView_SetItem(hListView_hk, &lv);
				lv.pszText = last->szParam;
				lv.cchTextMax = lstrlen(lv.pszText);
				lv.iSubItem = 2;
				ListView_SetItem(hListView_hk, &lv);
				last = last->next;
			}
		}
		CreateRMenuCtrl(hListView_hk, hwndDlg, (LPDWORD)dwBtnHK, 
			sizeof(dwBtnHK)/sizeof(DWORD));

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgHKSection);
		return TRUE;
	case WM_NOTIFY:
		if(wParam == IDC_LIST_HOTKEY)
			if (((LPNMHDR)lParam)->code == NM_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDC_BTN_HKEDIT, 0);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BTN_HKADD:
			DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_HOTKEY_EDIT, 
				hwndDlg, EditHKDataProc, -1);
			break;
		case IDC_BTN_HKDEL:
			{
				int i = ListView_GetNextItem(hListView_hk, -1, LVNI_SELECTED);
				if(i != -1)
				{
					PSHK_DATA last = hk_data_temp;
					LVITEM vi;
					// free PSHK_DATA data.
					memset(&vi, 0, sizeof(LVITEM));
					vi.iItem = i;
					vi.mask = LVIF_PARAM;
					ListView_GetItem(hListView_hk, &vi);
					if(vi.lParam)
					{
						if((PSHK_DATA)(vi.lParam) == hk_data_temp)
						{
							hk_data_temp = ((PSHK_DATA)vi.lParam)->next;
							hfree((PSHK_DATA)vi.lParam);
						}
						else
						{
							PSHK_DATA temp;
							while(last->next && last->next != (PSHK_DATA)(vi.lParam))
								last = last->next;
							temp = ((PSHK_DATA)(last->next))->next;
							hfree(last->next);
							last->next = temp;
						}
						ListView_DeleteItem(hListView_hk, i);
					}
				}
			}
			break;
		case IDC_BTN_HKEDIT:
			{
				int i = ListView_GetNextItem(hListView_hk, -1, LVNI_SELECTED);
				if(i != -1)
					DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_HOTKEY_EDIT, 
						hwndDlg, EditHKDataProc, i);
			}
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


char *Hotkey_Command[] = {
	HC_hide,
	HC_tray,
	HC_icon,
	HC_inc,
	HC_dec,
	HC_reset,
	HC_top,
	HC_fav,
	HC_hides,
	""								//must use null string in end.
};

static char cm_hide[] = "dm2_cmd_hidewin";
static char cm_tray[] = "dm2_cmd_traywin";
static char cm_icon[] = "dm2_cmd_floatwin";
static char cm_inc[] = "dm2_cmd_inc_opacity";
static char cm_dec[] = "dm2_cmd_dec_opacity";
static char cm_opa[] = "dm2_cmd_reset_opacity";
static char cm_top[] = "dm2_cmd_win_ontop";
static char cm_menu[] = "dm2_cmd_showfav";
static char cm_hides[] = "dm2_cmd_specifiedhide";
static char *DM2_HK_Command[] = {
	cm_hide,
	cm_tray,
	cm_icon,
	cm_inc,
	cm_dec,
	cm_opa,
	cm_top,
	cm_menu,
	cm_hides,
	""								//must use null string in end.
};

extern PFF_PLUGINS ffp;
//Get command comments for command string
char *HK_GetCmtText(char *dm2cmd)
{
	int i = 0;
	PFF_PLUGINS last = ffp;
	while(DM2_HK_Command[i][0] != '\0')
	{
		if(lstrcmp(DM2_HK_Command[i], dm2cmd) == 0)
			return Hotkey_Command[i];
		i++;
	}
	while(last)
	{
		if(last->PluginsCategory == FFPC_HOTKEY)
			if(lstrcmp(last->pCmd, dm2cmd) == 0)
				return last->pComments;
		last = last->Next;
	}
	return NULL;
}

//Get command string for command comments
char *HK_GetCmdText(char *dm2cmt)
{
	int i = 0;
	PFF_PLUGINS last = ffp;
	while(Hotkey_Command[i][0] != '\0')
	{
		if(lstrcmp(Hotkey_Command[i], dm2cmt) == 0)
			return DM2_HK_Command[i];
		i++;
	}
	while(last)
	{
		if(last->PluginsCategory == FFPC_HOTKEY)
			if(lstrcmp(last->pComments, dm2cmt) == 0)
				return last->pCmd;
		last = last->Next;
	}
	return NULL;
}

//Get command string for command index.
char *HK_GetCmdFromIndex(int cmd)
{
	int i = 0;
	PFF_PLUGINS last = ffp;
	while(DM2_HK_Command[i][0] != '\0')
	{
		if(i == cmd)
			return DM2_HK_Command[i];
		i++;
	}
	while(last)
	{
		if(last->PluginsCategory == FFPC_HOTKEY)
		{
			if(i == cmd)
				return last->pCmd;
			i++;
		}
		last = last->Next;
	}
	return NULL;
}

//function copy text to szHotKey.
void GetHotKeyText(DWORD Key)
{
	long lkey;
	BYTE byteKey = HIBYTE(Key);
	char *pEnd = szHotKey;

    if(byteKey & (HOTKEYF_EXT << 1))
	{
		pEnd = stradd(pEnd, "Win + ");
		byteKey &= ~(HOTKEYF_EXT << 1);
	}
	if(byteKey & HOTKEYF_CONTROL)
	{
		pEnd = stradd(pEnd, "Ctrl + ");
		byteKey &= ~HOTKEYF_CONTROL;
	}
	if(byteKey & HOTKEYF_SHIFT)
	{
		pEnd = stradd(pEnd, "Shift + ");
		byteKey &= ~HOTKEYF_SHIFT;
	}
	if(byteKey & HOTKEYF_ALT)
	{
		pEnd = stradd(pEnd, "Alt + ");
		byteKey &= ~HOTKEYF_ALT;
	}

	lkey = MapVirtualKey(LOBYTE(Key), 0)<<0x10;
	if(byteKey & HOTKEYF_EXT)
		lkey |= 0x1000000;
	GetKeyNameText(lkey, 
		&szHotKey[pEnd - szHotKey], 12);
}

BOOL IsEditHK = FALSE;
PSHK_DATA pLast_hk;
HWND hCombo;
char *EditDlgHKSection = "SettingsDialogHotKeyEdit";
INT_PTR CALLBACK EditHKDataProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int i = 0;
			PFF_PLUGINS last = ffp;
			char szBuf[MAX_PATH];
			hCombo = GetDlgItem(hwndDlg, IDC_COMBO_CMD);
			//List all command
			while(Hotkey_Command[i][0] != '\0')
			{
				SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)Hotkey_Command[i]);
				i++;
			}
			while(last)
			{
				if(last->PluginsCategory == FFPC_HOTKEY)
					SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)last->pComments);
				last = last->Next;
			}
			//Set mode and title
			IsEditHK = (lParam != -1);

			SendDlgItemMessage(hwndDlg, IDC_EDIT_HKPARAM, EM_LIMITTEXT, MAXRULE, 0);

			if(IsEditHK)
			{
				LVITEM vi;
				int cmd_index;
				char szParam[MAXRULE2];
				memset(&vi, 0, sizeof(LVITEM));
				vi.iItem = lParam;
				vi.mask = LVIF_PARAM;
				ListView_GetItem(hListView_hk, &vi);
				pLast_hk = (PSHK_DATA)vi.lParam;
				cmd_index = SendMessage(hCombo, CB_FINDSTRING, -1, 
					(LPARAM)HK_GetCmtText(pLast_hk->szCMD));
				if(cmd_index == CB_ERR)
					cmd_index = 0;
				SendMessage(hCombo, CB_SETCURSEL, cmd_index, 0);
				SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_EDIT), 
					HKM_SETHOTKEY, pLast_hk->dwKey, 0);
				
				if (pLast_hk->dwKey & (HOTKEYF_EXT << 9))
				    SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_EXT),
                                BM_SETCHECK, 1, 0);
				
				vi.mask = LVIF_TEXT;
				vi.iSubItem = 2;
				vi.pszText = szParam;
				vi.cchTextMax = MAXRULE2;
				ListView_GetItem(hListView_hk, &vi);
				SendDlgItemMessage(hwndDlg, IDC_EDIT_HKPARAM, 
					WM_SETTEXT, 0, (LPARAM)szParam);
				LoadLanguageString(lang_file, EditDlgHKSection, 100, szBuf, MAX_PATH);
			}
			else
			{
				LoadLanguageString(lang_file, EditDlgHKSection, 101, szBuf, MAX_PATH);
				SendMessage(hCombo, CB_SETCURSEL, 0, 0);
			}
			SetWindowText(hwndDlg, szBuf);
		}

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, EditDlgHKSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				LVITEM lv;
				char szStr[MAX_PATH], *pParam;
				int i = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_CMD), 
					CB_GETCURSEL, 0, 0);
				DWORD dwKey = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_EDIT), 
					HKM_GETHOTKEY, 0, 0);
				int check = SendMessage(GetDlgItem(hwndDlg, IDC_HOTKEY_EXT),
				    BM_GETCHECK, 0, 0);					
				int select = ListView_GetNextItem(hListView_hk, -1, LVNI_SELECTED);

				//none key
				if(dwKey == 0)
					return FALSE;
                if(check)
					dwKey = dwKey | (HOTKEYF_EXT << 9);
               if (0 == dwKey >> 8)   //no any modifier, this shouldn't be a hotkey?
                    return FALSE;
                
				memset(&lv, 0, sizeof(lv));
				lv.mask = LVIF_PARAM;
				//if Key already exist!
				{
					int m = ListView_GetItemCount(hListView_hk);
					for(; lv.iItem<m; lv.iItem++)
					{
						register PSHK_DATA hk;
						if(lv.iItem == select)
							continue;
						if(ListView_GetItem(hListView_hk, &lv) == FALSE)
							break;
						hk = (PSHK_DATA)(lv.lParam);
						if(hk->dwKey == dwKey)
						{
							LocalMsgBox(hwndDlg, lang_file, 0, 122, MB_OK|MB_ICONINFORMATION);
							return FALSE;
						}
					}
				}

				memset(&lv, 0, sizeof(lv));
				lv.mask = LVIF_TEXT|LVIF_PARAM;
				SendMessage(hCombo, CB_GETLBTEXT, 
					SendMessage(hCombo, CB_GETCURSEL, 0, 0), (LPARAM)szStr);
				if(IsEditHK)
				{
					lv.iItem = select;
					GetHotKeyText(dwKey);
					lv.pszText = szHotKey;
					lv.cchTextMax = lstrlen(lv.pszText);
					lv.iSubItem = 0;
					lv.lParam = (LPARAM)pLast_hk;
					pLast_hk->dwKey = dwKey;
					lstrcpy(pLast_hk->szCMD, HK_GetCmdText(szStr));
					ListView_SetItem(hListView_hk, &lv);
					pParam = pLast_hk->szParam;
				}
				else
				{
					PSHK_DATA new_data = GetNewHotKey_Data(&hk_data_temp);
					lv.mask = LVIF_TEXT|LVIF_PARAM;
					lv.iItem = ListView_GetItemCount(hListView_hk);
					GetHotKeyText(dwKey);
					lv.pszText = szHotKey;
					lv.cchTextMax = lstrlen(lv.pszText);
					lv.iSubItem = 0;
					lv.lParam = (LPARAM)new_data;
					new_data->dwKey = dwKey;
					lstrcpy(new_data->szCMD, HK_GetCmdText(szStr));
					ListView_InsertItem(hListView_hk, &lv);
					pParam = new_data->szParam;
				}
				lv.mask = LVIF_TEXT;
				lv.pszText = szStr;
				lv.cchTextMax = lstrlen(lv.pszText);
				lv.iSubItem = 1;
				ListView_SetItem(hListView_hk, &lv);
				GetDlgItemText(hwndDlg, IDC_EDIT_HKPARAM, pParam, MAXRULE2);
				lv.pszText = pParam;
				lv.cchTextMax = lstrlen(lv.pszText);
				lv.iSubItem = 2;
				ListView_SetItem(hListView_hk, &lv);
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

void AddHideWindow(HWND hWnd)
{
	PWND_DATA pdata = HideWindow(hWnd);
	//if window hide, not need hbmp and settings.
	if(pdata)
	{
		if(pdata->settings)
		{
			hfree(pdata->settings);
			pdata->settings = NULL;
		}
		if(pdata->hbmp)
		{
			DeleteObject(pdata->hbmp);
			pdata->hbmp = NULL;
		}
	}
}

BOOL CALLBACK HideSpecifiedWindows(HWND hwnd, LPARAM lParam)
{
	if(getCompatibleSystemMenu(hwnd) && IsException(hwnd, EXCEPT_OTHER) == FALSE)
	{
		char szTitle[MAX_PATH];
		GetWindowText(hwnd, szTitle, MAX_PATH);
		if(regexec_and_free(regcomp((char *)lParam), szTitle))
			AddHideWindow(hwnd);
	}
	return TRUE;
}

extern int make_argv(char *arg, char ***argv);

extern PSHARED_DATA pshared;
extern HICON hIcon;
BOOL bWinEnable = FALSE;
void DoHotKey_Command(int cmd)
{
	HWND hWnd = GetForegroundWindow();
	PFF_PLUGINS last = ffp;
	//Get SHK_DATA
	int i = 0;
	PSHK_DATA hk_last = hk_data;
	for(; i<cmd; i++)
	{
		if(hk_last)
			hk_last = hk_last->next;
	}

	if(lstrcmpi(hk_last->szCMD, cm_hide) == 0)
	{
		if(getCompatibleSystemMenu(hWnd) == NULL ||
			IsException(hWnd, EXCEPT_OTHER))
			return;
		AddHideWindow(hWnd);
	}
	else if(lstrcmpi(hk_last->szCMD, cm_tray) == 0)
	{
		if(getCompatibleSystemMenu(hWnd) == NULL ||
			IsException(hWnd, EXCEPT_OTHER))
			return;
		MinimizeWndToTray(pshared->DM2wnd, hWnd);
	}
	else if(lstrcmpi(hk_last->szCMD, cm_icon) == 0)
	{
		if(getCompatibleSystemMenu(hWnd) == NULL ||
			IsException(hWnd, EXCEPT_OTHER))
			return;
		MinimizeWndToFloatingIcon(hWnd);
	}
	else if(lstrcmpi(hk_last->szCMD, cm_inc) == 0)
	{
		int i = setIntRange(atoi(hk_last->szParam), 10, 90);
		if(getCompatibleSystemMenu(hWnd) == NULL || 
			IsException(hWnd, EXCEPT_OTHER))
			return;
		AdjustWindowOpacity(hWnd, TRUE, i);					
	}
	else if(lstrcmpi(hk_last->szCMD, cm_dec) == 0)
	{
		int i = setIntRange(atoi(hk_last->szParam), 10, 90);
		if(getCompatibleSystemMenu(hWnd) == NULL || 
			IsException(hWnd, EXCEPT_OTHER))
			return;
		AdjustWindowOpacity(hWnd, FALSE, i);					
	}
	else if(lstrcmpi(hk_last->szCMD, cm_opa) == 0)
	{
		if(getCompatibleSystemMenu(hWnd) == NULL ||
			IsException(hWnd, EXCEPT_OTHER))
			return;
		{
			PWND_DATA pwnd = GetWndData(hWnd);
			SetWindowOpacity(hWnd, 255);
			RemoveWndData(pwnd);
		}
	}
	else if(lstrcmpi(hk_last->szCMD, cm_top) == 0)
	{
		if(getCompatibleSystemMenu(hWnd) == NULL)
			return;
		{
			if(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
				SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			else
				SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		}
	}
	else if(lstrcmpi(hk_last->szCMD, cm_menu) == 0)
		ShowFavMenu();
	else if(lstrcmpi(hk_last->szCMD, cm_hides) == 0)
	{
		EnumWindows(HideSpecifiedWindows, (LPARAM)hk_last->szParam);
	}
	else
	{
		// do plugins command
		while(last)
		{
			if(lstrcmpi(hk_last->szCMD, last->pCmd) == 0)
			{
				//plugins uninstall? and PluginsCategory
				if(last->Module && last->PluginsCategory == FFPC_HOTKEY)
				{
					if(last->State && last->State() == TRUE)
					{
						last->Execute(pshared->DM2wnd, NULL, 0, TRUE);
					}
					else
					{
						char szParam[MAXRULE2], **argv;
						int argc;
						lstrcpy(szParam, hk_last->szParam);
						argc = make_argv(szParam, &argv);
						last->Execute(pshared->DM2wnd, argv, argc, FALSE);
						free((LPVOID)argv);
					}
				}
				break;
			}
			last = last->Next;
		}
	}
}

void UnregisterAllHotKey(PSHK_DATA hk)
{
	int i = 0;
	while(hk)
	{
		UnregisterHotKey(pshared->DM2wnd, HK_CMD+1+i);
		i++;
		hk = hk->next;
	}
}

void ReRegisterHotKey()
{
	int i = 0;
	PSHK_DATA last = hk_data;

	while(last)
	{
		RegisterHotKey(pshared->DM2wnd, (HK_CMD+1) + i, 
			ConversionHotkey(last->dwKey), last->dwKey & 0xFF);

		i++;
		last = last->next;
	}
}

extern PSFAV_DATA Fav_data;
extern char szFavApp[];
extern char szFavDef[];
void ShowFavMenu()
{
	POINT point;
	HMENU hPop = CreatePopupMenu();
	HMENU hApp = CreatePopupMenu();
	PSFAV_DATA pDef = NULL, pApp = NULL;
	char *pMem;
	MENUITEMINFO menuItem;
	
	GetCursorPos(&point);
	pDef = GetFavDataMenuItem(Fav_data, szFavDef);
	pApp = GetFavDataMenuItem(Fav_data, szFavApp);
	pMem = halloc(lstrlen(pDef->szMenuItem)+1);
	lstrcpy(pMem, pDef->szMenuItem);
	CreateMenuFromString(hPop, pMem, IDM_TBB_DEF);
	hfree(pMem);
	
	memset(&menuItem, 0, sizeof(MENUITEMINFO));
	//#define MIIM_STRING      0x00000040
	menuItem.fMask = MIIM_ID | 0x00000040 | MIIM_SUBMENU;
	menuItem.cbSize = sizeof(MENUITEMINFO);
	menuItem.fType = MFT_STRING;
	menuItem.fState = MFS_ENABLED;
	menuItem.wID = 173;
	menuItem.dwTypeData = szFavApp;
	menuItem.cch = lstrlen(szFavApp);
	menuItem.hSubMenu = hApp;
	InsertMenuItem(hPop, GetMenuItemCount(hPop), TRUE, &menuItem);
	pMem = halloc(lstrlen(pApp->szMenuItem)+1);
	lstrcpy(pMem, pApp->szMenuItem);
	CreateMenuFromString(hApp, pMem, IDM_TBB_APP);
	hfree(pMem);

	TrackPopupMenuEx(hPop, 
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | 
		TPM_RIGHTBUTTON, point.x, point.y, pshared->DM2wnd, NULL);
	DestroyMenu(hPop);
}


void DM2FavMenu_Command(int cmd)
{
	BOOL bExec = FALSE;
	PSFAV_DATA pFav = NULL;
	char *pMem, *pTemp;
	int i = -1, num = 0;
	if(cmd >= IDM_TBB_DEF && cmd < IDM_TBB_APP)
	{
		pFav = GetFavDataMenuItem(Fav_data, szFavDef);
		bExec = FALSE;
		num = cmd - IDM_TBB_DEF;
	}
	else if(cmd >= IDM_TBB_APP && cmd < IDM_TBB_CST)
	{
		pFav = GetFavDataMenuItem(Fav_data, szFavApp);
		bExec = TRUE;
		num = cmd - IDM_TBB_APP;
	}

	if(pFav)
	{
		pMem = halloc(lstrlen(pFav->szMenuItem)+1);
		lstrcpy(pMem, pFav->szMenuItem);
		pTemp = pMem;

		while(TRUE)
		{
			pTemp = strchr(pTemp, '=');
			if(pTemp == NULL)
				break;
			if(*(pTemp+1) == '|')
				pTemp += 2;
			else if(*(pTemp+1) == '\0')
				break;
			else
			{
				pTemp++;
				i++;
			}
			if(i == num)
			{
				char *pTmp = strchr(pTemp, '|');
				if(pTmp)
					*pTmp = '\0';
				
				if(bExec)
				{
					STARTUPINFO si;
					PROCESS_INFORMATION pi;
					memset(&si, 0, sizeof(STARTUPINFO));
					si.cb = sizeof(STARTUPINFO);
					memset(&pi, 0, sizeof(PROCESS_INFORMATION));
					if(CreateProcess(NULL, pTemp, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
						NULL, NULL, &si, &pi) == 0)
					{
						LocalMsgBox(NULL, lang_file, 0, 103, MB_OK|MB_ICONINFORMATION);
					}
				}
				else
				{
					CreateDirectory(pTemp, NULL);
					ShellExecute(pshared->DM2wnd, "explore", pTemp, NULL, NULL, SW_SHOW);
				}
				break;
			}
		}

		hfree(pMem);
	}
}
