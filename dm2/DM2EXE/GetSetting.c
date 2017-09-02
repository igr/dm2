/* http://dm2.sf.net */
#include <windows.h>
#include "GetSetting.h"
#include "..\tools.h"
#include "util.h"
#include <Tlhelp32.h>

static char section_all[]					= "icon_default";
static char section_icon_on[]				= "icon.on";
static char section_icon_init[]				= "icon.init";
static char section_icon_follow[]			= "icon.follow";
static char section_icon_move[]				= "icon.move";
static char section_icon_trans_enable[]		= "icon.trans.enable";
static char section_icon_trans[]			= "icon.trans";
static char section_icon_top[]				= "icon.top";
static char section_icon_size[]				= "icon.size";
static char section_icon_scrsh[]			= "icon.scrsh";
static char section_icon_scrsh_stretch[]	= "icon.scrsh.stretch";
static char section_icon_scrsh_width[]		= "icon.scrsh.width";
static char section_icon_scrsh_height[]		= "icon.scrsh.height";
static char section_icon_color_background[]	= "icon.color.background";
static char section_icon_color_border[]		= "icon.color.border";
static char default_icon_color_border[]		= "00303030";
static char default_icon_color_background[]	= "00F5EEC5";

SGEN_DATA Gen_data;
SADV_DATA Adv_data;
PSFAV_DATA Fav_data;
PSCUSTOM_DATA Custom_data = NULL;
PSEXCEPT_DATA Except_data = NULL;
void GetGenSetting(char *szini, PSGEN_DATA data)
{
	char sztemp[9];

	data->ontop = (GetPrivateProfileInt(section_all, 
		section_icon_top, 1, szini) != 0);
	data->follow = (GetPrivateProfileInt(section_all, 
		section_icon_follow, 1, szini) != 0);
	data->move = (GetPrivateProfileInt(section_all, 
		section_icon_move, 1, szini) != 0);
	data->opacity = (BYTE)setIntRange(GetPrivateProfileInt(section_all, 
		section_icon_trans, 75, szini), 10, 100);
	data->initpos = GetPrivateProfileInt(section_all, 
		section_icon_init, 0, szini);
	data->screenshots = (GetPrivateProfileInt(section_all, 
		section_icon_scrsh, 1, szini) != 0);
	data->stretch = (GetPrivateProfileInt(section_all, 
		section_icon_scrsh_stretch, 0, szini) != 0);
	data->ficon_size = setIntRange(GetPrivateProfileInt(section_all, 
		section_icon_size, 0, szini), 0, 2);
	data->shWidth = setIntRange(GetPrivateProfileInt(section_all, 
		section_icon_scrsh_width, 128, szini), 16, 640);
	data->shHeight = setIntRange(GetPrivateProfileInt(section_all, 
		section_icon_scrsh_height, 96, szini), 16, 480);
	data->enableopa = (GetPrivateProfileInt(section_all, 
		section_icon_trans_enable, 1, szini) != 0);
	
	GetPrivateProfileString(section_all, section_icon_color_border, 
		default_icon_color_border, sztemp, 9, szini);
	data->bcolor = str2int(sztemp) & 0xFFFFFF;
	GetPrivateProfileString(section_all, section_icon_color_background, 
		default_icon_color_background, sztemp, 9, szini);
	data->bgcolor = str2int(sztemp) & 0xFFFFFF;
}

void WriteGenSetting(char *szini, PSGEN_DATA data)
{
	char sztemp[9];

	wsprintf(sztemp, "%06X", data->bcolor);
	WritePrivateProfileString(section_all, section_icon_color_border, 
		sztemp, szini);
	wsprintf(sztemp, "%06X", data->bgcolor);
	WritePrivateProfileString(section_all, section_icon_color_background, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->ontop);
	WritePrivateProfileString(section_all, section_icon_top, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->ficon_size);
	WritePrivateProfileString(section_all, section_icon_size, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->opacity);
	WritePrivateProfileString(section_all, section_icon_trans, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->follow);
	WritePrivateProfileString(section_all, section_icon_follow, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->move);
	WritePrivateProfileString(section_all, section_icon_move, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->initpos);
	WritePrivateProfileString(section_all, section_icon_init, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->screenshots);
	WritePrivateProfileString(section_all, section_icon_scrsh, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->stretch);
	WritePrivateProfileString(section_all, section_icon_scrsh_stretch, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->shWidth);
	WritePrivateProfileString(section_all, section_icon_scrsh_width, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->shHeight);
	WritePrivateProfileString(section_all, section_icon_scrsh_height, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->enableopa);
	WritePrivateProfileString(section_all, section_icon_trans_enable, 
		sztemp, szini);
}

