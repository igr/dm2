/* http://dm2.sf.net */
#include <windows.h>
#include <commctrl.h>
#include "..\global.h"
#include "resource.h"

typedef struct {
	HWND hToolBar;
	HWND hEdit;
	HWND hCombo;
} FILEDIALOG, *PFILEDIALOG;

BOOL DetectFileDialog(HWND, PFILEDIALOG);
HWND FindWindowExID(HWND, HWND, char*, int);
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void Dm2_Fav_Command(HWND, char *, int, BOOL);
void CreateFavMenu(HWND);
void FreeTempMemory();

extern HHOOK Dlghook;
extern HINSTANCE hdll;

char dialogClassName[] = "#32770";
FILEDIALOG FileDialog = {0};

/* Hook procedure */
WNDPROC oldProc;
BOOL bClickCancel;		//for recent menu, detect the file dialog is open or cancel.
HWND dlg;
int ndx;
char *szDM2 = "DM2";
LRESULT CALLBACK dlgHookProc(int code, WPARAM wParam, LPARAM lParam) {
	//HWND dlg;
	PCWPRETSTRUCT pcwp;
	char dlgClassName[CLASSNAME_SIZE];
	
	if (code == HC_ACTION) {
		pcwp = (PCWPRETSTRUCT) lParam;
		if (pcwp->message == WM_INITDIALOG) {
			dlg = pcwp->hwnd;
			if (GetClassName(dlg, dlgClassName, CLASSNAME_SIZE) != 0) {
				if (lstrcmp(dlgClassName, dialogClassName) == 0) {				// dialog (#32770) found
					SendMessage(dlg, IDM_FAV_INSTALL, 0, 0);
				}
			}
		}
		else if(pcwp->message == IDM_FAV_UNINSTALL)
		{
			SetWindowLong(pcwp->hwnd, GWL_WNDPROC, (LONG) oldProc);
			SendMessage(FileDialog.hToolBar, TB_DELETEBUTTON, ndx + 1, 0);
			SendMessage(FileDialog.hToolBar, TB_DELETEBUTTON, ndx, 0);
		}
		else if(pcwp->message == IDM_FAV_INSTALL)
		{
			dlg = pcwp->hwnd;
			if(DetectFileDialog(dlg, &FileDialog) && SendMessage(pshared->DM2wnd, 
				IDM_IS_FAV_SKIP, (WPARAM)dlg, 0) == FALSE)
			{
				RECT rect;
				TBBUTTON separator = {0, 0, TBSTATE_ENABLED, 
					TBSTYLE_SEP, 0, 0};
				TBBUTTON newButton = {14, TB_FAVBUTTON_ID, 
					TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0};
				HIMAGELIST hi;

				newButton.iString = SendMessage(FileDialog.hToolBar, TB_ADDSTRING, 
					0, (LPARAM)szDM2);

				hi = (HIMAGELIST) SendMessage(FileDialog.hToolBar, TB_GETIMAGELIST, 0, 0);
				ndx = ImageList_AddIcon(hi, 
					LoadIcon(hdll, (LPCTSTR)IDI_ICON_FAV));
				newButton.iBitmap = ndx;
				SendMessage(FileDialog.hToolBar, TB_SETIMAGELIST, 0, (LPARAM) hi);

				GetWindowRect(FileDialog.hToolBar, &rect);
				SetWindowPos(FileDialog.hToolBar, 0, 0, 0, rect.right - rect.left + 16, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOOWNERZORDER);

				ndx = SendMessage(FileDialog.hToolBar, TB_BUTTONCOUNT, 0, (LPARAM) NULL);
				SendMessage(FileDialog.hToolBar, TB_INSERTBUTTON, ndx, (LPARAM) &separator);
				SendMessage(FileDialog.hToolBar, TB_INSERTBUTTON, ndx + 1, (LPARAM) &newButton);

				// subclassing
				oldProc = (WNDPROC) GetWindowLong(dlg, DWL_DLGPROC);
				SetWindowLong(dlg, DWL_DLGPROC, (LONG) &WindowProc);
				bClickCancel = FALSE;	//for recent menu, when init to cancel unclick.
			}
		}
	}
	return CallNextHookEx(Dlghook, code, wParam, lParam);
}


