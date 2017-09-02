#ifndef WNDDATA_H
#define WNDDATA_H

#include <windows.h>
#include "dm2.h"

//float window can have all member, none but float window to have flico member.
//opacity window only have hwnd member and settings member
//tray window only have icon member and hwnd member.
//hide window only have hwnd member and none settings/hbmp member.
typedef struct {
	HWND hwnd;					// HWND of the minimized window (either in tray of floating)
	HWND flico;					// HWND of floating icon, NULL if it is tray icon
	HICON icon;					// HICON of the floating icon or tray icon
	HBITMAP hbmp;				// window screenshot, or NULL if not used
	HWND owner;					// HWND of the visible owner, otherwise NULL
	HWND tooltip;				// HWND of floating icon ToolTip control, when floating icon restore it need destroy.
	void* next;					// pointer to next structure, NULL if end
	PSGEN_DATA settings;		// all joined window settings
} WND_DATA, *PWND_DATA;


extern void InitWndData();
extern PWND_DATA NewWndDataEx();
extern PWND_DATA GetWndData(HWND);
extern PWND_DATA GetWndDataOfFloatingWnd(HWND);
extern void RemoveWndData(PWND_DATA);


extern void DumpWndData(char *);


#endif