PSGEN_DATA GetWinSetting(HWND hwnd)
{
	PSCUSTOM_DATA pd = Custom_data;
	char szWinTitle[MAX_PATH];
	void *pmem = halloc(sizeof(SGEN_DATA));
	GetWindowText(hwnd, szWinTitle, MAX_PATH);
	while(pd)
	{
		if(regexec_and_free(regcomp(pd->szRule), szWinTitle))
		{
			MakeSettingFromCustom(pd->data1, pd->data2, pd->data3, pmem);
			return pmem;
		}
		pd = pd->next;
	}
	memcpy(pmem, &Gen_data, sizeof(SGEN_DATA));
	return pmem;
}

static char settings_section[] = "settings";
static char settings_app_trayrun[] = "app.trayrun";
static char settings_app_menu[] = "app.systemmenu";
static char settings_icon_dyntitle[] = "icon.dyntitle";
static char settings_icon_detectclose[] = "icon.detectclose";
static char settings_app_startmode[] = "app.startmode";
static char settings_app_hotkey[] = "app.hotkey";
static char settings_app_hotkey_value[] = "app.hotkey.value";
static char settings_app_lock_win[] = "app.lockwin";
static char settings_fav_cmp[] = "fav.compare.with";
static char settings_fav_rec_num[] = "fav.recent.num";

void GetAdvSetting(char *szini, PSADV_DATA data)
{
	char sztemp[9];

	data->sysmenu = (GetPrivateProfileInt(settings_section, 
		settings_app_menu, 1, szini) != 0);
	data->runtray = (GetPrivateProfileInt(settings_section, 
		settings_app_trayrun, 1, szini) != 0);
	data->detectclose = (GetPrivateProfileInt(settings_section, 
		settings_icon_detectclose, 0, szini) != 0);
	data->dyntitle = (GetPrivateProfileInt(settings_section, 
		settings_icon_dyntitle, 0, szini) != 0);
	data->startmode = (GetPrivateProfileInt(settings_section, 
		settings_app_startmode, 1, szini) != 0);
	data->EnableHotKey = (GetPrivateProfileInt(settings_section, 
		settings_app_hotkey, 0, szini) != 0);
	GetPrivateProfileString(settings_section, settings_app_hotkey_value, 
		"", sztemp, 9, szini);
	data->dwHotKey = str2int(sztemp);
	data->LockWinArea = (GetPrivateProfileInt(settings_section, 
		settings_app_lock_win, 1, szini) != 0);
	data->FavCmpMode = (GetPrivateProfileInt(settings_section, 
		settings_fav_cmp, 0, szini) != 0);
	GetPrivateProfileString(settings_section, settings_fav_rec_num, 
		"8", sztemp, 3, szini);
	data->FavRecNum = setIntRange(str2int(sztemp), 0, MAX_REC_ITEM);
}

void WriteAdvSetting(char *szini, PSADV_DATA data)
{
	char sztemp[9];

	wsprintf(sztemp, "%d", data->sysmenu);
	WritePrivateProfileString(settings_section, settings_app_menu, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->runtray);
	WritePrivateProfileString(settings_section, settings_app_trayrun, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->startmode);
	WritePrivateProfileString(settings_section, settings_app_startmode, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->EnableHotKey);
	WritePrivateProfileString(settings_section, settings_app_hotkey, 
		sztemp, szini);
	wsprintf(sztemp, "%08X", data->dwHotKey);
	WritePrivateProfileString(settings_section, settings_app_hotkey_value, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->LockWinArea);
	WritePrivateProfileString(settings_section, settings_app_lock_win, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->detectclose);
	WritePrivateProfileString(settings_section, settings_icon_detectclose, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->dyntitle);
	WritePrivateProfileString(settings_section, settings_icon_dyntitle, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->FavCmpMode);
	WritePrivateProfileString(settings_section, settings_fav_cmp, 
		sztemp, szini);
	wsprintf(sztemp, "%d", data->FavRecNum);
	WritePrivateProfileString(settings_section, settings_fav_rec_num, 
		sztemp, szini);
}


