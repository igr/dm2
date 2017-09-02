/* http://dm2.sf.net */
#define _WIN32_WINNT 0x5001

#include <windows.h>
#include <commctrl.h>

#include "dm2.h"
#include "util.h"

extern HANDLE processHeap;

/**
 * Allocates memory on process heap.
 */
//memtest module
#ifndef NDEBUG
extern int mem_test;
#endif
LPVOID halloc(int size) {
	#ifndef NDEBUG
	LPVOID i = HeapAlloc(processHeap, HEAP_NO_SERIALIZE, size);
	if(i)
		mem_test++;
	return i;
	#else
	return HeapAlloc(processHeap, HEAP_NO_SERIALIZE, size);
	#endif
}

/**
 * Frees memory from process heap.
 */
BOOL hfree(LPVOID heap) {
	#ifndef NDEBUG
	int i = HeapFree(processHeap, HEAP_NO_SERIALIZE, heap);
	if(i)
		mem_test--;
	return i;
	#else
	return HeapFree(processHeap, HEAP_NO_SERIALIZE, heap);
	#endif
}


/**
 * Adds task bar icon.
 */
void AddTaskBarIcon(HWND hwnd, HICON hicon, char *tooltip, int id) {
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = id;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = DM2_TRAY_MSG;
	nid.hIcon = hicon;
	if (tooltip) {
		lstrcpyn(nid.szTip, tooltip, sizeof(nid.szTip));
	} else {
		nid.szTip[0] = '\0';
	}
	Shell_NotifyIcon(NIM_ADD, &nid);
	return;
}

/**
 * Removes task bar icon.
 */
void DeleteTaskBarIcon(HWND hwnd, int id) {
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = id;
	Shell_NotifyIcon(NIM_DELETE, &nid);
	return;
}



/**
 * Creates tool-tip.
 *
 * flyfancy modification, because tooltips need destroy.
 */
HWND CreateTooltip(HWND hwnd, char* strTT, int MaxWidth, int icon, char *szTitle) {
	HWND hwndTT;
	TOOLINFO ti;
	RECT rect;
	
	/* creates a tooltip window */
	hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							NULL, NULL, processHandle, NULL);
	
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	GetClientRect(hwnd, &rect);			// get coordinates of the main client area
	
	/* initialize members of the toolinfo structure */
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwnd;
	ti.hinst = processHandle;
	ti.uId = 177;
	ti.lpszText = (LPSTR) strTT;
	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);		// send an addtool message to the tooltip control window
	if(MaxWidth)
		SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, MaxWidth);
	if(szTitle && *szTitle != '\0')	//TTM_SETTITLE - Requirements: Version 5.80 and later of Comctl32.dll
		SendMessage(hwndTT, (WM_USER+32), icon, (LPARAM)szTitle);

	return hwndTT;
}


/**
 * Sets or remove WS_EX_LAYERD attribute for a window.
 *
 * @param hWnd   window to set the attributes on
 * @param flag   TRUE to set the attribute, FALSE to remove them
 */
void SetWsExLayered(HWND hWnd, BOOLEAN flag) {
	if (flag == TRUE) {
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	} else {
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
	}
	return;
}


/**
 * Sets window opacity
 */
void SetWindowOpacity(HWND hWnd, int opacity) {
	if (SetLayeredWindowAttributesImp == NULL) {
		return;
	}
	if ( (opacity < 0) || (opacity >= 255) ) {
		SetWsExLayered(hWnd, FALSE);
	} else {
		SetWsExLayered(hWnd, TRUE);
		SetLayeredWindowAttributesImp(hWnd, (COLORREF)NULL, (BYTE)opacity, LWA_ALPHA);
	}
	return;
}


/**
 * Sets the floating icon position.
 *
 * @param hwnd       floating window
 * @param currentWnd big window
 */
