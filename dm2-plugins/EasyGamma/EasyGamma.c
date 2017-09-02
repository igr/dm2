#include <windows.h>
#include "../DM2Plugins.h"
#include <windows.h>

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

double g_gamma = 1.0;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

void __cdecl ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "EasyGamma plugins v0.01\r--easy set the gamma.\r\rwrite by flyfancy, 2006.08.18", "About",
		MB_OK|MB_ICONINFORMATION);
}

//gamma available value 0 - 2.0
BOOL __cdecl SetGamma(HDC hDC, double gamma)
{
	BOOL bReturn = FALSE;
	HDC hGammaDC = hDC;

	if (hDC == NULL)
		hGammaDC = GetDC(NULL);

	if (hGammaDC != NULL)
	{
		int iIndex = 0;
		WORD GammaArray[3][256];

		for (; iIndex < 256; iIndex++)
		{
			WORD iArrayValue = (WORD)min(65535, max(0, pow((iIndex+1) / 256.0, gamma) * 65535 + 0.5));

			GammaArray[0][iIndex] = 
			GammaArray[1][iIndex] = 
			GammaArray[2][iIndex] = (WORD)iArrayValue;
			
		}

		bReturn = SetDeviceGammaRamp(hGammaDC, GammaArray);
	}

	if (hDC == NULL)
		ReleaseDC(NULL, hGammaDC);

	return bReturn;
}

void __cdecl ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	if(argc == 1)
	{
		double a;
		char sign = '\0';
		char *pCMD = argv[0];
		if(*pCMD == '+' || *pCMD == '-')
		{
			sign = *pCMD;
			pCMD++;
		}
		
		if(*pCMD == '\0')
			a = 0.02;
		else
			a = atof(pCMD);
		
		if(a < 0 || a > 2.0)
			return;

		if(sign == '+')
			g_gamma += a;
		else if(sign == '-')
			g_gamma -= a;
		else if(sign == '\0')
			g_gamma = a;
		else
			return;

		if(g_gamma < 0) g_gamma=0;
		else if(g_gamma > 2.0) g_gamma =2.0;

		SetGamma(NULL, g_gamma);
	}
	else if(argc == 0)
	{
		g_gamma = 1.0;
		SetGamma(NULL, 1.0);
	}
}

static char *ver = "EasyGamma v0.01";			//plugins name an version
static char *cmd = "dm2_ext_cmd_easygamma";			//plugins command string.
static char *cmdcmt = "EasyGamma (Plugins)";		//plugins comments

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
