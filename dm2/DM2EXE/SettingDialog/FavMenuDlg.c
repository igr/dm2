/* http://dm2.sf.net */
#include <Windows.h>
#include <Windowsx.h>
#include "../GetSetting.h"
#include "../util.h"
#include "../../global.h"
#include "SettingDlg.h"

void Tree2String(HTREEITEM hParent, char *pStr);
void DeleteTreeItem(HTREEITEM hItem);

#define SLOWSCROLL_FREQUENCY 3
enum SCROLLMODE {SCROLL_UP_SLOW, SCROLL_DOWN_SLOW, SCROLL_UP_NORMAL, SCROLL_DOWN_NORMAL};
BOOL begin_drag(LPNMTREEVIEW p_nmtree);
BOOL IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent);
BOOL TransferItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop);
BOOL g_fdragging = FALSE;
int iUpperYCoor;
int iLowerYCoor;
int ScrollBarSize;
int SlowScrollTimeout;
HTREEITEM hItemDrag, hItemDrop;
HCURSOR hTreeCursor;
extern HIMAGELIST hImgTree;

extern PSFAV_DATA Fav_data;
extern PSFAV_DATA Fav_data_temp;
HANDLE hListBox_menu;
HANDLE hTreeView;
PSFAV_DATA pLast;
int LastSelectSec = 0;

DWORD dwBtnFav[] = {IDC_BTN_EXADD, IDC_BTN_EXEDIT, IDC_BTN_EXDEL};
DWORD dwBtnMenu[] = {IDC_BTN_EXMADD, IDC_BTN_EXMEDIT, IDC_BTN_EXMDEL};

