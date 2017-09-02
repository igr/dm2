/* http://dm2.sf.net */
#include <windows.h>
#include "../global.h"
#include "ModifySysMenu.h"
#include "DllMain.h"

HWND lastHit;
extern HHOOK mouseHook;

/**
 * MouseHook procedure, intercepts mouse right clicks on minimize/maximize button.
 */
LRESULT CALLBACK MouseHookProc(int code, WPARAM wParam, LPARAM lParam) {
	HWND hwnd;
	UINT HitTestCode;
	if (code < 0) {
		return CallNextHookEx(mouseHook, code, wParam, lParam);
	}

	hwnd = ((MOUSEHOOKSTRUCT*)lParam)->hwnd;
	HitTestCode = ((MOUSEHOOKSTRUCT*)lParam)->wHitTestCode;
	if (HitTestCode == HTREDUCE || HitTestCode == HTZOOM || HitTestCode == HTCLOSE) {
		if (getCompatibleSystemMenu(hwnd) != NULL) {
			if (wParam == WM_NCRBUTTONDOWN || wParam == WM_NCMBUTTONDOWN) {
				lastHit = hwnd;
			} else if (wParam == WM_NCRBUTTONUP || wParam == WM_NCMBUTTONUP) {
				HWND activeWnd = hwnd;
				if (lastHit == activeWnd) {
					DWORD dwParam = 0;
					lastHit = NULL;
					
					SetForegroundWindow(pshared->DM2wnd);
					if(wParam == WM_NCRBUTTONUP) dwParam |= IDM_ACTION_RCLICK;
					else if(wParam == WM_NCMBUTTONUP) dwParam |= IDM_ACTION_MCLICK;

					if(HitTestCode == HTREDUCE) dwParam |= IDM_ACTION_MIN;
					else if(HitTestCode == HTZOOM) dwParam |= IDM_ACTION_MAX;
					else if(HitTestCode == HTCLOSE) dwParam |= IDM_ACTION_CLOSE;

					if(GetKeyState(VK_SHIFT)<0) dwParam |= IDM_ACTION_SHIFT;
					else if(GetKeyState(VK_CONTROL)<0) dwParam |= IDM_ACTION_CTRL;
					else if(GetKeyState(VK_MENU)<0) dwParam |= IDM_ACTION_ALT;
					
					PostMessage(pshared->DM2wnd, WM_DM2_CMD, dwParam, 
						(LPARAM)activeWnd);
				}
			}
		}
	}

	return CallNextHookEx(mouseHook, code, wParam, lParam);
}