LPNMTTDISPINFO ttdi;
MENUITEMINFO menuSeparator = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_SEPARATOR, 0};
//#define MIIM_STRING      0x00000040
MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_ID | 0x00000040, 
						MFT_STRING, MFS_ENABLED, 173, 0};
RECT buttonRect, barRect;
static char *pDef = NULL;
static char *pApp = NULL;
static char *pRec = NULL;
static char *pItem = NULL;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND:
			if (LOWORD(wParam) == TB_FAVBUTTON_ID) {
				FreeTempMemory();
				GetWindowRect(FileDialog.hToolBar, &barRect);
				SendMessage(FileDialog.hToolBar, TB_GETRECT, TB_FAVBUTTON_ID, 
					(LPARAM) &buttonRect);
				//Get New Fav Menu data.
				SendMessage(pshared->DM2wnd, IDM_NEED_FAV, (WPARAM)hwnd, 0);
				SendMessage(pshared->DM2wnd, IDM_GET_RECENT, (WPARAM)hwnd, 0);
				CreateFavMenu(hwnd);
				break;
			}
			else if(LOWORD(wParam) == IDCANCEL)
			{
				bClickCancel = TRUE;
			}
			else if (HIWORD(wParam) == 0)
			{
				int iCmd = LOWORD(wParam);
				// show DM2 Settings Dialog
				if(iCmd == 'dm')
				{
					SetForegroundWindow(pshared->DM2wnd);
					//#define IDM_DM2				100
					PostMessage(pshared->DM2wnd, WM_COMMAND, 100, 0);
					break;
				}
				else if(iCmd >= IDM_RECENT_START && iCmd < IDM_RECENT_END)
				{
					Dm2_Fav_Command(hwnd, pRec, iCmd - IDM_RECENT_START, FALSE);
				}
				else if(iCmd >= IDM_TBB_DEF && iCmd < IDM_TBB_APP)
				{
					Dm2_Fav_Command(hwnd, pDef, iCmd - IDM_TBB_DEF, FALSE);
				}
				else if(iCmd >= IDM_TBB_APP && iCmd < IDM_TBB_CST)
				{
					Dm2_Fav_Command(hwnd, pApp, iCmd - IDM_TBB_APP, TRUE);
				}
				else if(iCmd >= IDM_TBB_CST)
				{
					if(pItem)
						Dm2_Fav_Command(hwnd, pItem, iCmd - IDM_TBB_CST, FALSE);
				}
			}
			break;
		case WM_COPYDATA:
			{
				PCOPYDATASTRUCT pcs = (PCOPYDATASTRUCT)lParam;
				if(pcs->dwData == 'dm')
				{
					char *pMem;
					pDef = pcs->lpData;
					pApp = strchr(pcs->lpData, '\r');

					*pApp = '\0';
					pApp++;
					pItem = strchr(pApp, '\r');
					if(pItem)
					{
						*pItem = '\0';
						pItem++;
					}
					
					//malloc temp
					pMem = malloc(lstrlen(pDef)+1);
					lstrcpy(pMem, pDef);
					pDef = pMem;
					pMem = malloc(lstrlen(pApp)+1);
					lstrcpy(pMem, pApp);
					pApp = pMem;
					if(pItem)
					{
						pMem = malloc(lstrlen(pItem)+1);
						lstrcpy(pMem, pItem);
						pItem = pMem;
					}
				}
				else if(pcs->dwData == 'dm'+1)	//get recent data
				{
					pRec = malloc(pcs->cbData);
					lstrcpyn(pRec, (char *)pcs->lpData, pcs->cbData);
				}
			}
			break;
		case WM_DESTROY:
			if(bClickCancel ==FALSE)
			{
				GetCurrentDirectory(MAX_PATH, pshared->szRecent);
				SendMessage(pshared->DM2wnd, IDM_ADD_RECENT, 0, 0);
			}
			
			FreeTempMemory();
			break;
	}
	return CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
}

