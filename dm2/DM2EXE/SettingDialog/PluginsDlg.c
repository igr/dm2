/* http://dm2.sf.net */
#include <windows.h>
#include "../Plugins.h"
#include "SettingDlg.h"
#include "../util.h"


HWND hPlugins_list;
extern PFF_PLUGINS ffp;
void ListPlugins()
{
	char szModuleName[MAX_PATH];
	PFF_PLUGINS last = ffp;
	char *pName, *pMem;
	SendMessage(hPlugins_list, LB_RESETCONTENT, 0, 0);
	while(last)
	{
		if(last->Module)
		{
			GetModuleFileName(last->Module, szModuleName, MAX_PATH);
			pName = strrchr(szModuleName, '\\')+1;
			pMem = halloc(lstrlen(last->pName)+lstrlen(pName)+4);
			wsprintf(pMem, "%s [%s]", last->pName, pName);
			SendMessage(hPlugins_list, LB_SETITEMDATA, 
				SendMessage(hPlugins_list, LB_ADDSTRING, 0, 
				(LPARAM)pMem), (LPARAM)last);
			hfree(pMem);
		}
		last = last->Next;
	}
}

DWORD dwBtnPlugins[] = {IDC_BTN_CONFIG, IDC_BTN_ABOUT, 0, IDC_BTN_UNINST};
extern HFONT hLocalFont;
extern BOOL StaticToURLControl(HWND hDlg, UINT staticid, char *szURL, COLORREF crLink);
INT_PTR CALLBACK SettingDlgPluginsProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		hPlugins_list = GetDlgItem(hwndDlg, IDC_LIST_PLUGINS);
		ListPlugins();

		StaticToURLControl(hwndDlg, IDC_STATIC_GETPLUG, 
			"http://dm2.sourceforge.net", 0xFF0000);
		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, "SettingsDialogPlugins");
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_LIST_PLUGINS:
			if(HIWORD(wParam) != LBN_DBLCLK)
				break;
		case IDC_BTN_ABOUT:
			{
				int i = SendMessage(hPlugins_list, LB_GETCURSEL, 0, 0);
				PFF_PLUGINS cur_ffp;
				if(i == LB_ERR)
					break;
				cur_ffp = (PFF_PLUGINS)SendMessage(hPlugins_list, 
					LB_GETITEMDATA, i, 0);
				if(cur_ffp->About)
					cur_ffp->About(hwndDlg);
			}
			break;
		case IDC_BTN_CONFIG:
			{
				int i = SendMessage(hPlugins_list, LB_GETCURSEL, 0, 0);
				PFF_PLUGINS cur_ffp;
				if(i == LB_ERR)
					break;
				cur_ffp = (PFF_PLUGINS)SendMessage(hPlugins_list, 
					LB_GETITEMDATA, i, 0);
				if(cur_ffp->Config)
					cur_ffp->Config(hwndDlg, inifile);
				else
					LocalMsgBox(hwndDlg, lang_file, 0, 104, MB_OK|MB_ICONINFORMATION);
			}
			break;
		case IDC_BTN_UNINST:
			{
				int i = SendMessage(hPlugins_list, LB_GETCURSEL, 0, 0);
				if(i == LB_ERR)
					break;
				if(LocalMsgBox(hwndDlg, lang_file, 0, 121, MB_YESNO|MB_ICONQUESTION) == IDYES)

				{
					PFF_PLUGINS cur_ffp;
					cur_ffp = (PFF_PLUGINS)SendMessage(hPlugins_list, 
						LB_GETITEMDATA, i, 0);
					UninstallPlugins(cur_ffp);
					ListPlugins();
				}
			}
			break;
		}
		break;
	case WM_SETCURSOR:
		if((HWND)wParam == hPlugins_list && HIWORD(lParam) == WM_RBUTTONUP)
		{
			//only for listbox.
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(hPlugins_list, &point);
			SendMessage(hPlugins_list, LB_SETCURSEL, 
				SendMessage(hPlugins_list, LB_ITEMFROMPOINT, 0, 
				MAKELPARAM(point.x, point.y)), 0);
			
			//pop menu
			PopContextMenuFromButton(hwndDlg, (LPDWORD)&dwBtnPlugins, 
				sizeof(dwBtnPlugins)/sizeof(DWORD));
		}
		break;
	default:
		break;
	}
	
	return FALSE;
}