static char custom_section[] = "icon_custom";
static char default_NULL[] = "";
//Custom data example
//name1=rule name
//rule1=rule
//setting1=FFFFFFFF|FFFFFFFF|FFFFFFFF
PSCUSTOM_DATA GetCustomSetting(char *szini)
{
	PSCUSTOM_DATA data = NULL;
	PSCUSTOM_DATA last = NULL;
	char szKey[20];
	char szTemp[MAXRULE2];
	int i = 1;
	while(TRUE)
	{
		wsprintf(szKey, "name%d", i);
		GetPrivateProfileString(custom_section, szKey, default_NULL, szTemp, 
			MAXRULENAME2, szini);
		if(*szTemp == 0)
		{
			break;
		}
		else
		{
			last = GetNewCustom_Data(&data);
			lstrcpy(last->szRuleName, szTemp);
			wsprintf(szKey, "rule%d", i);
			GetPrivateProfileString(custom_section, szKey, default_NULL, szTemp, 
				MAXRULE2, szini);
			lstrcpy(last->szRule, szTemp);
			wsprintf(szKey, "setting%d", i);
			if(GetPrivateProfileString(custom_section, szKey, default_NULL, szTemp, 
				27, szini) == 26)
			{
				szTemp[8] = 0;
				szTemp[17] = 0;
				last->data1 = str2int(szTemp);
				last->data2 = str2int(&szTemp[9]);
				last->data3 = str2int(&szTemp[18]);
			}
		}
		i++;
	}

	return data;
}

PSCUSTOM_DATA GetNewCustom_Data(PSCUSTOM_DATA *pOld)
{
	register PSCUSTOM_DATA pNow;
	pNow = halloc(sizeof(SCUSTOM_DATA));
	memset(pNow, 0, sizeof(SCUSTOM_DATA));
	pNow->szRuleName = halloc(MAXRULENAME2);
	*(pNow->szRuleName) = 0;
	pNow->szRule = halloc(MAXRULE2);
	*(pNow->szRule) = 0;

	if(*pOld == NULL)
	{
		*pOld = pNow;
	}
	else
	{
		register PSCUSTOM_DATA pTemp = *pOld;
		
		while(pTemp->next)
		{
			pTemp = pTemp->next;
		}
		pTemp->next = pNow;
	}

	return pNow;
}

void RemoveCustom(PSCUSTOM_DATA *pData, PSCUSTOM_DATA pRemove)
{
	if(pData && *pData && pRemove)
	{
		if(pRemove->szRule)
			hfree(pRemove->szRule);
		if(pRemove->szRuleName)
			hfree(pRemove->szRuleName);

		if(pRemove == *pData)
		{
			*pData = (*pData)->next;
		}
		else
		{
			PSCUSTOM_DATA pLast = *pData;
			while(pLast)
			{
				if(pLast->next == pRemove)
				{
					pLast->next = pRemove->next;
					break;
				}
				pLast = pLast->next;
			}
		}
		
		hfree(pRemove);
	}
}

void RemoveAllCustom(PSCUSTOM_DATA *pData)
{
	PSCUSTOM_DATA pLast = *pData;
	while(pLast)
	{
		PSCUSTOM_DATA pNext = NULL;

		if(pLast->szRuleName)
			hfree(pLast->szRuleName);
		if(pLast->szRule)
			hfree(pLast->szRule);
		if(pLast->next)
			pNext = pLast->next;
		hfree(pLast);
		pLast = pNext;
	}
	*pData = 0;
}

//if changed Custom struct must change this function
void CopyCustomData(PSCUSTOM_DATA pDest, PSCUSTOM_DATA pSrc)
{
	lstrcpy(pDest->szRuleName, pSrc->szRuleName);
	lstrcpy(pDest->szRule, pSrc->szRule);
	pDest->data1 = pSrc->data1;
	pDest->data2 = pSrc->data2;
	pDest->data3 = pSrc->data3;
}

