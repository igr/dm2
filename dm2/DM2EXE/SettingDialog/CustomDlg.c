/* http://dm2.sf.net */
#include <Windows.h>
#include "../GetSetting.h"
#include "../util.h"
#include "../../global.h"
#include "SettingDlg.h"

extern PSCUSTOM_DATA Custom_data_temp;
extern SGEN_DATA Gen_data;
HWND hDM2_Custom;
char *RuleTestResult = NULL;
int RuleTest = 0;
char *DlgCustomSection = "SettingsDialogCustom";
extern HFONT hLocalFont;

DWORD dwBtnCust[] = {IDC_BTN_CADD, IDC_BTN_CEDIT, IDC_BTN_CDEL, 0, IDC_BTN_CTEST};

INT_PTR CALLBACK SettingDlgCustomProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		//Init ListView
		hDM2_Custom = GetDlgItem(hwndDlg, IDC_LIST_CUSTOM);
		ListView_SetExtendedListViewStyle(hDM2_Custom, 
			LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT);
		{
			char szBuf[MAX_PATH];
			LVCOLUMN lc;
			memset(&lc, 0, sizeof(lc));
			lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lc.fmt = LVCFMT_LEFT;
			lc.cx = 80;
			LoadLanguageString(lang_file, DlgCustomSection, 0, szBuf, MAX_PATH);
			lc.pszText = szBuf;
			ListView_InsertColumn(hDM2_Custom, 0, &lc);
			lc.cx = 230;
			LoadLanguageString(lang_file, DlgCustomSection, 1, szBuf, MAX_PATH);
			ListView_InsertColumn(hDM2_Custom, 1, &lc);
		}
		CreateRMenuCtrl(hDM2_Custom, hwndDlg, (LPDWORD)dwBtnCust, 
			sizeof(dwBtnCust)/sizeof(DWORD));

		RefreshCustomList();

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgCustomSection);
		return TRUE;
	case WM_NOTIFY:
		if(wParam == IDC_LIST_CUSTOM)
			if (((LPNMHDR)lParam)->code == NM_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDC_BTN_CEDIT, 0);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BTN_CADD:
			{
				PSCUSTOM_DATA pstm = NULL;
				pstm = GetNewCustom_Data(&Custom_data_temp);
				MakeCustomFromSetting(&pstm, Gen_data);
				if (DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_CUSTOM_EDIT, 
					hwndDlg, EditCustomDataProc, (LPARAM)pstm) == IDOK)
				{
					LVITEM lv;
					memset(&lv, 0, sizeof(lv));
					lv.mask = LVIF_TEXT|LVIF_PARAM;
					lv.iItem = ListView_GetItemCount(hDM2_Custom);
					lv.pszText = pstm->szRuleName;
					lv.cchTextMax = lstrlen(lv.pszText);
					lv.iSubItem = 0;
					lv.lParam = (LPARAM)pstm;
					ListView_InsertItem(hDM2_Custom, &lv);
					lv.mask = LVIF_TEXT;
					lv.pszText = pstm->szRule;
					lv.cchTextMax = lstrlen(lv.pszText);
					lv.iSubItem = 1;
					ListView_SetItem(hDM2_Custom, &lv);
				}
				else
				{
					RemoveCustom(&Custom_data_temp, pstm);
				}
			}
			break;
		case IDC_BTN_CDEL:
			ListView_DeleteItem(hDM2_Custom, 
				ListView_GetNextItem(hDM2_Custom, -1, LVNI_SELECTED));
			break;
		case IDC_BTN_CEDIT:
			{
				int i = ListView_GetNextItem(hDM2_Custom, -1, LVNI_SELECTED);
				if (i != -1)
				{
					LVITEM lv;
					memset(&lv, 0, sizeof(lv));
					lv.mask = LVIF_PARAM;
					lv.iItem = i;
					ListView_GetItem(hDM2_Custom, &lv);
					if (DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_CUSTOM_EDIT, 
						hwndDlg, EditCustomDataProc, lv.lParam) == IDOK)
						RefreshCustomList();
				}
			}
			break;
		case IDC_BTN_CTEST:
			{
				int i = ListView_GetNextItem(hDM2_Custom, -1, LVNI_SELECTED);
				if (i != -1)
				{
					char szBuf[MAX_PATH];
					char *pstr;
					LVITEM lv;
					char *pEnd;
					RuleTestResult = halloc(1100);
					memset(&lv, 0, sizeof(lv));
					lv.mask = LVIF_PARAM;
					lv.iItem = i;
					ListView_GetItem(hDM2_Custom, &lv);
					pstr = ((PSCUSTOM_DATA)(lv.lParam))->szRule;
					i = LoadLanguageString(lang_file, "Messages", 116, RuleTestResult, 1100);
					pEnd = stradd(RuleTestResult+i, " \r\r");
					EnumWindows(EnumWindowsForRuleProc, (LPARAM)pstr);
					pEnd = RuleTestResult + lstrlen(RuleTestResult);
					if(RuleTest == 0)
					{
						LoadLanguageString(lang_file, "Messages", 117, szBuf, MAX_PATH);
						stradd(pEnd, szBuf);
					}
					else if(RuleTest > 1000)
					{
						pEnd = stradd(pEnd, "\r");
						LoadLanguageString(lang_file, "Messages", 118, szBuf, MAX_PATH);
						stradd(pEnd, szBuf);
					}
					else
					{
						pEnd = stradd(pEnd, "\r");
						LoadLanguageString(lang_file, "Messages", 119, szBuf, MAX_PATH);
						stradd(pEnd, szBuf);
					}
					LoadLanguageString(lang_file, "Messages", 120, szBuf, MAX_PATH);
					MessageBox(hwndDlg, RuleTestResult, szBuf, 
						MB_ICONINFORMATION|MB_OK);
					hfree(RuleTestResult);
					RuleTestResult = NULL;
					RuleTest = 0;
				}
			}
			break;
		}
		break;
	case WM_DM2_SAVESETTING:
		{
			// ReBuilding Custom_data
			PSCUSTOM_DATA temp = NULL;
			LVITEM lv;
			int i = 0;
			memset(&lv, 0, sizeof(LVITEM));
			lv.mask = LVIF_PARAM;
 			for(; i<ListView_GetItemCount(hDM2_Custom); i++)
			{
				PSCUSTOM_DATA last = NULL;
				lv.iItem = i;
				ListView_GetItem(hDM2_Custom, &lv);
				last = GetNewCustom_Data(&temp);
				CopyCustomData(last, (PSCUSTOM_DATA)lv.lParam);
			}
			RemoveAllCustom(&Custom_data_temp);
			Custom_data_temp = temp;
		}
		break;
	case WM_DESTROY:
		hDM2_Custom = 0;
		break;
	default:
		break;
	}
	
	return FALSE;
}


