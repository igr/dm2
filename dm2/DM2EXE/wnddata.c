/* http://dm2.sf.net */
/* maintaince the list of the minimized windows */

#include "wnddata.h"
#include "util.h"

PWND_DATA wndData;				// array of all minimized windows

/**
 * Initializae windows data.
 */
void InitWndData() {
	wndData = NULL;
}


/**
 * Adds new WndData to the list.
 */
PWND_DATA NewWndData() {
	PWND_DATA pwnd;
	register PWND_DATA last;

	// allocate memory
	pwnd = halloc(sizeof(WND_DATA));
	if (pwnd == NULL) {
		return NULL;
	}
	ZeroMemory(pwnd, sizeof(WND_DATA));

	// find last wnddata in list and add new
	last = wndData;
	if (last == NULL) {
		wndData = pwnd;
		return pwnd;
	}
	while (last->next != NULL) {
		last = last->next;
	}
	last->next = pwnd;
	return pwnd;
}


/**
 * Adds new WndData to the list.
 *
 * @param hwnd   window to be minimzed.
 *
 * @return
 */
PWND_DATA NewWndDataEx(HWND hwnd) {
	PWND_DATA pwnd;

	pwnd = NewWndData();
	if (pwnd == NULL) {
		return NULL;
	}
	pwnd->hwnd = hwnd;
	
	pwnd->settings = GetWinSetting(hwnd);

	return pwnd;
}


/**
 * Returns WndData for a window. If window is not minimized, NULL is
 * returned.
 *
 * @param hwnd
 *
 * @return
 */
PWND_DATA GetWndData(HWND hwnd) {
	register PWND_DATA pwnd = wndData;

	while (pwnd != NULL) {
		if (pwnd->hwnd == hwnd) {
			return pwnd;
		}
		pwnd = pwnd->next;
	}
	return NULL;
}


/**
 * Returns WndData for floating wnd.
 */
PWND_DATA GetWndDataOfFloatingWnd(HWND floatingHwnd) {
	register PWND_DATA pwnd = wndData;
	while (pwnd != NULL) {
		if (pwnd->flico == floatingHwnd) {
			return pwnd;
		}
		pwnd = pwnd->next;
	}
	return NULL;
}




/**
 * Remove wnd from the list.
 */
extern BOOL IsWinNT;
void RemoveWndData(PWND_DATA pwnd) {

	if (pwnd->hbmp != NULL) {
		DeleteObject(pwnd->hbmp);
	}
	if (pwnd->icon != NULL) {
		DeleteObject(pwnd->icon);
	}
	if (pwnd->settings != NULL) {
		hfree(pwnd->settings);
	}
	if (pwnd->tooltip != NULL) {
		DestroyWindow(pwnd->tooltip);
	}

	if (pwnd == wndData) {
		// code by Weird(modification by flyfancy)
		wndData = wndData->next;
	} else {
		PWND_DATA prev;		// wire previous windata with the following

		prev = wndData;
		while (prev != NULL) {
			if (prev->next == pwnd) {
				prev->next = pwnd->next;
				break;
			}
			prev = prev->next;
		}
	}
	hfree(pwnd);
	if(IsWinNT) SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
}



/**
 * TEST: dump windows data.
 */
void DumpWndData(char *filename) {
	char tmp[1024];
	DWORD written;
	PWND_DATA pwnd = wndData;
	HANDLE testfile = NULL;
	int count;

	testfile = CreateFile(filename, FILE_ALL_ACCESS, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(testfile, 0, NULL, FILE_END);
	wsprintf(tmp, "\n\n---window data dump------------------\n");
	WriteFile(testfile, tmp, lstrlen(tmp), &written, NULL);

	count = 0;
	while (pwnd != NULL) {
		wsprintf(tmp, "[%i] %X -> %X\n", count, pwnd->hwnd, pwnd->next);
		WriteFile(testfile, tmp, lstrlen(tmp), &written, NULL);
		wsprintf(tmp, "icon.scrsh=%i\n", pwnd->settings->opacity);
		WriteFile(testfile, tmp, lstrlen(tmp), &written, NULL);
		pwnd = pwnd->next;
	}

	CloseHandle(testfile);
}