//Custom data note
/*data1 12bit save width, 12bit save Height, 8bit save opacity
**data2 8bit save bool setting and 24bit save border color
**data3 8bit save init position, 24bit save background color*/
void MakeSettingFromCustom(DWORD data1, DWORD data2, DWORD data3, PSGEN_DATA pGen)
{
	BYTE temp;
	pGen->shWidth = (WORD)setIntRange(((data1 & 0xFFF00000) >> 20), 16, 640);
	pGen->shHeight = (WORD)setIntRange(((data1 & 0x000FFF00) >> 8), 16, 480);
	pGen->opacity = (BYTE)setIntRange((data1 & 0x000000FF), 10, 100);
	pGen->bcolor = data2 & 0x00FFFFFF;
	pGen->bgcolor = data3 & 0x00FFFFFF;
	temp = (BYTE)((data2 & 0xFF000000) >> 24);
	pGen->ontop = temp & C_TOP;
	pGen->follow = ((temp & C_FOLLOW) != 0);
	pGen->move = ((temp & C_MOVE) != 0);
	pGen->enableopa = ((temp & C_OPA) != 0);
	pGen->screenshots = ((temp & C_MSH) != 0);
	pGen->stretch = ((temp & C_FIT) != 0);
	pGen->initpos = (BYTE)((data3 & 0xFF000000) >> 24);
	//float window size
	if(temp & C_FSIZE1)
		pGen->ficon_size = 1;
	else if(temp & C_FSIZE2)
		pGen->ficon_size = 2;
	else
		pGen->ficon_size = 0;
}

void MakeCustomFromSetting(PSCUSTOM_DATA *pstm, SGEN_DATA Gen)
{
	BYTE temp = 0;
	PSCUSTOM_DATA pd = *pstm;
	
	pd->data1 = Gen.shWidth;
	pd->data1 = (pd->data1)<<12|Gen.shHeight;
	pd->data1 = (pd->data1)<<8|Gen.opacity;

	if(Gen.ontop)
		temp |= C_TOP;
	if(Gen.follow)
		temp |= C_FOLLOW;
	if(Gen.enableopa)
		temp |= C_OPA;
	if(Gen.screenshots)
		temp |= C_MSH;
	if(Gen.stretch)
		temp |= C_FIT;
	if(Gen.ficon_size == 1)			//float window size
		temp |= C_FSIZE1;
	else if(Gen.ficon_size == 2)
		temp |= C_FSIZE2;
	if(Gen.move)
		temp |= C_MOVE;
	pd->data2 = temp;
	pd->data2 = (pd->data2<<24)|Gen.bcolor;

	pd->data3 = Gen.initpos;
	pd->data3 = (pd->data3<<24)|Gen.bgcolor;
}

void WriteCustomSetting(char *szini, PSCUSTOM_DATA data)
{
	char szTemp[MAXRULENAME2];
	char szTemp1[27];
	int i = 1;
	
	//clear section
	WritePrivateProfileSection(custom_section, "", szini);

	while(data)
	{
		wsprintf(szTemp, "name%d", i);
		WritePrivateProfileString(custom_section, szTemp, 
			data->szRuleName, szini);
		wsprintf(szTemp, "rule%d", i);
		WritePrivateProfileString(custom_section, szTemp, 
			data->szRule, szini);
		wsprintf(szTemp1, "%08X|%08X|%08X", data->data1, data->data2, data->data3);
		wsprintf(szTemp, "setting%d", i);
		WritePrivateProfileString(custom_section, szTemp, 
			szTemp1, szini);
		
		i++;
		data = data->next;
	}
}

