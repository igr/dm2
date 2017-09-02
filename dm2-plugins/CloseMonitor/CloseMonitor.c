#include <windows.h>
#include "../DM2Plugins.h"

#ifdef NDEBUG
#pragma optimize("gsy", on)
#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/IGNORE:4078 /IGNORE:4089")
#pragma comment(linker, "/RELEASE")
#pragma comment(linker, "/merge:.rdata=.data")
#pragma comment(linker, "/merge:.text=.data")
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

void __cdecl ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "CloseMonitor plugins v0.02\r--fast close you monitor.\r\rwrite by flyfancy, 2006.08.02", "About",
		MB_OK|MB_ICONINFORMATION);
}

void __cdecl ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	Sleep(1000);
	SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, 0xF170, 0x2);
}

static char *ver = "CloseMonitor v0.02";			//plugins name an version
static char *cmd = "dm2_ext_cmd_closemon";			//plugins command string.
static char *cmdcmt = "CloseMonitor (Plugins)";		//plugins comments

void __cdecl dm2_get_interface(PFF_PLUGINS ffp)
{
	ffp->About = ShowAbout;			//if have about window, set it.
	ffp->Execute = ExecPlugins;		//must set it.
	ffp->pName = ver;				//return plugins name an version, must.
	ffp->pComments = cmdcmt;		//return comments, must.
	ffp->pCmd = cmd;				//return command string, must.
	ffp->PluginsCategory = FFPC_HOTKEY;	//return plugins Category, now support 
										//FFPC_HOTKEY only. must.
	ffp->SDKversion = PLUGINSVER;	//must. SDKversion alway equal to PLUGINSVER
}
