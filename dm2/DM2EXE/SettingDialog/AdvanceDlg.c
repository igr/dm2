/* http://dm2.sf.net */
#include <Windows.h>
#include "../GetSetting.h"
#include "../util.h"
#include "../../global.h"
#include "SettingDlg.h"
#include "../../DM2DLL/DllMain.h"

extern SADV_DATA Adv_data;
extern SADV_DATA adv_data_temp;
extern char szAutoRun[];
HWND hOpacityTip;	//use in advance settings dialog.
HWND hFavTip;	//use in advance settings dialog.
char *DlgAdvSection = "SettingsDialogAdvance";
extern char szLang_temp[];
extern HFONT hLocalFont;
extern BOOL IsWinNT;
extern BOOL StaticToURLControl(HWND hDlg, UINT staticid, char *szURL, COLORREF crLink);
INT_PTR CALLBACK SettingDlgAdvanceProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND htemp;
			char szBuf[MAX_PATH];
			//init control
			CheckDlgButton(hwndDlg, IDC_CHK_MSM, adv_data_temp.sysmenu);
			CheckDlgButton(hwndDlg, IDC_CHK_DWC, adv_data_temp.detectclose);
			CheckDlgButton(hwndDlg, IDC_CHK_DYNT, adv_data_temp.dyntitle);
			CheckDlgButton(hwndDlg, IDC_CHK_TRAY, adv_data_temp.runtray);
			CheckDlgButton(hwndDlg, IDC_CHK_HKDM2, adv_data_temp.EnableHotKey);
			htemp = GetDlgItem(hwndDlg, IDC_COMBO_RUNMODE);
			LoadLanguageString(lang_file, DlgAdvSection, 100, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			LoadLanguageString(lang_file, DlgAdvSection, 101, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			SendMessage(htemp, CB_SETCURSEL, adv_data_temp.startmode, 0);
			CheckDlgButton(hwndDlg, IDC_CHK_RUN, adv_data_temp.isAutorun);
			EnableWindow(htemp, adv_data_temp.isAutorun);
			if(adv_data_temp.dwHotKey)
				SendDlgItemMessage(hwndDlg, IDC_HOTKEY_DM2, HKM_SETHOTKEY, 
					adv_data_temp.dwHotKey, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEY_DM2), 
				adv_data_temp.EnableHotKey);
			LoadLanguageString(lang_file, DlgAdvSection, 103, szBuf, MAX_PATH);
			hFavTip = CreateTooltip(GetDlgItem(hwndDlg, IDC_CHK_FAVCMP), szBuf, 0, 0, NULL);
			CheckDlgButton(hwndDlg, IDC_CHK_MO, adv_data_temp.LockWinArea);
			CheckDlgButton(hwndDlg, IDC_CHK_FAVCMP, adv_data_temp.FavCmpMode);
			//Set recent num v1.20
			wsprintf(szBuf, "%d", adv_data_temp.FavRecNum);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_REC, EM_LIMITTEXT, 2, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_REC, WM_SETTEXT, 0, (LPARAM)szBuf);

			//List All Language
			htemp = GetDlgItem(hwndDlg, IDC_COMBO_LANG);
			//SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"Default");
			SendMessage(htemp, CB_SETCURSEL, 0, 0);
			{
				//Get Language file info
				char szPath[MAX_PATH], szBuf[MAX_PATH], LangString[MAX_PATH];
				WIN32_FIND_DATA FindFileData;
				HANDLE hFind;
				GetModuleFileName(NULL, szPath, MAX_PATH);
				lstrcpy(strrchr(szPath, '\\')+1, "language\\*.lang");
				lstrcpy(szBuf, szPath);
				hFind = FindFirstFile(szPath, &FindFileData);
				if(hFind != INVALID_HANDLE_VALUE)
				{
					do
					{
						lstrcpy(strrchr(szBuf, '\\')+1, FindFileData.cFileName);
						GetPrivateProfileString("LangOptions", "LanguageName", 
							"", LangString, MAX_PATH, szBuf);
						if(*LangString != '\0')
						{
							int i = SendMessage(htemp, CB_ADDSTRING, 0, 
								(LPARAM)LangString);
							if(lstrcmpi(strrchr(szLang_temp, '\\'), 
								strrchr(szBuf, '\\')) == 0)
								SendMessage(htemp, CB_SETCURSEL, i, 0);
						}
					}
					while(FindNextFile(hFind, &FindFileData));
					FindClose(hFind);
				}
			}
		}

		StaticToURLControl(hwndDlg, IDC_STATIC_GETLANG, 
			"http://dm2.sourceforge.net", 0xFF0000);
		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgAdvSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHK_RUN:
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_RUNMODE), 
				IsDlgButtonChecked(hwndDlg, IDC_CHK_RUN));
			break;
		case IDC_CHK_HKDM2:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEY_DM2), 
				IsDlgButtonChecked(hwndDlg, IDC_CHK_HKDM2));
			break;
		}
		break;
	case WM_DM2_SAVESETTING:
		adv_data_temp.sysmenu = IsDlgButtonChecked(hwndDlg, IDC_CHK_MSM);
		adv_data_temp.detectclose = IsDlgButtonChecked(hwndDlg, IDC_CHK_DWC);
		adv_data_temp.dyntitle = IsDlgButtonChecked(hwndDlg, IDC_CHK_DYNT);
		adv_data_temp.runtray = IsDlgButtonChecked(hwndDlg, IDC_CHK_TRAY);
		adv_data_temp.startmode = SendDlgItemMessage(hwndDlg, IDC_COMBO_RUNMODE, 
			CB_GETCURSEL, 0, 0);
		adv_data_temp.isAutorun = IsDlgButtonChecked(hwndDlg, IDC_CHK_RUN);
		adv_data_temp.EnableHotKey = IsDlgButtonChecked(hwndDlg, IDC_CHK_HKDM2);
		adv_data_temp.dwHotKey = SendDlgItemMessage(hwndDlg, IDC_HOTKEY_DM2, 
			HKM_GETHOTKEY, 0, 0);
		adv_data_temp.LockWinArea = IsDlgButtonChecked(hwndDlg, IDC_CHK_MO);
		adv_data_temp.FavCmpMode = IsDlgButtonChecked(hwndDlg, IDC_CHK_FAVCMP);
		{	//get recent setting rang 0~16, max 2 char.
			char szNum[3];
			SendDlgItemMessage(hwndDlg, IDC_EDIT_REC, WM_GETTEXT, 3, (LPARAM)&szNum);
			adv_data_temp.FavRecNum = setIntRange(atoi(szNum), 0, MAX_REC_ITEM);
		}
		//Get language settings
		{
			char Lang_Text[MAX_PATH];
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			HWND hw = GetDlgItem(hwndDlg, IDC_COMBO_LANG);
			SendMessage(hw, CB_GETLBTEXT, SendMessage(hw, CB_GETCURSEL, 0, 0), 
				(LPARAM)Lang_Text);
			//if(lstrcmpi(Lang_Text, "Default") == 0) *szLang_temp = '\0';
			//else
			{
				char szPath[MAX_PATH], LangString[MAX_PATH];
				GetModuleFileName(NULL, szPath, MAX_PATH);
				lstrcpy(strrchr(szPath, '\\')+1, "language\\*.lang");
				lstrcpy(szLang_temp, szPath);
				hFind = FindFirstFile(szPath, &FindFileData);
				if(hFind != INVALID_HANDLE_VALUE)
				{
					do
					{
						lstrcpy(strrchr(szLang_temp, '\\')+1, FindFileData.cFileName);
						GetPrivateProfileString("LangOptions", "LanguageName", 
							"", LangString, MAX_PATH, szLang_temp);
						if(lstrcmp(LangString, Lang_Text) == 0)
							break;
					}
					while(FindNextFile(hFind, &FindFileData));
					FindClose(hFind);
				}
			}
		}
		break;
	case WM_DESTROY:
		DestroyWindow(hOpacityTip);
		DestroyWindow(hFavTip);
		break;
	default:
		break;
	}
	
	return FALSE;
}