extern void CreateDefaultSection(PSFAV_DATA *);
static char fav_section[] = "favorites_menu";
static char fav_enable[] = "app.favorites.menu.enable";
BOOL bEnableFavMenu;
PSFAV_DATA GetFavSetting(char *szini)
{
	HANDLE hFile;
	DWORD dwSize = 0;
	char *pMem;
	PSFAV_DATA data = NULL;
	bEnableFavMenu = (GetPrivateProfileInt(settings_section, 
		fav_enable, 1, szini) != 0);

	hFile = CreateFile(szini, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
	}
	
	pMem = halloc(dwSize+1);
	if(dwSize && GetPrivateProfileSection(fav_section, pMem, dwSize, szini))
	{
		char *pStr = NULL;
		char *pCurrent = pMem;
		PSFAV_DATA last;
		last = halloc(sizeof(SFAV_DATA));
		memset(last, 0, sizeof(SFAV_DATA));
		data = last;
		while(TRUE)
		{
			pStr = strchr(pCurrent, '=');
			//can't find '=' or none name
			if(pStr == NULL || pCurrent == pStr)
				break;

			*pStr = '\0';
			last->szName = halloc(lstrlen(pCurrent)+1);
			lstrcpy(last->szName, pCurrent);
			pCurrent = pStr+1;
			pStr = strchr(pCurrent, '\0');
			last->szMenuItem = halloc(lstrlen(pCurrent)+1);
			lstrcpy(last->szMenuItem, pCurrent);

			if(*(pStr+1) == '\0')
				break;
			pCurrent = pStr+1;

			last->next = halloc(sizeof(SFAV_DATA));
			memset(last->next, 0, sizeof(SFAV_DATA));
			last = last->next;
		}
	}
	CreateDefaultSection(&data);

	hfree(pMem);
	return data;
}

void RemoveAllFavMenu(PSFAV_DATA *pData)
{
	PSFAV_DATA last = *pData;
	void *temp;
	while(last)
	{
		temp = last->next;
		if(last->szName)
			hfree(last->szName);
		if(last->szMenuItem)
			hfree(last->szMenuItem);
		hfree(last);
		last = temp;
	}
}

PSFAV_DATA GetFavDataMenuItem(PSFAV_DATA data, char *menu_name)
{
	while(data)
	{
		if(lstrcmpi(data->szName, menu_name) == 0)
			return data;
		data = data->next;
	}
	return NULL;
}

void WriteFavSetting(char *szini, PSFAV_DATA data)
{
	char sztemp[2];
	PSFAV_DATA last;

	wsprintf(sztemp, "%d", bEnableFavMenu);
	WritePrivateProfileString(settings_section, fav_enable, 
		sztemp, szini);

	//clear section
	WritePrivateProfileSection(fav_section, "", szini);

	last = data;
	while(last)
	{
		WritePrivateProfileString(fav_section, last->szName, 
			last->szMenuItem, szini);
		last = last->next;
	}
}

static char except_section[] = "exception";
PSEXCEPT_DATA GetExceptSetting(char *szini)
{
	PSEXCEPT_DATA data = NULL;
	PSEXCEPT_DATA last = NULL;
	char szKey[20];
	char szTemp[MAXRULE2];
	int i = 1;
	while(TRUE)
	{
		wsprintf(szKey, "rule%d", i);
		GetPrivateProfileString(except_section, szKey, default_NULL, szTemp, 
			MAXRULE2, szini);
		if(*szTemp == 0)
		{
			break;
		}
		else
		{
			last = GetNewExcept_Data(&data);
			lstrcpy(last->szRule, szTemp);
			wsprintf(szKey, "setting%d", i);
			if(GetPrivateProfileString(except_section, szKey, default_NULL, szTemp, 
				9, szini) == 8)
			{
				last->Settings = str2int(szTemp);
			}
		}
		i++;
	}

	return data;
}

PSEXCEPT_DATA GetNewExcept_Data(PSEXCEPT_DATA *pOld)
{
	register PSEXCEPT_DATA pNow;
	pNow = halloc(sizeof(SEXCEPT_DATA));
	memset(pNow, 0, sizeof(SEXCEPT_DATA));
	pNow->szRule = halloc(MAXRULE2);
	*(pNow->szRule) = 0;

	if(*pOld == NULL)
	{
		*pOld = pNow;
	}
	else
	{
		register PSEXCEPT_DATA pTemp = *pOld;
		
		while(pTemp->next)
		{
			pTemp = pTemp->next;
		}
		pTemp->next = pNow;
	}

	return pNow;
}

