#ifndef SETTING_H
#define SETTING_H


#define C_TOP			1
#define C_FOLLOW		2
#define C_OPA			4
#define C_MSH			8
#define C_FIT			16
#define C_FSIZE1		32
#define C_FSIZE2		64
#define C_MOVE			128
typedef struct {
	BOOL ontop;
	BOOL follow;
	BOOL move;
	BOOL enableopa;
	int opacity;
	BYTE initpos;
	BYTE ficon_size;
	DWORD bcolor;
	DWORD bgcolor;
	BOOL screenshots;
	BOOL stretch;
	WORD shWidth;
	WORD shHeight;
} SGEN_DATA, *PSGEN_DATA;

typedef struct {
	BOOL sysmenu;
	BOOL dyntitle;
	BOOL detectclose;
	BOOL runtray;
	BOOL startmode;
	BOOL isAutorun;
	BOOL EnableHotKey;
	DWORD dwHotKey;
	BOOL LockWinArea;
	BOOL FavCmpMode;
	int  FavRecNum;
} SADV_DATA, *PSADV_DATA;

typedef struct {
	char *szRuleName;
	char *szRule;
	DWORD data1;
	DWORD data2;
	DWORD data3;
	void *next;
} SCUSTOM_DATA, *PSCUSTOM_DATA;
#define MAXRULENAME		100
#define MAXRULE			200
#define MAXRULENAME2	MAXRULENAME+1
#define MAXRULE2		MAXRULE+1

typedef struct {
	char *szName;
	char *szMenuItem;
	void *next;
} SFAV_DATA, *PSFAV_DATA;

typedef struct {
	char *szRule;
	DWORD Settings;
	void *next;
} SEXCEPT_DATA, *PSEXCEPT_DATA;

#define HK_CMD		0x646D
//#define HK_HIDE		(HK_CMD+1)
//#define HK_TRAY		(HK_CMD+2)
//#define HK_ICON		(HK_CMD+3)
//#define HK_INC		(HK_CMD+4)
//#define HK_DEC		(HK_CMD+5)
//#define HK_OPA		(HK_CMD+6)
//#define HK_TOP		(HK_CMD+7)
//#define HK_FAV		(HK_CMD+8)
//#define HK_PASTE	(HK_CMD+9)
//#define HK_WINEN	(HK_CMD+10)
//#define HK_END		(HK_CMD+11)
#define MAX_CMD_LEN 100
typedef struct {
	DWORD dwKey;
	char szCMD[MAX_CMD_LEN];
	char szParam[MAXRULE2];
	void *next;
} SHK_DATA, *PSHK_DATA;

//action add(1.20)
#define MAX_ACTION 24
typedef struct {
	char szCMD[MAX_ACTION][MAX_CMD_LEN];
} SAC_DATA, *PSAC_DATA;

//recent add(1.20)
#define MAX_REC_ITEM 15
typedef struct {
	int iNew;
	char szRecentBuf[MAX_REC_ITEM][MAX_PATH+1];
} SREC_DATA, *PSREC_DATA;
//===================================================================
void GetGenSetting(char *szini, PSGEN_DATA data);
void WriteGenSetting(char *szini, PSGEN_DATA data);

void GetAdvSetting(char *szini, PSADV_DATA data);
void WriteAdvSetting(char *szini, PSADV_DATA data);

PSCUSTOM_DATA GetCustomSetting(char *szini);
PSCUSTOM_DATA GetNewCustom_Data(PSCUSTOM_DATA *pOld);
void RemoveCustom(PSCUSTOM_DATA *pData, PSCUSTOM_DATA pRemove);
void RemoveAllCustom(PSCUSTOM_DATA *pData);
void WriteCustomSetting(char *szini, PSCUSTOM_DATA data);
void CopyCustomData(PSCUSTOM_DATA pDest, PSCUSTOM_DATA pSrc);
void MakeSettingFromCustom(DWORD data1, DWORD data2, DWORD data3, PSGEN_DATA pGen);
void MakeCustomFromSetting(PSCUSTOM_DATA *pstm, SGEN_DATA Gen);

PSFAV_DATA GetFavSetting(char *szini);
void WriteFavSetting(char *szini, PSFAV_DATA data);
void RemoveAllFavMenu(PSFAV_DATA *pData);
PSFAV_DATA GetFavDataMenuItem(PSFAV_DATA data, char *menu_name);

PSHK_DATA GetHotKeySetting(char *szini);
PSHK_DATA GetNewHotKey_Data(PSHK_DATA *pOld);
void RemoveAllHotKey(PSHK_DATA *pData);
void WriteHotKeySetting(char *szini, PSHK_DATA data);

PSEXCEPT_DATA GetExceptSetting(char *szini);
PSEXCEPT_DATA GetNewExcept_Data(PSEXCEPT_DATA *pOld);
void RemoveAllExcept(PSEXCEPT_DATA *pData);
void WriteExceptSetting(char *szini, PSEXCEPT_DATA data);

PSGEN_DATA GetWinSetting(HWND hwnd);
BOOL IsException(HWND hwnd, int mask);

PSREC_DATA LoadRecentList(char *szini, int iMAX);
void RemoveRecentList(PSREC_DATA *pNow, int iMAX, char *szini);

PSAC_DATA GetActionSetting(char *szini);
void WriteActionSetting(char *szini, PSAC_DATA data);

#endif
