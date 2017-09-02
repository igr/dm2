/* http://dm2.sf.net */
#include <windows.h>
#include "MultiLanguage.h"


char *lastSection;
BOOL CALLBACK TranslateChildLanguage(HWND hwnd, LPARAM lParam)
{
	char szText[MAX_PATH];
	int num = GetDlgCtrlID(hwnd);
	LoadLanguageStringEx((char *)lParam, lastSection, num, "", szText, MAX_PATH);
	if(*szText != '\0')
		SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)szText);
	return TRUE;
}

void TranslateLanguage(char *lang_file, HWND Parent, char *Section)
{
	lastSection = Section;
	EnumChildWindows(Parent, TranslateChildLanguage, (LPARAM)lang_file);
}

BOOL CALLBACK SetChildLanguageFont(HWND hwnd, LPARAM lParam)
{
	SendMessage(hwnd, WM_SETFONT, (WPARAM)lParam, 1);
	return TRUE;
}

void SetLanguageFont(HFONT hFont, HWND Parent)
{
	EnumChildWindows(Parent, SetChildLanguageFont, (LPARAM)hFont);
}

// v1.20: The return value is the number of characters copied to the buffer, not including the terminating null character.
int LoadLanguageString(char *lang_file, char *Section, int StringNum, 
						char *dest, DWORD nSize)
{
	char szNum[11];
	char szErrTxt[128];
	wsprintf(szNum, "%d", StringNum);
	wsprintf(szErrTxt, "%d[%s]", StringNum, Section);
	if (*lang_file == '\0' || *Section == '\0')
	{
		lstrcpy(dest, szErrTxt);
		return lstrlen(dest);
	}
	else
	{
		return GetPrivateProfileString(Section, szNum, szErrTxt, dest, nSize, lang_file);
	}
}

// old version
int LoadLanguageStringEx(char *lang_file, char *Section, int StringNum, 
						char *Def, char *dest, DWORD nSize)
{
	char szNum[11];
	if(*lang_file == '\0' || *Section == '\0')
	{
		lstrcpy(dest, Def);
		return lstrlen(dest);
	}
	else
	{
		wsprintf(szNum, "%d", StringNum);
		return GetPrivateProfileString(Section, szNum, Def, dest, nSize, lang_file);
	}
}



//if have local language message, return true. else return false. 
//you must use default language show message.
char *MsgSection = "Messages";
int LocalMsgBoxEx(HWND hWnd, char *lang_file, char *def_Title, char *def_Text, 
				 int TitleNum, int TextNum, UINT uType)
{
	char Title[MAX_PATH], Text[MAX_PATH];
	if(*lang_file == '\0')
		return MessageBox(hWnd, def_Text, def_Title, uType);
	LoadLanguageString(lang_file, MsgSection, TitleNum, Title, MAX_PATH);
	LoadLanguageString(lang_file, MsgSection, TextNum, Text, MAX_PATH);
	if(*Title == '\0')
		lstrcpy(Title, def_Title);
	if(*Text == '\0')
		lstrcpy(Text, def_Text);
	return MessageBox(hWnd, Text, Title, uType);
}

int LocalMsgBox(HWND hWnd, char *lang_file, int TitleNum, int TextNum, UINT uType)
{
	char Title[MAX_PATH], Text[MAX_PATH];
	LoadLanguageString(lang_file, MsgSection, TitleNum, Title, MAX_PATH);
	LoadLanguageString(lang_file, MsgSection, TextNum, Text, MAX_PATH);
	return MessageBox(hWnd, Text, Title, uType);
}



void TranslateMenuLanguage(char *lang_file, HMENU hMenu, char *Section)
{
	int i = 0;
	int j = GetMenuItemCount(hMenu);
	char szBuf[MAX_PATH];
	MENUITEMINFO mi;
	ZeroMemory(&mi, sizeof(MENUITEMINFO));
	mi.cbSize = sizeof(MENUITEMINFO);
	//#define MIIM_FTYPE 0x100
	//#define MIIM_STRING 0x40
	mi.fMask = 0x100|MIIM_ID|0x40|MIIM_SUBMENU;
	mi.dwTypeData = szBuf;
	for(; i<j; i++)
	{
		mi.cch = MAX_PATH;
		GetMenuItemInfo(hMenu, i, TRUE, &mi);
		if(mi.fType & MFT_SEPARATOR || mi.fType & MFT_BITMAP)
			continue;
		else if(mi.hSubMenu)
		{
			//only for DM2_FLICO_POPUP menu. can't process Multi-Sub Menu.
			LoadLanguageString(lang_file, Section, 0, szBuf, MAX_PATH);
			if(*szBuf != '\0')
				SetMenuItemInfo(hMenu, i, TRUE, &mi);
			TranslateMenuLanguage(lang_file, mi.hSubMenu, Section);
		}
		else if(mi.wID)
		{
			LoadLanguageString(lang_file, Section, mi.wID, szBuf, MAX_PATH);
			if(*szBuf != '\0')
				SetMenuItemInfo(hMenu, i, TRUE, &mi);
		}
	}
}

HFONT GetLanguageFont(HWND hWnd, char *lang_file)
{
	LOGFONT f;
	HDC hdc = GetDC(hWnd);
	int fsize = GetPrivateProfileInt("LangOptions", "FontSize", 8, lang_file);
	ZeroMemory(&f, sizeof(LOGFONT));
	f.lfCharSet = DEFAULT_CHARSET;
	f.lfHeight = -MulDiv(fsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(hWnd, hdc);
	GetPrivateProfileString("LangOptions", "FontName", "Tahoma", f.lfFaceName, LF_FACESIZE, lang_file);
	return CreateFontIndirect(&f);
}