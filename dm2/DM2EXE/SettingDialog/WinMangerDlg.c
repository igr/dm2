/* http://dm2.sf.net */
#include <Windows.h>
#include "../util.h"
#include "../../global.h"
#include "SettingDlg.h"


extern PWND_DATA wndData;
extern PSHARED_DATA pshared;
HWND hListView_dm2;
HIMAGELIST hil;
#define MAX_STATUS 30
char szNoneTitle[MAX_STATUS] = "None Title";
char szWinStatusF[MAX_STATUS] = "Float icon";
char szWinStatusT[MAX_STATUS] = "Tray icon";
char szWinStatusH[MAX_STATUS] = "Hide window";
char szWinStatusO[MAX_STATUS] = "Opacity %d%%";

DWORD dwBtnWin[] = {IDC_BTN_SHOW, IDC_BTN_SHOWALL, 0, IDC_BTN_REF, 0, IDC_BTN_CLOSE};
char *DlgWinSection = "SettingsDialogWindowManager";
extern HFONT hLocalFont;
INT_PTR CALLBACK SettingDlgWinManagerProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		//Init ListView
		hListView_dm2 = GetDlgItem(hwndDlg, IDC_LIST_DM2WIN);
		ListView_SetExtendedListViewStyle(hListView_dm2, 
			LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT);
		{
			char szBuf[MAX_PATH];
			LVCOLUMN lc;
			memset(&lc, 0, sizeof(lc));
			lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lc.fmt = LVCFMT_LEFT;
			lc.cx = 205;
			LoadLanguageString(lang_file, DlgWinSection, 0, szBuf, MAX_PATH);
			lc.pszText = szBuf;
			ListView_InsertColumn(hListView_dm2, 0, &lc);
			lc.cx = 100;
			LoadLanguageString(lang_file, DlgWinSection, 1, szBuf, MAX_PATH);
			ListView_InsertColumn(hListView_dm2, 1, &lc);
		}
		hil = ImageList_Create(16, 16, ILC_COLOR16, 2, 4);
		ListView_SetImageList(hListView_dm2, hil, LVSIL_SMALL);

		{
			LoadLanguageString(lang_file, DlgWinSection, 100, szNoneTitle, MAX_STATUS);
			LoadLanguageString(lang_file, DlgWinSection, 101, szWinStatusF, MAX_STATUS);
			LoadLanguageString(lang_file, DlgWinSection, 102, szWinStatusT, MAX_STATUS);
			LoadLanguageString(lang_file, DlgWinSection, 103, szWinStatusH, MAX_STATUS);
			LoadLanguageString(lang_file, DlgWinSection, 104, szWinStatusO, MAX_STATUS);
		}
		CreateRMenuCtrl(hListView_dm2, hwndDlg, (LPDWORD)dwBtnWin, 
			sizeof(dwBtnWin)/sizeof(DWORD));
		//Get wndData and insert item
		RefreshWinList();

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgWinSection);
		return TRUE;
	case WM_NOTIFY:
		if(wParam == IDC_LIST_DM2WIN)
			if (((LPNMHDR)lParam)->code == NM_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDC_BTN_SHOW, 0);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BTN_SHOW:
			{
				LVITEM lv;
				PWND_DATA wnd;
				int i = ListView_GetNextItem(hListView_dm2, -1, LVNI_SELECTED);
				if(i >= 0)
				{
					memset(&lv, 0, sizeof(lv));
					lv.mask = LVIF_PARAM;
					lv.iItem = i;
					ListView_GetItem(hListView_dm2, &lv);
					wnd = (PWND_DATA)lv.lParam;

					//if window already restore and WND_DATA already free
					if(IsBadReadPtr(wnd, sizeof(WND_DATA)))
						break;

					RemoveWindowData(wnd);

					RefreshWinList();
				}
			}
			break;
		case IDC_BTN_SHOWALL:
			SendMessage(pshared->DM2wnd, WM_COMMAND, IDM_RESTORE_ALL, 0);
			RefreshWinList();
			break;
		case IDC_BTN_REF:
			RefreshWinList();
			break;
		case IDC_BTN_CLOSE:
			{
				LVITEM lv;
				PWND_DATA wnd;
				int i = ListView_GetNextItem(hListView_dm2, -1, LVNI_SELECTED);
				if(i >= 0)
				{
					memset(&lv, 0, sizeof(lv));
					lv.mask = LVIF_PARAM;
					lv.iItem = i;
					ListView_GetItem(hListView_dm2, &lv);
					wnd = (PWND_DATA)lv.lParam;

					//if window already restore and WND_DATA already free
					if(IsBadReadPtr(wnd, sizeof(WND_DATA)))
						break;
					
					if(!TryCloseWindow(wnd->hwnd))
						break;

					RemoveWindowData(wnd);

					RefreshWinList();
				}
			}
			break;
		}
		break;
	case WM_DESTROY:
		ImageList_Destroy(hil);
		hListView_dm2 = 0;
		break;
	default:
		break;
	}
	
	return FALSE;
}


