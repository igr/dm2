#ifndef GLOBAL_H
#define GLOBAL_H

//for vc++ compiler. dm2 have less size.
#ifdef NDEBUG
#pragma comment(lib, "msvcrt.lib")
#pragma comment(linker, "/NODEFAULTLIB:LIBC.lib")
#else
#pragma comment(lib, "msvcrtd.lib")
#pragma comment(linker, "/NODEFAULTLIB:LIBCD.lib")
#endif

//[flyfancy&Weird], 0
//0x5B, 0x66, 0x6C, 0x79, 0x66, 0x61, 0x6E, 0x63, 0x79, 0x26, 
//0x57, 0x65, 0x69, 0x72, 0x64, 0x5D, 0x00
#define OEP_ASM __asm jmp OEP \
__asm _emit 0x5B __asm _emit 0x66 __asm _emit 0x6C \
__asm _emit 0x79 __asm _emit 0x66 __asm _emit 0x61 \
__asm _emit 0x6E __asm _emit 0x63 __asm _emit 0x79 \
__asm _emit 0x26 __asm _emit 0x57 __asm _emit 0x65 \
__asm _emit 0x69 __asm _emit 0x72 __asm _emit 0x64 \
__asm _emit 0x5D __asm _emit 0x00 __asm OEP:

// misc

#define MAX_PATH2 (MAX_PATH+1)

#define IDM_SYS_SEPARATOR	499
#define IDM_SYS_TRAY		500
#define IDM_SYS_TOP			501
#define IDM_SYS_FLICO		502
//Note: Opacity command used IDM_SYS_OPA+1 ~ IDM_SYS_OPA+10 !
#define IDM_SYS_OPA			503
#define IDM_SYS_DOPA		513
//new Priority menu (v1.20)
#define IDM_SYS_PRIO		514
#define IDM_SYS_PRIO_I		515
#define IDM_SYS_PRIO_B		516
#define IDM_SYS_PRIO_N		517
#define IDM_SYS_PRIO_A		518
#define IDM_SYS_PRIO_H		519
#define IDM_SYS_PRIO_R		520
#define IDM_IS_SYS_SKIP		(WM_USER+5)		//detect skip subclassing dialog.

//for MultiLanguage
#define IDM_NEED_LANG		(WM_USER+4)

#define CLASSNAME_SIZE 		16

//for fav menu
#define TB_FAVBUTTON_ID		0x173
//message for fav menu
#define IDM_IS_FAV_SKIP		(WM_USER+2)		//detect skip subclassing dialog.
#define IDM_NEED_FAV		(WM_USER+3)		//wParam == HWND
#define IDM_FAV_INSTALL		0x646C			//use for fav menu need install.
#define IDM_FAV_UNINSTALL	0x646D			//use for fav menu need uninstall.
#define IDM_TBB_DEF			(WM_USER+0x100)	//Def Commmand
#define IDM_TBB_APP			(WM_USER+0x700)	//App Command
#define IDM_TBB_CST			(WM_USER+0x1300)//Custom Command

// fav menu add or get the recent
#define IDM_ADD_RECENT		(WM_USER+6)		//copy path to PSHARED_DATA->szRecent, wparam and lparam is NULL.
#define IDM_GET_RECENT		(WM_USER+7)		//dm2exe Send WM_COPYDATA to dm2dll. wParam is Dest hWnd.
#define IDM_RECENT_START	(WM_USER+0x90)	//define 16 recent item commandid
#define IDM_RECENT_END		(WM_USER+0x9F)

// for exception
#define EXCEPT_FAV			1
#define EXCEPT_OTHER		2
#define EXCEPT_SYSMENU		4

#define MAX_MENU_LEN 30
// shared memory
typedef struct {
	HWND DM2wnd;					// main DM2 window
	DWORD Pid;						// dm2 process id.
	BOOL IsWinNT;
	char szMenu[MAX_MENU_LEN+1];	// for translate system menu text.
	char szRecent[MAX_PATH2];		// for add to recent
} SHARED_DATA, *PSHARED_DATA;

extern PSHARED_DATA pshared;		// both exe and dll has this variable



// messages
#define WM_DM2_CMD			(WM_USER + 1)
#define IDM_ACTION_RCLICK	0x01000000
#define IDM_ACTION_MCLICK	0x02000000
#define IDM_ACTION_SHIFT	0x00010000
#define IDM_ACTION_ALT		0x00020000
#define IDM_ACTION_CTRL		0x00040000
#define IDM_ACTION_MIN		0x00000001
#define IDM_ACTION_MAX		0x00000002
#define IDM_ACTION_CLOSE	0x00000004

#endif
