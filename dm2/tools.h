#include <windows.h>

typedef struct {
	HANDLE	hMapObject;
	LPVOID	lpvMem;
	DWORD	MemSize;
	char	*ShareMemName;
} DLLSHAREMEMORY, *PDLLSHAREMEMORY;

LPVOID CreateDllSharedMemory(PDLLSHAREMEMORY);
void DeleteSharedMemory(PDLLSHAREMEMORY);
HBITMAP CreateWndScreenShoot(HWND, int, int, int, BOOL);
int str2int(char *);
int setIntRange(int, int, int);
HICON GetWindowIcon(HWND, BOOL);
void GetDesktopRect(RECT *rect);
int make_argv(char *arg, char ***argv);
BOOL GetPriorityFromhWnd(HWND hWnd, DWORD *pdwPrio);