void RemoveAllExcept(PSEXCEPT_DATA *pData)
{
	PSEXCEPT_DATA pLast = *pData;
	while(pLast)
	{
		PSEXCEPT_DATA pNext = NULL;

		if(pLast->szRule)
			hfree(pLast->szRule);
		if(pLast->next)
			pNext = pLast->next;
		hfree(pLast);
		pLast = pNext;
	}
	*pData = 0;
}

void WriteExceptSetting(char *szini, PSEXCEPT_DATA data)
{
	char szTemp[MAXRULE2];
	char szTemp1[9];
	int i = 1;

	//clear section
	WritePrivateProfileSection(except_section, "", szini);
	
	while(data)
	{
		wsprintf(szTemp, "rule%d", i);
		WritePrivateProfileString(except_section, szTemp, 
			data->szRule, szini);
		wsprintf(szTemp1, "%08X", data->Settings);
		wsprintf(szTemp, "setting%d", i);
		WritePrivateProfileString(except_section, szTemp, 
			szTemp1, szini);

		i++;
		data = data->next;
	}
}

/**
 * detect window need exception.
 *
 * @param hWnd   window handle
 *
 * if need exception return true.
 */
BOOL IsException(HWND hwnd, int mask)
{
	char szTitlt[MAX_PATH];
	DWORD pid = 0;
	HANDLE hTSP;
	PROCESSENTRY32 pe;
	char *pStr;
	PSEXCEPT_DATA last = Except_data;

	memset(&pe, 0, sizeof(PROCESSENTRY32));
	pe.dwSize = sizeof(PROCESSENTRY32);

	GetWindowThreadProcessId(hwnd, &pid);
	hTSP = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hTSP != INVALID_HANDLE_VALUE)
	{
		Process32First(hTSP, &pe);
		do
		{
			if(pe.th32ProcessID == pid)
				break;
		}
		while(Process32Next(hTSP, &pe));
		CloseHandle(hTSP);

		pStr = strrchr(pe.szExeFile, '\\');
		if(pStr == NULL)
			pStr = pe.szExeFile;
		else
			pStr++;
	}

	GetWindowText(hwnd, szTitlt, MAX_PATH);

	while(last)
	{
		if(last->Settings & mask)
		{
			if(HIWORD(last->Settings) == 1)
			{
				if(regexec_and_free(regcomp(last->szRule), pStr))
					return TRUE;
			}
			else
			{
				if(regexec_and_free(regcomp(last->szRule), szTitlt))
					return TRUE;
			}
		}
		last = last->next;
	}

	return FALSE;
}


static char hotkey_section[] = "hotkey_settings";
PSHK_DATA hk_data;
PSHK_DATA GetHotKeySetting(char *szini)
{
	PSHK_DATA data = NULL;
	PSHK_DATA last = NULL;
	char szKey[20];
	char szTemp[9];
	char szTemp1[MAX_CMD_LEN];
	int i = 1;
	while(TRUE)
	{
		wsprintf(szKey, "HotKey%d", i);
		GetPrivateProfileString(hotkey_section, szKey, default_NULL, szTemp, 
			9, szini);
		if(*szTemp == '\0')
		{
			break;
		}
		else
		{
			wsprintf(szKey, "Command%d", i);
			if(GetPrivateProfileString(hotkey_section, szKey, default_NULL, szTemp1, 
				MAX_CMD_LEN, szini) != 0)
			{
				last = GetNewHotKey_Data(&data);
				last->dwKey = str2int(szTemp) & 0xFFFF;
				lstrcpy(last->szCMD, szTemp1);
				wsprintf(szKey, "Param%d", i);
				GetPrivateProfileString(hotkey_section, szKey, default_NULL, 
					last->szParam, MAXRULE, szini);
			}
		}
		i++;
	}

	return data;
}

void WriteHotKeySetting(char *szini, PSHK_DATA data)
{
	char szTemp[20];
	char szTemp1[9];
	int i = 1;
	
	//clear section
	WritePrivateProfileSection(hotkey_section, "", szini);
	
	while(data)
	{
		wsprintf(szTemp, "HotKey%d", i);
		wsprintf(szTemp1, "%04X", data->dwKey);
		WritePrivateProfileString(hotkey_section, szTemp, 
			szTemp1, szini);
		wsprintf(szTemp, "Command%d", i);
		WritePrivateProfileString(hotkey_section, szTemp, 
			data->szCMD, szini);
		wsprintf(szTemp, "Param%d", i);
		WritePrivateProfileString(hotkey_section, szTemp, 
			data->szParam, szini);

		i++;
		data = data->next;
	}
}

