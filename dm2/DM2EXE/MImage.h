typedef struct {
	int Width;
	int Height;
} MIMAGE_BASEINFO, *PMIMAGE_BASEINFO;

HBITMAP LoadImageFromRes(HMODULE hModule, LPCTSTR lpName, LPCTSTR lpType, PMIMAGE_BASEINFO info);
HBITMAP LoadImageFromFile(LPCTSTR lpFile, PMIMAGE_BASEINFO info);
