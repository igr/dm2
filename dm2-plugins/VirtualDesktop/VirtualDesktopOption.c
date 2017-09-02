#include "resource.h"
#include "VirtualDesktop.h"

extern char *pSection;
extern char *pKeyEnable;
extern char *pKeyCount;
extern char *pKeyTray;
extern char *pKeyName;
extern char *pDefaultName;
extern char *Ini;
extern void LoadVirtualDesktop(LPVDD);
extern LPVDD *VDD_Array;
extern LPVDOPT VDOpt;
extern NOTIFYICONDATA nid;
extern char *pTip;
extern int CVD;
extern HINSTANCE hPlugin;

HWND hOpt;
INT_PTR CALLBACK VDCfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		hOpt = hwndDlg;
		CheckDlgButton(hwndDlg, IDC_CHECK_ENABLE, VDOpt->bEnable);
		SendMessage(hwndDlg, WM_COMMAND, IDC_CHECK_ENABLE, 0);

		CheckDlgButton(hwndDlg, IDC_CHECK_TRAY, VDOpt->bShowTray);

		{
			char szNum[2];
			int i=0;
			wsprintf(szNum, "%d", VDOpt->VDCount);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_COUNT, EM_LIMITTEXT, 1, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_COUNT, WM_SETTEXT, 0, (LPARAM)szNum);
			for(; i<8; i++)
			{
				SendDlgItemMessage(hwndDlg, IDC_EDIT_D1+i, EM_LIMITTEXT, MAX_VDNAME, 0);
				SendDlgItemMessage(hwndDlg, IDC_EDIT_D1+i, WM_SETTEXT, 0, (LPARAM)VDOpt->VDName[i]);
			}
		}
		return TRUE;
	case WM_DESTROY:
		hOpt = NULL;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				//Get all config
				char szNum[6];
				int i = 0;
				int iOrgCount = VDOpt->VDCount;
				BOOL bOrgTray = VDOpt->bShowTray;
				SendDlgItemMessage(hwndDlg, IDC_EDIT_COUNT, WM_GETTEXT, 2, (LPARAM)szNum);
				VDOpt->VDCount = setIntRange(atoi(szNum), 2, 8);
				VDOpt->bEnable = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_ENABLE) == BST_CHECKED);
				VDOpt->bShowTray = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_TRAY) == BST_CHECKED);
				for(; i<8; i++)
				{
					VDOpt->VDName[i][0] = '\0';
					SendDlgItemMessage(hwndDlg, IDC_EDIT_D1+i, WM_GETTEXT, MAX_VDNAME2, 
					(LPARAM)VDOpt->VDName[i]);
					if(VDOpt->VDName[i][0] == '\0')
						wsprintf(VDOpt->VDName[i], pDefaultName, i+1);
				}

				//write config
				wsprintf(szNum, "%d", VDOpt->VDCount);
				WritePrivateProfileString(pSection, pKeyCount, szNum, Ini);
				wsprintf(szNum, "%d", VDOpt->bEnable);
				WritePrivateProfileString(pSection, pKeyEnable, szNum, Ini);
				wsprintf(szNum, "%d", VDOpt->bShowTray);
				WritePrivateProfileString(pSection, pKeyTray, szNum, Ini);
				for(i=0; i<8; i++)
				{
					wsprintf(szNum, pKeyName, i+1);
					WritePrivateProfileString(pSection, szNum, VDOpt->VDName[i], Ini);
				}

				//And if VDOpt->bEnable == false, need free all virtual desktop.
				if(VDOpt->bEnable)
				{
					InitVirtualDesktop();
					
					//Show/Hide Tray icon
					if(VDOpt->bShowTray)
					{
						if(VDOpt->bShowTray)
						{
							nid.hIcon = LoadIcon(hPlugin, (LPCTSTR)CVD+IDI_VD1);
							wsprintf(nid.szTip, pTip, VDOpt->VDName[CVD]);
							Shell_NotifyIcon(NIM_ADD, &nid);
						}
						else
							Shell_NotifyIcon(NIM_DELETE, &nid);
					}
				}
				else
				{
					DestroyVirtualDesktop();
					Shell_NotifyIcon(NIM_DELETE, &nid);
				}
				
				//if Set count < org count, need free some org count
				if(VDOpt->bEnable && iOrgCount > VDOpt->VDCount)
				{
					for(; iOrgCount>VDOpt->VDCount; iOrgCount--)
						LoadVirtualDesktop(VDD_Array[iOrgCount-1]);
				}
			}
			EndDialog(hwndDlg, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		case IDC_CHECK_ENABLE:
			{
				int i = 0;
				BOOL bEnable = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_ENABLE) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_COUNT), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_TRAY), bEnable);
				for(; i<8; i++)
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_D1+i), bEnable);
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
