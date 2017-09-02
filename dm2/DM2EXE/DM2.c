/*
	DM2
	coded by weird & flyfancy
	http://dm2.sf.net
*/

#define WINVER 0x5000

#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>
#include "../global.h"
#include "util.h"
#include "dm2WndProc.h"
#include "float.h"
#include "dm2.h"
#include "wnddata.h"
#include "../DM2DLL/DllMain.h"
#include "GetSetting.h"
#include "Plugins.h"

// ---------------------------------------------------------------- globals

HMODULE processHandle;		// process handle
HANDLE processHeap;			// process heap

char *inifile;				// full path to dm2 ini file
char *lang_file;			// full path to dm2 language file
//char *lang_file_eng;		// full path to dm2 english language file (global)
HICON hIcon;

PSHARED_DATA pshared;
DLLSHAREMEMORY dsm = {NULL, NULL, sizeof(SHARED_DATA), "DM2_SharedMem"};
pSetLayeredWindowAttributes SetLayeredWindowAttributesImp;

// ---------------------------------------------------------------- init

char dm2WndName[] = "DM2 Server";
char dm2ClassName[] = "DM2 Server class";
char dm2FlicoClassName[] = "DM2 flico class";

PFF_PLUGINS ffp;

char *DM2_Ver = "DM2 v1.23";
BOOL IsWinNT = FALSE;

//for VC++ application EntryPoint, DM2.exe have less size.
int EntryPoint() 
{ 
	register int result;
	OEP_ASM
	result= WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
	ExitProcess(result);
} 

/**
 * Registeres two classes and creates main message-only window.
 * Loads ini file.
 */
HWND InitApplication() {
	HWND hWnd;
	WNDCLASSEX wc;

 	InitWndData();

	// main class
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = DM2WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = processHandle;
	wc.hIcon		 = NULL;
	wc.hCursor		 = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName	 = NULL;
	wc.lpszClassName = dm2ClassName;
	wc.hIconSm		 = NULL;
	if (RegisterClassEx(&wc) == FALSE) {
		return NULL;
	}

	wc.style		 = CS_DBLCLKS;
	wc.lpfnWndProc	 = FloatWndProc;
	wc.lpszClassName = dm2FlicoClassName;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);		// must !!!

	if (RegisterClassEx(&wc) == FALSE) {
		return NULL;
	}

	if(IsWinNT)
	{
		hWnd = CreateWindow(dm2ClassName, dm2WndName, 0, -1, -1, 0, 0, 
 			HWND_MESSAGE, NULL, processHandle, NULL);
	}
	else
		hWnd = CreateWindow(dm2ClassName, dm2WndName, 0, -1, -1, 0, 0, 
			NULL, NULL, processHandle, NULL);
	
	if (hWnd == NULL) {
		return NULL;
	}

	ShowWindow(hWnd, SW_HIDE);
	return hWnd;
}


extern SGEN_DATA Gen_data;
extern SADV_DATA Adv_data;
extern PSFAV_DATA Fav_data;
extern PSCUSTOM_DATA Custom_data;
extern PSEXCEPT_DATA Except_data;
extern PSHK_DATA hk_data;
extern PSREC_DATA pRec;
extern PSAC_DATA Action_data;

extern void UnregisterAllHotKey();

//memtest module
#ifndef NDEBUG
int mem_test = 0;
#endif

