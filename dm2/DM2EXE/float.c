/* http://dm2.sf.net */
#include <windows.h>
#include "dm2.h"
#include "wnddata.h"
#include "util.h"

// ---------------------------------------------------------------- helpers

/**
 * Hides a window and stores all its data to WndData list.
 */
PWND_DATA HideWindow(HWND hwnd) {
	PWND_DATA pwnd = NULL;
	PSGEN_DATA pdata = NULL;
	HWND owner;

	//detect dm2 window
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if(pid == GetCurrentProcessId())
	{
		return NULL;
	}

	pwnd = GetWndData(hwnd);

	if (pwnd != NULL) {
		return NULL;			// window is already process
	}

	pwnd = NewWndDataEx(hwnd);	// add new window data
	if (pwnd == NULL) {
		return NULL;
	}

	pdata = pwnd->settings;
	if (pdata->screenshots == 1) {
		pwnd->hbmp = CreateWndScreenShoot(hwnd, pdata->shWidth, pdata->shHeight, HALFTONE, pdata->stretch);
	} else {
		pwnd->hbmp = NULL;
	}

	// hide
	ShowWindow(hwnd, SW_HIDE);
	owner = GetWindow(hwnd, GW_OWNER);
	if (owner != NULL) {
		if (IsWindowVisible(owner) == FALSE) {
			owner = NULL;
		}
	}
	if (owner != NULL) {
		ShowWindow(owner, SW_HIDE);
	}
	pwnd->owner = owner;
	return pwnd;
}

/**
 * Restores a hidden window.
 */
void RestoreWindow(HWND hwnd) {
	HWND owner;
	PWND_DATA wnd;

	wnd = GetWndData(hwnd);
	if (wnd == NULL) {
		return;		// window is not minimized
	}

	ShowWindow(hwnd, SW_SHOW);
	owner = wnd->owner;
	if (owner != NULL) {
		ShowWindow(owner, SW_SHOW);
	}

	SetForegroundWindow(hwnd);
	RemoveWndData(wnd);
	return;
}



// ---------------------------------------------------------------- tray


/**
 * Restores tray window.
 */
void RestoreTrayWindow(HWND dm2wnd, HWND hidden) {
	RestoreWindow(hidden);
	DeleteTaskBarIcon(dm2wnd, (int) hidden);
}


/**
 * Minimizes window to tray.
 */
BOOL MinimizeWndToTray(HWND dm2wnd, HWND hwnd) {
	char winname[MAX_WIN_TITLE];
	PWND_DATA pwnd = HideWindow(hwnd);

	if (pwnd == NULL) {
		return FALSE;
	}

	pwnd->icon = GetWindowIcon(hwnd, FALSE);
	if (pwnd->icon == NULL) {
		pwnd->icon = LoadIcon(NULL, IDI_WINLOGO);
	}

	GetWinTitle(hwnd, winname);
	AddTaskBarIcon(dm2wnd, pwnd->icon, winname, (int) hwnd);

	//not need Setting
	if(pwnd->settings)
	{
		hfree(pwnd->settings);
		pwnd->settings = NULL;
	}

	return TRUE;
}


// for Multi-Language
#include "MultiLanguage.h"
extern char *lang_file;

/**
 * Show tray menu for minimized window.
 */
void ShowTrayMenu(HWND dm2hwnd, HWND hwnd) {
	HMENU menu;
	HMENU menuPopup;
	POINT p;
	PWND_DATA pwnd;

	GetCursorPos(&p);
	menu = LoadMenu(processHandle, "DM2_TRAY_POPUP");
	TranslateMenuLanguage(lang_file, menu, "MenuTray");
	if (menu == NULL) {
		return;
	}
	menuPopup = GetSubMenu(menu, 0);

	pwnd = GetWndData(hwnd);
	if (pwnd->hbmp != NULL) {
		AppendMenu(menuPopup, MF_SEPARATOR, 0, NULL);
		AppendMenu(menuPopup, MF_BITMAP, IDM_RESTORE_TRAYWND, (PTSTR) (LONG) pwnd->hbmp);
	}

	SetForegroundWindow(dm2hwnd);
	TrackPopupMenuEx(menuPopup, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, p.x, p.y, dm2hwnd, NULL);
	PostMessage(dm2hwnd, WM_NULL, 0, 0);
	DestroyMenu(menu);
}




// ---------------------------------------------------------------- floating


/**
 * Restore floating window.
 * code by Weird
 *
 * flyfancy modification code, avoid window move out screen.
 */
