/* http://dm2.sf.net */
#include <windows.h>
#include "SettingDlg.h"
#include "../util.h"
#include "../Plugins.h"
#include "../../DM2DLL/DllMain.h"
#include "../dm2WndProc.h"

char *Action_GetCmtText(char *dm2cmd);
char *Action_GetCmdText(char *dm2cmt);
char *Action_GetCmdFromIndex(int cmd);

INT_PTR WINAPI AlignDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

char AC_none[MAX_COMMENT];
char AC_hide[MAX_COMMENT];
char AC_tray[MAX_COMMENT];
char AC_icon[MAX_COMMENT];
char AC_2bottom[MAX_COMMENT];
char AC_resize[MAX_COMMENT];
char AC_align[MAX_COMMENT];
char AC_inc[MAX_COMMENT];
char AC_dec[MAX_COMMENT];
char AC_reset[MAX_COMMENT];
char AC_top[MAX_COMMENT];

char *Action_Command[] = {
	AC_none,
	AC_hide,
	AC_tray,
	AC_icon,
	AC_2bottom,
	AC_resize,
	AC_align,
	AC_inc,
	AC_dec,
	AC_reset,
	AC_top,
	""								//must use null string in end.
};

extern PFF_PLUGINS ffp;
extern PSAC_DATA Action_data_temp;
extern PSAC_DATA Action_data;
extern HFONT hLocalFont;
extern char *DlgHKSection;
char *DlgActionSection = "SettingsDialogAction";
INT_PTR WINAPI SettingDlgActionProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		LoadLanguageString(lang_file, DlgActionSection, 100, AC_none, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgActionSection, 101, AC_2bottom, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgActionSection, 102, AC_resize, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgActionSection, 103, AC_align, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgHKSection, 100, AC_hide, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgHKSection, 101, AC_tray, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgHKSection, 102, AC_icon, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgHKSection, 103, AC_inc, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgHKSection, 104, AC_dec, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgHKSection, 105, AC_reset, MAX_COMMENT);
		LoadLanguageString(lang_file, DlgHKSection, 106, AC_top, MAX_COMMENT);
		{
			int i=0, j;
			PFF_PLUGINS last = ffp;
			while(*Action_Command[i] != '\0')
			{
				for(j=IDC_COMBO_RIB; j<=IDC_COMBO_CMCB; j++)
					SendDlgItemMessage(hwndDlg, j, CB_ADDSTRING, 0, (LPARAM)Action_Command[i]);
				i++;
			}
			while(last)
			{
				if(last->PluginsCategory == FFPC_ACTION)
				{
					for(j=IDC_COMBO_RIB; j<=IDC_COMBO_CMCB; j++)
						SendDlgItemMessage(hwndDlg, j, CB_ADDSTRING, 0, (LPARAM)last->pComments);
				}
				last = last->Next;
			}
			
			for(j=IDC_COMBO_RIB; j<=IDC_COMBO_CMCB; j++)
			{
				SendDlgItemMessage(hwndDlg, j, CB_SELECTSTRING, -1, 
					(LPARAM)Action_GetCmtText(Action_data_temp->szCMD[j-IDC_COMBO_RIB]));
				SendDlgItemMessage(hwndDlg, j, CB_SETDROPPEDWIDTH, 250, 0);
			}
		}
		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgActionSection);
		return TRUE;
	case WM_DM2_SAVESETTING:
		{
			char szSelect[MAX_COMMENT];
			int iIndex, j;
			for(j=IDC_COMBO_RIB; j<=IDC_COMBO_CMCB; j++)
			{
				iIndex = SendDlgItemMessage(hwndDlg, j, CB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwndDlg, j, CB_GETLBTEXT, iIndex, (LPARAM)szSelect);
				lstrcpy(Action_data_temp->szCMD[j-IDC_COMBO_RIB], 
					Action_GetCmdText(szSelect));
			}
		}
		break;
	}

	return FALSE;
}

