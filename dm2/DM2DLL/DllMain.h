void __cdecl EnableDM2(void);
void __cdecl DisableDM2(void);
void __cdecl EnableDM2Menu(void);
void __cdecl DisableDM2Menu(void);
void __cdecl EnableFavMenu(void);
void __cdecl DisableFavMenu(void);
HMENU __cdecl getCompatibleSystemMenu(HWND hwnd);
void __cdecl ToggleWinEnable(BOOL);
int __cdecl CreateMenuFromString(HMENU hMenu, char *szMenuItem, int Cmd_Start);