PSHK_DATA GetNewHotKey_Data(PSHK_DATA *pOld)
{
	register PSHK_DATA pNow;
	pNow = halloc(sizeof(SHK_DATA));
	memset(pNow, 0, sizeof(SHK_DATA));

	if(*pOld == NULL)
	{
		*pOld = pNow;
	}
	else
	{
		register PSHK_DATA pTemp = *pOld;
		
		while(pTemp->next)
		{
			pTemp = pTemp->next;
		}
		pTemp->next = pNow;
	}

	return pNow;
}

void RemoveAllHotKey(PSHK_DATA *pData)
{
	PSHK_DATA pLast = *pData;
	while(pLast)
	{
		PSHK_DATA pNext = NULL;

		if(pLast->next)
			pNext = pLast->next;
		hfree(pLast);
		pLast = pNext;
	}
	*pData = 0;
}

static char recent_section[] = "RecentList";
static char recent_list[] = "Recent%d";
PSREC_DATA LoadRecentList(char *szini, int iMAX)
{
	register PSREC_DATA pNow;
	char szItem[9];
	int i = 0;

	pNow = halloc(sizeof(SREC_DATA));
	memset(pNow, 0, sizeof(SREC_DATA));

	for(;i<iMAX; i++)
	{
		wsprintf(szItem, recent_list, i+1);
		GetPrivateProfileString(recent_section, szItem, default_NULL, pNow->szRecentBuf[i], 
			MAX_PATH+1, szini);
		if(*szItem = '\0')
			break;
	}

	return pNow;
}

void RemoveRecentList(PSREC_DATA *pNow, int iMAX, char *szini)
{
	PSREC_DATA pRec = *pNow;
	int i = 0;
	int m = pRec->iNew;

	//clear section
	WritePrivateProfileSection(recent_section, "", szini);

	while(iMAX)
	{
		char szItem[9];
		if(pRec->szRecentBuf[m][0] != '\0')
		{
			wsprintf(szItem, recent_list, i+1);
			WritePrivateProfileString(recent_section, szItem, 
				pRec->szRecentBuf[m], szini);
			i++;
			iMAX--;
		}
		
		if(++m == MAX_REC_ITEM)
			m = 0;
		if(m == pRec->iNew)
			break;
	}
	
	hfree(pRec);
}

PSAC_DATA Action_data;
static char action_section[] = "Action";
static char action_list[] = "Action%d";
PSAC_DATA GetActionSetting(char *szini)
{
	int i = 0;
	char szItem[9];
	PSAC_DATA pNew = halloc(sizeof(SAC_DATA));

	for(;i<MAX_ACTION; i++)
	{
		wsprintf(szItem, action_list, i+1);
		GetPrivateProfileString(action_section, szItem, default_NULL, pNew->szCMD[i], 
			MAX_CMD_LEN, szini);
		if(i==0 && pNew->szCMD[i][0] == '\0')
			lstrcpy(pNew->szCMD[i], "dm2_cmd_traywin");
		else if(i==1 && pNew->szCMD[i][0] == '\0')
			lstrcpy(pNew->szCMD[i], "dm2_cmd_floatwin");
		else if(i==2 && pNew->szCMD[i][0] == '\0')
			lstrcpy(pNew->szCMD[i], "dm2_cmd_hidewin");
		else if(pNew->szCMD[i][0] == '\0')
			lstrcpy(pNew->szCMD[i], "dm2_cmd_none");
	}

	return pNew;
}

void WriteActionSetting(char *szini, PSAC_DATA data)
{
	int i = 0;
	char szItem[9];
	for(;i<MAX_ACTION; i++)
	{
		wsprintf(szItem, action_list, i+1);
		WritePrivateProfileString(action_section, szItem, 
			data->szCMD[i], szini);
	}
}