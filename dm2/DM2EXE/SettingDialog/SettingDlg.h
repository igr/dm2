#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include "../GetSetting.h"
#include "../dm2.h"
#include "../wnddata.h"
#include "../float.h"

// for Multi-Language
#include "../MultiLanguage.h"
extern char *lang_file;

#define	WM_DM2_SAVESETTING	(WM_USER+13)
#define MAX_COMMENT 50

INT_PTR CALLBACK SettingDlgProc(HWND, UINT, WPARAM, LPARAM);
HTREEITEM AddTreeViewItem(char *, HTREEITEM, HTREEITEM, LPARAM);
void RefreshWinList(void);
void RefreshCustomList(void);
BOOL TryCloseWindow(HWND);
void ApplyDM2Setting();
void RemoveWindowData(PWND_DATA);
void ListMenuItemFromSelect(int);
char *HK_GetCmtText(char *dm2cmd);
char *HK_GetCmdText(char *dm2cmt);
char *HK_GetCmdFromIndex(int cmd);
void GetHotKeyText(DWORD Key);
void DoHotKey_Command(int cmd);
void UnregisterAllHotKey();
void ReRegisterHotKey();
void CreateDefaultSection(PSFAV_DATA *);
void PasteSerialsNumber(HWND hWnd);
void SetSnText(HWND last, char *SN, char part);
void ShowFavMenu();
void DM2FavMenu_Command(int cmd);
void Do_Action_Command(HWND hWnd, DWORD Info);

//Dialog Proc
INT_PTR CALLBACK SettingDlgGeneralProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgAdvanceProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgHotKeyProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgFavMenuProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgCustomProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgExceptionProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgWinManagerProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgAboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgActionProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingDlgPluginsProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK EditCustomDataProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK EditExceptDataProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK EditFavMenuDataProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK EditFavMenuItemProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK EditHKDataProc(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK EnumWindowsForRuleProc(HWND, LPARAM);
