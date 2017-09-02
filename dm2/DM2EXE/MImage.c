/* http://dm2.sf.net */
/* Mini image library. 
 * Conversion jpeg and gif(don't include the gif animation) image to bitmap.
 *
 * History:
 * v0.12 (2005/08/31): fix a bug.
 * v0.11 (2005/04/05): added: Read image from file.
 * v0.10 (2005/04/04): bug fixed: pMem too small when image conversion.
 *
 * When you no longer need the bitmap, call the DeleteObject function to delete it.
 *
 * code by flyfancy. 2005
 */

#include <windows.h>
#include <olectl.h>
#include "MImage.h"


HBITMAP ConversionImage(LPVOID pRes, DWORD size, PMIMAGE_BASEINFO info)
{
	LPVOID pMem;
	IPicture* pPicture = NULL;
	LPSTREAM pStream = 0;
	HBITMAP hBmp = NULL, hOldBmp;
	OLE_XSIZE_HIMETRIC width;
	OLE_YSIZE_HIMETRIC height;
	int bw, bh;

	CoInitialize(NULL);
	pMem = CoTaskMemAlloc(size + 500*1024);
	memcpy(pMem, pRes, size);
	CreateStreamOnHGlobal(pMem, TRUE, &pStream);
	OleLoadPicture(pStream, 0, TRUE, &IID_IPicture, (void **)&pPicture);
	if(pPicture)
	{
		HDC hDC = GetDC(NULL);
		HDC hMemDC = CreateCompatibleDC(hDC);
		if(hMemDC)
		{
			pPicture->lpVtbl->get_Width(pPicture, &width);
			pPicture->lpVtbl->get_Height(pPicture, &height);
			bw = MulDiv(width, GetDeviceCaps(hDC, LOGPIXELSX), 0x09EC);
			bh = MulDiv(height, GetDeviceCaps(hDC, LOGPIXELSY), 0x09EC);
			hBmp = CreateCompatibleBitmap(hDC, bw, bh);
			if(hBmp)
			{
				hOldBmp = SelectObject(hMemDC, hBmp);
				pPicture->lpVtbl->Render(pPicture, hMemDC, 0, 0, bw, bh, 0,
					height, width, -height, NULL);
				SelectObject(hMemDC, hOldBmp);
				//Get Image base info
				if(info)
				{
					info->Width = bw;
					info->Height = bh;
				}
			}
		}
		ReleaseDC(NULL, hDC);
		DeleteDC(hMemDC);
		pPicture->lpVtbl->Release(pPicture);
	}
	pStream->lpVtbl->Release(pStream);
	CoUninitialize();
	return hBmp;
}

HBITMAP LoadImageFromRes(HMODULE hModule, LPCTSTR lpName, LPCTSTR lpType, PMIMAGE_BASEINFO info)
{
	LPVOID pRes;
	DWORD dwRes;
	HRSRC jpeg = FindResource(hModule, lpName, lpType);
	if(jpeg)
	{
		if(dwRes = SizeofResource(hModule, jpeg))
		{
			pRes = LockResource(LoadResource(hModule, jpeg));
			return ConversionImage(pRes, dwRes, info);
		}
	}
	return NULL;
}

HBITMAP LoadImageFromFile(LPCTSTR lpFile, PMIMAGE_BASEINFO info)
{
	HANDLE hFile = CreateFile(lpFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, GetFileAttributes(lpFile), NULL);
	HBITMAP hBmp = NULL;
	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwRead;
		DWORD dwSize = GetFileSize(hFile, NULL);
		LPVOID pMem = malloc(dwSize);
		ReadFile(hFile, pMem, dwSize, &dwRead, NULL);
		hBmp = ConversionImage(pMem, dwSize, info);
		free(pMem);
		CloseHandle(hFile);
	}
	
	return hBmp;
}