char szFavApp[] = "Application";
char szFavDef[] = "Default";
extern BOOL bEnableFavMenu_temp;
char *DlgFavSection = "SettingsDialogFav";
extern HFONT hLocalFont;
INT_PTR CALLBACK SettingDlgFavMenuProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			PSFAV_DATA last = Fav_data_temp;
			hListBox_menu = GetDlgItem(hwndDlg, IDC_LIST_MENU);
			hTreeView = GetDlgItem(hwndDlg, IDC_TREE_MENUITEM);

			SendMessage(hListBox_menu, LB_ADDSTRING, 0, (LPARAM)szFavDef);
			SendMessage(hListBox_menu, LB_ADDSTRING, 0, (LPARAM)szFavApp);
			while(last)
			{
				if(lstrcmpi(last->szName, szFavDef) != 0 &&
					lstrcmpi(last->szName, szFavApp) != 0)
					SendMessage(hListBox_menu, LB_ADDSTRING, 0, (LPARAM)last->szName);
				last = last->next;
			}

			CheckDlgButton(hwndDlg, IDC_CHK_EF, bEnableFavMenu_temp);
			SendMessage(hwndDlg, WM_COMMAND, IDC_CHK_EF, 0);

			SendMessage(hListBox_menu, LB_SETCURSEL, LastSelectSec, 0);
			ListMenuItemFromSelect(LastSelectSec);
		}

		//Drag Drop Tree
		TreeView_SetImageList(hTreeView, hImgTree, TVSIL_NORMAL);
		ScrollBarSize = GetSystemMetrics(SM_CYHSCROLL);
		hTreeCursor = LoadCursor(NULL, IDC_NO);

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, DlgFavSection);
		return TRUE;
	case WM_MOUSEMOVE:
		{
		   POINT pnt;
		   HTREEITEM h_item = NULL;

		   pnt.x = GET_X_LPARAM(lParam);
		   pnt.y = GET_Y_LPARAM(lParam);
		   if(g_fdragging)
		   {
				TVHITTESTINFO tv_ht;
				ImageList_DragLeave(NULL);
				ClientToScreen(hwndDlg, &pnt);
				ZeroMemory(&tv_ht, sizeof(TVHITTESTINFO));
				tv_ht.flags = TVHT_ONITEM;
				tv_ht.pt.x = pnt.x;
				tv_ht.pt.y = pnt.y;
				ScreenToClient(hTreeView, &(tv_ht.pt));
				h_item = (HTREEITEM)SendMessage(hTreeView, TVM_HITTEST, 0, (LPARAM)&tv_ht);
				if(h_item)
					SendMessage(hTreeView, TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)h_item);

				{
					TVHITTESTINFO tvi;
					TVITEM tvDrop;
					HTREEITEM hitem;
					memset(&tvi, 0, sizeof(TVHITTESTINFO));
					GetCursorPos(&tvi.pt);
					ScreenToClient(hTreeView, &tvi.pt);
					if ((hitem = TreeView_HitTest(hTreeView, &tvi)) != NULL)
					{
						TreeView_SelectDropTarget(hTreeView, hitem);
						hItemDrop = hitem;
						
						tvDrop.mask = TVIF_CHILDREN | TVIF_HANDLE;
						tvDrop.hItem = hitem;
						TreeView_GetItem(hTreeView, &tvDrop);
						if((tvDrop.cChildren == 0 || IsChildNodeOf(hItemDrop, hItemDrag))
							&& hItemDrop != hItemDrag)
							SetCursor(hTreeCursor);
						else
							SetCursor(NULL);
					}
				}
				ImageList_DragMove(pnt.x,pnt.y);
				if(pnt.y < iUpperYCoor || pnt.y > iLowerYCoor)
				{
					HTREEITEM hitem;
					int nScrollMode = pnt.y > iLowerYCoor + ScrollBarSize ? SCROLL_DOWN_NORMAL :
									pnt.y > iLowerYCoor ? SCROLL_DOWN_SLOW :
									pnt.y < iUpperYCoor - ScrollBarSize ? SCROLL_UP_NORMAL :
									SCROLL_UP_SLOW;
					switch(nScrollMode) 
					{
						case SCROLL_UP_SLOW:
						case SCROLL_DOWN_SLOW:
							if( SlowScrollTimeout == 0)
								SendMessage(hTreeView, WM_VSCROLL, 
									nScrollMode == SCROLL_UP_SLOW ? SB_LINEUP : SB_LINEDOWN, 0);
							SlowScrollTimeout = ++SlowScrollTimeout%SLOWSCROLL_FREQUENCY;
							break;
						case SCROLL_UP_NORMAL:
						case SCROLL_DOWN_NORMAL:
							SendMessage(hTreeView, WM_VSCROLL, 
								nScrollMode == SCROLL_UP_NORMAL ? SB_LINEUP : SB_LINEDOWN, 0);
							break;
					}
					ImageList_DragLeave(hTreeView);
					hitem = TreeView_GetFirstVisible(hTreeView);
					switch(nScrollMode)
					{
						case SCROLL_UP_SLOW:
						case SCROLL_UP_NORMAL:
								TreeView_SelectDropTarget(hTreeView, hitem);
								hItemDrop = hitem;
								break;
						case SCROLL_DOWN_SLOW:
						case SCROLL_DOWN_NORMAL:
							{
								int i=0;
								int nCount = TreeView_GetVisibleCount(hTreeView);
								for(;i<nCount-1;++i)
									hitem = TreeView_GetNextVisible(hTreeView, hitem);
								TreeView_SelectDropTarget(hTreeView, hitem);
								hItemDrop = hitem;
								break;
							}
					}
				}
				ImageList_DragEnter(NULL, pnt.x, pnt.y);
			}
			break;
		}
	case WM_LBUTTONUP:
		{
			TVHITTESTINFO tv_ht;
			TVITEM tv;
			ZeroMemory(&tv, sizeof(TVITEM));
			ZeroMemory(&tv_ht, sizeof(TVHITTESTINFO));
			
			if(g_fdragging)
			{
				ImageList_DragLeave(NULL);
				ImageList_EndDrag();
				ReleaseCapture();
				if (hItemDrag != NULL && hItemDrag != hItemDrop && 
					!IsChildNodeOf(hItemDrop, hItemDrag))
				{
					TVITEM tvDrop;
					tvDrop.mask = TVIF_CHILDREN | TVIF_HANDLE;
					tvDrop.hItem = hItemDrop;
					TreeView_GetItem(hTreeView, &tvDrop);
					if(tvDrop.cChildren == 0) 
						hItemDrop = TreeView_GetParent(hTreeView, hItemDrop);
					if(hItemDrop == NULL) hItemDrop = TVI_ROOT;
					
					TransferItem(hItemDrag, hItemDrop);
					TreeView_DeleteItem(hTreeView, hItemDrag);
				}

				GetCursorPos(&(tv_ht.pt));
				ScreenToClient(hTreeView, &(tv_ht.pt));
					tv_ht.flags = TVHT_ONITEM;
				g_fdragging = FALSE;
				TreeView_SelectDropTarget(hTreeView, NULL);
			}
			break;
		}
	case WM_NOTIFY:
		if(wParam == IDC_TREE_MENUITEM)
		{
			if(((LPNMHDR)lParam)->code == TVN_SELCHANGED)
			{
				TVITEM tv;

				memset(&tv, 0, sizeof(TVITEM));
				tv.mask = TVIF_PARAM|TVIF_HANDLE;
				tv.hItem = TreeView_GetSelection(hTreeView);
				TreeView_GetItem(hTreeView, &tv);
				if(tv.lParam == -1 || tv.lParam == -2)
					SendDlgItemMessage(hwndDlg, IDC_EDIT_SHOWPATH, WM_SETTEXT, 0, (LPARAM)"");
				else if(tv.lParam)
					SendDlgItemMessage(hwndDlg, IDC_EDIT_SHOWPATH, WM_SETTEXT, 0, tv.lParam);
			}
			else if(((LPNMHDR)lParam)->code == TVN_BEGINDRAG)
			{
				RECT rect;

				GetClientRect(hTreeView, &rect);
				ClientToScreen(hTreeView, (LPPOINT)&rect);
				ClientToScreen(hTreeView, ((LPPOINT)&rect)+1);
				iUpperYCoor = rect.top;
				iLowerYCoor = rect.bottom;

				begin_drag((LPNMTREEVIEW)lParam);
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_LIST_MENU:
			if(HIWORD(wParam) == LBN_DBLCLK)
			{
				SendMessage(hwndDlg, WM_COMMAND, IDC_BTN_EXMEDIT, 0);
			}
			else if(HIWORD(wParam) == LBN_SELCHANGE)
			{
				int i = SendMessage(hListBox_menu, LB_GETCURSEL, 0, 0);
				//Make New Item data.
				if(pLast)
				{
					int count = TreeView_GetCount(hTreeView);
					char *Buf = halloc(count*(MAX_PATH+2 + MAX_PATH+1)+1);
					char *Buf1 = halloc(count*(MAX_PATH+2 + MAX_PATH+2)+2);
					char *pCurrent = Buf;
					int n = 0;

					lstrcpy(Buf1, "=");
					*Buf = '\0';

					Tree2String(TreeView_GetRoot(hTreeView), Buf1);
					lstrcpyn(Buf, Buf1+1, lstrlen(Buf1)-1);

					hfree(Buf1);
					if(pLast->szMenuItem)
						hfree(pLast->szMenuItem);
					pLast->szMenuItem = Buf;
				}
				if(i != LB_ERR && lParam != -1)
				{
					ListMenuItemFromSelect(i);
					LastSelectSec = i;
				}
			}
			break;
		case IDC_BUTTON_UP:
			{
				HTREEITEM hSelect = TreeView_GetSelection(hTreeView);
				if(hSelect)
				{
					HTREEITEM hParent = TreeView_GetParent(hTreeView, hSelect);
					if(hParent)
					{
						hParent = TreeView_GetParent(hTreeView, hParent);
						if(hParent == NULL) hParent = TVI_ROOT;
						TransferItem(hSelect, hParent);
						TreeView_DeleteItem(hTreeView, hSelect);
					}
				}
			}
			break;
		case IDC_CHK_EF:
			{
				BOOL bCheck = IsDlgButtonChecked(hwndDlg, IDC_CHK_EF);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LIST_MENU), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_MENUITEM), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EXMADD), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EXMDEL), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EXMEDIT), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EXADD), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EXDEL), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EXEDIT), bCheck);
			}
			break;
		case IDC_BTN_EXMADD:
			DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_FAVMENU_EDIT, 
				hwndDlg, EditFavMenuDataProc, LB_ERR);
			break;
		case IDC_BTN_EXMDEL:
			{
				int i = SendMessage(hListBox_menu, LB_GETCURSEL, 0, 0);
				char szName[MAXRULE2];
				PSFAV_DATA pData, last;
				if(i < 2 && i != LB_ERR)
				{
					LocalMsgBox(hwndDlg, lang_file, 0, 105, MB_OK|MB_ICONINFORMATION);
					break;
				}
				SendMessage(hListBox_menu, LB_GETTEXT, i, (LPARAM)szName);
				SendMessage(hwndDlg, WM_COMMAND, 
					MAKEWPARAM(IDC_LIST_MENU, LBN_SELCHANGE), -1);
				pData = GetFavDataMenuItem(Fav_data_temp, szName);
				last = Fav_data_temp;
				if(Fav_data_temp == pData)
					Fav_data_temp = pData->next;
				else
				{
					while(last && last->next != pData)
					{
						last = last->next;
					}
					last->next = pData->next;
				}
				if(pData->szName)
					hfree(pData->szName);
				if(pData->szMenuItem)
					hfree(pData->szMenuItem);
				hfree(pData);
				SendMessage(hListBox_menu, LB_DELETESTRING, i, 0);
				pLast = NULL;
				if(i == SendMessage(hListBox_menu, LB_GETCOUNT, 0, 0))
					i--;
				SendMessage(hListBox_menu, LB_SETCURSEL, i, 0);
				LastSelectSec = i;
				SendMessage(hwndDlg, WM_COMMAND, 
					MAKEWPARAM(IDC_LIST_MENU, LBN_SELCHANGE), 0);
			}
			break;
		case IDC_BTN_EXMEDIT:
			{
				int i = SendMessage(hListBox_menu, LB_GETCURSEL, 0, 0);
				if(i < 2 && i != LB_ERR)
				{
					LocalMsgBox(hwndDlg, lang_file, 0, 106, MB_OK|MB_ICONINFORMATION);
					break;
				}
				if(i != LB_ERR)
					DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_FAVMENU_EDIT, 
						hwndDlg, EditFavMenuDataProc, i);
			}
			break;
		case IDC_BTN_EXADD:
			DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_FAVMENUITEM_EDIT, 
				hwndDlg, EditFavMenuItemProc, -1);
			break;
		case IDC_BTN_EXDEL:
			{
				HTREEITEM hItem = TreeView_GetSelection(hTreeView);
				if(hItem) DeleteTreeItem(hItem);
			}
			break;
		case IDC_BTN_EXEDIT:
			if(TreeView_GetSelection(hTreeView))
			{
				TVITEM tv;

				memset(&tv, 0, sizeof(TVITEM));
				tv.mask = TVIF_HANDLE|TVIF_PARAM;
				tv.hItem = TreeView_GetSelection(hTreeView);
				TreeView_GetItem(hTreeView, &tv);

				if(tv.lParam != -2)
					DialogBoxParam(processHandle, (LPCTSTR)IDD_DM2_FAVMENUITEM_EDIT, 
						hwndDlg, EditFavMenuItemProc, 0);
				else
					LocalMsgBox(hwndDlg, lang_file, 0, 108, MB_OK|MB_ICONINFORMATION);
			}
			break;
		default:
			break;
		}
		break;
	case WM_DM2_SAVESETTING:
		SendMessage(hwndDlg, WM_COMMAND, 
			MAKEWPARAM(IDC_LIST_MENU, LBN_SELCHANGE), -1);
		bEnableFavMenu_temp = IsDlgButtonChecked(hwndDlg, IDC_CHK_EF);
		break;
	case WM_SETCURSOR:
		if((HWND)wParam == hListBox_menu && HIWORD(lParam) == WM_RBUTTONUP)
		{
			//only for listbox.
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(hListBox_menu, &point);
			SendMessage(hListBox_menu, LB_SETCURSEL, 
				SendMessage(hListBox_menu, LB_ITEMFROMPOINT, 0, 
				MAKELPARAM(point.x, point.y)), 0);
			
			//pop menu
			PopContextMenuFromButton(hwndDlg, (LPDWORD)&dwBtnMenu, 
				sizeof(dwBtnMenu)/sizeof(DWORD));
		}
		break;
	case WM_DESTROY:
		DestroyCursor(hTreeCursor);
		break;
	default:
		break;
	}
	
	return FALSE;
}

