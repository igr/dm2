void TranslateLanguage(char *lang_file, HWND Parent, char *Section);
void SetLanguageFont(HFONT hFont, HWND Parent);
int LoadLanguageString(char *lang_file, char *Section, 
						int StringNum, char *dest, DWORD nSize);
BOOL LocalMsgBoxEx(HWND hWnd, char *lang_file, char *def_Title, 
				 char *def_Text, int TitleNum, int TextNum, UINT uType);
int LocalMsgBox(HWND hWnd, char *lang_file, int TitleNum, int TextNum, UINT uType);
void TranslateMenuLanguage(char *lang_file, HMENU hMenu, char *Section);
HFONT GetLanguageFont(HWND hWnd, char *lang_file);
int LoadLanguageStringEx(char *lang_file, char *Section, int StringNum, 
						char *Def, char *dest, DWORD nSize);