/**
 * Application entry point.
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					 LPSTR lpCmdLine, int nShowCmd) {
	MSG msg;
	HANDLE user32_dll;
	HWND hwnd, exist;
	INITCOMMONCONTROLSEX iccex;
	char Ini[MAX_PATH];
	char Language[MAX_PATH];
	//char LanguageEng[MAX_PATH];
	char plugins[MAX_PATH];

	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osv);
	if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
		IsWinNT = TRUE;

	processHandle = GetModuleHandle(NULL);
	processHeap = GetProcessHeap();

	// find existing window
	exist = FindWindow(NULL, dm2WndName);

	// command line processing: exit
	if (strstr(lpCmdLine, "-exit") != NULL) {
		SendMessage(exist, WM_CLOSE, 0, 0);
		return 0;
	}
	// prevent duplicates
	if (exist != NULL) {
		return 0;
	}
	
	__try
	{
		// initialize common controls
		iccex.dwICC = ICC_WIN95_CLASSES;
		iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCommonControlsEx(&iccex);

		// create ini file path
		GetModuleFileName(NULL, Ini, MAX_PATH);
		lstrcpy(strrchr(Ini, '\\')+1, "dm2.ini");
		inifile = (char *)&Ini;
		GetGenSetting(inifile, &Gen_data);
		GetAdvSetting(inifile, &Adv_data);
		Custom_data = GetCustomSetting(inifile);
		Except_data = GetExceptSetting(inifile);
		Fav_data = GetFavSetting(inifile);
		hk_data = GetHotKeySetting(inifile);
		Action_data = GetActionSetting(inifile);
		pRec = LoadRecentList(inifile, Adv_data.FavRecNum);

		// create language path
		{
			char szLang[MAX_PATH];
			GetPrivateProfileString("settings", "Language", "", szLang, MAX_PATH, inifile);
			lang_file = Language;
			if (*szLang == '\0') {
				strcpy(szLang, "English.lang");	// default language file
			}
			GetModuleFileName(NULL, Language, MAX_PATH);
			stradd(stradd(strrchr(Language, '\\') + 1, "language\\"), szLang);

			//lang_file_eng = LanguageEng;
			//strcpy(lang_file_eng, lang_file);
			//stradd(strrchr(lang_file_eng, '\\') + 1, "English.lang");

			/*if(*szLang != '\0')
			{
				GetModuleFileName(NULL, Language, MAX_PATH);
				stradd(stradd(strrchr(Language, '\\')+1, "language\\"), szLang);
			}
			else {
				*Language = '\0';
			}*/
		}

		// create plugins path
		GetModuleFileName(NULL, plugins, MAX_PATH);
		lstrcpy(strrchr(plugins, '\\')+1, "plugins\\*.dll");
		ffp = LoadAllPlugins("dm2_get_interface", plugins);

		// create shared memory
		pshared = (PSHARED_DATA)CreateDllSharedMemory(&dsm);
		pshared->Pid = GetCurrentProcessId();
		pshared->IsWinNT = IsWinNT;

		//Get "SetLayeredWindowAttributes" function address
		user32_dll = LoadLibrary("user32.dll");
		if (IsWinNT && user32_dll != NULL) {
 			SetLayeredWindowAttributesImp = (pSetLayeredWindowAttributes) GetProcAddress(
				user32_dll, "SetLayeredWindowAttributes");
		} else {
			SetLayeredWindowAttributesImp = NULL;
		}

		hIcon = LoadIcon(processHandle, "appIcon");
		/* start */
		hwnd = InitApplication();
		if (hwnd == NULL)
 			return FALSE;

 		EnableDM2();

		if(IsWinNT) SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	__finally
	{
		//remove all data
		if(Adv_data.EnableHotKey)
			UnregisterHotKey(pshared->DM2wnd, HK_CMD);
		DisableDM2Menu();
		DisableFavMenu();
		DisableDM2();
		RemoveAllCustom(&Custom_data);
		RemoveAllExcept(&Except_data);
		RemoveAllFavMenu(&Fav_data);
		UnregisterAllHotKey(hk_data);
		RemoveAllHotKey(&hk_data);
		hfree(Action_data);
		RemoveRecentList(&pRec, Adv_data.FavRecNum, inifile);
		UnLoadAllPlugins(&ffp);
		DeleteSharedMemory(&dsm);

		#ifndef NDEBUG
		//memtest module
		if(mem_test)
		{
			wsprintf(Ini, "have %d block memory don't free!", mem_test);
			MessageBox(NULL, Ini, "Error", MB_ICONINFORMATION|MB_OK);
		}
		#endif
	}

	return msg.wParam;
}
