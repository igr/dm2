#include <windows.h>
#include "VirtualDesktop.h"

#define MAXWIN 1024

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if(GetWindowLong(hwnd,GWL_STYLE) & WS_VISIBLE)
	{
		char szName[MAX_PATH];
		LPVDD pVdd = (LPVDD)lParam;
		DWORD pid;
		GetClassName(hwnd, szName, MAX_PATH);
		if(lstrcmp(szName, "Shell_TrayWnd") == 0) //if not the taskbar
			return TRUE;
		GetWindowText(hwnd, szName, MAX_PATH);
		if(lstrcmp(szName, "Program Manager") == 0) //if not the desktop
			return TRUE;
		GetWindowThreadProcessId(hwnd, &pid);
		if(GetCurrentProcessId() == pid)	//skip all window about dm2
			return TRUE;

		pVdd->List[pVdd->WinCount++] = hwnd;
	}
	
	return TRUE;
}

void SaveVirtualDesktop(LPVDD vdd)
{
	int i = 0;

	if(vdd->List || vdd->WinCount)
		return;	//maybe have error, need return.

	vdd->List = malloc(sizeof(HWND)*MAXWIN);		//MAX 1024 window, it enough?
	EnumWindows(EnumWindowsProc, (LPARAM)vdd);			//Get Current desktop all window.

	//ok, start hide all desktop window.
	for(; i<vdd->WinCount; i++)
		ShowWindow(vdd->List[i], SW_HIDE);
}

void LoadVirtualDesktop(LPVDD vdd)
{
	int i = 0;
	if(vdd->List)
	{
		for(; i<vdd->WinCount; i++)
			ShowWindow(vdd->List[i], SW_SHOW);
		free(vdd->List);
		vdd->List = NULL;
		vdd->WinCount = 0;
	}
}

int CurrentVD;
LPVDD *VDD_Array = NULL;
//when you first use the Virtual Desktop, you must init it!!!!
void InitVirtualDesktop()
{
	if(NULL == VDD_Array)
	{
		int i = 0;
		VDD_Array = malloc(sizeof(LPVDD)*8);
		for(; i<8; i++)
		{
			VDD_Array[i] = malloc(sizeof(VDD));
			memset(VDD_Array[i], 0, sizeof(VDD));
		}
		CurrentVD = 0;			// Current VirtualDesktop is vd1;
	}
}

//if you no need Virtual Desktop, you must Destroy it!!!
void DestroyVirtualDesktop()
{
	if(VDD_Array)
	{
		int i = 0;
		for(; i<8; i++)
		{
			LoadVirtualDesktop(VDD_Array[i]);
			free(VDD_Array[i]);
		}

		free(VDD_Array);
		VDD_Array = NULL;
	}
}

// if you want to virtual desktop 5, you need post param WantToVD to 4.
void SwitchVirtualDesktop(int WantToVD)
{
	SaveVirtualDesktop(VDD_Array[CurrentVD]);
	LoadVirtualDesktop(VDD_Array[WantToVD]);
	CurrentVD = WantToVD;
}

void AddWindowtoVD(int VD, HWND hWnd)
{
	if(hWnd && VDD_Array && VDD_Array[VD]->WinCount < MAXWIN)
	{
		int i = 0;
		for(; i<VDD_Array[VD]->WinCount; i++)
			if(VDD_Array[VD]->List[i] == hWnd)
				return;
		if(VDD_Array[VD]->List == NULL) VDD_Array[VD]->List = malloc(sizeof(HWND)*MAXWIN);
		VDD_Array[VD]->List[VDD_Array[VD]->WinCount++] = hWnd;
	}
}

void RemoveWindowFromVD(int VD, HWND hWnd)
{
	if(hWnd && VDD_Array && VDD_Array[VD]->List)
	{
		int i = 0;
		for(; i< VDD_Array[VD]->WinCount; i++)
		{
			if(VDD_Array[VD]->List[i] == hWnd)
			{
				VDD_Array[VD]->WinCount--;
				for(; i< VDD_Array[VD]->WinCount; i++)
					VDD_Array[VD]->List[i] = VDD_Array[VD]->List[i+1];
				break;
			}
		}

		if(VDD_Array[VD]->WinCount == 0)
		{
			free(VDD_Array[VD]->List);
			VDD_Array[VD]->List = NULL;
		}
	}
}