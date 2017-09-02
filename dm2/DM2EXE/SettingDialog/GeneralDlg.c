/* http://dm2.sf.net */
#include <Windows.h>
#include "../GetSetting.h"
#include "../../global.h"
#include "SettingDlg.h"

PSGEN_DATA pgen = NULL;
HBRUSH hBrush_bc;
HBRUSH hBrush_bgc;
CHOOSECOLOR cc;
static COLORREF acrCustClr[16];
char *DlgGenSection = "SettingsDialogGeneral";
extern HFONT hLocalFont;
extern BOOL IsWinNT;
INT_PTR CALLBACK SettingDlgGeneralProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			//init control
			HWND htemp;
			char szBuf[MAX_PATH];
			char sztemp[9];
			pgen = (PSGEN_DATA)lParam;
			
			CheckDlgButton(hwndDlg, IDC_CHK_FLOATTOP, pgen->ontop);
			CheckDlgButton(hwndDlg, IDC_CHK_FOLLOWMODE, pgen->follow);
			CheckDlgButton(hwndDlg, IDC_CHK_OPA, pgen->enableopa);
			htemp = GetDlgItem(hwndDlg, IDC_COMBO_OPA);
			EnableWindow(htemp, pgen->enableopa);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"10");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"20");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"30");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"40");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"50");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"60");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"70");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"80");
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)"90");
			SendMessage(htemp, CB_LIMITTEXT, 2, 0);
			wsprintf(sztemp, "%d", pgen->opacity);
			SetWindowText(htemp, sztemp);
			htemp = GetDlgItem(hwndDlg, IDC_COMBO_POS);
			LoadLanguageString(lang_file, DlgGenSection, 100, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			LoadLanguageString(lang_file, DlgGenSection, 101, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			LoadLanguageString(lang_file, DlgGenSection, 102, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			LoadLanguageString(lang_file, DlgGenSection, 103, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			LoadLanguageString(lang_file, DlgGenSection, 104, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			SendMessage(htemp, CB_SETCURSEL, pgen->initpos, 0);
			htemp = GetDlgItem(hwndDlg, IDC_EDIT_BC);
			SendMessage(htemp, EM_LIMITTEXT, 6, 0);
			wsprintf(sztemp, "%06X", pgen->bcolor);
			SetWindowText(htemp, sztemp);
			htemp = GetDlgItem(hwndDlg, IDC_EDIT_BGC);
			SendMessage(htemp, EM_LIMITTEXT, 6, 0);
			wsprintf(sztemp, "%06X", pgen->bgcolor);
			SetWindowText(htemp, sztemp);
			CheckDlgButton(hwndDlg, IDC_CHK_MSS, pgen->screenshots);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SF), pgen->screenshots);
			CheckDlgButton(hwndDlg, IDC_CHK_SF, pgen->stretch);
			htemp = GetDlgItem(hwndDlg, IDC_EDIT_SSW);
			SendMessage(htemp, EM_LIMITTEXT, 3, 0);
			wsprintf(sztemp, "%d", pgen->shWidth);
			SetWindowText(htemp, sztemp);
			EnableWindow(htemp, pgen->screenshots);
			htemp = GetDlgItem(hwndDlg, IDC_EDIT_SSH);
			SendMessage(htemp, EM_LIMITTEXT, 3, 0);
			wsprintf(sztemp, "%d", pgen->shHeight);
			SetWindowText(htemp, sztemp);
			EnableWindow(htemp, pgen->screenshots);

			htemp = GetDlgItem(hwndDlg, IDC_COMBO_FSIZE);
			LoadLanguageString(lang_file, DlgGenSection, 105,  szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			LoadLanguageString(lang_file, DlgGenSection, 106, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			LoadLanguageString(lang_file, DlgGenSection, 107, szBuf, MAX_PATH);
			SendMessage(htemp, CB_ADDSTRING, 0, (LPARAM)szBuf);
			SendMessage(htemp, CB_SETCURSEL, pgen->ficon_size, 0);
			
			CheckDlgButton(hwndDlg, IDC_CHK_MOVE, pgen->move);

			hBrush_bc = CreateSolidBrush(pgen->bcolor);
			hBrush_bgc = CreateSolidBrush(pgen->bgcolor);
			
			ZeroMemory(&cc, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR); 
			cc.hwndOwner = hwndDlg;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			cc.lpCustColors = (LPDWORD) acrCustClr;

			if(!IsWinNT)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_OPA), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_OPA), FALSE);
			}
		}

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgGenSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHK_OPA:
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_OPA), 
				IsDlgButtonChecked(hwndDlg, IDC_CHK_OPA));
			break;
		case IDC_CHK_MSS:
			{
				BOOL bEnable;
				bEnable = IsDlgButtonChecked(hwndDlg, IDC_CHK_MSS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SF), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_SSW), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_SSH), bEnable);
			}
			break;
		case IDC_BTN_BGC:
			{
				char sztemp[9];
				GetDlgItemText(hwndDlg, IDC_EDIT_BGC, sztemp, 9);
				cc.rgbResult = str2int(sztemp) & 0x00FFFFFF;
				if(ChooseColor(&cc) == IDOK)
				{
					DeleteObject(hBrush_bgc);
					hBrush_bgc = CreateSolidBrush(cc.rgbResult);
					wsprintf(sztemp, "%08X", cc.rgbResult);
					SetDlgItemText(hwndDlg, IDC_EDIT_BGC, sztemp);
					InvalidateRect(GetDlgItem(hwndDlg, IDC_BTN_BGC), NULL, TRUE);
				}
			}
			break;
		case IDC_BTN_BC:
			{
				char sztemp[9];
				GetDlgItemText(hwndDlg, IDC_EDIT_BC, sztemp, 9);
				cc.rgbResult = str2int(sztemp) & 0x00FFFFFF;
				if(ChooseColor(&cc) == IDOK)
				{
					DeleteObject(hBrush_bc);
					hBrush_bc = CreateSolidBrush(cc.rgbResult);
					wsprintf(sztemp, "%08X", cc.rgbResult);
					SetDlgItemText(hwndDlg, IDC_EDIT_BC, sztemp);
					InvalidateRect(GetDlgItem(hwndDlg, IDC_BTN_BC), NULL, TRUE);
				}
			}
			break;
		case IDC_EDIT_BGC:
			if(HIWORD(wParam) == EN_KILLFOCUS)
			{
				char sztemp[9];
				GetDlgItemText(hwndDlg, IDC_EDIT_BGC, sztemp, 9);
				DeleteObject(hBrush_bgc);
				hBrush_bgc = CreateSolidBrush(str2int(sztemp) & 0x00FFFFFF);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_BTN_BGC), NULL, TRUE);
			}
			break;
		case IDC_EDIT_BC:
			if(HIWORD(wParam) == EN_KILLFOCUS)
			{
				char sztemp[9];
				GetDlgItemText(hwndDlg, IDC_EDIT_BC, sztemp, 9);
				DeleteObject(hBrush_bc);
				hBrush_bc = CreateSolidBrush(str2int(sztemp) & 0x00FFFFFF);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_BTN_BC), NULL, TRUE);
			}
			break;
		}
		break;
	case WM_DM2_SAVESETTING:
		pgen->ontop = IsDlgButtonChecked(hwndDlg, IDC_CHK_FLOATTOP);
		pgen->follow = IsDlgButtonChecked(hwndDlg, IDC_CHK_FOLLOWMODE);
		pgen->move = IsDlgButtonChecked(hwndDlg, IDC_CHK_MOVE);
		pgen->enableopa = IsDlgButtonChecked(hwndDlg, IDC_CHK_OPA);
		pgen->screenshots = IsDlgButtonChecked(hwndDlg, IDC_CHK_MSS);
		pgen->stretch = IsDlgButtonChecked(hwndDlg, IDC_CHK_SF);
		pgen->initpos = (BYTE)SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_POS), 
			CB_GETCURSEL, 0, 0);
		pgen->ficon_size = (BYTE)SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_FSIZE), 
			CB_GETCURSEL, 0, 0);
		{
			char sztemp[9];
			GetDlgItemText(hwndDlg, IDC_EDIT_BC, sztemp, 9);
			pgen->bcolor = str2int(sztemp);
			GetDlgItemText(hwndDlg, IDC_EDIT_BGC, sztemp, 9);
			pgen->bgcolor = str2int(sztemp);
			GetDlgItemText(hwndDlg, IDC_EDIT_SSW, sztemp, 9);
			pgen->shWidth = setIntRange(atoi(sztemp), 16, 640);
			GetDlgItemText(hwndDlg, IDC_EDIT_SSH, sztemp, 9);
			pgen->shHeight = setIntRange(atoi(sztemp), 16, 480);
			GetDlgItemText(hwndDlg, IDC_COMBO_OPA, sztemp, 3);
			pgen->opacity = atoi(sztemp);
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBrush_bc);
		DeleteObject(hBrush_bgc);
		break;
	case WM_CTLCOLORBTN:
		switch(GetDlgCtrlID((HWND)lParam))
		{
		case IDC_BTN_BGC:
			return (long)hBrush_bgc;
		case IDC_BTN_BC:
			return (long)hBrush_bc;
		}
		break;
	default:
		break;
	}
	
	return FALSE;
}