void RemoveWindowData(PWND_DATA wnd)
{
	//Remove float window , tray icon, hide window, opacity window.
	if(wnd->flico)
		SendMessage(wnd->flico, WM_COMMAND, IDM_RESTORE_TRAYWND, 0);
	else if(wnd->settings)
	{
		SetWindowOpacity(wnd->hwnd, 255);
		RemoveWndData(wnd);
	}
	else if(wnd->icon)
		RestoreTrayWindow(pshared->DM2wnd, wnd->hwnd);
	else if(wnd->settings == NULL)
		RestoreWindow(wnd->hwnd);
}


BOOL TryCloseWindow(HWND hwnd)
{
	//if window close have prompt
	SetForegroundWindow(hwnd);
	SendMessage(hwnd, WM_CLOSE, 0, 0);
	
	//determines window handle
	if(IsWindow(hwnd))
	{
		//some window need hide again. for example: EmEditor
		if(IsWindowVisible(hwnd))
			ShowWindow(hwnd, SW_HIDE);
		return FALSE;
	}
	
	return TRUE;
}


void RefreshWinList(void)
{
	PWND_DATA last = wndData;
	LVITEM lv;
	char szTitle[MAX_WIN_TITLE];
	
	//Window Manager Create?
	if(hListView_dm2 == 0)
		return;

	memset(&lv, 0, sizeof(lv));
	ListView_DeleteAllItems(hListView_dm2);
	ImageList_RemoveAll(hil);
	while(last)
	{
		lv.iItem = ListView_GetItemCount(hListView_dm2);
		
		//Add First Item
		lv.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		GetWinTitle(last->hwnd, szTitle);
		lv.cchTextMax = lstrlen(szTitle);
		if(lv.cchTextMax == 0)
		{
			lv.cchTextMax = sizeof(szNoneTitle)/sizeof(char)-1;
			lv.pszText = szNoneTitle;
		}
		else
		{
			lv.pszText = szTitle;
		}
		lv.iSubItem = 0;
		lv.lParam = (LPARAM)last;
		lv.iImage = ImageList_AddIcon(hil, GetWindowIcon(last->hwnd, FALSE));
		ListView_InsertItem(hListView_dm2, &lv);
		
		//Add Sub Item
		lv.mask = LVIF_TEXT;
		if(last->flico)
		{
			lv.pszText = szWinStatusF;
			lv.cchTextMax = sizeof(szWinStatusF)/sizeof(char)-1;
		}
		else if(last->settings)
		{
			char szOpaStatus[20];
			lv.cchTextMax = wsprintf(szOpaStatus, szWinStatusO, last->settings->opacity);
			lv.pszText = szOpaStatus;
		}
		else if(last->icon)
		{
			lv.pszText = szWinStatusT;
			lv.cchTextMax = sizeof(szWinStatusT)/sizeof(char)-1;
		}
		else if(last->settings == NULL)
		{
			lv.pszText = szWinStatusH;
			lv.cchTextMax = sizeof(szWinStatusH)/sizeof(char)-1;
		}
		lv.iSubItem = 1;
		ListView_SetItem(hListView_dm2, &lv);
		
		//Get next
		last = last->next;
	}
}