extern SADV_DATA Adv_data;
void RestoreFloatWindow(HWND hwndFloat) {
	HWND hwnd;
	RECT r2;
	RECT r;
	PWND_DATA pwnd;
	RECT rd;
	int x, y;
	GetDesktopRect(&rd);

	pwnd = GetWndDataOfFloatingWnd(hwndFloat);
	if (pwnd == NULL) {
		return;
	}
	hwnd = pwnd->hwnd;

	GetWindowRect(hwnd, &r);
	if (IsZoomed(hwnd) == FALSE) {				// maximized windows do not follows floating icons
		if (pwnd->settings->follow == 1) {		// disable follow?
			GetWindowRect(hwndFloat, &r2);
			switch (pwnd->settings->initpos) {
				case 0:
					x = r2.left;
					y = r2.top;
					break;
				case 1:
					x = r2.left - (r.right - r.left - 38);
					y = r2.top;
					break;
				case 2:
					x = r2.left;
					y = r2.top - (r.bottom - r.top - 38);
					break;
				case 3:
					x = r2.left - (r.right - r.left - 38);
					y = r2.top - (r.bottom - r.top - 38);
					break;
				case 4:
					x = r2.left - ((r.right - r.left) / 2 - 19);
					y = r2.top - ((r.bottom - r.top) / 2 - 19);
					break;
			}
			if(Adv_data.LockWinArea)
			{
				r.left = setIntRange(x, rd.left, rd.right - (r.right - r.left));
				r.top = setIntRange(y, rd.top, rd.bottom - (r.bottom - r.top));
			}
			else
			{
				r.left = x;
				r.top = y;
			}
		}
	}
	
	SetWindowPos(hwnd, HWND_TOP, r.left, r.top, 0, 0, SWP_NOSIZE);
	RestoreWindow(hwnd);
}


extern BOOL IsWinNT;
/* Shows FLICO menu */
void ShowFlicoMenu(HWND flico) {
	HMENU menu;
	HMENU menuPopup;
	POINT p;
	int op;
    HBITMAP hBitmap;
	PWND_DATA pwnd;

	pwnd = GetWndDataOfFloatingWnd(flico);

	GetCursorPos(&p);
	menu = LoadMenu(processHandle, "DM2_FLICO_POPUP");
	if (menu == NULL) {
		return;
	}
	menuPopup = GetSubMenu(menu, 0);
	TranslateMenuLanguage(lang_file, menuPopup, "MenuFloat");
	if (pwnd->settings->ontop == 1) {
		CheckMenuItem(menuPopup, IDM_FLOAT_ONTOP, MF_CHECKED | MF_BYCOMMAND);
	}
	
	if(pwnd->settings->enableopa && IsWinNT) {
		op = pwnd->settings->opacity;
		if (op >= 10) {
			op = op / 10 - 1;
			CheckMenuItem(menuPopup, IDM_OP_10 + op, MF_CHECKED | MF_BYCOMMAND);
		} else {
			CheckMenuItem(menuPopup, IDM_OP_100, MF_CHECKED | MF_BYCOMMAND);
		}
	}
	else {
		DeleteMenu(menuPopup, 2, MF_BYPOSITION);
	}

	hBitmap = pwnd->hbmp;
	if (hBitmap != NULL) {
		AppendMenu(menuPopup, MF_SEPARATOR, 0, NULL);
		AppendMenu(menuPopup, MF_BITMAP, IDM_RESTORE_TRAYWND, (PTSTR) (LONG) hBitmap);
	}

	SetForegroundWindow(flico);
	TrackPopupMenuEx(menuPopup, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_HORIZONTAL | TPM_RIGHTBUTTON, p.x, p.y, flico, NULL);
	PostMessage(flico, WM_NULL, 0, 0);
	DestroyMenu(menu);
}

void ToggleOnTop(HWND flico) {
	PWND_DATA pwnd;
	pwnd = GetWndDataOfFloatingWnd(flico);
	if (pwnd->settings->ontop == 1) {
		pwnd->settings->ontop = 0;
		SetWindowOnTop(flico, 0);
	} else {
		pwnd->settings->ontop = 1;
		SetWindowOnTop(flico, 1);
	}
}


void SetOpacity(HWND flico, int op) {
	PWND_DATA pwnd;
	pwnd = GetWndDataOfFloatingWnd(flico);
	if(IsWinNT && SetWindowOpacity)
	{
		SetWindowOpacity(flico, (op * 255) / 100);
		pwnd->settings->opacity = op;
	}
}



/**
 * Float windows procedure.
 */