BOOL IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent)
{
	do
	{
		if (hitemChild == hitemSuspectedParent)
			break;
	}
	while ((hitemChild = TreeView_GetParent(hTreeView, hitemChild)) != NULL);

	return (hitemChild != NULL);
}

BOOL TransferItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop)
{
	TVINSERTSTRUCT tvstruct;
	CHAR sztBuffer[MAX_PATH2];
	HTREEITEM hNewItem, hFirstChild;

	tvstruct.item.hItem = hitemDrag;
	tvstruct.item.cchTextMax = MAX_PATH;
	tvstruct.item.pszText = sztBuffer;
	tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM | 
		TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	TreeView_GetItem(hTreeView, &tvstruct.item);
	tvstruct.hParent = hitemDrop;
	tvstruct.hInsertAfter = TVI_LAST;
	tvstruct.item.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
		TVIF_PARAM | TVIF_TEXT;
	hNewItem = TreeView_InsertItem(hTreeView, &tvstruct);

	while ((hFirstChild = TreeView_GetChild(hTreeView, hitemDrag)) != NULL)
	{
		TransferItem(hFirstChild, hNewItem);
		TreeView_DeleteItem(hTreeView, hFirstChild);	
	}
	return TRUE;
}

BOOL begin_drag(LPNMTREEVIEW p_nmtree) 
{
	HIMAGELIST h_drag = TreeView_CreateDragImage(hTreeView, p_nmtree->itemNew.hItem);
	TVHITTESTINFO tvi;

	ImageList_BeginDrag(h_drag, 0, 0, 0);
	SetCursor(NULL);
	SetCapture(GetParent(hTreeView));
	g_fdragging = TRUE;
	
	memset(&tvi, 0, sizeof(TVHITTESTINFO));
	tvi.pt.x = p_nmtree->ptDrag.x;
	tvi.pt.y = p_nmtree->ptDrag.y;
	hItemDrag = TreeView_HitTest(hTreeView, &tvi);
	hItemDrop = NULL;

	return TRUE;
} 

