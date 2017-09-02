/* http://dm2.sf.net */
#include "tools.h"

//CreateSharedMemory already Declared in Ntsecpkg.h
VOID *CreateDllSharedMemory(PDLLSHAREMEMORY psm)
{
    BOOL fInit;

	psm->hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,  
		PAGE_READWRITE, 0, psm->MemSize, psm->ShareMemName);
	if (psm->hMapObject == NULL) 
		return NULL; 
	
	fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 
	
	psm->lpvMem = MapViewOfFile(psm->hMapObject, FILE_MAP_WRITE, 0, 0, 0);
    if (psm->lpvMem == NULL) 
	{
		CloseHandle(psm->hMapObject);
		return NULL; 
	}
    
	if (fInit) 
        memset(psm->lpvMem, 0, psm->MemSize);
	
	return psm->lpvMem;
}


void DeleteSharedMemory(PDLLSHAREMEMORY psm)
{
	if(psm)
	{
		if(psm->lpvMem)
		{
			UnmapViewOfFile(psm->lpvMem);
			psm->lpvMem = NULL;
		}
		if(psm->hMapObject)
		{
			CloseHandle(psm->hMapObject);
			psm->hMapObject = NULL;
		}
	}
}


//this function find in dialogmate source code. flyfancy modification.
/**
 * Takes a screenshoot of the window in compatible bitmap.
 *
 * @param hWnd   window
 * @param w      width of target bitmap
 * @param h      height of the target bitmap
 * @param iStretchMode
 *               stretch mode (as defined in SetBrushOrgEx
 *
 * @return handle of screenshoot bitmap
 */
HBITMAP CreateWndScreenShoot(HWND hWnd, int w, int h, int iStretchMode, BOOL bFit) {
	HBITMAP wndhbmp = NULL;
	HBITMAP holdbmp = NULL;
	BOOLEAN stretch = TRUE;
	RECT r,rd;
	HDC hdcwnd = NULL;
	HDC hdcmem = NULL;
	int x = 0, y = 0;

	//determines  the specified window is minimized.
	if(IsIconic(hWnd))
		return NULL;

	//determines window whether out the screen
	GetWindowRect(hWnd, &r);
	GetDesktopRect(&rd);
	if(r.right > rd.right)
	{
		if(r.left >= rd.right)
			return NULL;
		r.right = rd.right;
	}
	if(r.bottom > rd.bottom)
	{
		if(r.top >= rd.bottom)
			return NULL;
		r.bottom = rd.bottom;
	}
	if(r.left < rd.left)
	{
		if(r.right <= rd.left)
			return NULL;
		x = -r.left;
		r.left = 0;
	}
	if(r.top < rd.top)
	{
		if(r.bottom <= rd.top)
			return NULL;
		y = -r.top;
		r.top = 0;
	}

	hdcmem = CreateCompatibleDC(hdcwnd);		// capture dc
	hdcwnd = GetDCEx(hWnd, NULL, DCX_WINDOW);

	if (w == 0) {
		w = r.right - r.left;
		h = r.bottom - r.top;
		stretch = FALSE;
	}
	
	if(stretch && bFit == FALSE)
	{
		int ww = r.right - r.left;
		int wh = r.bottom - r.top;
		if(wh > h || ww > w)
		{
			int LWidth = 0;
			int LHeight = 0;
			if((float)wh/h > (float)ww/w)
				VarI4FromR4(ww*((float)h/wh), (LONG *)&w);
			else
				VarI4FromR4(wh*((float)w/ww), (LONG *)&h);
		}
		else
		{
			h = wh;
			w = ww;
		}
	}
	
	wndhbmp = CreateCompatibleBitmap(hdcwnd, w, h);
	if (wndhbmp != NULL) {
		holdbmp = SelectObject(hdcmem, wndhbmp);
		if (holdbmp != NULL) {
			SetStretchBltMode(hdcmem, iStretchMode);
			if (iStretchMode == HALFTONE) {
				POINT prev;
				SetBrushOrgEx(hdcmem, 0, 0, &prev);
			}
			if (stretch == TRUE) {
				StretchBlt(hdcmem, 0, 0, w, h, hdcwnd, x, y, r.right - r.left, r.bottom - r.top, SRCCOPY);
			} else {
				BitBlt(hdcmem, 0, 0, w, h, hdcwnd, 0, 0, SRCCOPY);
			}
		} else {
			DeleteObject(wndhbmp);
			wndhbmp = NULL;
		}
	}
	SelectObject(hdcmem, holdbmp);
	DeleteDC(hdcmem);
	ReleaseDC(hWnd, hdcwnd);
	return wndhbmp;
}