extern HICON hIcon;
POINTS p;
LRESULT WINAPI FloatWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CREATE: {
			int op;
			HWND currentWnd = (HWND) ((LPCREATESTRUCT)lParam)->lpCreateParams;
			PWND_DATA pwnd;
			HICON hicon;
			char winname[MAX_WIN_TITLE];

			pwnd = HideWindow(currentWnd);
			if(pwnd == NULL)
			{
				DestroyWindow(hWnd);
				return TRUE;
			}

			pwnd->flico = hWnd;
			SetRelativeWindowPos(hWnd, currentWnd, pwnd->settings->initpos, 
				pwnd->settings->ontop, FLOATWIN_PIXEL - 
				(pwnd->settings->ficon_size*8));
			hicon = GetWindowIcon(currentWnd, TRUE);
			if (hicon == NULL) {
				hicon = hIcon;
			}
			pwnd->icon = hicon;
			op = pwnd->settings->opacity;

			if (op < 0 || !pwnd->settings->enableopa) {
				op = 0;
			} else {
				SetWindowOpacity(hWnd, (op * 255) / 100);
			}

			GetWinTitle(currentWnd, winname);
			pwnd->tooltip = CreateTooltip(hWnd, winname, 0, 0, NULL);
			return 0;
		}

		case WM_PAINT: {
			HDC hdc;
			PAINTSTRUCT ps;
			HBRUSH hbrush;
			HPEN hpen;
			HBRUSH holdbrush;
			HPEN holdpen;
			PWND_DATA pwnd;
			int fsize;
			pwnd = GetWndDataOfFloatingWnd(hWnd);
			if (pwnd == NULL) {
				return 0;
			}
			hdc = BeginPaint(hWnd, &ps);
			hbrush = CreateSolidBrush(pwnd->settings->bgcolor);
			hpen = CreatePen(PS_SOLID, FLOATWIN_DORDER, pwnd->settings->bcolor);
			holdpen = SelectObject(hdc, hpen);
			holdbrush = SelectObject(hdc, hbrush);
			fsize = FLOATWIN_PIXEL - (pwnd->settings->ficon_size*8);
			Rectangle(hdc, 0, 0, fsize, fsize);
			DrawIconEx(hdc, FLOATWIN_ICON_POS, FLOATWIN_ICON_POS, 
				pwnd->icon, fsize - FLOATWIN_ICON_POS*2, 
				fsize - FLOATWIN_ICON_POS*2, 0, NULL, DI_NORMAL);
			SelectObject(hdc, holdpen);
			SelectObject(hdc, holdbrush);
			DeleteObject(hpen);
			DeleteObject(hbrush);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_LBUTTONDOWN:				// left click and drag wnd
			{
				PWND_DATA pwnd;
				pwnd = GetWndDataOfFloatingWnd(hWnd);
				if (pwnd == NULL) return FALSE;
				if(pwnd->settings->move)
				{
					SetCapture(hWnd);
					p = MAKEPOINTS(lParam);
				}
				else
					SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
			}
			break;
		case WM_MOUSEMOVE:
 			if(wParam == MK_LBUTTON)
			{
				POINT point;
				GetCursorPos(&point);
				SetWindowPos(hWnd, NULL, point.x - p.x, point.y - p.y, 
					0, 0, SWP_NOSIZE);
			}
			break;
		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_RBUTTONUP:					// right click menu
			ShowFlicoMenu(hWnd);
			return 0;

		case WM_CLOSE:
		case WM_LBUTTONDBLCLK:
            RestoreFloatWindow(hWnd);
			DestroyWindow(hWnd);
			return 0;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_DM2:
					SendMessage(pshared->DM2wnd, WM_COMMAND, IDM_DM2, 0);
					return 0;
				case IDM_RESTORE_TRAYWND:
					RestoreFloatWindow(hWnd);
					DestroyWindow(hWnd);
					return 0;
				case IDM_FLOAT_ONTOP:
					ToggleOnTop(hWnd);
					return 0;
			}
			if ((LOWORD(wParam) >= IDM_OP_10) && (LOWORD(wParam) <= IDM_OP_100)){
				int op = (LOWORD(wParam) - IDM_OP_10 + 1) * 10;
				SetOpacity(hWnd, op);
				return 0;
			}
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/**
 * Minimize window to a floating icon.
 */
HWND MinimizeWndToFloatingIcon(HWND hwnd) {
	PWND_DATA pwnd = GetWndData(hwnd);
	if (pwnd != NULL) {
		return NULL;			// window is already process
	}

	return CreateWindowEx(WS_EX_TOOLWINDOW, dm2FlicoClassName, NULL, WS_POPUP, -50, -50, 
		0, 0, NULL, NULL, processHandle, hwnd);
}


// ---------------------------------------------------------------- all

extern PWND_DATA wndData;

/**
 * Restore all windows, one by one.
 *
 * @param hWnd   DM2 hwnd
 */
void RestoreAllWindows(HWND dm2hwnd) {
	register PWND_DATA last = wndData;
	while (last != NULL) {
		if(IsBadReadPtr(last, sizeof(PWND_DATA)))
			break;
		if (last->flico) {
			SendMessage(last->flico, WM_CLOSE, 0, (LPARAM) NULL);
		} else if (last->settings) {
			SetWindowOpacity(last->hwnd, 255);
			RemoveWndData(last);
		} else if (last->icon) {
			RestoreTrayWindow(dm2hwnd, last->hwnd);
		} else if (last->settings == NULL) {
			RestoreWindow(last->hwnd);
		}
		last = last->next;
	}
}