static char cm_nono[] = "dm2_cmd_none";
static char cm_hide[] = "dm2_cmd_hidewin";
static char cm_tray[] = "dm2_cmd_traywin";
static char cm_icon[] = "dm2_cmd_floatwin";
static char cm_2bottom[] = "dm2_cmd_win_2bottom";
static char cm_resize[] = "dm2_cmd_win_resize";
static char cm_align[] = "dm2_cmd_win_align";
static char cm_inc[] = "dm2_cmd_inc_opacity";
static char cm_dec[] = "dm2_cmd_dec_opacity";
static char cm_opa[] = "dm2_cmd_reset_opacity";
static char cm_top[] = "dm2_cmd_win_ontop";
static char *DM2_AC_Command[] = {
	cm_nono,
	cm_hide,
	cm_tray,
	cm_icon,
	cm_2bottom,
	cm_resize,
	cm_align,
	cm_inc,
	cm_dec,
	cm_opa,
	cm_top,
	""								//must use null string in end.
};

//Get command comments for command string
char *Action_GetCmtText(char *dm2cmd)
{
	int i = 0;
	PFF_PLUGINS last = ffp;
	while(DM2_AC_Command[i][0] != '\0')
	{
		if(lstrcmp(DM2_AC_Command[i], dm2cmd) == 0)
			return Action_Command[i];
		i++;
	}
	while(last)
	{
		if(last->PluginsCategory == FFPC_ACTION)
			if(lstrcmp(last->pCmd, dm2cmd) == 0)
				return last->pComments;
		last = last->Next;
	}
	return NULL;
}

//Get command string for command comments
char *Action_GetCmdText(char *dm2cmt)
{
	int i = 0;
	PFF_PLUGINS last = ffp;
	while(Action_Command[i][0] != '\0')
	{
		if(lstrcmp(Action_Command[i], dm2cmt) == 0)
			return DM2_AC_Command[i];
		i++;
	}
	while(last)
	{
		if(last->PluginsCategory == FFPC_ACTION)
			if(lstrcmp(last->pComments, dm2cmt) == 0)
				return last->pCmd;
		last = last->Next;
	}
	return NULL;
}

//Get command string for command index.
char *Action_GetCmdFromIndex(int cmd)
{
	int i = 0;
	PFF_PLUGINS last = ffp;
	while(DM2_AC_Command[i][0] != '\0')
	{
		if(i == cmd)
			return DM2_AC_Command[i];
		i++;
	}
	while(last)
	{
		if(last->PluginsCategory == FFPC_ACTION)
		{
			if(i == cmd)
				return last->pCmd;
			i++;
		}
		last = last->Next;
	}
	return NULL;
}

