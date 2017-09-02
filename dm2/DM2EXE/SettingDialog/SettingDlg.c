/* http://dm2.sf.net */
#include <shlobj.h>
#include "../GetSetting.h"
#include "../util.h"
#include "../../global.h"
#include "SettingDlg.h"

HWND hTree;
HWND hDlgTitle;
HWND hDlgParent;
TVINSERTSTRUCT tvi;
HWND hLastDlg = NULL;
int iLastDlg = 0;
int dx, dy;

extern HWND oldSettingWin;
extern HMODULE processHandle;

extern char *inifile;
extern SGEN_DATA Gen_data;
SGEN_DATA Gen_data_temp;
extern SADV_DATA Adv_data;
SADV_DATA adv_data_temp;
extern PSCUSTOM_DATA Custom_data;
PSCUSTOM_DATA Custom_data_temp = NULL;
extern PSEXCEPT_DATA Except_data;
PSEXCEPT_DATA Except_data_temp = NULL;
extern PSFAV_DATA Fav_data;
PSFAV_DATA Fav_data_temp;
extern BOOL bEnableFavMenu;
BOOL bEnableFavMenu_temp;
extern PSHK_DATA hk_data;
PSHK_DATA hk_data_temp;
extern PSAC_DATA Action_data;
PSAC_DATA Action_data_temp;

//Save temp language file path.
char szLang_temp[MAX_PATH];
HFONT hLocalFont;

char szAutoRun[] = "DM2";

extern HICON hIcon;
char *DlgSection = "SettingsDialog";
extern char *DM2_Ver;
extern BOOL IsWinNT;

HIMAGELIST hImgTree;