BOOL CALLBACK EnumWindowsForRuleProc(HWND hwnd, LPARAM lParam)
{
	if (GetParent(hwnd) == NULL)
	{
		char szTitle[MAX_PATH];
		int i = GetWindowText(hwnd, szTitle, MAX_PATH);
		if(regexec_and_free(regcomp((char *)lParam), szTitle))
		{
			RuleTest += i;
			if(RuleTest > 1000)
				return FALSE;

			stradd(stradd(RuleTestResult+lstrlen(RuleTestResult), szTitle), "\r");
		}
	}
	
	return TRUE;
}


void RefreshCustomList(void)
{
	PSCUSTOM_DATA last = Custom_data_temp;
	LVITEM lv;
	memset(&lv, 0, sizeof(lv));
	ListView_DeleteAllItems(hDM2_Custom);
	while(last)
	{
		lv.mask = LVIF_TEXT|LVIF_PARAM;
		lv.iItem = ListView_GetItemCount(hDM2_Custom);
		lv.pszText = last->szRuleName;
		lv.cchTextMax = lstrlen(lv.pszText);
		lv.iSubItem = 0;
		lv.lParam = (LPARAM)last;
		ListView_InsertItem(hDM2_Custom, &lv);
		lv.mask = LVIF_TEXT;
		lv.pszText = last->szRule;
		lv.cchTextMax = lstrlen(lv.pszText);
		lv.iSubItem = 1;
		ListView_SetItem(hDM2_Custom, &lv);
		last = last->next;
	}
}


BOOL isEdit;
SGEN_DATA Gen;
HWND hChild;
PSCUSTOM_DATA pstm;
char *EditDlgCustomSection = "SettingsDialogCustomEdit";
INT_PTR CALLBACK EditCustomDataProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char szBuf[MAX_PATH];
			int x,y;
			POINT po = {0, 0};
			POINT pom = {0, 0};
			//Get point
			ClientToScreen(GetDlgItem(hwndDlg, IDC_STATIC_DLG), &po);
			ClientToScreen(hwndDlg, &pom);
			x = po.x - pom.x;
			y = po.y - pom.y;
		
			pstm = (PSCUSTOM_DATA)lParam;

			//Set mode and title
			isEdit = (*(pstm->szRuleName) != 0);
			
			if (isEdit)
				LoadLanguageString(lang_file, EditDlgCustomSection, 100, szBuf, MAX_PATH);
			else
				LoadLanguageString(lang_file, EditDlgCustomSection, 101, szBuf, MAX_PATH);
			SetWindowText(hwndDlg, szBuf);

			SendDlgItemMessage(hwndDlg, IDC_EDIT_RN, EM_LIMITTEXT, MAXRULENAME, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_RE, EM_LIMITTEXT, MAXRULE, 0);

			//Set Text and made SGEN_DATA
			SetDlgItemText(hwndDlg, IDC_EDIT_RN, pstm->szRuleName);
			SetDlgItemText(hwndDlg, IDC_EDIT_RE, pstm->szRule);
			MakeSettingFromCustom(pstm->data1, pstm->data2, pstm->data3, &Gen);
				
			//Create Child window
			hChild = CreateDialogParam(processHandle, 
				(LPCTSTR)IDD_DM2_GENERAL, hwndDlg, 
				SettingDlgGeneralProc, (LPARAM)&Gen);
			SetWindowPos(hChild, 0, x, y, 0, 0, SWP_NOSIZE);
			ShowWindow(hChild, SW_SHOW);
		}

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, EditDlgCustomSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			//test 
			GetDlgItemText(hwndDlg, IDC_EDIT_RN, pstm->szRuleName, MAXRULENAME2);
			GetDlgItemText(hwndDlg, IDC_EDIT_RE, pstm->szRule, MAXRULE2);
			if(*(pstm->szRuleName) == 0 || *(pstm->szRule) == 0)
			{
				LocalMsgBox(hwndDlg, lang_file, 0, 100, MB_OK|MB_ICONINFORMATION);
				return FALSE;
			}

			SendMessage(hChild, WM_DM2_SAVESETTING, 0, 0);
			
			MakeCustomFromSetting(&pstm, Gen);
			GetDlgItemText(hwndDlg, IDC_EDIT_RN, pstm->szRuleName, MAXRULENAME2);
			GetDlgItemText(hwndDlg, IDC_EDIT_RE, pstm->szRule, MAXRULE2);
			
			EndDialog(hwndDlg, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		memset(&Gen, 0, sizeof(Gen));
		hChild = NULL;
		pstm = NULL;
		break;
	default:
		break;
	}
	
	return FALSE;
}