//if need uninstall, lParam equal to FALSE.
BOOL CALLBACK FavMenuCallback(HWND hwnd, LPARAM lParam)
{
	char dlgClassName[CLASSNAME_SIZE];
	if (GetClassName(hwnd, dlgClassName, CLASSNAME_SIZE) != 0)
	{
		if ((lstrcmp(dlgClassName, dialogClassName) == 0) && 
			DetectFileDialog(hwnd, &FileDialog))
		{
			if(lParam)
				SendMessage(hwnd, IDM_FAV_INSTALL, 0, 0);
			else
				SendMessage(hwnd, IDM_FAV_UNINSTALL, 0, 0);
		}
	}
	
	return TRUE;
}

char toolbarClassName[] = "ToolbarWindow32";
char comboexClassName[] = "ComboBoxEx32";
char buttonClassName[] = "Button";
char editClassName[] = "Edit";

/* Detect file fialog. If detected, return toolbar handle */
BOOL DetectFileDialog(HWND dlg, PFILEDIALOG pfd) {
	HWND toolbar = NULL, edit = NULL, combo = NULL, button = NULL;
	toolbar = FindWindowExID(dlg, NULL, toolbarClassName, 0x440);		// windows XP
	if (toolbar == NULL) {
		toolbar = FindWindowExID(dlg, NULL, toolbarClassName, 0x001);	// windows 2k
	}
	edit = FindWindowExID(dlg, NULL, editClassName, 0x480);			// edit field
	combo = FindWindowExID(dlg, NULL, comboexClassName, 0x47C);		// comboboxex field
	button = FindWindowExID(dlg, NULL, buttonClassName, 0x001);		// second button
	if (toolbar && (combo || edit) && button) {
		pfd->hCombo = combo;
		pfd->hEdit = edit;
		pfd->hToolBar = toolbar;
		return TRUE;
	}
	return FALSE;
}


/** this function get from wlib
 * FindWindowEx with last parameter changed to ID of a control.
 * May be used for finding a specific control on a dialog.
 * If ID is 0, first window is found.
 *
 * @param dlg       HWND of a dialog
 * @param startFrom window handle to start from
 * @param className name of the window class
 * @param ctrlId    dialog ID of the control
 *
 * @return Not NULL if found.
 */
HWND FindWindowExID(HWND dlg, HWND startFrom, char* className, int ctrlId) {
	HWND ctrl = startFrom;
	while (1) {
		ctrl = FindWindowEx(dlg, ctrl, className, NULL);
		if (ctrlId == 0) {
			return ctrl;
		}
		if (ctrl != NULL) {
			int id = GetDlgCtrlID(ctrl);
			if (id == ctrlId) {
				return ctrl;
			}
		} else {
			return NULL;
		}
	}
}