void DeleteTreeItem(HTREEITEM hItem)
{
	TVITEM tv;
	memset(&tv, 0,sizeof(TVITEM));
	tv.mask = TVIF_PARAM|TVIF_HANDLE;
	tv.hItem = hItem;
	TreeView_GetItem(hTreeView, &tv);
	if(tv.lParam == -1)
	{
		HTREEITEM hChild = TreeView_GetChild(hTreeView, hItem);
		do
		{
			if(hChild)
				DeleteTreeItem(hChild);
			else 
				break;
			hChild = TreeView_GetChild(hTreeView, hItem);
		}
		while(TRUE);
	}
	else if(tv.lParam != -2 && tv.lParam)
		hfree((LPVOID)tv.lParam);

	TreeView_DeleteItem(hTreeView, hItem);
}

void Tree2String(HTREEITEM hParent, char *pStr)
{
	char szName[MAX_PATH2];
	char *pEnd;
	TVITEM tv;
	int c = lstrlen(pStr);
	pEnd = pStr+c;
	
	memset(&tv, 0,sizeof(TVITEM));
	tv.mask = TVIF_CHILDREN|TVIF_TEXT|TVIF_PARAM|TVIF_HANDLE;
	tv.hItem = hParent;
	tv.pszText = szName;
	tv.cchTextMax = MAX_PATH;

	do
	{
		if(TreeView_GetItem(hTreeView, &tv) == FALSE)
			break;
		if(tv.cChildren == 1)
		{
			HTREEITEM hItem = TreeView_GetChild(hTreeView, tv.hItem);
			pEnd = stradd(stradd(stradd(pEnd, "-"), szName), "=|");
			if(hItem)
				Tree2String(hItem, pStr);
			pEnd = stradd(pStr+lstrlen(pStr), "-=|");
		}
		else
		{
			if(tv.lParam == -2)
			{
				pEnd = stradd(pEnd, "--=|");
			}
			else
			{
				pEnd = stradd(stradd(stradd(stradd(pEnd, szName), "="), (char *)tv.lParam), "|");
				hfree((char *)tv.lParam);
				tv.lParam = 0;
				TreeView_SetItem(hTreeView, &tv);
			}
		}
	}
	while(tv.hItem = TreeView_GetNextItem(hTreeView, tv.hItem, TVGN_NEXT));
}

