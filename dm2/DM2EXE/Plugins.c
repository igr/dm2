/* http://dm2.sf.net */
#include <windows.h>
#include "Plugins.h"
#include "../global.h"

PFF_PLUGINS GetNewPluginsData(PFF_PLUGINS *ffp)
{
	PFF_PLUGINS last = *ffp;
	PFF_PLUGINS next = malloc(sizeof(FF_PLUGINS));
	ZeroMemory(next, sizeof(FF_PLUGINS));
	
	if(last == NULL)
		*ffp = next;
	else
	{
		while(last->Next)
		{
			last = last->Next;
		}
		last->Next = next;
	}
	return next;
}

/* Load all plugins module.
 * param1: szInterface (for example: "Get_FF_Plugins_Interface")
 * param2: Path (for example: "c:\plugins\*.dll")
 */
extern char *inifile;
PFF_PLUGINS LoadAllPlugins(char *szInterface, char *Path)
{
	PFF_PLUGINS All = NULL;
	PFF_PLUGINS last = NULL;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char szModule[MAX_PATH];
	hFind = FindFirstFile(Path, &FindFileData);
	lstrcpy(szModule, Path);
	if(hFind == INVALID_HANDLE_VALUE)
		return NULL;
	do
	{
		HMODULE plugins;
		lstrcpy(strrchr(szModule, '\\')+1, FindFileData.cFileName);
		//if plugins name is !xxxx.dll, skip it.
		if(*(FindFileData.cFileName) == '!')
			continue;
		plugins = LoadLibrary(szModule);
		if(plugins)
		{
			pInterface Interface = (pInterface)GetProcAddress(plugins, szInterface);
			if(Interface)
			{
				last = GetNewPluginsData(&All);
				if(last)
				{
					last->Module = plugins;
					Interface(last);
					if(last->InitPlugins)
						last->InitPlugins(inifile);
				}
			}
			else
			{
				FreeLibrary(plugins);
			}
		}
	}
	while(FindNextFile(hFind, &FindFileData));
	FindClose(hFind);

	return All;
}

/* free all plugins module and memory.
 * param ffp need return by LoadAllPlugins
 */
void UnLoadAllPlugins(PFF_PLUGINS *ffp)
{
	register PFF_PLUGINS last = *ffp;
	PFF_PLUGINS temp;
	while(last)
	{
		temp = last->Next;
		if(last->Module)
		{
			if(last->State && last->State() == TRUE && last->Execute)
				last->Execute(pshared->DM2wnd, NULL, 0, TRUE);
			FreeLibrary(last->Module);
		}
		free(last);
		last = temp;
	}
	*ffp = NULL;
}

// Nonzero indicates success. Zero indicates failure. 
BOOL UninstallPlugins(PFF_PLUGINS ffp)
{
	BOOL ret = FALSE;
	char path[MAX_PATH];
	if(ffp->State && ffp->State() == TRUE && ffp->Execute)
		ffp->Execute(pshared->DM2wnd, NULL, 0, TRUE);
	if(ffp->Module)
	{
		GetModuleFileName(ffp->Module, path, MAX_PATH);
		ret = FreeLibrary(ffp->Module);
		ffp->Module = NULL;
	}

	if(ret)
	{
		if(ret = DeleteFile(path) == 0)
		{
			char path1[MAX_PATH];
			char *pCh;
			lstrcpy(path1, path);
			pCh = strrchr(path1, '\\');
			if(pCh)
			{
				*(pCh + 1) = '!';
				*(pCh + 2) = '\0';
			}
			pCh = strrchr(path, '\\');
			if(pCh)
			{
				pCh++;
				lstrcat(path1, pCh);
				ret = MoveFile(path, path1);
			}
		}
	}

	return (ret != 0);
}