extern BOOL bEnableFavMenu;
extern HICON hIcon;
extern char *DM2_Ver;
void ApplyDM2Setting()
{
	if(Adv_data.runtray)
		AddTaskBarIcon(pshared->DM2wnd, hIcon, DM2_Ver, 0);
	else
		DeleteTaskBarIcon(pshared->DM2wnd, 0);
	
	if(Adv_data.sysmenu)
	{
		DisableDM2Menu();
		EnableDM2Menu();
	}
	else
		DisableDM2Menu();
	
	if(Adv_data.EnableHotKey)
	{
		RegisterHotKey(pshared->DM2wnd, HK_CMD, 
			ConversionHotkey(Adv_data.dwHotKey), Adv_data.dwHotKey & 0xFF);
	}
	else
	{
		UnregisterHotKey(pshared->DM2wnd, HK_CMD);
	}

	if(Adv_data.detectclose)
		SetTimer(pshared->DM2wnd, IDC_CHK_DWC, 2000, NULL);
	else
		KillTimer(pshared->DM2wnd, IDC_CHK_DWC);
	
	if(Adv_data.dyntitle)
		SetTimer(pshared->DM2wnd, IDC_CHK_DYNT, 1000, NULL);
	else
		KillTimer(pshared->DM2wnd, IDC_CHK_DYNT);

	if(bEnableFavMenu)
		EnableFavMenu();
	else
		DisableFavMenu();

	ReRegisterHotKey();
}