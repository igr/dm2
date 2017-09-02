#include <windows.h>
#include "../DM2Plugins.h"

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

/*show you about window. must __cdecl*/
//ffp->About = ShowAbout;
void __cdecl ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "Example plugins vx.xx\r--plugins comments.\r\rauthor info", "About",
		MB_OK|MB_ICONINFORMATION);
}

/*Execute you plugins function, if bUnNeed == TRUE, you 
 *need Close all handle and free memory etc. 
 *plugins will free at function return. must __cdecl*/
//ffp->Execute = ExecPlugins;
void __cdecl ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	MessageBox(hwnd_dm2, "plugins running!", "About",
		MB_OK|MB_ICONINFORMATION);
}

/*if you plugins need load config. must __cdecl*/
//ffp->InitPlugins = InitPlugins;
void __cdecl InitPlugins(char *szini)
{
	//load config.
}

/*if you plugins need show config window. must __cdecl*/
//ffp->Config = NeedConfig;
void __cdecl NeedConfig(HWND hwnd_dm2, char *szini)
{
	//show you config window and save you config.
}

//return Plugins state. if return TRUE, when plugins free, dm2 will call
//ExecPlugins(hwnd,TRUE).  must __cdecl;
BOOL __cdecl PluginsState(void)
{
	return TRUE;
}

static char *ver = "Example plugins v0.01";			//plugins name an version
static char *cmd = "dm2_ext_cmd_Example";			//plugins command string.
static char *cmdcmt = "HOWTO: Write dm2 plugins (Plugins)";	//plugins comments

//must __cdecl
void __cdecl dm2_get_interface(PFF_PLUGINS ffp)
{
	ffp->About = ShowAbout;			//if have about window, set it.
	ffp->InitPlugins = InitPlugins;	//if need init, set it.
	ffp->Execute = ExecPlugins;		//must set it.
	ffp->Config = NeedConfig;		//if have config window. set it.
	ffp->State = PluginsState;		//if no set it, dm2 think plugins stop.
	ffp->pName = ver;				//return plugins name an version, must.
	ffp->pComments = cmdcmt;		//return comments, must.
	ffp->pCmd = cmd;				//return command string, must.
	ffp->PluginsCategory = FFPC_HOTKEY;	//return plugins Category, now support 
										//FFPC_HOTKEY only. must.
	ffp->SDKversion = PLUGINSVER;	//must. SDKversion alway equal to PLUGINSVER
}