void SetRelativeWindowPos(HWND hwnd, HWND currentWnd, int init, int top, int size) {
	RECT r;
	RECT rd;
	GetWindowRect(currentWnd, &r);
	GetDesktopRect(&rd);
	
	//determines whether the specified window is minimized.
	if(IsIconic(currentWnd)) {
		POINT pos;
		GetCursorPos(&pos);
		r.left = pos.x;
		r.top = pos.y;
	}
	else
	{
		switch (init) {
			case 1:
				r.left = r.right - size;
				break;
			case 2:
				r.top = r.bottom - size;
				break;
			case 3:
				r.left = r.right - size;
				r.top = r.bottom - size;
				break;
			case 4:
				r.left = r.left + (r.right - r.left) / 2 - size / 2;
				r.top = r.top + (r.bottom - r.top) / 2 - size / 2;
				break;
		}
	}

	//avoid move out screen
	r.left = setIntRange(r.left, rd.left, rd.right - size);
	r.top = setIntRange(r.top, rd.top, rd.bottom - size);

	SetWindowPos(hwnd, NULL, r.left, r.top, 
		size, size, SWP_SHOWWINDOW);
	//at xp sp2, DM2 first create float window can't set window on 
	//top(WS_EX_TOPMUST can't set). so set again. other system i don't know.
	SetWindowPos(hwnd, (top != FALSE) ? HWND_TOPMOST : HWND_TOP, 0, 0, 
		0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

/**
 * Toggle window on-top flag.
 *
 * @param hwnd   target window
 * @param top    1 for setting top flag
 */
void SetWindowOnTop(HWND hwnd, int top) {
	if (top == 1) {
		SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	} else {
		SetWindowPos(hwnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	}
}

/**
 * Get Windows title, want to process multi-byte characters. 
 * if Windows title too lenght, copy "..." in the end.
 *
 * Can't process Unicode.
 *
 * @param hwnd		target window
 * @param lpBuf		copy title to lpBuf, the buffer size equal to MAX_WIN_TITLE;
 */
void GetWinTitle(HWND hWnd, char *lpBuf)
{
	char szTitle[MAX_WIN_TITLE+2];
	*szTitle = '\0';

	if(GetWindowText(hWnd, szTitle, MAX_WIN_TITLE+2) >= MAX_WIN_TITLE)
	{
		char *lpPrev = CharPrev(szTitle, &szTitle[MAX_WIN_TITLE-3]);
		lstrcpy(lpPrev, "...");
	}

	lstrcpy(lpBuf, szTitle);
}


//must chang data, retrieves from HKM_GETHOTKEY message, 
//because HOTKEYF_ALT not equal MOD_ALT and HOTKEYF_SHIFT not equal MOD_SHIFT

/*#define HOTKEYF_ALT	4
**#define HOTKEYF_SHIFT	1
**#define MOD_ALT 1
**#define MOD_SHIFT 4*/
DWORD ConversionHotkey(DWORD dwHK)
{
	DWORD wHK = (dwHK & 0xFF00) >> 8;
	
	if((wHK & (HOTKEYF_ALT|HOTKEYF_SHIFT)) != (HOTKEYF_ALT|HOTKEYF_SHIFT))
	{
		if(wHK & HOTKEYF_ALT)
			wHK = (wHK & ~HOTKEYF_ALT) | MOD_ALT;
		else if(wHK & HOTKEYF_SHIFT)
			wHK = (wHK & ~HOTKEYF_SHIFT) | MOD_SHIFT;
	}
	if(wHK & HOTKEYF_EXT)
		wHK = (wHK & ~HOTKEYF_EXT);
	//if win key
	if(wHK & (HOTKEYF_EXT << 1))
		wHK = (wHK & ~(HOTKEYF_EXT << 1)) | HOTKEYF_EXT;
	
	return wHK;
}


MENUITEMINFO MenuSepa = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_SEPARATOR, 0};
//#define MIIM_STRING      0x00000040
MENUITEMINFO MenuItem = {sizeof(MENUITEMINFO), MIIM_ID | 0x00000040, 
						MFT_STRING, MFS_ENABLED, 0, 0};
void PopContextMenuFromButton(HWND Parent, LPDWORD pdwBtn, DWORD dwBtnNum)
{
	if(dwBtnNum)
	{
		DWORD i = 0;
		POINT point;
		char szMenuItem[MAX_PATH];
		HMENU hPop = CreatePopupMenu();
		GetCursorPos(&point);
		for(; i<dwBtnNum; i++)
		{
			if(pdwBtn[i])
			{
				// insert MenuItem
				GetDlgItemText(Parent, pdwBtn[i], szMenuItem, MAX_PATH);
				MenuItem.wID = pdwBtn[i];
				MenuItem.dwTypeData = szMenuItem;
				MenuItem.cch = lstrlen(szMenuItem);
				InsertMenuItem(hPop, i, TRUE, &MenuItem);
			}
			else
			{
				// insert Separator
				InsertMenuItem(hPop, i, TRUE, &MenuSepa);
			}
		}

		TrackPopupMenuEx(hPop, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | 
			TPM_RIGHTBUTTON, point.x, point.y, Parent, NULL);
		DestroyMenu(hPop);
	}
}


typedef struct
{
	WNDPROC OldProc;
	HWND Parent;
	LPDWORD dwMenuItem;
	DWORD dwNum;
} RMenuCtrl, *PRMenuCtrl;
LRESULT CALLBACK RightMenuProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PRMenuCtrl rm = (PRMenuCtrl)GetWindowLong(hwnd, GWL_USERDATA);
	WNDPROC OldProc = rm->OldProc;
	switch(uMsg)
	{
	case WM_RBUTTONDOWN:
		{
			LRESULT ret = CallWindowProc(OldProc, hwnd, uMsg, wParam, lParam);
			PopContextMenuFromButton(rm->Parent, rm->dwMenuItem, 
				rm->dwNum);
			return ret;
		}
	case WM_DESTROY:
		SetWindowLong(hwnd, GWL_WNDPROC, (LONG)OldProc);
		free(rm);
		break;
	}
	return CallWindowProc(OldProc, hwnd, uMsg, wParam, lParam);
}

void CreateRMenuCtrl(HWND Ctrl, HWND Parent, LPDWORD dwMenuItem, DWORD dwNum)
{
	PRMenuCtrl rm = malloc(sizeof(RMenuCtrl));
	rm->OldProc = (WNDPROC)GetWindowLong(Ctrl, GWL_WNDPROC);
	rm->Parent = Parent;
	rm->dwMenuItem = dwMenuItem;
	rm->dwNum = dwNum;
	SetWindowLong(Ctrl, GWL_USERDATA, (LONG)rm);
	SetWindowLong(Ctrl, GWL_WNDPROC, (LONG)RightMenuProc);
}

//if use strcat add more str, strlen will call more, it slow. 
//the function fast than that instance
//Param: dest point the str end char '\0'.
//Param: src point the str start.
//return point the str end char '\0', can use stradd(stradd(p1, p2), p3) syntax.
char *stradd(char *dest, char *src)
{
	register char *r_dest = dest;
	register char *r_src = src;
	if(r_dest == NULL)
		return NULL;
	if(r_src == NULL || *r_src == '\0')
		return r_dest;

	do
	{
		*r_dest++ = *r_src++;
	}
	while(*r_src);
	*r_dest = '\0';
	
	return r_dest;
}

int regexec_and_free(regexp *prog, char * string)
{
	int result = regexec(prog, string);
	free(prog);
	return result;
}
