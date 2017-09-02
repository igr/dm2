/* http://dm2.sf.net */
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "../global.h"
#include "../tools.h"
#include "hook.h"
#include "ModifySysMenu.h"

//for vc++ compiler. dm2.dll have less size.
#ifdef NDEBUG
#pragma optimize("gsy", on)
#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/IGNORE:4078 /IGNORE:4089")
#pragma comment(linker, "/RELEASE")
#pragma comment(linker, "/merge:.rdata=.data")
//#pragma comment(linker, "/merge:.text=.data")
#pragma comment(linker, "/merge:.reloc=.data")
#if _MSC_VER >= 1000
#pragma comment(linker, "/FILEALIGN:0x200")
#endif
#endif

HINSTANCE hdll;
HHOOK mouseHook = NULL;
HHOOK MenuHook = NULL;
HHOOK msgHook = NULL;
HHOOK Dlghook = NULL;

DLLSHAREMEMORY dsm = {NULL, NULL, sizeof(SHARED_DATA), "DM2_SharedMem"};
PSHARED_DATA pshared;

//-------------------------------------------------------------------------
typedef DWORD (WINAPI *GETMODULEFILENAMEEX)(
  HANDLE hProcess,    // handle to the process
  HMODULE hModule,    // handle to the module
  LPTSTR lpFilename,  // buffer that receives the path
  DWORD nSize         // size of the buffer
);
typedef BOOL (WINAPI *OPENPROCESSTOKEN)(
  HANDLE ProcessHandle, // handle to process
  DWORD DesiredAccess,  // desired access to process
  PHANDLE TokenHandle   // pointer to handle of open access token
);
typedef BOOL (WINAPI *LOOKUPPRIVILEGEVALUE)(
  LPCTSTR lpSystemName,
                   // address of string specifying the system
  LPCTSTR lpName,  // address of string specifying the privilege
  PLUID lpLuid     // address of locally unique identifier
);
typedef BOOL (WINAPI *ADJUSTTOKENPRIVILEGES)(
  HANDLE TokenHandle,  // handle to token that contains privileges
  BOOL DisableAllPrivileges,
                       // flag for disabling all privileges
  PTOKEN_PRIVILEGES NewState,
                       // pointer to new privilege information
  DWORD BufferLength,  // size, in bytes, of the PreviousState buffer
  PTOKEN_PRIVILEGES PreviousState,
                       // receives original state of changed 
                       // privileges
  PDWORD ReturnLength  // receives required size of the 
                       // PreviousState buffer
);
//-------------------------------------------------------------------------

BOOL APIENTRY DllMain(HINSTANCE hDll, DWORD fdwReason, LPVOID lpvReserved) {
	OEP_ASM
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH: {
				hdll = hDll;
				pshared = (PSHARED_DATA)CreateDllSharedMemory(&dsm);

				if(pshared->IsWinNT)
				{
					OPENPROCESSTOKEN pOpenProcessToken;
					LOOKUPPRIVILEGEVALUE pLookupPrivilegeValue;
					ADJUSTTOKENPRIVILEGES pAdjustTokenPrivileges;
					HANDLE hToken;
					TOKEN_PRIVILEGES Privileges;
					LUID luid;

					HINSTANCE hDll = LoadLibrary("ADVAPI32.DLL");
					pOpenProcessToken =(OPENPROCESSTOKEN)GetProcAddress(hDll, 
						"OpenProcessToken");
					pLookupPrivilegeValue =(LOOKUPPRIVILEGEVALUE)GetProcAddress(hDll, 
						"LookupPrivilegeValueA");
					pAdjustTokenPrivileges =(ADJUSTTOKENPRIVILEGES)GetProcAddress(hDll, 
						"AdjustTokenPrivileges");
					
					pOpenProcessToken(GetCurrentProcess(), 
						TOKEN_ADJUST_PRIVILEGES, &hToken);
					Privileges.PrivilegeCount = 1;
					pLookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);
					Privileges.Privileges[0].Luid = luid;
					Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
					pAdjustTokenPrivileges(hToken, FALSE, &Privileges, 0, NULL, NULL);
				}
			}
			break;
		case DLL_PROCESS_DETACH:
			DeleteSharedMemory(&dsm);
			break;
	}
	return TRUE;
	UNREFERENCED_PARAMETER(lpvReserved);
}

/**
 * Set all hooks.
 */
// rename SetHook function to EnableDM2
void EnableDM2(void)
{
	if (mouseHook == NULL)
		mouseHook = SetWindowsHookEx(WH_MOUSE, MouseHookProc, hdll, 0);
}

/**
 * Remove all DM2 hooks.
 */
// rename Unhook function to DisableDM2
void DisableDM2(void)
{
	if (mouseHook) 
	{
		UnhookWindowsHookEx(mouseHook);
		mouseHook = NULL;
	}
}

//Enable Modify Caption Menu
void EnableDM2Menu(void)
{
	if (MenuHook == NULL) {
		MenuHook = SetWindowsHookEx(WH_CALLWNDPROCRET, ProcMenuRetHookProc, hdll, 0);
		EnumWindows(ModifyWndCaptionMenuCallback, TRUE);
	}
	if(msgHook == NULL)
		msgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgHookProc, hdll, 0);
}

//Disable Modify Caption Menu
void DisableDM2Menu(void)
{
	if (MenuHook) {
		UnhookWindowsHookEx(MenuHook);
		MenuHook = NULL;
		EnumWindows(ModifyWndCaptionMenuCallback, FALSE);
	}
	if(msgHook)
	{
		UnhookWindowsHookEx(msgHook);
		msgHook = NULL;
	}
}

extern BOOL CALLBACK FavMenuCallback(HWND hwnd, LPARAM lParam);
//Enable fav menu
void EnableFavMenu(void)
{
	if(Dlghook == NULL)
	{
		Dlghook = SetWindowsHookEx(WH_CALLWNDPROCRET, dlgHookProc, hdll, 0);
		EnumWindows(FavMenuCallback, TRUE);
	}
}

//Disable fav menu
void DisableFavMenu(void)
{
	if (Dlghook) {
		EnumWindows(FavMenuCallback, FALSE);
		UnhookWindowsHookEx(Dlghook);
		Dlghook = NULL;
	}
}