void String2Tree(HTREEITEM hParent, char *pStr)
{
	char *pNextName;
	char *pTemp, *pTemp1;
	HTREEITEM hItem;
	TVINSERTSTRUCT tv;
	memset(&tv, 0, sizeof(TVINSERTSTRUCT));
	tv.hParent = hParent;
	tv.hInsertAfter = TVI_LAST;
	tv.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_CHILDREN|TVIF_IMAGE|TVIF_SELECTEDIMAGE;

	pNextName = pStr;
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
				tv.item.pszText = "--------------------------------";
				tv.item.cchTextMax = 2;
				tv.item.cChildren = 0;
				tv.item.lParam = -2; //is separator.
				tv.item.iImage = 1;
				tv.item.iSelectedImage = 1;
				TreeView_InsertItem(hTreeView, &tv);
				pNextName = pTemp1;
				continue;
			}

			pNextName++;
			pMenu = strchr(pNextName, '=');
			if(*pMenu && *(pMenu+1) == '|')
			{
				int i = pMenu-pNextName+1;
				char *szItem = halloc(i);
				lstrcpyn(szItem, pNextName, i);
				tv.item.pszText = szItem;
				tv.item.cchTextMax = i;
				tv.item.cChildren = 1;
				tv.item.lParam = -1; //is sub.
				tv.item.iImage = 0;
				tv.item.iSelectedImage = 0;
				hItem = TreeView_InsertItem(hTreeView, &tv);
				hfree(szItem);
			}
			//Get String range
			pMenu = pNextName;
			i++;
			while(i)
			{
				pMenu = strchr(pMenu, '-');
				if(pMenu++ == NULL) return;
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
			String2Tree(hItem, pMem);
			free(pMem);
			pTemp = pMenu+1;
		}
		else
		{
			int i;
			char *szPath;
			*(pTemp-1) = '\0';
			if(pTemp1)
				*(pTemp1-1) = '\0';
			
			i = lstrlen(pNextName);
			szPath = halloc(MAX_PATH2);
			lstrcpy(szPath, pNextName+i+1);
			tv.item.pszText = pNextName;
			tv.item.cchTextMax = i;
			tv.item.cChildren = 0;
			tv.item.lParam = (LPARAM)szPath;
			tv.item.iImage = 1;
			tv.item.iSelectedImage = 1;
			TreeView_InsertItem(hTreeView, &tv);
			pTemp = pTemp1;
		}

		pNextName = pTemp;
	}
}

