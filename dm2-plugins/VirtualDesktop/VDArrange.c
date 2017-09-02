#include "VirtualDesktop.h"
#include "resource.h"

extern LPVDOPT VDOpt;
extern int CVD;

int iNowCount = 0;
HWND *phWnd;
BOOL CALLBACK EnumNowWindowsProc(HWND hwnd, LPARAM lParam)
{
	if(GetWindowLong(hwnd,GWL_STYLE) & WS_VISIBLE)
	{
		char szName[MAX_PATH];
		LPVDD pVdd = (LPVDD)lParam;
		DWORD pid;
		GetClassName(hwnd, szName, MAX_PATH);
		if(lstrcmp(szName, "Shell_TrayWnd") == 0) //if not the taskbar
			return TRUE;
		GetWindowText(hwnd, szName, MAX_PATH);
		if(lstrcmp(szName, "Program Manager") == 0) //if not the desktop
			return TRUE;
		GetWindowThreadProcessId(hwnd, &pid);
		if(GetCurrentProcessId() == pid)	//skip all window about dm2
			return TRUE;

		phWnd[iNowCount++] = hwnd;
	}
	
	return TRUE;
}

HWND hTab, hList, hArrange = NULL;
void ListNowWindow()
{
	int i = 0;
	char szTitle[MAX_PATH];
	phWnd = malloc(sizeof(HWND) * MAXWIN);
	EnumWindows(EnumNowWindowsProc, 0);
	for(; i<iNowCount; i++)
	{
		GetWindowText(phWnd[i], szTitle, MAX_PATH);
		if(*szTitle == '\0')
		{
			char szClassName[MAX_PATH-60];
			GetClassName(phWnd[i], szClassName, MAX_PATH-60);
			wsprintf(szTitle, "NoneTitle Window [WindowClass: %s]", szClassName);
		}
		SendMessage(hList, LB_SETITEMDATA, 
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szTitle), (LPARAM)phWnd[i]);
	}
	iNowCount = 0;
	free(phWnd);
}

void ClearList()
{
	int i = SendMessage(hList, LB_GETCOUNT, 0, 0);
	for(; i>0; i--)
		SendMessage(hList, LB_DELETESTRING, 0, 0);
}

HWND GetCurrenthWnd(BOOL bDelIt)
{
	HWND hwnd = NULL;
	int i = -1;
	for(; i<SendMessage(hList, LB_GETCOUNT, 0, 0); i++)
		if(SendMessage(hList, LB_GETSEL, i, 0) > 0)
			break;

	if(i > -1)
	{
		hwnd = (HWND)SendMessage(hList, LB_GETITEMDATA, i, 0);
		if(bDelIt)
			SendMessage(hList, LB_DELETESTRING, i, 0);
	}
	return hwnd;
}

extern LPVDD *VDD_Array;
extern HINSTANCE hPlugin;
extern void AddWindowtoVD(int VD, HWND hWnd);
extern void RemoveWindowFromVD(int VD, HWND hWnd);
extern void SwitchVD();
extern MENUITEMINFO VDMenu;
extern char *szVDMenuItem;
int iCurrentTab;
INT_PTR CALLBACK VDArrangeDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		hArrange = hwndDlg;
		hTab = GetDlgItem(hwndDlg, IDC_TAB_VD);
		hList = GetDlgItem(hwndDlg, IDC_LIST_VDITEM);
		{
			int i=0;
			TCITEM tc;
			memset(&tc, 0, sizeof(TCITEM));
			tc.mask = TCIF_TEXT;
			for(; i<VDOpt->VDCount; i++)
			{
				tc.pszText = VDOpt->VDName[i];
				TabCtrl_InsertItem(hTab, i, &tc);
			}
			
			TabCtrl_SetCurSel(hTab, 0);
			ClearList();
			ListNowWindow();
			iCurrentTab = 0;
		}

		SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		return TRUE;
	case WM_DESTROY:
		hArrange = NULL;
		break;
	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code == TCN_SELCHANGE)
		{
			int i = TabCtrl_GetCurSel(hTab);
			iCurrentTab = i;
			ClearList();
			if(i == CVD)
				ListNowWindow();
			else
			{
				int n =0;
				char szTitle[MAX_PATH];
				for(; n<VDD_Array[i]->WinCount; n++)
				{
					GetWindowText(VDD_Array[i]->List[n], szTitle, MAX_PATH);
					if(*szTitle == '\0')
					{
						char szClassName[MAX_PATH-60];
						GetClassName(phWnd[i], szClassName, MAX_PATH-60);
						wsprintf(szTitle, "NoneTitle Window [WindowClass: %s]", szClassName);
					}
					SendMessage(hList, LB_SETITEMDATA, 
						SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szTitle), 
						(LPARAM)VDD_Array[i]->List[n]);
				}
			}
		}
		break;
	case WM_SETCURSOR:
		if((HWND)wParam == hList && HIWORD(lParam) == WM_RBUTTONUP)
		{
			POINT point;
			HMENU hPop, hSub;
			char szMIBuf[MAX_PATH];
			int i = 0;
			GetCursorPos(&point);
			ScreenToClient(hList, &point);
			SendMessage(hList, LB_SETCURSEL, 
				SendMessage(hList, LB_ITEMFROMPOINT, 0, 
				MAKELPARAM(point.x, point.y)), 0);
			
			hPop = LoadMenu(hPlugin, (LPCTSTR)IDR_MENU_ARRANGE);
			if (hPop == NULL) return FALSE;
			hPop = GetSubMenu(hPop, 0);
			hSub = GetSubMenu(hPop, 0);
			DeleteMenu(hSub, 2, MF_BYPOSITION);
			for(; i<VDOpt->VDCount; i++)
			{
				VDMenu.wID = ID_ST_VD1+i;
				wsprintf(szMIBuf, szVDMenuItem, i+1, VDOpt->VDName[i]);
				
				VDMenu.dwTypeData = szMIBuf;
				VDMenu.cch = lstrlen(szMIBuf);
				InsertMenuItem(hSub, 2+i, TRUE, &VDMenu);
			}
			GetCursorPos(&point);
			TrackPopupMenuEx(hPop, TPM_LEFTALIGN | TPM_TOPALIGN | 
				TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, hwndDlg, NULL);
			DestroyMenu(hPop);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_ST_ALLVD:
			{
				int i = 0;
				for(; i<VDOpt->VDCount; i++)
				{
					if(i != CVD)
						AddWindowtoVD(i, GetCurrenthWnd(FALSE));
				}
			}
			break;
		case ID_SETACTIVE:
			if(iCurrentTab < VDOpt->VDCount)
			{
				CVD = iCurrentTab;
				SwitchVD();
			}
			break;
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		default:
			if(LOWORD(wParam) >= ID_ST_VD1 && LOWORD(wParam) <= ID_ST_VD8)
			{
				int i = LOWORD(wParam) - ID_ST_VD1;
				if(i != iCurrentTab)
				{
					HWND hWnd = GetCurrenthWnd(TRUE);
					if(i == CVD)
					{
						//need show window
						ShowWindow(hWnd, SW_SHOW);
						RemoveWindowFromVD(iCurrentTab, hWnd);
					}
					else if(iCurrentTab == CVD)
					{
						//need hide window
						ShowWindow(hWnd, SW_HIDE);
						AddWindowtoVD(i, hWnd);
					}
					else
					{
						//none change
						AddWindowtoVD(i, hWnd);
						RemoveWindowFromVD(iCurrentTab, hWnd);
					}
				}
			}
			break;
		}
		break;
	default:
		break;
	}
	return FALSE;
}
