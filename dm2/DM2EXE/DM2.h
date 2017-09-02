#ifndef DM2_H
#define DM2_H

#include "../global.h"
#include "../tools.h"
#include "resource.h"
#include "GetSetting.h"

#ifdef NDEBUG
#pragma comment(lib, "..\\DM2DLL\\Release\\DM2.lib")
//=====for vc++ compiler. dm2.dll have less size.=====
#pragma optimize("gsy", on)
#pragma comment(linker, "/ENTRY:EntryPoint")
#pragma comment(linker, "/IGNORE:4078 /IGNORE:4089")
#pragma comment(linker, "/RELEASE")
#pragma comment(linker, "/merge:.rdata=.data")
//#pragma comment(linker, "/merge:.text=.data")
#pragma comment(linker, "/merge:.reloc=.data")
#if _MSC_VER >= 1000
#pragma comment(linker, "/FILEALIGN:0x200")
#endif
//====================================================
#else
#pragma comment(lib, "..\\DM2DLL\\Debug\\DM2.lib")
#endif

typedef BOOL (WINAPI *pSetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD);

extern HMODULE processHandle;
extern HANDLE processHeap;
extern HMODULE dllHandle;
extern char *inifile;
extern char dm2FlicoClassName[];
extern pSetLayeredWindowAttributes SetLayeredWindowAttributesImp;

#define MAX_WIN_TITLE 64

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED	0x080000
#endif

#define LWA_ALPHA       0x00000002


#define DM2_TRAY_MSG	WM_APP + 2


#define IDM_DM2				100
#define IDM_RESTORE_ALL		101
#define IDM_QUIT			102
#define IDM_RESTORE_TRAYWND	103
#define IDM_FLOAT_ONTOP		104
#define IDM_OP_10			105
#define IDM_OP_20			106
#define IDM_OP_30			107
#define IDM_OP_40			108
#define IDM_OP_50			109
#define IDM_OP_60			110
#define IDM_OP_70			111
#define IDM_OP_80			112
#define IDM_OP_90			113
#define IDM_OP_100			114



#endif