int str2int(char *HexStr)
{
	char   *pCH, c;
	register DWORD  dwVal = 0, dw;
	
	pCH = HexStr;
	while (*pCH)
	{
		c = toupper(*pCH++);
		if (c >= 'A' && c <= 'F')
			dw = (DWORD)c - ((DWORD)'A' - 10);
		else if (c >= '0' && c <= '9')
			dw = (DWORD)c - (DWORD)'0';
		else
			return 0; // invalid hex char
		dwVal = (dwVal << 4) + dw;
	}
	
	return dwVal;
}

int setIntRange(int a, int b, int c)
{
	if(a>c)
		return c;
	if(a<b)
		return b;

	return a;
}

HICON GetWindowIcon(HWND hWnd, BOOL IsBigIcon)
{
	HICON hIcon;
	if(IsBigIcon){
		hIcon = (HICON)SendMessage(hWnd, WM_GETICON, (WPARAM)ICON_BIG, (LPARAM)NULL);
		if(!hIcon)
			hIcon = (HICON)SendMessage(hWnd, WM_GETICON, (WPARAM)ICON_SMALL, (LPARAM)NULL);
		if(!hIcon)
			hIcon = (HICON)GetClassLong(hWnd, GCL_HICON);
		if(!hIcon)
			hIcon = (HICON)GetClassLong(hWnd, GCL_HICONSM);
	}
	else
	{
		hIcon = (HICON)SendMessage(hWnd, WM_GETICON, (WPARAM)ICON_SMALL, (LPARAM)NULL);
		if(!hIcon)
			hIcon = (HICON)SendMessage(hWnd, WM_GETICON, (WPARAM)ICON_BIG, (LPARAM)NULL);
		if(!hIcon)
			hIcon = (HICON)GetClassLong(hWnd, GCL_HICONSM);
		if(!hIcon)
			hIcon = (HICON)GetClassLong(hWnd, GCL_HICON);
	}
	
	return hIcon;
}


void GetDesktopRect(RECT *rect)
{
	HWND hDesktop = FindWindow("Progman","Program Manager");
	if(hDesktop)
	{
		hDesktop = FindWindowEx(hDesktop, NULL, "SHELLDLL_DefView", NULL);
		if(hDesktop)
		{
			GetWindowRect(hDesktop, rect);
			return;
		}
	}

	//Can't find? make rect equal to screen size.
	rect->left = 0;
	rect->top = 0;
	rect->right = GetSystemMetrics(SM_CXSCREEN);
	rect->bottom = GetSystemMetrics(SM_CYSCREEN);
}

// Make argv and argc from any arg. flyfancy. 2005
//
// Note: 1. function will change the string arg, so don't use the const char etc!
//       2. you need call free((LPVOID)argv) to free the memory, if not use!
//
// example:
//	char **argv;
//	int argc = make_argv(arg, &argv);
//	do_something(argc, argv, other);
//	free((LPVOID)argv);
int make_argv(char *arg, char ***argv)
{
	int argc = 0;
	char *p;
	
    *argv = (char **)malloc(sizeof(char *));
	while(TRUE)
	{
		while(*arg == ' ' || *arg == '\t')
			arg++;
		if(*arg == '\0') break;
		
		*argv = (char **)realloc(*argv, (argc+2)*sizeof(char *));
		(*argv)[argc] = arg;
		argc++;
		
		if(*arg == '\"')
		{
			p = arg + 1;
			p = strchr(p, '\"');
			if(p == NULL) break;
			p++;
			if(*p == ' ') {*p = '\0'; p++;}
			arg = p;
		}
		else
		{
			p = strchr(arg, ' ');
			if(p == NULL) break;
			*p = '\0'; p++;
			arg = p;
		}
	}

	(*argv)[argc] = 0;

	return argc;
}

BOOL GetPriorityFromhWnd(HWND hWnd, DWORD *pdwPrio)
{
	HANDLE hProcess;
	DWORD PID;

	GetWindowThreadProcessId(hWnd, &PID);
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
	if(hProcess)
	{
		*pdwPrio = GetPriorityClass(hProcess);
		CloseHandle(hProcess);
		return TRUE;
	}

	return FALSE;
}
