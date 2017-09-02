//Use RegExp lib
#include "..\RegExp\regexp.h"
#pragma comment(lib, "..\\regexp\\regexp.lib")

#define FLOATWIN_PIXEL 38
#define FLOATWIN_ICON_POS 3
//FLOATWIN_DORDER must <= FLOATWIN_ICON_POS - 1
#define FLOATWIN_DORDER 1

extern LPVOID halloc(int );
extern BOOL hfree(LPVOID );
extern void AddTaskBarIcon(HWND, HICON, char*, int);
extern void DeleteTaskBarIcon(HWND, int);
extern void SetWindowOpacity(HWND, int);
extern HWND CreateTooltip(HWND, char*, int, int, char *);
extern void SetWsExLayered(HWND, BOOLEAN);
extern void SetRelativeWindowPos(HWND, HWND, int, int, int);
extern void SetWindowOnTop(HWND, int);
extern void SetOpacity(HWND, int);
extern void GetWinTitle(HWND , char *);
extern DWORD ConversionHotkey(DWORD dwHK);
extern void PopContextMenuFromButton(HWND, LPDWORD, DWORD);
extern void CreateRMenuCtrl(HWND, HWND, LPDWORD, DWORD);
char *stradd(char *dest, char *src);
int regexec_and_free(regexp *prog, char * string);