//list menu item in listview box, from listbox seltct item.
void ListMenuItemFromSelect(int i)
{
	char szSection[MAXRULE2];
	PSFAV_DATA last = Fav_data_temp;
	
	TreeView_DeleteAllItems(hTreeView);
	SendMessage(hListBox_menu, LB_GETTEXT, i, (LPARAM)szSection);

	while(last)
	{
		if(lstrcmpi(last->szName, szSection) == 0)
		{
			char *pString;
			char *pCurrent;
			LVITEM lv;
			
			if(*(last->szMenuItem) == '\0')
			{
				pLast = last;
				return;
			}

			pString = halloc(lstrlen(last->szMenuItem)+1);
			pCurrent = pString;
			memset(&lv, 0, sizeof(lv));
			lstrcpy(pString, last->szMenuItem);
			//Add item from string
			String2Tree(NULL, pString);

			hfree(pString);
			pLast = last;			//get last access data.
			return;
		}
		last = last->next;
	}
	pLast = NULL;			//none find name.
}


BOOL IsEditMenu = FALSE;
char *SecDlgFavSection = "SettingsDialogFavSection";
INT_PTR CALLBACK EditFavMenuDataProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char szBuf[MAX_PATH];
			//Set mode and title
			IsEditMenu = (lParam != LB_ERR);

			SendDlgItemMessage(hwndDlg, IDC_EDIT_SECTION, EM_LIMITTEXT, MAXRULE, 0);
			if(IsEditMenu)
			{
				char szTemp[MAXRULE2];
				LoadLanguageString(lang_file, SecDlgFavSection, 100, szBuf, MAX_PATH);
				SendMessage(hListBox_menu, LB_GETTEXT, (WPARAM)lParam, (LPARAM)szTemp);
				SetDlgItemText(hwndDlg, IDC_EDIT_SECTION, szTemp);
			}
			else
				LoadLanguageString(lang_file, SecDlgFavSection, 101, szBuf, MAX_PATH);
			SetWindowText(hwndDlg, szBuf);
		}

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, SecDlgFavSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char szTemp[MAXRULE2];
				DWORD setting = 0;
				char *Buf;
				int i;
				//test 
				GetDlgItemText(hwndDlg, IDC_EDIT_SECTION, szTemp, MAXRULE2);
				if(*szTemp == 0)
				{
					LocalMsgBox(hwndDlg, lang_file, 0, 109, MB_OK|MB_ICONINFORMATION);
					return FALSE;
				}
				//detect
				i = SendMessage(hListBox_menu, LB_GETCOUNT, 0, 0)-1;
				for(; i>=0; i--)
				{
					char szName[MAXRULE2];
					SendMessage(hListBox_menu, LB_GETTEXT, i, (LPARAM)szName);
					if(lstrcmpi(szName, szTemp) == 0)
					{
						if(!IsEditMenu || 
							SendMessage(hListBox_menu, LB_GETCURSEL, 0, 0) != i)
						{	
							LocalMsgBox(hwndDlg, lang_file, 0, 110, MB_OK|MB_ICONINFORMATION);
							return FALSE;
						}
					}
				}

				Buf = halloc(lstrlen(szTemp)+1);
				lstrcpy(Buf, szTemp);
				if(IsEditMenu)
				{
					char LastName[MAXRULE2];
					PSFAV_DATA pCurrent;
					SendMessage(hListBox_menu, LB_GETTEXT, 
						(WPARAM)SendMessage(hListBox_menu, LB_GETCURSEL, 0, 0), 
						(LPARAM)LastName);
					pCurrent = GetFavDataMenuItem(Fav_data_temp, LastName);
					hfree(pCurrent->szName);
					pCurrent->szName = Buf;
					i = SendMessage(hListBox_menu, LB_GETCURSEL, 0, 0);
					SendMessage(hListBox_menu, LB_DELETESTRING, i, 0);
					i = SendMessage(hListBox_menu, LB_INSERTSTRING, i, (LPARAM)szTemp);
					SendMessage(hListBox_menu, LB_SETCURSEL, i, 0);
				}
				else
				{
					PSFAV_DATA pCurrent = Fav_data_temp;
					while(pCurrent)
					{
						if(pCurrent->next == NULL)
							break;
						pCurrent = pCurrent->next;
					}
					pCurrent->next = halloc(sizeof(SFAV_DATA));
					pCurrent = pCurrent->next;
					pCurrent->szName = Buf;
					pCurrent->szMenuItem = halloc(1);
					pCurrent->next = NULL;
					*(pCurrent->szMenuItem) = '\0';
					//pLast = pCurrent;
					i = SendMessage(hListBox_menu, LB_GETCOUNT, 0, 0);
					SendMessage(hListBox_menu, LB_INSERTSTRING, i, (LPARAM)szTemp);
				}
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
	}
	
	return FALSE;
}


BOOL CheckMenuItemString(char *String)
{
	int iLen = lstrlen(String);
	if(iLen)
	{
		if(strchr(String, '|')) return FALSE;
		if(String[0] == '-') return FALSE;
		if(String[iLen-1] == '-') return FALSE;
		if( strstr(String, "--")) return FALSE;
	}
	return TRUE;
}

