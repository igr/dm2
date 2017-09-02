#include <windows.h>
#include "../DM2Plugins.h"

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

#pragma comment(lib, "Winmm.lib")

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

void __cdecl ShowAbout(HWND hwnd_dm2)
{
	MessageBox(hwnd_dm2, "Volume Control v0.01\r--The convenient control volume.\r\rCode by flyfancy. 2005.08", "About",
		MB_OK|MB_ICONINFORMATION);
}

int GetIntFromStr(char *Str, int *Flag)
{
	char *pStr;
	int iInt;

	if(*Str == '+')
	{
		*Flag = 1;
		Str++;
	}
	else if(*Str == '-')
	{
		*Flag = -1;
		Str++;
	}
	else
		*Flag = 0;

	pStr = Str;
	while(*pStr)
	{
		if(*pStr == '%')
		{
			*pStr = '\0';
			break;
		}
		pStr++;
	}

	iInt = atoi(Str);
	if(iInt > 100)
		iInt = 100;
	else if(iInt < 0)
		iInt = 0;

	return iInt;
}

void ControlVolume(BOOL bMix, int Flag, int vol, BOOL bMute)
{
	HMIXER hMixer;
	MMRESULT mmr;
	int iMixerControlID;
	DWORD dwChannels;

	MIXERLINE mxl;
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;

	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u;

	if(mixerGetNumDevs() < 1)
		return;
	mmr= mixerOpen(&hMixer, 0, 0, 0, CALLBACK_NULL);
	if (mmr != MMSYSERR_NOERROR)
		return;
	
	mxl.cbStruct = sizeof(MIXERLINE);
	if(bMix) mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
		else mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;

	if (mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_OBJECTF_HMIXER 	| MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
	{
		mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
		mxlc.dwLineID = mxl.dwLineID;
		if(bMute)
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
		else
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = &mxc;
		if (mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR)
		{
			iMixerControlID = mxc.dwControlID;
			dwChannels = mxl.cChannels;
			if (dwChannels > 0)
				dwChannels--;

			mxcd.cbStruct = sizeof(mxcd);
			mxcd.dwControlID = iMixerControlID;
			mxcd.cChannels = dwChannels;
			mxcd.cMultipleItems = 0;
			mxcd.cbDetails = sizeof(mxcd_u);
			mxcd.paDetails = &mxcd_u;

			mmr = mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd, 0);
			if (MMSYSERR_NOERROR == mmr)
			{
				if(bMute)
				{
					mxcd_u.dwValue = (DWORD)(!(BOOL)mxcd_u.dwValue);
				}
				else
				{
					if(Flag == 0)
						VarI4FromR4(0xFFFF*((float)vol/100), (LONG *)&(mxcd_u.dwValue));
					else if(Flag == 1)
						VarI4FromR4((float)mxcd_u.dwValue+0xFFFF*((float)vol/100),(LONG *)&(mxcd_u.dwValue));
					else if(Flag == -1){
						if((float)mxcd_u.dwValue-0xFFFF*((float)vol/100) >= 0)	//additional checks before the assignment
							VarI4FromR4((float)mxcd_u.dwValue-0xFFFF*((float)vol/100),(LONG *)&(mxcd_u.dwValue));
						else	//otherwise mute
							mxcd_u.dwValue = 0;
					}

					if(mxcd_u.dwValue > 0xFFFF)
						mxcd_u.dwValue = 0xFFFF;
					/*else if(mxcd_u.dwValue < 0)		//Unsigned DWORD cannot go below zero
						mxcd_u.dwValue = 0;*/
				}
				
				mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, 0);
			}
		}
	}
	
	mixerClose(hMixer);
}

void __cdecl ExecPlugins(HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed)
{
	int VOLFlag = 0;			//0 set vol; 1 up vol; -1 down vol.
	int VOL = 0;
	BOOL bMixDev = TRUE;		//TRUE the mixer device(default); FALSE the wave device.
	BOOL bSwitchMUTE = FALSE;

	if(argc == 2)
	{
		VOL = GetIntFromStr(argv[1], &VOLFlag);
		if(lstrcmpi(argv[0], "w") == 0)
			bMixDev = FALSE;
		else if(lstrcmpi(argv[0], "m") != 0)
			return;				//dev select error!
	}
	else if(argc == 1)
	{
		if(lstrcmpi(argv[0], "w") == 0)
		{
			bMixDev = FALSE;
			ControlVolume(bMixDev, VOLFlag, VOL, TRUE);
			return;
		}
		else if(lstrcmpi(argv[0], "m") == 0)
		{
			ControlVolume(bMixDev, VOLFlag, VOL, TRUE);
			return;
		}

		VOL = GetIntFromStr(argv[0], &VOLFlag);
	}
	else if(argc == 0)
	{
		MessageBox(hwnd_dm2, "Sorry, volume control plugin need param!", "Warning",
			MB_OK|MB_ICONWARNING);
		return;
	}

	ControlVolume(bMixDev, VOLFlag, VOL, FALSE);
}

static char *ver = "Volume Control v0.01";
static char *cmd = "dm2_ext_cmd_Volume";
static char *cmdcmt = "Volume Control (Plugins)";

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