HWND LastReszieWin = NULL;
HWND LastAlignWin = NULL;
extern void AddHideWindow(HWND hWnd);
void Do_Action_Command(HWND hWnd, DWORD Info)
{
	int i;
	char *pCMD;
	PFF_PLUGINS last = ffp;

	if((Info & IDM_ACTION_RCLICK) || (Info & IDM_ACTION_MCLICK))
	{
		if(Info & IDM_ACTION_RCLICK) i=0;
		else if(Info & IDM_ACTION_MCLICK) i=12;
		
		if(Info & IDM_ACTION_SHIFT) i += 3;
		else if(Info & IDM_ACTION_ALT) i += 6;
		else if(Info & IDM_ACTION_CTRL) i += 9;
		
		if(Info & IDM_ACTION_MIN) i += 0;
		else if(Info & IDM_ACTION_MAX) i += 1;
		else if(Info & IDM_ACTION_CLOSE) i += 2;

		pCMD = Action_data->szCMD[i];

		if(lstrcmpi(pCMD, cm_hide) == 0)
		{
			if(getCompatibleSystemMenu(hWnd) == NULL ||
				IsException(hWnd, EXCEPT_OTHER))
				return;
			AddHideWindow(hWnd);
		}
		else if(lstrcmpi(pCMD, cm_tray) == 0)
		{
			if(getCompatibleSystemMenu(hWnd) == NULL ||
				IsException(hWnd, EXCEPT_OTHER))
				return;
			MinimizeWndToTray(pshared->DM2wnd, hWnd);
		}
		else if(lstrcmpi(pCMD, cm_icon) == 0)
		{
			if(getCompatibleSystemMenu(hWnd) == NULL ||
				IsException(hWnd, EXCEPT_OTHER))
				return;
			MinimizeWndToFloatingIcon(hWnd);
		}
		else if(lstrcmpi(pCMD, cm_2bottom) == 0)
			SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		else if(lstrcmpi(pCMD, cm_resize) == 0)
		{
			HMENU menu;
			HMENU menuPopup;
			POINT p;

			GetCursorPos(&p);
			menu = LoadMenu(processHandle, (LPCSTR)IDR_MENU_RESIZE);
			if (menu == NULL) return;
			menuPopup = GetSubMenu(menu, 0);
			TranslateMenuLanguage(lang_file, menuPopup, "MenuResize");
			LastReszieWin = hWnd;
			SetForegroundWindow(pshared->DM2wnd);
			TrackPopupMenuEx(menuPopup, TPM_CENTERALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | 
				TPM_RIGHTBUTTON, p.x, p.y, pshared->DM2wnd, NULL);
			DestroyMenu(menu);
		}
		else if(lstrcmpi(pCMD, cm_align) == 0)
		{
			LastAlignWin = hWnd;
			DialogBox(processHandle, (LPCTSTR)IDD_DM2_ACTION_ALIGN, 
				NULL, AlignDlgProc);
			LastAlignWin = NULL;
		}
		else if(lstrcmpi(pCMD, cm_inc) == 0)
		{
			if(getCompatibleSystemMenu(hWnd) == NULL || 
				IsException(hWnd, EXCEPT_OTHER))
				return;
			AdjustWindowOpacity(hWnd, TRUE, 10);					
		}
		else if(lstrcmpi(pCMD, cm_dec) == 0)
		{
			if(getCompatibleSystemMenu(hWnd) == NULL || 
				IsException(hWnd, EXCEPT_OTHER))
				return;
			AdjustWindowOpacity(hWnd, FALSE, 10);					
		}
		else if(lstrcmpi(pCMD, cm_opa) == 0)
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
		else if(lstrcmpi(pCMD, cm_top) == 0)
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
		else
		{
			// do plugins command
			while(last)
			{
				if(lstrcmpi(pCMD, last->pCmd) == 0)
				{
					//plugins uninstall? and PluginsCategory
					if(last->Module && last->PluginsCategory == FFPC_ACTION)
						last->Execute(hWnd, NULL, 0, FALSE);
					break;
				}
				last = last->Next;
			}
		}
	}
}

void ResizeWindow(int w, int h, BOOL bIsPixel)
{
	if(LastReszieWin)
	{
		if(bIsPixel == FALSE)
		{
			RECT rect;
			float fw,fh;
			GetDesktopRect(&rect);
			fw = (float)((rect.right - rect.left)*w)/100;
			fh = (float)((rect.bottom -rect.top)*h)/100;
			VarI4FromR4(fw, (LONG *)&w);
			VarI4FromR4(fh, (LONG *)&h);
		}
		SetWindowPos(LastReszieWin, HWND_TOP, 0, 0, w, h, SWP_NOMOVE);
		LastReszieWin = NULL;
	}
}

