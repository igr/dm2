#include "wnddata.h"

extern LRESULT WINAPI FloatWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern void MinimizeWndToTray(HWND, HWND);
extern void RestoreWindow(HWND );
extern void RestoreTrayWindow(HWND, HWND);
extern HWND MinimizeWndToFloatingIcon(HWND);
extern void RestoreAllWindows(HWND);
extern void ShowTrayMenu(HWND, HWND);
extern void RestoreFloatWindow(HWND);
extern PWND_DATA HideWindow(HWND);