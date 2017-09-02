/* http://dm2.sf.net */
#include <windows.h>
#include "SettingDlg.h"
#include "../MImage.h"

static char szCode[] = "DM2 coded by weird & flyfancy";
static char szRegExp[] = "regexp by Henry Spencer";
static char szIcon[] = "icon by scrow.virtualplastic.net";
static char szThx[] = "thanx to spyder & plastic";
static char szLicense[] = "free for noncommercial use, released under GPL";

HBITMAP hLogoBmp;
MIMAGE_BASEINFO binfo;

void TextOutCenter(HDC hdc, int y, char *Text, DWORD TextLen,int width)
{
	SIZE size;
	GetTextExtentPoint(hdc, Text, TextLen, &size);
	TextOut(hdc, (width - size.cx)/2, y, Text, TextLen);
}

extern BOOL StaticToURLControl(HWND hDlg, UINT staticid, char *szURL, COLORREF crLink);
extern HFONT hLocalFont;
INT_PTR WINAPI SettingDlgAboutProc(HWND hwndDlg, UINT uMsg, 
										  WPARAM wParam, LPARAM lParam)
{
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP hOldBmp;
	RECT rect;
	HFONT font;
	HFONT oldfont;

	switch (uMsg) 
	{
	case WM_INITDIALOG:
		hLogoBmp = LoadImageFromRes(processHandle, (LPCTSTR)IDR_JPEG_DM2, "JPEG", &binfo);
		StaticToURLControl(hwndDlg, IDC_STATIC_HURL, 
			"http://dm2.sourceforge.net", 0xFF0000);
		SetLanguageFont(hLocalFont, hwndDlg);
		TranslateLanguage(lang_file, hwndDlg, "SettingsDialogAbout");
		return TRUE;
	
	case WM_PAINT:
		hdc = BeginPaint(hwndDlg, &ps);
		font = CreateFont(13, 5, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_ROMAN, "Tahoma");
		GetWindowRect(hwndDlg, &rect);
		SetTextColor(hdc, RGB(0xBB, 00, 00));
		TextOutCenter(hdc, binfo.Height+10, szCode, 
			sizeof(szCode)/sizeof(char)-1, rect.right - rect.left);
		if (font != NULL) {
			oldfont = SelectObject(hdc, font);
		}
		SetTextColor(hdc, RGB(0x55, 0x55, 0x55));
		TextOutCenter(hdc, binfo.Height+30, szIcon, 
			sizeof(szIcon)/sizeof(char)-1, rect.right - rect.left);
		TextOutCenter(hdc, binfo.Height+42, szThx, 
			sizeof(szThx)/sizeof(char)-1, rect.right - rect.left);
		TextOutCenter(hdc, binfo.Height+54, szRegExp, 
			sizeof(szRegExp)/sizeof(char)-1, rect.right - rect.left);
		TextOutCenter(hdc, binfo.Height+66, szLicense, 
			sizeof(szLicense)/sizeof(char)-1, rect.right - rect.left);
		if (font != NULL) {
			SelectObject(hdc, oldfont);
			DeleteObject(font);
		}
		// draw dm2 logo
		hMemDC = CreateCompatibleDC(hdc);
		hOldBmp = SelectObject(hMemDC, hLogoBmp);
		BitBlt(hdc, (rect.right - rect.left - binfo.Width)/2, 0, 
			binfo.Width, binfo.Height, hMemDC, 0, 0, SRCCOPY);
		SelectObject(hMemDC, hOldBmp);
		DeleteDC(hMemDC);
		EndPaint (hwndDlg, &ps);
		break;
	case WM_DESTROY:
		DeleteObject(hLogoBmp);
		break;
	}

	return FALSE;
}