static int CALLBACK BrowseFolderCallbackProc(HWND wnd, UINT uMsg,
										  WPARAM wParam, LPARAM lParam)
{
    if (BFFM_INITIALIZED == uMsg)
    {   
        //restore the selection
        char *szDir = (LPSTR)lParam;
        SendMessage(wnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)szDir);              
    }
    return 0;
}

BOOL IsEditMenuItem = FALSE;
char *MenuDlgFavSection = "SettingsDialogFavMenu";
extern HWND hLastDlg;
INT_PTR CALLBACK EditFavMenuItemProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char szBuf[MAX_PATH];
			//Set mode and title
			IsEditMenuItem = (lParam != -1);

			SendDlgItemMessage(hwndDlg, IDC_EDIT_NAME, EM_LIMITTEXT, MAX_PATH, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_PATH, EM_LIMITTEXT, MAX_PATH, 0);
			if(IsEditMenuItem)
			{
				char szName[MAX_PATH2];
				TVITEM tv;

				LoadLanguageString(lang_file, MenuDlgFavSection, 100, szBuf, MAX_PATH);

				memset(&tv, 0, sizeof(TVITEM));
				tv.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
				tv.pszText = szName;
				tv.cchTextMax = MAX_PATH;
				tv.hItem = TreeView_GetSelection(hTreeView);
				TreeView_GetItem(hTreeView, &tv);
				
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SUB), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SEP), FALSE);
				if(tv.lParam == -1)
				{
					SetDlgItemText(hwndDlg, IDC_EDIT_NAME, szName);
					CheckDlgButton(hwndDlg, IDC_CHK_SUB, BST_CHECKED);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_PATH), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_BRW), FALSE);
				}
				else if(tv.lParam)
				{
					SetDlgItemText(hwndDlg, IDC_EDIT_NAME, szName);
					SetDlgItemText(hwndDlg, IDC_EDIT_PATH, (char *)tv.lParam);
				}
			}
			else
				LoadLanguageString(lang_file, MenuDlgFavSection, 101, szBuf, MAX_PATH);
			SetWindowText(hwndDlg, szBuf);
		}

		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, MenuDlgFavSection);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char szName[MAX_PATH2];
				char szPath[MAX_PATH2];

				if(IsDlgButtonChecked(hwndDlg, IDC_CHK_SEP) != BST_CHECKED)
				{
					//test 
					GetDlgItemText(hwndDlg, IDC_EDIT_NAME, szName, MAX_PATH2);
					GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szPath, MAX_PATH2);
					if(*szName == '\0')
					{
						LocalMsgBox(hwndDlg, lang_file, 0, 111, MB_OK|MB_ICONINFORMATION);
						return FALSE;
					}
					// v1.18 changed
					// old: if(strchr(szName, '-'))
					if(CheckMenuItemString(szName) == FALSE)
					{
						LocalMsgBox(hwndDlg, lang_file, 0, 112, MB_OK|MB_ICONINFORMATION);
						return FALSE;
					}
					if(CheckMenuItemString(szPath) == FALSE)
					{
						LocalMsgBox(hwndDlg, lang_file, 0, 113, MB_OK|MB_ICONINFORMATION);
							return FALSE;
					}
					if(*szPath == '\0' && 
						IsDlgButtonChecked(hwndDlg, IDC_CHK_SUB) == BST_UNCHECKED)
					{
						LocalMsgBox(hwndDlg, lang_file, 0, 114, MB_OK|MB_ICONINFORMATION);
							return FALSE;
					}
				}
				
				if(IsEditMenuItem)
				{
					TVITEM tv;

					memset(&tv, 0, sizeof(TVITEM));
					tv.mask = TVIF_HANDLE|TVIF_PARAM;
					tv.hItem = TreeView_GetSelection(hTreeView);
					TreeView_GetItem(hTreeView, &tv);
					tv.pszText = szName;
					tv.cchTextMax = lstrlen(szName);
					tv.mask = TVIF_HANDLE|TVIF_TEXT;
					if(tv.lParam != -1 && tv.lParam != -2 && tv.lParam)
						lstrcpy((char *)tv.lParam, szPath);
					TreeView_SetItem(hTreeView, &tv);
					SendDlgItemMessage(hLastDlg, IDC_EDIT_SHOWPATH, WM_SETTEXT, 0, 
						tv.lParam);
				}
				else
				{
					TVITEM tv;
					TVINSERTSTRUCT tvis;

					memset(&tv, 0, sizeof(TVITEM));
					memset(&tvis, 0, sizeof(TVINSERTSTRUCT));
					tv.mask = TVIF_HANDLE|TVIF_PARAM;
					tv.hItem = TreeView_GetSelection(hTreeView);
					TreeView_GetItem(hTreeView, &tv);
					if(tv.lParam == -1)
						tvis.hParent = tv.hItem;
					else
						tvis.hParent = TVI_ROOT;

					//is sub menu
					tvis.item.mask = TVIF_HANDLE|TVIF_TEXT|TVIF_PARAM|TVIF_CHILDREN|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
					tvis.item.pszText = szName;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_SUB) == BST_CHECKED)
					{
						tvis.item.cchTextMax = lstrlen(szName);
						tvis.item.lParam = -1;
						tvis.item.cChildren = 1;
						tvis.item.iImage = 0;
						tvis.item.iSelectedImage = 0;
					}
					else if(IsDlgButtonChecked(hwndDlg, IDC_CHK_SEP) == BST_CHECKED)
					{
						lstrcpy(szName, "--------------------------------");
						tvis.item.cchTextMax = lstrlen(szName);
						tvis.item.lParam = -2;
						tvis.item.iImage = 1;
						tvis.item.iSelectedImage = 1;
					}
					else
					{
						tvis.item.lParam = (LPARAM)halloc(MAX_PATH2);
						tvis.item.iImage = 1;
						tvis.item.iSelectedImage = 1;
						lstrcpy((char *)tvis.item.lParam, szPath);
					}
					TreeView_InsertItem(hTreeView, &tvis);
				}
			}

			EndDialog(hwndDlg, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		case IDC_CHK_SUB:
			{
				BOOL bCheck = (IsDlgButtonChecked(hwndDlg, IDC_CHK_SUB) == BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_PATH), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_BRW), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_NAME), TRUE);
				CheckDlgButton(hwndDlg, IDC_CHK_SEP, BST_UNCHECKED);
			}
			break;
		case IDC_CHK_SEP:
			{
				BOOL bCheck = (IsDlgButtonChecked(hwndDlg, IDC_CHK_SEP) == BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_PATH), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_BRW), bCheck);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_NAME), bCheck);
				CheckDlgButton(hwndDlg, IDC_CHK_SUB, BST_UNCHECKED);
			}
			break;
		case IDC_BTN_BRW:
			{
				char szPath[MAX_PATH];
				*szPath = '\0';
				if(SendMessage(hListBox_menu, LB_GETCURSEL, 0, 0) == 1)
				{
					OPENFILENAME ofn;
					memset(&ofn, 0, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hInstance = processHandle;
					ofn.hwndOwner = hwndDlg;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFile = szPath;
					ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES;
					ofn.lpstrFilter = "Executable File(*.exe;*.com)\0*.exe;*.com\0All File(*.*)\0*.*\0\0";
					if(GetOpenFileName(&ofn) == IDOK)
						SetDlgItemText(hwndDlg, IDC_EDIT_PATH, szPath);
				}
				else
				{
					LPITEMIDLIST pidlRoot = NULL;
					BROWSEINFO bi = {0};
					char szOldPath[MAX_PATH];
					memset(szOldPath, 0, MAX_PATH);
					
					bi.hwndOwner = hwndDlg;
					bi.pidlRoot = pidlRoot;
					//#define BIF_NEWDIALOGSTYLE 64
					//#define BIF_UAHINT 256
					bi.ulFlags = BIF_DONTGOBELOWDOMAIN | 64 | BIF_RETURNONLYFSDIRS | 256 | BIF_EDITBOX;
					bi.lpfn = (BFFCALLBACK)BrowseFolderCallbackProc;
					GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szOldPath, MAX_PATH);
					bi.lParam = (LPARAM)szOldPath;
					SHGetPathFromIDList(SHBrowseForFolder(&bi), szPath);
					if(*szPath != '\0')
						SetDlgItemText(hwndDlg, IDC_EDIT_PATH, szPath);
				}
			}
			break;
		default:
			break;
		}
		break;
	}
	
	return FALSE;
}


void CreateDefaultSection(PSFAV_DATA *data)
{
	PSFAV_DATA Temp = *data;
	if(GetFavDataMenuItem(Temp, szFavApp) == NULL)
	{
		PSFAV_DATA pMem = halloc(sizeof(SFAV_DATA));
		pMem->next = Temp;
		pMem->szName = halloc(sizeof(szFavApp)/sizeof(char));
		pMem->szMenuItem = halloc(1);
		lstrcpy(pMem->szName, szFavApp);
		*(pMem->szMenuItem) = '\0';
		Temp = pMem;
	}
	if(GetFavDataMenuItem(Temp, szFavDef) == NULL)
	{
		PSFAV_DATA pMem = halloc(sizeof(SFAV_DATA));
		pMem->next = Temp;
		pMem->szName = halloc(sizeof(szFavDef)/sizeof(char));
		pMem->szMenuItem = halloc(1);
		lstrcpy(pMem->szName, szFavDef);
		*(pMem->szMenuItem) = '\0';
		Temp = pMem;
	}
	*data = Temp;
}