char *DlgResizeSection = "DialogResize";
HFONT hFont;
INT_PTR WINAPI ResizeDlgProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		CheckDlgButton(hwndDlg, IDC_RADIO_PIX, BST_CHECKED);
		SendMessage(hwndDlg, WM_COMMAND, IDC_RADIO_PIX, 0);
		SendDlgItemMessage(hwndDlg, IDC_EDIT_W, WM_SETTEXT, 0, (LPARAM)"0");
		SendDlgItemMessage(hwndDlg, IDC_EDIT_H, WM_SETTEXT, 0, (LPARAM)"0");

		hFont = GetLanguageFont(hwndDlg, lang_file);
		SetLanguageFont(hFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgResizeSection);

		SetForegroundWindow(pshared->DM2wnd);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_RADIO_PIX:
			SendDlgItemMessage(hwndDlg, IDC_EDIT_W, EM_LIMITTEXT, 4, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_H, EM_LIMITTEXT, 4, 0);
			break;
		case IDC_RADIO_PER:
			SendDlgItemMessage(hwndDlg, IDC_EDIT_W, EM_LIMITTEXT, 3, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_H, EM_LIMITTEXT, 3, 0);
			break;
		case IDOK:
			{
				char szNum[5];
				int w,h;
				BOOL IsPix = (IsDlgButtonChecked(hwndDlg, IDC_RADIO_PIX) == BST_CHECKED);
				SendDlgItemMessage(hwndDlg, IDC_EDIT_W, WM_GETTEXT, 5, (LPARAM)szNum);
				w = atoi(szNum);
				SendDlgItemMessage(hwndDlg, IDC_EDIT_H, WM_GETTEXT, 5, (LPARAM)szNum);
				h = atoi(szNum);
				if(IsPix)
				{
					w = setIntRange(w, 0, 1600);
					h = setIntRange(h, 0, 1200);
				}
				else
				{
					w = setIntRange(w, 0, 100);
					h = setIntRange(h, 0, 100);
				}
				ResizeWindow(w, h, IsPix);
			}
			EndDialog(hwndDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, FALSE);
			break;
		}
		break;
	case WM_NCDESTROY:
		DeleteObject(hFont);
		break;
	case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
			EndDialog(hwndDlg, FALSE);
		break;
	}

	return FALSE;
}

HIMAGELIST hImgAlign;
INT_PTR WINAPI AlignDlgProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		hImgAlign = ImageList_Create(18, 16, ILC_MASK | ILC_COLOR32, 0, 0);
		ImageList_AddMasked(hImgAlign, 
			LoadBitmap(processHandle, (LPCTSTR)IDB_BITMAP_ALIGN), (COLORREF)0xfffffe);
		{
			HICON hIcon;
			HWND hImg;
			int i=0;
			for(; i<9; i++)
			{
				hImg = GetDlgItem(hwndDlg, IDC_STATIC_TL+i);
				hIcon = ImageList_GetIcon(hImgAlign, i, 0);
				SetWindowLong(hImg, GWL_STYLE, GetWindowLong(hImg, GWL_STYLE)|SS_ICON);
				SendDlgItemMessage(hwndDlg, IDC_STATIC_TL+i, STM_SETICON, 
					(WPARAM)hIcon, 0);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg, FALSE);
			break;
		default:
			if(LOWORD(wParam) >= IDC_STATIC_TL && IDC_STATIC_TL <= IDC_STATIC_BR)
			{
				RECT dr, wr;
				int wid = LOWORD(wParam) - IDC_STATIC_TL;
				int x = 0, y = 0;
				int dw, dh, ww, wh;
				GetDesktopRect(&dr);
				GetWindowRect(LastAlignWin, &wr);
				dw = dr.right - dr.left;
				dh = dr.bottom - dr.top;
				ww = wr.right - wr.left;
				wh = wr.bottom - wr.top;
				
				if(wid<3)
					y = dr.top;
				else if(wid<6 && wid >=3)
					y = (dr.top + (dh-wh))/2;
				else if(wid<9 && wid >=6)
					y = dr.top + (dh-wh);

				wid = wid%3;
				if(wid == 0) x = dr.left;
				else if(wid == 1) x = (dr.left + (dw-ww))/2;
				else if(wid == 2) x = dr.left + (dw-ww);

				SetWindowPos(LastAlignWin, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
			}
			break;
		}
		break;
	case WM_NCDESTROY:
		ImageList_Destroy(hImgAlign);
		break;
	case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
			EndDialog(hwndDlg, FALSE);
		break;
	}

	return FALSE;
}
