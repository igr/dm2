#include <windows.h>
#include <commctrl.h>
#include "../../dm2/tools.h"

#pragma comment(lib, "comctl32.lib")

#define MAXWIN 1024
#define MAX_PATH2 (MAX_PATH+1)
#define MAX_VDNAME 25
#define MAX_VDNAME2 (MAX_VDNAME+1)
#define IDM_VD1 (WM_USER+1)
//#define IDM_VD2 WM_USER+2
//#define IDM_VD3 WM_USER+3
//#define IDM_VD4 WM_USER+4
//#define IDM_VD5 WM_USER+5
//#define IDM_VD6 WM_USER+6
//#define IDM_VD7 WM_USER+7
#define IDM_VD8 (WM_USER+8)
#define ID_ST_VD1 (WM_USER+11)
#define ID_ST_VD8 (WM_USER+18)

typedef struct
{
	BOOL bEnable;
	int VDCount;
	BOOL bShowTray;
	char VDName[8][MAX_VDNAME2];
} VDOPT, *LPVDOPT;

typedef struct
{
	int WinCount;
	HWND *List;
} VDD, *LPVDD;

extern void InitVirtualDesktop();
extern void DestroyVirtualDesktop();
extern void SwitchVirtualDesktop(int WantToVD);
extern INT_PTR CALLBACK VDArrangeDlgProc(HWND, UINT, WPARAM, LPARAM);
