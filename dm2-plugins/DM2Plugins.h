// DialogMate2 Plug-In definition file
// Code by flyfancy, 2005
// v1.00 (22 Mar 2005)  init version.
// v1.01 (11 Apr 2005)  plugins parameter support.
// v1.02 (14 Apr 2005)  plugins parameter part by dm2.
#define PLUGINSVER MAKEWPARAM(1, 2)		//ver 1.02

#define FFPC_HOTKEY 1
#define FFPC_ACTION 2

typedef void (__cdecl *pPlugins) (HWND hwnd_cfg);
//load init settings from ini file.
typedef void (__cdecl *pPluginsInit) (char *szini);
//use ini file to save plugins config.
typedef void (__cdecl *pPluginsCfg) (HWND hwnd_cfg, char *szini);
typedef void (__cdecl *pPluginsExec) (HWND hwnd_dm2, char **argv, int argc, BOOL bUnNeed);
typedef BOOL (__cdecl *pPluginsState) (void);

typedef struct {
	char *pName;				// module name and version.
	char *pComments;			// Comments string (for example: "toggle ??? module").
	char *pCmd;					// cmd string (for example: "dm2_cmd_ext_winEnable")

	pPlugins About;				// show about dialog. must return value.
								// if About == NULL not about window.

	pPluginsInit InitPlugins;	// Load all setting. if InitPlugins == NULL then 
								// not need init.

	pPluginsCfg Config;			// if plugins need config, otherwise return NULL. 
								// if Config == NULL then not need config.

	pPluginsExec Execute;		// Execute plugins function. if bUnNeed == true,
								// plugins need close all handle and free all memory,
								// dm2 will free the module at function return.
								// can be used as the toggle.

	pPluginsState State;		// return plugins state, if runing return true, else 
								// return false. if State == NULL then not need 
								// close handle.

	DWORD PluginsCategory;		// Set plugins category, now support FFPC_HOTKEY 
								// only. must set it.

	DWORD reserve;				/* reserve for plugins development kit updata, 
								   must be zero. */
	DWORD SDKversion;			/* set plugins development kit version
								   must be PLUGINSVER !!!
								   MajorVersion = (HIWORD(SDKversion))
								   MinorVersion = (LOWORD(SDKversion))*/
	HMODULE Module;				/* module for free module, Internal use only. */
	void *Next;					/* point to next plugins, Internal use only. */
} FF_PLUGINS, *PFF_PLUGINS;

typedef void (__cdecl *pInterface) (PFF_PLUGINS);