int CreateMenuFromString(HMENU hMenu, char *szMenuItem, int Cmd_Start)
{
	char *pNextName;
	char *pTemp, *pTemp1;
	HMENU hSub;
	int iMenu = GetMenuItemCount(hMenu);

	pNextName = szMenuItem;
	while(pNextName)
	{
		pTemp = strchr(pNextName, '=');
		if(pTemp == NULL)
			break;
		pTemp++;
		pTemp1 = strchr(pTemp, '|');
		if(pTemp1)
		{
			pTemp1++;
		}

		if(*pNextName == '-')
		{
			char *pMenu;
			int i = 0;
			char *pMem;
			if(*(pNextName+1) == '=')
			{
				pNextName = pTemp1;
				continue;
			}
			else if(*(pNextName+1) == '-')
			{
				*(pTemp-1) = '\0';
				if(pTemp1)
					*(pTemp1-1) = '\0';
				InsertMenuItem(hMenu, iMenu, TRUE, &menuSeparator);
				pNextName = pTemp1;
				iMenu++;
				continue;
			}

			pNextName++;
			//Get String range
			pMenu = pNextName;
			i++;
			while(i)
			{
				// v1.18 changed
				pMenu = strchr(pMenu, '-');
				if(pMenu++ == NULL) return Cmd_Start;
				if(*pMenu == '=')
					i--;
				else if(*(pMenu-2) != '-' && *(pMenu-2) != '|')
					continue;
				else
					i++;
			}
			pMenu -= 2;
			i = pMenu-pTemp1+1;
			pMem =malloc(i+1);
			lstrcpyn(pMem, pTemp1, i);
			
			hSub = CreatePopupMenu();
			menuItem.fMask |= MIIM_SUBMENU;
			menuItem.hSubMenu = hSub;
			menuItem.dwTypeData = pNextName;
			*(pTemp-1) = '\0';
			menuItem.cch = pTemp-pNextName;
			InsertMenuItem(hMenu, iMenu, TRUE, &menuItem);
			//menuItem.fMask &= ~MIIM_SUBMENU;
			Cmd_Start = CreateMenuFromString(hSub, pMem, Cmd_Start);
			free(pMem);
			pTemp = pMenu+1;
		}
		else
		{
			*(pTemp-1) = '\0';
			if(pTemp1)
				*(pTemp1-1) = '\0';
			menuItem.fMask &= ~MIIM_SUBMENU;
			menuItem.wID = Cmd_Start;
			menuItem.cch = lstrlen(pNextName);
			menuItem.dwTypeData = pNextName;
			InsertMenuItem(hMenu, iMenu, TRUE, &menuItem);
			Cmd_Start++;
			pTemp = pTemp1;
		}
		iMenu++;

		pNextName = pTemp;
	}
	return Cmd_Start;
}


void CreateFavMenu(HWND hwnd)
{
	HMENU hPop = CreatePopupMenu();
	char *pTemp;
	char *pMem;
	HMENU sub;

	sub = CreatePopupMenu();
	pMem = malloc(lstrlen(pDef)+1);
	lstrcpy(pMem, pDef);
	pTemp = strchr(pMem, '=');
	*pTemp = '\0';
	pTemp++;
	CreateMenuFromString(hPop, pTemp, IDM_TBB_DEF);
	free(pMem);

	sub = CreatePopupMenu();
	pMem = malloc(lstrlen(pApp)+1);
	lstrcpy(pMem, pApp);
	pTemp = strchr(pMem, '=');
	*pTemp = '\0';
	pTemp++;
	menuItem.fMask |= MIIM_SUBMENU;
	menuItem.dwTypeData = pMem;
	menuItem.cch = lstrlen(pMem);
	menuItem.hSubMenu = sub;
	InsertMenuItem(hPop, GetMenuItemCount(hPop), TRUE, &menuItem);
	CreateMenuFromString(sub, pTemp, IDM_TBB_APP);
	free(pMem);

	if(pItem)
	{
		InsertMenuItem(hPop, GetMenuItemCount(hPop), TRUE, &menuSeparator);
		pMem = malloc(lstrlen(pItem)+1);
		lstrcpy(pMem, pItem);
		pTemp = strchr(pMem, '=');
		*pTemp = '\0';
		pTemp++;
		CreateMenuFromString(hPop, pTemp, IDM_TBB_CST);
		free(pMem);
	}
	
	InsertMenuItem(hPop, GetMenuItemCount(hPop), TRUE, &menuSeparator);

	//add recent menu
	pMem = malloc(lstrlen(pRec)+1);
	lstrcpy(pMem, pRec);
	pTemp = strchr(pMem, '=');
	*pTemp = '\0';
	pTemp++;
	if(*pTemp)
	{
		sub = CreatePopupMenu();
		menuItem.fMask |= MIIM_SUBMENU;
		menuItem.dwTypeData = "&Recent";
		menuItem.cch = 6;
		menuItem.hSubMenu = sub;
		InsertMenuItem(hPop, GetMenuItemCount(hPop), TRUE, &menuItem);
		CreateMenuFromString(sub, pTemp, IDM_RECENT_START);
		InsertMenuItem(hPop, GetMenuItemCount(hPop), TRUE, &menuSeparator);
	}
	free(pMem);

	menuItem.fMask &= ~MIIM_SUBMENU;
	menuItem.dwTypeData = szDM2;
	menuItem.cch = lstrlen(szDM2);
	menuItem.wID = 'dm';
	InsertMenuItem(hPop, GetMenuItemCount(hPop), TRUE, &menuItem);
	
	TrackPopupMenuEx(hPop, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | 
		TPM_RIGHTBUTTON, barRect.left + buttonRect.left, barRect.bottom, hwnd, NULL);
	DestroyMenu(hPop);
}


