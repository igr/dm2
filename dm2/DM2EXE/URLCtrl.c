/* http://dm2.sf.net */
#include <windows.h>

typedef struct
{
	char	 szURL[MAX_PATH];	// desination URL
	WNDPROC  oldproc;			// old window procedure
	COLORREF crLink;			// current link colour
	COLORREF crVisited;			// colour when VISITED

} URLCtrl;

static HFONT	hfUnderlined = 0;
static HCURSOR	hCursor      = 0;


void CleanupURLControl()
{
	if(hfUnderlined)
	{
		DeleteObject(hfUnderlined);
		hfUnderlined = NULL;
	}
	if(hCursor)
	{
		DestroyCursor(hCursor);
		hCursor = NULL;
	}
}

LRESULT CALLBACK URLCtrlProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	URLCtrl		*url = (URLCtrl *)GetWindowLong(hwnd, GWL_USERDATA);
	WNDPROC		oldproc = url->oldproc;
	
	HDC			hdc;
	HANDLE		hOld;
	PAINTSTRUCT ps;
	RECT		rect;
	DWORD		dwStyle;
	DWORD		dwDTStyle = 0;
	UINT		ret;
	
	static BOOL fClicking;
	static char	szWinText[MAX_PATH];

	switch(iMsg)
	{
	case WM_NCDESTROY:
		free(url);
		break;
	case WM_PAINT:
		
		dwStyle = GetWindowLong(hwnd, GWL_STYLE);
		
		dwDTStyle = DT_SINGLELINE;
		
		if(dwStyle & SS_CENTER)	     dwDTStyle |= DT_CENTER;
		if(dwStyle & SS_RIGHT)		 dwDTStyle |= DT_RIGHT;
		if(dwStyle & SS_CENTERIMAGE) dwDTStyle |= DT_VCENTER;

		GetClientRect(hwnd, &rect);

		hdc = BeginPaint(hwnd, &ps);
		
		SetTextColor(hdc, url->crLink);
		SetBkColor  (hdc, GetSysColor(COLOR_3DFACE));
		
		if(hfUnderlined == 0)
		{
			LOGFONT lf;
			HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

			GetObject(hf, sizeof lf, &lf);
            lf.lfUnderline = TRUE;
			
			hfUnderlined = CreateFontIndirect(&lf);
		}
		
		hOld = SelectObject(hdc, hfUnderlined);

		GetWindowText(hwnd, szWinText, sizeof szWinText);
		DrawText(hdc, szWinText, -1, &rect, dwDTStyle);
		
		SelectObject(hdc, hOld);

		EndPaint(hwnd, &ps);
		break;
	case WM_SETTEXT:
		ret = CallWindowProc(oldproc, hwnd, iMsg, wParam, lParam);
		InvalidateRect(hwnd, 0, 0);
		return ret;
	case WM_SETCURSOR:
		if(hCursor == 0)
			hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(32649), IMAGE_CURSOR, 
				32, 32, LR_LOADTRANSPARENT|LR_SHARED);
		
		SetCursor(hCursor);
		return TRUE;
	case WM_LBUTTONDOWN:
		fClicking = TRUE;
		break;
	case WM_LBUTTONUP:
		if(fClicking)
		{
			fClicking = FALSE;
			url->crLink = url->crVisited;
			
			InvalidateRect(hwnd, 0, 0);
			UpdateWindow(hwnd);

			if(url->szURL[0])
			{
				ShellExecute(NULL, "open", url->szURL, NULL, NULL, SW_SHOWNORMAL);
			}
			else
			{
				GetWindowText(hwnd, szWinText, sizeof szWinText);
				ShellExecute(NULL, "open", szWinText, NULL, NULL, SW_SHOWNORMAL);
			}
		}
		break;
	case WM_NCHITTEST:
		return HTCLIENT;
	case WM_DESTROY:
		CleanupURLControl();
		break;
	}

	return CallWindowProc(oldproc, hwnd, iMsg, wParam, lParam);
}

//
//	Create a URL control from the specified static control
//
//	crLink --  -1 for default
//
BOOL StaticToURLControl(HWND hDlg, UINT staticid, char *szURL, COLORREF crLink)
{
	HWND hwndCtrl = GetDlgItem(hDlg, staticid);
	
	URLCtrl *url = (URLCtrl *)malloc(sizeof(URLCtrl));
	
    SetWindowLong(hwndCtrl, GWL_STYLE, GetWindowLong(hwndCtrl, GWL_STYLE) | SS_NOTIFY);

	if(szURL)
		lstrcpy(url->szURL, szURL);
	else
		url->szURL[0] = '\0';

	if(crLink != -1) url->crLink = crLink;
	else             url->crLink = RGB(0,0,255);

	url->crVisited = RGB(128,0,128);

	url->oldproc = (WNDPROC)SetWindowLong(hwndCtrl, GWL_WNDPROC, (LONG)URLCtrlProc);

	SetWindowLong(hwndCtrl, GWL_USERDATA, (LONG)url);
	
	return TRUE;
}