INT_PTR CALLBACK SettingDlgProc(HWND hwndDlg, UINT uMsg, 
								   WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			//The system displays the large icon in the ALT+TAB dialog box, 
			//and the small icon in the window caption. 
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		}
		SetWindowText(hwndDlg, DM2_Ver);

		lstrcpy(szLang_temp, lang_file);

		//fix autorun option register write bugs.
		{
			HKEY hKey;
			DWORD dwType = REG_SZ;
			if(Adv_data.startmode)
				RegOpenKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
			else
				RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
			Adv_data.isAutorun = (RegQueryValueEx(hKey, szAutoRun, 0, &dwType
				, NULL, NULL) == ERROR_SUCCESS);
			RegCloseKey(hKey);
		}

		//Copy Settings data
		memcpy(&Gen_data_temp, &Gen_data, sizeof(SGEN_DATA));
		memcpy(&adv_data_temp, &Adv_data, sizeof(SADV_DATA));
		{
			PSCUSTOM_DATA last = Custom_data;
			PSCUSTOM_DATA lastcopy = NULL;
			PSEXCEPT_DATA last_except = Except_data;
			PSEXCEPT_DATA lastcopy_except = NULL;
			PSFAV_DATA last_fav = Fav_data;
			PSFAV_DATA lastcopy_fav = NULL;
			PSHK_DATA last_hk = hk_data;
			PSHK_DATA lastcopy_hk = NULL;

			//copy custom data
			while(last)
			{
				lastcopy = GetNewCustom_Data(&Custom_data_temp);
				CopyCustomData(lastcopy, last);
				last = last->next;
			}

			//copy action data
			Action_data_temp = halloc(sizeof(SAC_DATA));
			memcpy(Action_data_temp, Action_data, sizeof(SAC_DATA));

			//copy hotkey data
			while(last_hk)
			{
				lastcopy_hk = GetNewHotKey_Data(&hk_data_temp);
				lastcopy_hk->dwKey = last_hk->dwKey;
				lstrcpy(lastcopy_hk->szCMD, last_hk->szCMD);
				lstrcpy(lastcopy_hk->szParam, last_hk->szParam);
				last_hk = last_hk->next;
			}

			//copy except data
			while(last_except)
			{
				lastcopy_except = GetNewExcept_Data(&Except_data_temp);
				lstrcpy(lastcopy_except->szRule, last_except->szRule);
				lastcopy_except->Settings = last_except->Settings;
				last_except = last_except->next;
			}

			//copy fav menu
			if(Fav_data)
			{
				lastcopy_fav = halloc(sizeof(SFAV_DATA));
				lastcopy_fav->next = NULL;
				Fav_data_temp = lastcopy_fav;
			}
			while(last_fav)
			{
				char *pName = halloc(lstrlen(last_fav->szName)+1);
				char *pItem = halloc(lstrlen(last_fav->szMenuItem)+1);
				lastcopy_fav->szName = pName;
				lastcopy_fav->szMenuItem = pItem;
				lstrcpy(lastcopy_fav->szName, last_fav->szName);
				lstrcpy(lastcopy_fav->szMenuItem, last_fav->szMenuItem);
				if(last_fav->next)
				{
					lastcopy_fav->next = halloc(sizeof(SFAV_DATA));
					lastcopy_fav = lastcopy_fav->next;
					lastcopy_fav->next = NULL;
				}
				last_fav = last_fav->next;
			}
			CreateDefaultSection(&Fav_data_temp);
			bEnableFavMenu_temp = bEnableFavMenu;
		}

		//Save Settings window handle.
		oldSettingWin = hwndDlg;

		hTree = GetDlgItem(hwndDlg, IDC_TREE_SET);
		hDlgTitle = GetDlgItem(hwndDlg, IDC_STATIC_TITLE);

		//Get point
		{
			POINT po = {0, 0};
			POINT pom = {0, 0};
			ClientToScreen(GetDlgItem(hwndDlg, IDC_STATIC_DLG), &po);
			ClientToScreen(hwndDlg, &pom);
			dx = po.x - pom.x;
			dy = po.y - pom.y;
		}

		//Get Parent window Handle
		hDlgParent = hwndDlg;
		
		//Add tree item.
		memset(&tvi, 0, sizeof(tvi));
		tvi.item.mask = TVIF_TEXT|TVIF_PARAM;
		{
			HTREEITEM htlast;
			char szBuf[MAX_PATH];
			LoadLanguageString(lang_file, DlgSection, 0, szBuf, MAX_PATH);
			htlast = AddTreeViewItem(szBuf, TVI_ROOT, TVI_ROOT, 0);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_GENERAL, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, htlast, TVI_LAST, IDD_DM2_GENERAL);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_ADVANCE, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, htlast, TVI_LAST, IDD_DM2_ADVANCE);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_ACTION, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, htlast, TVI_LAST, IDD_DM2_ACTION);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_HOTKEY, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, htlast, TVI_LAST, IDD_DM2_HOTKEY);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_FAVMENU, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, htlast, TVI_LAST, IDD_DM2_FAVMENU);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_CUSTOM, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, htlast, TVI_LAST, IDD_DM2_CUSTOM);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_EXCEPTION, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, htlast, TVI_LAST, IDD_DM2_EXCEPTION);
			SendMessage(hTree, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)htlast);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_PLUGINS, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, TVI_ROOT, TVI_LAST, IDD_DM2_PLUGINS);
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_MANAGER, szBuf, MAX_PATH);
			AddTreeViewItem(szBuf, TVI_ROOT, TVI_LAST, IDD_DM2_MANAGER);

			//Create Default Window.
			iLastDlg = 0;
			LoadLanguageString(lang_file, DlgSection, IDD_DM2_ABOUT, szBuf, MAX_PATH);
			TreeView_SelectItem(hTree, 
				AddTreeViewItem(szBuf, TVI_ROOT, TVI_LAST, IDD_DM2_ABOUT));
		}

		hImgTree = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, 6, 4);
		ImageList_AddIcon(hImgTree, LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_TREE));
		ImageList_AddIcon(hImgTree, LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_TREEITEM));

		SetForegroundWindow(hwndDlg);

		hLocalFont = GetLanguageFont(hwndDlg, lang_file);
		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			ShowWindow(hwndDlg, SW_MINIMIZE);
			SendMessage(hLastDlg, WM_DM2_SAVESETTING, 0, 0);

			//save Settings data
			memcpy(&Gen_data, &Gen_data_temp, sizeof(SGEN_DATA));
			WriteGenSetting(inifile, &Gen_data);
			memcpy(&Adv_data, &adv_data_temp, sizeof(SADV_DATA));
			WriteAdvSetting(inifile, &Adv_data);
			
			memcpy(Action_data, Action_data_temp, sizeof(SAC_DATA));
			WriteActionSetting(inifile, Action_data);

			RemoveAllCustom(&Custom_data);
			Custom_data = Custom_data_temp;
			WriteCustomSetting(inifile, Custom_data);
			Custom_data_temp = 0;
			
			RemoveAllExcept(&Except_data);
			Except_data = Except_data_temp;
			WriteExceptSetting(inifile, Except_data);
			Except_data_temp = 0;

			RemoveAllFavMenu(&Fav_data);
			Fav_data = Fav_data_temp;
			bEnableFavMenu = bEnableFavMenu_temp;
			WriteFavSetting(inifile, Fav_data);
			Fav_data_temp = 0;

			UnregisterAllHotKey(hk_data);
			RemoveAllHotKey(&hk_data);
			hk_data = hk_data_temp;
			WriteHotKeySetting(inifile, hk_data);
			hk_data_temp = 0;

			{
				HKEY hKey = NULL;

				//delete autorun first.
				if(RegOpenKey(HKEY_CURRENT_USER, 
					"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) == ERROR_SUCCESS)
				{
					RegDeleteValue(hKey, szAutoRun);
					RegCloseKey(hKey);
				}
				if(RegOpenKey(HKEY_LOCAL_MACHINE, 
					"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) == ERROR_SUCCESS)
				{
					RegDeleteValue(hKey, szAutoRun);
					RegCloseKey(hKey);
				}
				//Apply auto run.
				if(Adv_data.isAutorun)
				{
					char szExe[MAX_PATH];
					if(Adv_data.startmode)
						RegOpenKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
					else
						RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
					RegSetValueEx(hKey, szAutoRun, 0, REG_SZ, szExe, GetModuleFileName(NULL, szExe, MAX_PATH));
					RegCloseKey(hKey);
				}
			}

			//Save Language Settings
			lstrcpy(lang_file, szLang_temp);
			if(*lang_file != '\0')
				WritePrivateProfileString("settings", "Language", 
					strrchr(lang_file, '\\')+1, inifile);
			else
				WritePrivateProfileString("settings", "Language", "", inifile);

			//Apply Setting
			ApplyDM2Setting();
			
			DestroyWindow(hLastDlg);
			hLastDlg = NULL;
			EndDialog(hwndDlg, IDOK);
			if(IsWinNT) SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
			break;
		case IDCANCEL:
			SendMessage(hLastDlg, WM_DM2_SAVESETTING, 0, 0);
			DestroyWindow(hLastDlg);
			hLastDlg = NULL;
			EndDialog(hwndDlg, IDCANCEL);
			if(IsWinNT) SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
			break;
		case IDC_BTN_EXIT:
			SendMessage(hLastDlg, WM_DM2_SAVESETTING, 0, 0);
			DestroyWindow(hLastDlg);
			hLastDlg = NULL;
			EndDialog(hwndDlg, IDOK);
			SendMessage(pshared->DM2wnd, WM_COMMAND, IDM_QUIT, 0);
			break;
		default:
			//send child window command to child window.(for example: use Enter key)
			SendMessage(hLastDlg, WM_COMMAND, wParam, lParam);
			break;
		}
		break;
	case WM_NOTIFY:
			if(wParam == IDC_TREE_SET && ((LPNMHDR)lParam)->code == TVN_SELCHANGED)
			{
				TVITEM tv;
				HTREEITEM hSelect = NULL;
				char szItemText[MAX_PATH];
				*szItemText = 0;

				//Get Select Item info
				hSelect = TreeView_GetSelection(hTree);
				memset(&tv, 0, sizeof(tv));
				tv.mask = TVIF_TEXT|TVIF_PARAM|TVIF_HANDLE;
				tv.hItem = hSelect;
				tv.cchTextMax = MAX_PATH;
				tv.pszText = szItemText;
				TreeView_GetItem(hTree, &tv);

				//if have child item
				if(tv.lParam == 0)
				{
					hSelect = TreeView_GetNextItem(hTree, hSelect, TVGN_CHILD);
					TreeView_SelectItem(hTree, hSelect);
					tv.hItem = hSelect;
					tv.pszText = szItemText;
					TreeView_GetItem(hTree, &tv);
				}

				SetWindowText(hDlgTitle, szItemText);

				if(iLastDlg != tv.lParam)
				{
					SendMessage(hLastDlg, WM_DM2_SAVESETTING, 0, 0);
					DestroyWindow(hLastDlg);
					iLastDlg = tv.lParam;

					switch(tv.lParam)
					{
					case IDD_DM2_ADVANCE:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_ADVANCE, hDlgParent, 
							SettingDlgAdvanceProc);
						break;
					case IDD_DM2_FAVMENU:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_FAVMENU, hDlgParent, 
							SettingDlgFavMenuProc);
						break;
					case IDD_DM2_GENERAL:
						hLastDlg = CreateDialogParam(processHandle, 
							(LPCTSTR)IDD_DM2_GENERAL, hDlgParent, 
							SettingDlgGeneralProc, (LPARAM)&Gen_data_temp);
						break;
					case IDD_DM2_HOTKEY:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_HOTKEY, hDlgParent, 
							SettingDlgHotKeyProc);
						break;
					case IDD_DM2_MANAGER:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_MANAGER, hDlgParent, 
							SettingDlgWinManagerProc);
						break;
					case IDD_DM2_CUSTOM:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_CUSTOM, hDlgParent, 
							SettingDlgCustomProc);
						break;
					case IDD_DM2_EXCEPTION:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_EXCEPTION, hDlgParent, 
							SettingDlgExceptionProc);
						break;
					case IDD_DM2_PLUGINS:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_PLUGINS, hDlgParent, 
							SettingDlgPluginsProc);
						break;
					case IDD_DM2_ACTION:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_ACTION, hDlgParent, 
							SettingDlgActionProc);
						break;
					default:
						hLastDlg = CreateDialog(processHandle, 
							(LPCTSTR)IDD_DM2_ABOUT, hDlgParent, 
							SettingDlgAboutProc);
						break;
					}
					SetWindowPos(hLastDlg, 0, dx, dy, 0, 0, SWP_NOSIZE);
					//if use 'up'/'down' key control page
					SetFocus(GetDlgItem(hwndDlg, IDC_TREE_SET));
					ShowWindow(hLastDlg, SW_SHOW);
				}
			}
		break;
	case WM_DESTROY:
		RemoveAllCustom(&Custom_data_temp);
		RemoveAllExcept(&Except_data_temp);
		RemoveAllFavMenu(&Fav_data_temp);
		RemoveAllHotKey(&hk_data_temp);
		hfree(Action_data_temp);
		oldSettingWin = NULL;
		break;
	case WM_NCDESTROY:
		DeleteObject(hLocalFont);
		break;
	default:
		break;
	}
	
	return FALSE;
}


HTREEITEM AddTreeViewItem(char *pItemText, HTREEITEM hParent, 
						  HTREEITEM hInsertAfter, LPARAM lParam)
{
	tvi.hParent = hParent;
	tvi.hInsertAfter = hInsertAfter;
	tvi.item.pszText = pItemText;
	tvi.item.cchTextMax = lstrlen(pItemText);
	tvi.item.lParam = lParam;
	return TreeView_InsertItem(hTree, &tvi);
}