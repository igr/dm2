/* http://dm2.sf.net */
#include <Windows.h>
#include "../GetSetting.h"
#include "../util.h"
#include "../../global.h"
#include "SettingDlg.h"

DWORD dwBtnExcept[] = {IDC_BTN_EADD, IDC_BTN_EEDIT, IDC_BTN_EDEL};

extern PSEXCEPT_DATA Except_data_temp;
HWND hDM2_Except = NULL;
extern HFONT hLocalFont;
INT_PTR CALLBACK SettingDlgExceptionProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			PSEXCEPT_DATA last = Except_data_temp;
			int i = 0;
			hDM2_Except = GetDlgItem(hwndDlg, IDC_LIST_EXCEPTION);
			while(last)
			{
				i = SendMessage(hDM2_Except, LB_ADDSTRING, 0, (LPARAM)last->szRule);
				SendMessage(hDM2_Except, LB_SETITEMDATA, i, last->Settings);
				last = last->next;
			}
		}

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, "SettingsDialogExcept");
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BTN_EADD:
			DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_EXCEPTION_EDIT, 
				hwndDlg, EditExceptDataProc, -1);
			break;
		case IDC_BTN_EDEL:
			{
				int i = SendMessage(hDM2_Except, LB_GETCURSEL, 0, 0);
				SendMessage(hDM2_Except, LB_DELETESTRING, i, 0);
			}
			break;
		case IDC_LIST_EXCEPTION:
			if(HIWORD(wParam) != LBN_DBLCLK)
				break;
		case IDC_BTN_EEDIT:
			{
				int i = SendMessage(hDM2_Except, LB_GETCURSEL, 0, 0);
				if(i != LB_ERR)
					DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_EXCEPTION_EDIT, 
						hwndDlg, EditExceptDataProc, i);
			}
			break;
		default:
			break;
		}
		break;
	case WM_DM2_SAVESETTING:
		{
			// ReBuilding Except_data_temp
			PSEXCEPT_DATA temp = NULL;
			int i = 0;
			char szBuf[MAXRULE2];
			for(; i<SendMessage(hDM2_Except, LB_GETCOUNT, 0, 0); i++)
			{
				PSEXCEPT_DATA last = NULL;
				last = GetNewExcept_Data(&temp);
				SendMessage(hDM2_Except, LB_GETTEXT, i, (LPARAM)szBuf);
				lstrcpy(last->szRule, szBuf);
				last->Settings = SendMessage(hDM2_Except, LB_GETITEMDATA, i, 0);
			}
			RemoveAllExcept(&Except_data_temp);
			Except_data_temp = temp;
		}
		break;
	case WM_DESTROY:
		hDM2_Except = NULL;
		break;
	case WM_SETCURSOR:
		if((HWND)wParam == hDM2_Except && HIWORD(lParam) == WM_RBUTTONUP)
		{
			//only for listbox.
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(hDM2_Except, &point);
			SendMessage(hDM2_Except, LB_SETCURSEL, 
				SendMessage(hDM2_Except, LB_ITEMFROMPOINT, 0, 
				MAKELPARAM(point.x, point.y)), 0);
			
			//pop menu
			PopContextMenuFromButton(hwndDlg, (LPDWORD)&dwBtnExcept, 
				sizeof(dwBtnExcept)/sizeof(DWORD));
		}
		break;
	default:
		break;
	}
	
	return FALSE;
}


BOOL IsEditExcept = FALSE;
char *EditDlgExceotSection = "SettingsDialogExceptEdit";
INT_PTR CALLBACK EditExceptDataProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char szBuf[MAX_PATH];
			//Set mode and title
			IsEditExcept = (lParam != LB_ERR);

			SendDlgItemMessage(hwndDlg, IDC_EDIT_RULE, EM_LIMITTEXT, MAXRULE, 0);
			CheckDlgButton(hwndDlg, IDC_RADIO_TEXT, BST_CHECKED);
			
			if(IsEditExcept)
			{
				char szTemp[MAXRULE2];
				DWORD dwSet = SendMessage(hDM2_Except, LB_GETITEMDATA, (WPARAM)lParam, 0);
				LoadLanguageString(lang_file, EditDlgExceotSection, 100, szBuf, MAX_PATH);
				SendMessage(hDM2_Except, LB_GETTEXT, (WPARAM)lParam, (LPARAM)szTemp);
				if(HIWORD(dwSet) == 1)
				{
					CheckDlgButton(hwndDlg, IDC_RADIO_PROC, BST_CHECKED);
					CheckDlgButton(hwndDlg, IDC_RADIO_TEXT, BST_UNCHECKED);
				}
				CheckDlgButton(hwndDlg, IDC_CHK_DF, dwSet & EXCEPT_FAV);
				CheckDlgButton(hwndDlg, IDC_CHK_DDM, dwSet & EXCEPT_OTHER);
				CheckDlgButton(hwndDlg, IDC_CHK_DSYS, dwSet & EXCEPT_SYSMENU);
				SetDlgItemText(hwndDlg, IDC_EDIT_RULE, szTemp);
			}
			else
				LoadLanguageString(lang_file, EditDlgExceotSection, 101, szBuf, MAX_PATH);
			SetWindowText(hwndDlg, szBuf);
		}

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, EditDlgExceotSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char szTemp[MAXRULE2];
				DWORD setting = 0;
				int i;
				//test 
				GetDlgItemText(hwndDlg, IDC_EDIT_RULE, szTemp, MAXRULE2);
				if(*szTemp == 0)
				{
					LocalMsgBox(hwndDlg, lang_file, 0, 101, MB_OK|MB_ICONINFORMATION);
					return FALSE;
				}
				if(!IsDlgButtonChecked(hwndDlg, IDC_CHK_DF) &&
					!IsDlgButtonChecked(hwndDlg, IDC_CHK_DDM) &&
					!IsDlgButtonChecked(hwndDlg, IDC_CHK_DSYS))
				{
					LocalMsgBox(hwndDlg, lang_file, 0, 102, MB_OK|MB_ICONINFORMATION);
					return FALSE;
				}
				// add & edit
				if(IsDlgButtonChecked(hwndDlg, IDC_CHK_DF))
					setting += EXCEPT_FAV;
				if(IsDlgButtonChecked(hwndDlg, IDC_CHK_DDM))
					setting += EXCEPT_OTHER;
				if(IsDlgButtonChecked(hwndDlg, IDC_CHK_DSYS))
					setting += EXCEPT_SYSMENU;
				if(IsDlgButtonChecked(hwndDlg, IDC_RADIO_PROC))
					setting += 0x10000;
				if(IsEditExcept)
				{
					i = SendMessage(hDM2_Except, LB_GETCURSEL, 0, 0);
					SendMessage(hDM2_Except, LB_DELETESTRING, i, 0);
					i = SendMessage(hDM2_Except, LB_INSERTSTRING, i, (LPARAM)szTemp);
					SendMessage(hDM2_Except, LB_SETCURSEL, i, 0);
				}
				else
				{
					i = SendMessage(hDM2_Except, LB_GETCOUNT, 0, 0);
					SendMessage(hDM2_Except, LB_INSERTSTRING, i, (LPARAM)szTemp);
				}
				SendMessage(hDM2_Except, LB_SETITEMDATA, i, setting);
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
	case WM_DESTROY:
		break;
	default:
		break;
	}
	
	return FALSE;
}

