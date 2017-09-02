#include <windows.h>
#include "../dm2Plugins.h"
#include "../../tools.h"

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

#ifdef NDEBUG
#pragma comment(lib, "msvcrt.lib")
#pragma comment(linker, "/NODEFAULTLIB:LIBC.lib")
#else
#pragma comment(lib, "msvcrtd.lib")
#pragma comment(linker, "/NODEFAULTLIB:LIBCD.lib")
#endif

typedef struct
{
	int w;
	int h;
	HWND hWnd;
	void * next;
}WINORGDATA, *LPWINORGDATA;

typedef struct {
	HWND hwndServer;
} SHARED_DATA, *PSHARED_DATA;


LPWINORGDATA OrgData = NULL;

LPWINORGDATA GetNewData()
{
	LPWINORGDATA New = (LPWINORGDATA)malloc(sizeof(WINORGDATA));
	memset(New, 0, sizeof(WINORGDATA));
	if(OrgData == NULL)
		OrgData = New;
	else
	{
		LPWINORGDATA Last = OrgData;
		while(Last->next)
		{
			Last = Last->next;
		}
		Last->next = New;
	}
	return New;
}

void freeData()
{
	LPWINORGDATA Last;
	while(OrgData)
	{
		Last = OrgData->next;
		SetWindowPos(OrgData->hWnd, HWND_TOP, 0, 0, OrgData->w, OrgData->h, SWP_NOMOVE);
		free(OrgData);
		OrgData = Last;
	}
}

//use FindOrgData get the LPWINORGDATA, need free it.
LPWINORGDATA FindOrgData(HWND hwnd)
{
	LPWINORGDATA Find = NULL;
	LPWINORGDATA last = OrgData, Pre = OrgData;
	while(last)
	{
		if(last->hWnd == hwnd)
		{
			Find = last;
			if(last == OrgData)
				OrgData = last->next;
			else
				Pre->next = last->next;
			break;
		}
		Pre = last;
		last = last->next;
	}
	return Find;
}

DLLSHAREMEMORY dsm = {NULL, NULL, sizeof(SHARED_DATA), "DM2_RollWindow_SharedMem"};
PSHARED_DATA pshared;

HHOOK hHook = NULL;
LRESULT CALLBACK WndProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		CWPSTRUCT  *pct = (CWPSTRUCT *)lParam;

		if(pct->message == WM_CLOSE)
			SendMessage(pshared->hwndServer, WM_USER, (WPARAM)pct->hwnd, 0);
	}

	return CallNextHookEx(hHook, code, wParam, lParam);
}

HWND hServer;
LRESULT WINAPI RWProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(msg == WM_USER)
	{
		LPWINORGDATA pData = FindOrgData((HWND)wParam);
		if(pData)
		{
			SetWindowPos(pData->hWnd, HWND_TOP, 0, 0, pData->w, pData->h, SWP_NOMOVE);
			free(pData);
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

char *RWServer = "DM2_Plugin_RollWindow_Server";
HINSTANCE hPlugin;
DWORD WINAPI RunServer(LPVOID lpParameter)
{
	MSG msg;
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = RWProc;
	wc.hInstance	 = hPlugin;
	wc.lpszClassName = RWServer;
	if (RegisterClassEx(&wc) == FALSE) {
		return FALSE;
	}
	hServer = CreateWindow(RWServer, "", 0, -1, -1, 0, 0, 
		NULL, NULL, hPlugin, NULL);
	pshared->hwndServer = hServer;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}

HANDLE hThread = NULL;
HANDLE hMutex = NULL;
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hPlugin = hModule;
			pshared = (PSHARED_DATA)CreateDllSharedMemory(&dsm);
			break;
		case DLL_PROCESS_DETACH:
			DeleteSharedMemory(&dsm);
			break;
	}
	return TRUE;
}

void ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "RollWindows plugins v0.02 beta\r--Roll Up or UnRoll windows, like the Mac OS.\r\rCode by flyfancy. 2005.09", "About",
		MB_OK|MB_ICONINFORMATION);
}

BOOL PluginsState(void)
{
	return TRUE;
}

void InitPlugins(char *szini)
{
	DWORD tid;
	hThread = CreateThread(NULL, 0, RunServer, NULL, 
		NORMAL_PRIORITY_CLASS, &tid);
	hHook = SetWindowsHookEx(WH_CALLWNDPROC, WndProc, hPlugin, 0);
}

void ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	LPWINORGDATA pData = FindOrgData(hwnd_dm2);
	if(bUnNeed)
	{
		UnhookWindowsHookEx(hHook);
		ReleaseMutex(hMutex);
		DestroyWindow(hServer);
		TerminateThread(hThread, TRUE);
		CloseHandle(hThread);
		freeData();
		return;
	}

	if(pData)
	{
		SetWindowPos(hwnd_dm2, HWND_TOP, 0, 0, pData->w, pData->h, SWP_NOMOVE);
		free(pData);
	}
	else
	{
		RECT rect;
		pData = GetNewData();
		pData->hWnd = hwnd_dm2;
		GetWindowRect(hwnd_dm2, &rect);
		pData->w = rect.right-rect.left;
		pData->h = rect.bottom - rect.top;
		SetWindowPos(hwnd_dm2, HWND_TOP, 0, 0, rect.right-rect.left, 0, SWP_NOMOVE);
	}
}

static char *ver = "RollWindows v0.02 beta";
static char *cmd = "dm2_ext_cmd_rollwindows";
static char *cmdcmt = "Roll Up/UnRoll Windows (Plugins)";

void dm2_get_interface(PFF_PLUGINS ffp)
{
	ffp->About = ShowAbout;
	ffp->Execute = ExecPlugins;
	ffp->State = PluginsState;
	ffp->InitPlugins = InitPlugins;
	ffp->pName = ver;
	ffp->pComments = cmdcmt;
	ffp->pCmd = cmd;
	ffp->PluginsCategory = FFPC_ACTION;
	ffp->SDKversion = PLUGINSVER;
}