void SimpleTranslateEnvironment(char *Buf)
{
	if(*Buf == '%' && *(Buf+1) != '%')
	{
		char szPath[MAX_PATH];
		char *pTmp = strchr(Buf+1, '%');
		char *pEnvName = malloc(pTmp-Buf+2);
		if(pTmp == NULL)
			return;
		*pTmp = '\0';
		lstrcpy(szPath, pTmp+1);
		lstrcpy(pEnvName, Buf+1);
		GetEnvironmentVariable(pEnvName, Buf, MAX_PATH);
		lstrcat(Buf, szPath);
		free(pEnvName);
	}
}

char szNoDir[] = "Directory: \"%s\" not exist,\nDo you want to create it?";
void Dm2_Fav_Command(HWND hwnd, char *Cmd, int num, BOOL bExec)
{
	int i = -1;
	char *pMem;
	char *pTemp, *pTemp1;
	
	//detect rule
	pTemp = strchr(Cmd, '=');
	if(pTemp == NULL && *(pTemp+1) == '\0')
		return;

	pMem = malloc(lstrlen(pTemp+1)+MAX_PATH2);
	lstrcpy(pMem, pTemp+1);
	pTemp = pMem;
	
	while(TRUE)
	{
		pTemp = strchr(pTemp, '=');
		if(pTemp == NULL)
			break;
		if(*(pTemp+1) == '|')
			pTemp += 2;
		else if(*(pTemp+1) == '\0')
			break;
		else
		{
			pTemp++;
			i++;
		}
		if(i == num)
		{
			pTemp1 = strchr(pTemp, '|');
			if(pTemp1)
				*pTemp1 = '\0';
			
			SimpleTranslateEnvironment(pTemp);
			if(bExec)
			{
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				memset(&si, 0, sizeof(STARTUPINFO));
				si.cb = sizeof(STARTUPINFO);
				memset(&pi, 0, sizeof(PROCESS_INFORMATION));
				if(CreateProcess(NULL, pTemp, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
					NULL, NULL, &si, &pi) == 0)
				{
					MessageBox(hwnd, "Can't run you select application! Please check path.", 
						"Error", MB_OK|MB_ICONINFORMATION);
				}
			}
			else
			{
				char szBuf[MAX_PATH];
				HWND hEdit;
				if(FileDialog.hEdit)
					hEdit = FileDialog.hEdit;
				else
					hEdit = FileDialog.hCombo;
				if(hEdit)
				{
					if(CreateDirectory(pTemp, NULL))
					{
						char *pStr;
						int result;
						RemoveDirectory(pTemp);
						pStr = malloc(lstrlen(pTemp) + sizeof(szNoDir)/sizeof(char)+1);
						wsprintf(pStr, szNoDir, pTemp);
						result = MessageBox(hwnd, pStr, "Note", 
							MB_YESNO|MB_ICONINFORMATION);
						free(pStr);
						if(result == IDYES)
							CreateDirectory(pTemp, NULL);
						else
							break;
					}
					GetWindowText(hEdit, szBuf, MAX_PATH);
					SetWindowText(hEdit, pTemp);
					SetFocus(hEdit);
					SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED),0);
					SetWindowText(hEdit, szBuf);
				}
			}
			break;
		}
	}
	free(pMem);
}

void FreeTempMemory()
{
	if(pDef)
	{
		free(pDef);
		pDef = NULL;
	}
	if(pApp)
	{
		free(pApp);
		pApp = NULL;
	}
	if(pRec)
	{
		free(pRec);
		pRec = NULL;
	}
	if(pItem)
	{
		free(pItem);
		pItem = NULL;
	}
}