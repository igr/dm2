#include <windows.h>
#include "../dm2Plugins.h"

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


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

void ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "PasteSn plugins v0.01\r--help paste serials number.\r\rCode by flyfancy. 2005", "About",
		MB_OK|MB_ICONINFORMATION);
}

void SetSnText(HWND last, char *SN, char part)
{
	HWND hEdit = last;
	char *pLast, *pCurrent, szClass[MAX_PATH];
	pCurrent = SN;
	pLast = strchr(pCurrent, part);
	while(TRUE)
	{
		if(hEdit == NULL)
			break;
		if(pLast)
		{
			*pLast = '\0';
			pLast++;
		}
		SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)pCurrent);
		pCurrent = pLast;
		if(pCurrent == NULL || hEdit == NULL)
			break;
		while(TRUE)
		{
			hEdit = GetWindow(hEdit, GW_HWNDNEXT);
			if(hEdit == NULL)
				break;
			*szClass = '\0';
			GetClassName(hEdit, szClass, MAX_PATH);
			if(strstr(szClass, "Edit"))
				break;
		}
		pLast = strchr(pCurrent, part);
	}
}


//for VC++, i don't know why can't use GUITHREADINFO struct 
//and GetGUIThreadInfo function.
/*typedef struct tagGUITHREADINFO
{
    DWORD   cbSize;
    DWORD   flags;
    HWND    hwndActive;
    HWND    hwndFocus;
    HWND    hwndCapture;
    HWND    hwndMenuOwner;
    HWND    hwndMoveSize;
    HWND    hwndCaret;
    RECT    rcCaret;
} GUITHREADINFO, *PGUITHREADINFO;*/
WINUSERAPI BOOL WINAPI GetGUIThreadInfo(DWORD idThread, PGUITHREADINFO pgui);
void ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	GUITHREADINFO gi;
	DWORD Pid;
	char szBuf[MAX_PATH];
	HWND hWnd = GetForegroundWindow();
	memset(&gi, 0, sizeof(GUITHREADINFO));
	gi.cbSize = sizeof(GUITHREADINFO);
	GetGUIThreadInfo(GetWindowThreadProcessId(hWnd, &Pid), &gi);

	//want process class "Edit", Borland class "TEdit", or other borland Edit window.
	GetClassName(gi.hwndFocus, szBuf, MAX_PATH);
	if(gi.hwndCaret && strstr(szBuf, "Edit"))
	{
		if(OpenClipboard(gi.hwndFocus))
		{
			HGLOBAL hglb; 
			LPTSTR lptstr;

			hglb = GetClipboardData(CF_TEXT);
			*szBuf = '\0';
			if(hglb)
			{
				lptstr = GlobalLock(hglb);
				if(lptstr)
				{
					lstrcpy(szBuf, lptstr);
					GlobalUnlock(hglb);
				}
			}
			CloseClipboard();
			if(strchr(szBuf, '-'))
			{
				SetSnText(gi.hwndFocus, szBuf, '-');
			}
			else if(strchr(szBuf, ' '))
			{
				SetSnText(gi.hwndFocus, szBuf, ' ');
			}
			//wsprintf(szBuf, "%08X", gi.hwndFocus);
			//MessageBox(hWnd, szBuf, "dm2", MB_OK);
		}
	}
}

static char *ver = "PasteSn v0.01";
static char *cmd = "dm2_ext_cmd_pastesn";
static char *cmdcmt = "Paste Serials Number (Plugins)";

void dm2_get_interface(PFF_PLUGINS ffp)
{
	ffp->About = ShowAbout;
	ffp->Execute = ExecPlugins;
	ffp->pName = ver;
	ffp->pComments = cmdcmt;
	ffp->pCmd = cmd;
	ffp->PluginsCategory = FFPC_HOTKEY;
	ffp->SDKversion = PLUGINSVER;
}
