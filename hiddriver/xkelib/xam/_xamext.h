#ifndef __XAMEXT_DEFINES_H
#define __XAMEXT_DEFINES_H

#pragma comment(lib, "xkelib")
#pragma comment(lib, "xav")

#include "xamXContent.h"
#include "xamTask.h"
#include "xamNet.h"

typedef PVOID                           HXAMAPP, *PHXAMAPP;

typedef enum
{
	XHUDOPENSTATE_NONE = 0x0,
	XHUDOPENSTATE_HALF = 0x1,
	XHUDOPENSTATE_FULL = 0x2,
	XHUDOPENSTATE_ERROR = 0x3,
	XHUDOPENSTATE_NUI_FULL = 0x4,
	XHUDOPENSTATE_NUI_ERROR = 0x5,
	XHUDOPENSTATE_COUNT = 0x6,
} XHUDOPENSTATE, *PXHUDOPENSTATE;

typedef enum {
	XNOTIFYUI_TYPE_FRIENDONLINE = 0,
	XNOTIFYUI_TYPE_GAMEINVITE = 1,
	XNOTIFYUI_TYPE_FRIENDREQUEST = 2,
	XNOTIFYUI_TYPE_GENERIC = 3, // mail icon
	XNOTIFYUI_TYPE_MULTIPENDING = 4,
	XNOTIFYUI_TYPE_PERSONALMESSAGE = 5,
	XNOTIFYUI_TYPE_SIGNEDOUT = 6,
	XNOTIFYUI_TYPE_SIGNEDIN = 7,
	XNOTIFYUI_TYPE_SIGNEDINLIVE = 8,
	XNOTIFYUI_TYPE_SIGNEDINNEEDPASS = 9,
	XNOTIFYUI_TYPE_CHATREQUEST = 10,
	XNOTIFYUI_TYPE_CONNECTIONLOST = 11,
	XNOTIFYUI_TYPE_DOWNLOADCOMPLETE = 12,
	XNOTIFYUI_TYPE_SONGPLAYING = 13, // music icon
	XNOTIFYUI_TYPE_PREFERRED_REVIEW = 14, // happy face icon
	XNOTIFYUI_TYPE_AVOID_REVIEW = 15, // sad face icon
	XNOTIFYUI_TYPE_COMPLAINT = 16, // hammer icon
	XNOTIFYUI_TYPE_CHATCALLBACK = 17,
	XNOTIFYUI_TYPE_REMOVEDMU = 18,
	XNOTIFYUI_TYPE_REMOVEDGAMEPAD = 19,
	XNOTIFYUI_TYPE_CHATJOIN = 20,
	XNOTIFYUI_TYPE_CHATLEAVE = 21,
	XNOTIFYUI_TYPE_GAMEINVITESENT = 22,
	XNOTIFYUI_TYPE_CANCELPERSISTENT = 23,
	XNOTIFYUI_TYPE_CHATCALLBACKSENT = 24,
	XNOTIFYUI_TYPE_MULTIFRIENDONLINE = 25,
	XNOTIFYUI_TYPE_ONEFRIENDONLINE = 26,
	XNOTIFYUI_TYPE_ACHIEVEMENT = 27,
	XNOTIFYUI_TYPE_HYBRIDDISC = 28,
	XNOTIFYUI_TYPE_MAILBOX = 29, // mailbox icon
	XNOTIFYUI_TYPE_VIDEOCHATINVITE = 30,
	XNOTIFYUI_TYPE_DOWNLOADCOMPLETEDREADYTOPLAY = 31,
	XNOTIFYUI_TYPE_CANNOTDOWNLOAD = 32,
	XNOTIFYUI_TYPE_DOWNLOADSTOPPED = 33,
	XNOTIFYUI_TYPE_CONSOLEMESSAGE = 34,
	XNOTIFYUI_TYPE_GAMEMESSAGE = 35,
	XNOTIFYUI_TYPE_DEVICEFULL = 36,
	XNOTIFYUI_TYPE_CHATMESSAGE = 38,
	XNOTIFYUI_TYPE_MULTIACHIEVEMENTS = 39,
	XNOTIFYUI_TYPE_NUDGE = 40,
	XNOTIFYUI_TYPE_MESSENGERCONNECTIONLOST = 41,
	XNOTIFYUI_TYPE_MESSENGERSIGNINFAILED = 43,
	XNOTIFYUI_TYPE_MESSENGERCONVERSATIONMISSED = 44,
	XNOTIFYUI_TYPE_FAMILYTIMERREMAINING = 45,
	XNOTIFYUI_TYPE_CONNECTIONLOSTRECONNECT = 46,
	XNOTIFYUI_TYPE_EXCESSIVEPLAYTIME = 47,
	XNOTIFYUI_TYPE_PARTYJOINREQUEST = 49,
	XNOTIFYUI_TYPE_PARTYINVITESENT = 50,
	XNOTIFYUI_TYPE_PARTYGAMEINVITESENT = 51,
	XNOTIFYUI_TYPE_PARTYKICKED = 52,
	XNOTIFYUI_TYPE_PARTYDISCONNECTED = 53,
	XNOTIFYUI_TYPE_PARTYCANNOTCONNECT = 56,
	XNOTIFYUI_TYPE_PARTYSOMEONEJOINED = 57,
	XNOTIFYUI_TYPE_PARTYSOMEONELEFT = 58,
	XNOTIFYUI_TYPE_GAMERPICTUREUNLOCKED = 59,
	XNOTIFYUI_TYPE_AVATARAWARDUNLOCKED = 60,
	XNOTIFYUI_TYPE_PARTYJOINED = 61,
	XNOTIFYUI_TYPE_REMOVEDUSB = 62,
	XNOTIFYUI_TYPE_PLAYERMUTED = 63,
	XNOTIFYUI_TYPE_PLAYERUNMUTED = 64,
	XNOTIFYUI_TYPE_CHATMESSAGE2 = 65,
	XNOTIFYUI_TYPE_KINECTCONNECTED = 66,
	XNOTIFYUI_TYPE_KINECTBREAK = 67,
	XNOTIFYUI_TYPE_ETHERNET = 68,
	XNOTIFYUI_TYPE_KINECTPLAYERRECOGNIZED = 69,
	XNOTIFYUI_TYPE_CONSOLESHUTTINGDOWNSOONALERT = 70,
	XNOTIFYUI_TYPE_PROFILESIGNEDINELSEWHERE = 71,
	XNOTIFYUI_TYPE_LASTSIGNINELSEWHERE = 73,
	XNOTIFYUI_TYPE_KINECTDEVICEUNSUPPORTED = 74,
	XNOTIFYUI_TYPE_WIRELESSDEVICETURNOFF = 75,
	XNOTIFYUI_TYPE_UPDATING = 76,
	XNOTIFYUI_TYPE_SMARTGLASSAVAILABLE = 77
} XNOTIFYQUEUEUI_TYPE;

typedef enum {
	XNOTIFYUI_PRIORITY_LOW = 0,
	XNOTIFYUI_PRIORITY_DEFAULT = 1,
	XNOTIFYUI_PRIORITY_HIGH = 2,
	XNOTIFYUI_PRIORITY_PERSISTENT = 3,
} XNOTIFYUI_PRIORITY;

typedef enum {
	XTILETYPE_ACHIEVEMENT = 0x0,
	XTILETYPE_GAME_ICON = 0x1,
	XTILETYPE_GAMER_TILE = 0x2,
	XTILETYPE_GAMER_TILE_SMALL = 0x3,
	XTILETYPE_LOCAL_GAMER_TILE = 0x4,
	XTILETYPE_LOCAL_GAMER_TILE_SMALL = 0x5,
	XTILETYPE_BKGND = 0x6,
	XTILETYPE_AWARDED_GAMER_TILE = 0x7,
	XTILETYPE_AWARDED_GAMER_TILE_SMALL = 0x8,
	XTILETYPE_GAMER_TILE_BY_IMAGE_ID = 0x9,
	XTILETYPE_PERSONAL_GAMER_TILE = 0xA,
	XTILETYPE_PERSONAL_GAMER_TILE_SMALL = 0xB,
	XTILETYPE_GAMER_TILE_BY_KEY = 0xC,
	XTILETYPE_AVATAR_GAMER_TILE = 0xD,
	XTILETYPE_AVATAR_GAMER_TILE_SMALL = 0xE,
	XTILETYPE_AVATAR_FULL_BODY = 0xF,
} XTILETYPE;

typedef enum {
	XamLoaderStateNone = 0x0,
	XamLoaderStateBooting = 0x1,
	XamLoaderStateTitleUnloading = 0x2,
	XamLoaderStateTitleLoading = 0x3,
	XamLoaderStateTitleStartup = 0x4,
	XamLoaderStateTitleRunning = 0x5,
} XamLoaderState;

typedef enum  {
	XAM_DEFAULT_IMAGE_SYSTEM = 0x0, 	// "xam" 		"defaultsystemimage.png"
	XAM_DEFAULT_IMAGE_DASHICON = 0x1,	// "xam" 		"dashicon.png"
	XAM_DEFAULT_IMAGE_SETTINGS = 0x2,	// "shrdres" 	"ico_64x_licensestore.png"
	XAM_DEFAULT_IMAGE_ID_COUNT = 0x3,
} XAM_DEFAULT_IMAGE_ID;

typedef enum _DVD_MEDIA_TYPES {
	DVD_MEDIA_TYPE_NONE					= 0x00,
	DVD_MEDIA_TYPE_GAME_XBOX_360		= 0x01,
	DVD_MEDIA_TYPE_GAME_XBOX_ORIGINAL	= 0x02,
	DVD_MEDIA_TYPE_UNKNOWN				= 0x03,
	DVD_MEDIA_TYPE_DVD_AUDIO			= 0x04,
	DVD_MEDIA_TYPE_DVD_MOVIE			= 0x05,
	DVD_MEDIA_TYPE_CD_VIDEO				= 0x06,
	DVD_MEDIA_TYPE_CD_AUDIO				= 0x07,
	DVD_MEDIA_TYPE_CD_DATA				= 0x08,
	DVD_MEDIA_TYPE_GAME_MOVIE_HYBRID	= 0x09,
	DVD_MEDIA_TYPE_DVD_HD				= 0x0A,
} DVD_MEDIA_TYPES;

typedef enum _DVD_TRAY_STATE {
	DVD_TRAY_STATE_EMPTY,
	DVD_TRAY_STATE_CLOSING,
	DVD_TRAY_STATE_OPEN,
	DVD_TRAY_STATE_OPENING,
	DVD_TRAY_STATE_CLOSED,
	DVD_TRAY_STATE_ERROR
} DVD_TRAY_STATE;

typedef enum _STAGING_MODE {
	STAGING_MODE_PRODUCTION = 0x0,
	STAGING_MODE_STAGING = 0x1,
} STAGING_MODE;

typedef VOID (CALLBACK *PFNMSGBOXRETURN)(
	IN      INT                         iButtonPressed,
	OUT     PXHUDOPENSTATE              pHudRestoreState
	);

typedef struct _XUIBRUSH*	HXUIBRUSH;

typedef DWORD (*MBOXRESULT)(DWORD r3);

//#define XSSUI_FLAGS_LOCALSIGNINONLY                 0x00000001
//#define XSSUI_FLAGS_SHOWONLYONLINEENABLED           0x00000002
//#define XSSUI_FLAGS_ALLOW_SIGNOUT                   0x00000004
#define XSSUI_FLAGS_NUI			0x00000008
//#define XSSUI_FLAGS_DISALLOW_PLAYAS                 0x00000010
//#define XSSUI_FLAGS_ADDUSER                         0x00010000
//#define XSSUI_FLAGS_ENABLE_GUEST                    0x00080000
//#define XSSUI_FLAGS_CONVERTOFFLINETOGUEST           0x00400000
//#define XSSUI_FLAGS_DISALLOW_GUEST                  0x01000000
//													0x24030000
//													0x20000000
//													0x04000000
//													0x00020000
//													0x00010000 XSSUI_FLAGS_ADDUSER

typedef struct _XSHOWSIGNINUI_PARAMS { 
	DWORD dwTrackingID; // 0x0 sz:0x4
	DWORD dwUserIndex; // 0x4 sz:0x4
	DWORD cPanes; // 0x8 sz:0x4
	DWORD dwFlags; // 0xC sz:0x4
	PXOVERLAPPED pOverlapped; // 0x10 sz:0x4
} XSHOWSIGNINUI_PARAMS, *PXSHOWSIGNINUI_PARAMS; // size 20
C_ASSERT(sizeof(XSHOWSIGNINUI_PARAMS) == 0x14);

typedef struct _MESSAGEBOX_PARAMS { 
	DWORD dwTrackingID; // 0x0 sz:0x4
	DWORD dwUserIndex; // 0x4 sz:0x4
	DWORD eHudType; // 0x8 sz:0x4 enum HUDSCENE
	wchar_t szTitle[0x2B]; // 0xC sz:0x56
	wchar_t szText[0x180]; // 0x62 sz:0x300
	// 2 bytes align when packed
	DWORD cButtons; // 0x364 sz:0x4
	wchar_t rgButtons[3][0x2B]; // 0x368 sz:0x102
	// 2 bytes align when packed
	DWORD dwFocusButton; // 0x46C sz:0x4
	DWORD dwFlags; // 0x470 sz:0x4
	PMESSAGEBOX_RESULT pResult; // 0x474 sz:0x4
	PXOVERLAPPED pOverlapped; // 0x478 sz:0x4
} MESSAGEBOX_PARAMS, *PMESSAGEBOX_PARAMS; // size 1148
C_ASSERT(sizeof(MESSAGEBOX_PARAMS) == 0x47C);

typedef struct _XINPUT_DEVICE_STATS {
	DWORD BatteryLevel; // 0x0 sz:0x4
	DWORD BatteryType; // 0x4 sz:0x4
	DWORD SignalStrength; // 0x8 sz:0x4
	DWORD DeviceType; // 0xC sz:0x4
	DWORD DevicePort; // 0x10 sz:0x4
	DWORD DeviceVersion; // 0x14 sz:0x4
	int Jumped; // 0x18 sz:0x4
} XINPUT_DEVICE_STATS, *PXINPUT_DEVICE_STATS; // size 28
C_ASSERT(sizeof(XINPUT_DEVICE_STATS) == 0x1C);

typedef enum {
	XDASHLAUNCHDATA_COMMAND_DEFAULT = 0, // 14719
	XDASHLAUNCHDATA_COMMAND_SIGNUP = 1,
	XDASHLAUNCHDATA_COMMAND_NETWORKTROUBLESHOOTER = 2,
	XDASHLAUNCHDATA_COMMAND_ARCADE = 3,
	XDASHLAUNCHDATA_COMMAND_MEMORY = 4,
	//XDASHLAUNCHDATA_COMMAND_ = 5,
	XDASHLAUNCHDATA_COMMAND_MUSICPLAYER = 6, // 14719
	XDASHLAUNCHDATA_COMMAND_PLAYDVD = 7, // 14719
	XDASHLAUNCHDATA_COMMAND_MEDIA_CENTER = 8, // 14719
	XDASHLAUNCHDATA_COMMAND_MEDIANETWORKTROUBLESHOOTER = 9,
	XDASHLAUNCHDATA_COMMAND_ACCOUNTMANAGE_HOME = 10,
	XDASHLAUNCHDATA_COMMAND_ACCOUNTMANAGE_PRIVACY = 11,
	XDASHLAUNCHDATA_COMMAND_ACCOUNTMANAGE_ACCEPTTOU = 12,
	XDASHLAUNCHDATA_COMMAND_ACCOUNTMANAGE_BILLING = 13,
	XDASHLAUNCHDATA_COMMAND_GAMES_DEMOS = 14,
	XDASHLAUNCHDATA_COMMAND_GAMES_TRAILERS = 15,
	XDASHLAUNCHDATA_COMMAND_RETURNFROMENTRYPOINT_MUSIC = 16,
	XDASHLAUNCHDATA_COMMAND_MARKETPLACE_TILES = 17,
	XDASHLAUNCHDATA_COMMAND_SETTINGS = 18,
	XDASHLAUNCHDATA_COMMAND_RETURNFROMENTRYPOINT_GAMES = 19,
	XDASHLAUNCHDATA_COMMAND_FAMILY_SETTINGS = 20, // 14719
	XDASHLAUNCHDATA_COMMAND_RETURNFROMENTRYPOINT_SYSTEM = 21,
	XDASHLAUNCHDATA_COMMAND_OOBE_WELCOME = 22,
	XDASHLAUNCHDATA_COMMAND_GAMES_ARCADE = 23,
	//XDASHLAUNCHDATA_COMMAND_ = , // 24-38
	XDASHLAUNCHDATA_COMMAND_VIDEOPLAYER = 39, // 14719
	XDASHLAUNCHDATA_COMMAND_PICTURE_VIEWER = 44, // 14719
	XDASHLAUNCHDATA_COMMAND_SYSTEM_SETTINGS = 47, // 14719
} XDASHLAUNCHDATA_COMMAND;

typedef struct _XDASHLAUNCHDATA { 
	DWORD dwVersion; // 0x0 sz:0x4
	DWORD dwCommand; // 0x4 sz:0x4
	DWORD dwUserIndex; // 0x8 sz:0x4
	BYTE Reserved[0x3F0]; // 0xC sz:0x3F0
} XDASHLAUNCHDATA, *PXDASHLAUNCHDATA; // size 1020
C_ASSERT(sizeof(XDASHLAUNCHDATA) == 0x3FC);

typedef struct _FU_LAUNCH_DATA { 
	DWORD Source; // 0x0 sz:0x4
	DWORD Reason; // 0x4 sz:0x4
	DWORD LiveEnabled; // 0x8 sz:0x4
	BYTE Padding[0x1F0]; // 0xC sz:0x1F0
	char XBE[0x200]; // 0x1FC sz:0x200
} FU_LAUNCH_DATA, *PFU_LAUNCH_DATA; // size 1020
C_ASSERT(sizeof(FU_LAUNCH_DATA) == 0x3FC);

typedef struct _XLAUNCHDATA_EPIX { 
	DWORD dwID; // 0x0 sz:0x4
	DWORD dwUserIndex; // 0x4 sz:0x4
	DWORD dwFlags; // 0x8 sz:0x4
	wchar_t Param[0x80]; // 0xC sz:0x100
} XLAUNCHDATA_EPIX, *PXLAUNCHDATA_EPIX; // size 268
C_ASSERT(sizeof(XLAUNCHDATA_EPIX) == 0x10C);

typedef struct _XLAUNCHDATA_CAFEBABE { 
	DWORD dwID; // 0x0 sz:0x4 << 0xCAFEBABE
	BYTE Param[0x2C]; // 0x4 sz:0x
	DWORD deviceId; // 0x30 sz:0x4
} XLAUNCHDATA_CAFEBABE, *PXLAUNCHDATA_CAFEBABE; // size 52
C_ASSERT(sizeof(XLAUNCHDATA_CAFEBABE) == 0x34);

// from Xna_TitleLauncher.xex
typedef struct _XLAUNCHDATA_CAFEBABE2 { 
	XLAUNCHDATA_CAFEBABE param; // 0x0
	DWORD unused; // 0x34
	QWORD qwUnk; // 0x38
	BYTE bParm[0x134]; // 0x40
	DWORD dwUnk; // 0x174
} XLAUNCHDATA_CAFEBABE2, *PXLAUNCHDATA_CAFEBABE2; // size 376
C_ASSERT(sizeof(XLAUNCHDATA_CAFEBABE2) == 0x178);

// from Xna_TitleLauncher.xex
typedef struct _XLAUNCHDATA_CAFEBABE3 { 
	XLAUNCHDATA_CAFEBABE param; // 0x0
	DWORD unused; // 0x34
	BYTE bParm[0x200]; // 0x38
} XLAUNCHDATA_CAFEBABE3, *PXLAUNCHDATA_CAFEBABE3; // size 568
C_ASSERT(sizeof(XLAUNCHDATA_CAFEBABE3) == 0x238);

typedef struct _XDASHLAUNCHDATA_XAMUIAPP { 
	DWORD dwID; // 0x0 sz:0x4
	BYTE Reserved[0x3EC]; // 0x4 sz:0x3EC
} XDASHLAUNCHDATA_XAMUIAPP, *PXDASHLAUNCHDATA_XAMUIAPP; // size 1008
C_ASSERT(sizeof(XDASHLAUNCHDATA_XAMUIAPP) == 0x3F0);

typedef struct _XLAUNCHDATA_DASHAPP { 
	char AppName[0x40]; // 0x0 sz:0x40
	DWORD AppParamsSize; // 0x40 sz:0x4
	BYTE AppParams[0x200]; // 0x44 sz:0x200
} XLAUNCHDATA_DASHAPP, *PXLAUNCHDATA_DASHAPP; // size 580
C_ASSERT(sizeof(XLAUNCHDATA_DASHAPP) == 0x244);

typedef struct _XAVATAR_AWARDS_COUNTER { 
	BYTE bEarned; // 0x0 sz:0x1
	BYTE bPossible; // 0x1 sz:0x1
} XAVATAR_AWARDS_COUNTER, *PXAVATAR_AWARDS_COUNTER; // size 2
C_ASSERT(sizeof(XAVATAR_AWARDS_COUNTER) == 0x2);

typedef struct _XUSER_TITLE_PLAYED { 
	DWORD dwTitleId; // 0x0 sz:0x4
	DWORD dwAchievementsPossible; // 0x4 sz:0x4
	DWORD dwAchievementsEarned; // 0x8 sz:0x4
	DWORD dwCredPossible; // 0xC sz:0x4
	DWORD dwCredEarned; // 0x10 sz:0x4
	WORD wReservedAchievementCount; // 0x14 sz:0x2
	XAVATAR_AWARDS_COUNTER AllAvatarAwards; // 0x16 sz:0x2
	XAVATAR_AWARDS_COUNTER MaleAvatarAwards; // 0x18 sz:0x2
	XAVATAR_AWARDS_COUNTER FemaleAvatarAwards; // 0x1A sz:0x2
	DWORD dwReservedFlags; // 0x1C sz:0x4
	FILETIME ftLastLoaded; // 0x20 sz:0x8
	wchar_t wszTitleName[0x40]; // 0x28 sz:0x80
} XUSER_TITLE_PLAYED, *PXUSER_TITLE_PLAYED; // size 168
C_ASSERT(sizeof(XUSER_TITLE_PLAYED) == 0xA8);


// *********** PACKING STARTS HERE **************//
#pragma pack(push, 1)
// *********** PACKING STARTS HERE **************//
typedef struct _XAMACCOUNTINFO { 
	DWORD dwReserved; // 0x0 sz:0x4
	DWORD dwLiveFlags; // 0x4 sz:0x4
	WCHAR szGamerTag[0x10]; // 0x8 sz:0x20
	XUID xuidOnline; // 0x28 sz:0x8
	DWORD dwCachedUserFlags; // 0x30 sz:0x4
	DWORD dwOnlineServiceNetworkID; // 0x34 sz:0x4
	BYTE rgbPasscode[0x4]; // 0x38 sz:0x4
	char szOnlineDomain[0x14]; // 0x3C sz:0x14
	char szOnlineKerberosRealm[0x18]; // 0x50 sz:0x18
	BYTE rgbOnlineKey[0x10]; // 0x68 sz:0x10
	char szUserPassportMembername[0x72]; // 0x78 sz:0x72
	char rgcUserPassportPassword[0x20]; // 0xEA sz:0x20
	char szOwnerPassportMembername[0x72]; // 0x10A sz:0x72
} XAMACCOUNTINFO, *PXAMACCOUNTINFO; // size 380
C_ASSERT(sizeof(XAMACCOUNTINFO) == 0x17C);

typedef struct _PROFILEENUMRESULT { 
	XUID xuidOffline; // 0x0 sz:0x8
	XAMACCOUNTINFO xai; // 0x8 sz:0x17C
	DWORD DeviceID; // 0x184 sz:0x4
} PROFILEENUMRESULT, *PPROFILEENUMRESULT; // size 392
C_ASSERT(sizeof(PROFILEENUMRESULT) == 0x188);


typedef enum _XAM_CACHE_FILE_TYPE { 
		XAM_CACHE_SYSTEM_UPDATE = 0xC,  
		XAM_CACHE_GAMER_TILE = 0x3,  
		XAM_CACHE_GAME_TILE = 0x1,  
		XAM_CACHE_NUI_HIVE_SETTING = 0x1A,  
		XAM_CACHE_ACHIEVEMENT_TILE = 0x2,  
		XAM_CACHE_CUSTOM_GAMER_TILE = 0x5,  
		XAM_CACHE_NUI_SESSION = 0x17,  
		XAM_CACHE_XSTS_TOKEN = 0x1E,  
		XAM_CACHE_GAMER_TAG = 0x10,  
		XAM_CACHE_GAMER_TILE_SMALL = 0x4,  
		XAM_CACHE_XLFS_UPLOADER = 0x1B,  
		XAM_CACHE_PROFILE_SETTINGS = 0x13,  
		XAM_CACHE_DASH_CACHESEEDFILE = 0x1C,  
		XAM_CACHE_FRIEND_MUTE_LIST = 0x7,  
		XAM_CACHE_NUI_BIOMETRIC = 0x18,  
		XAM_CACHE_TITLE_NAMES = 0x9,  
		XAM_CACHE_MESSENGER_BUDDIES = 0x11,  
		XAM_CACHE_QOS_HISTORY = 0x12,  
		XAM_CACHE_GAME_INVITE = 0xF,  
		XAM_CACHE_TITLE_UPDATE = 0xB,  
		XAM_CACHE_ZEST_AUTHENTICATION = 0x1D,  
		XAM_CACHE_VALIDATE_CERT = 0x16,  
		XAM_CACHE_TICKETS = 0x8,  
		XAM_CACHE_ALL = 0x0,  
		XAM_CACHE_SPA_FILE = 0xE,  
		XAM_CACHE_NUI_TROUBLESHOOTER = 0x19,  
		XAM_CACHE_LIVEID_DEVICE = 0x1F,  
		XAM_CACHE_CERT_STORAGE = 0x15,  
		XAM_CACHE_CUSTOM_GAMER_TILE_SMALL = 0x6,  
		XAM_CACHE_AVATAR_GAMER_TILE = 0x14,  
		XAM_CACHE_RECENT_PLAYERS = 0xA,  
		XAM_CACHE_DASHBOARD_APP = 0xD  
	} XAM_CACHE_FILE_TYPE;


// *********** PACKING ENDS HERE **************//
#pragma pack(pop)
// *********** PACKING ENDS HERE **************//


#ifdef __cplusplus
extern "C" {
#endif

	NTSYSAPI
	EXPORTNUM(309)
	HRESULT
	NTAPI
	XNetLogonGetMachineID(
		IN OUT	QWORD* pqwMachineID
	);
	
	NTSYSAPI
	EXPORTNUM(407)
	DWORD
	NTAPI
	XamInputGetDeviceStats(
		IN		DWORD dwUserIndex,
		OUT		PXINPUT_DEVICE_STATS pDeviceStats
	);

	NTSYSAPI
	EXPORTNUM(419)
	NTSTATUS
	NTAPI
	XamLoaderGetMediaInfo(
		IN OUT	PDWORD pdwMediaType,
		IN OUT	PDWORD pdwTitleId OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(420)
	VOID
	NTAPI
	XamLoaderLaunchTitle(
		IN		LPCSTR szLaunchPath,
		IN		DWORD DwFlags
		);

	NTSYSAPI
	EXPORTNUM(421)
	NTSTATUS
	NTAPI
	XamLoaderLaunchTitleEx(
		IN		LPCSTR szLaunchPath,
		IN		LPCSTR szMountPath,
		IN		LPCSTR szCmdLine,
		IN		DWORD DwFlags
		);

	NTSYSAPI
	EXPORTNUM(422)
	NTSTATUS
	NTAPI
	XamLoaderSetLaunchData(
		IN		PVOID launchData,
		IN		DWORD cbLaunchData
	);

	NTSYSAPI
	EXPORTNUM(424)
	NTSTATUS
	NTAPI
	XamLoaderGetLaunchData(
		IN		PVOID launchData,
		IN		DWORD cbLaunchData
	);

	NTSYSAPI
	EXPORTNUM(426)
	DVD_TRAY_STATE
	NTAPI
	XamLoaderGetDvdTrayState(
		VOID
	);

	// UNTESTED!!!
	NTSYSAPI
	EXPORTNUM(427)
	HRESULT
	NTAPI
	XamLoaderGetGameInfo(
		IN OUT	LPCWSTR wszTitleName,
		IN		DWORD dwTitleId,
		OUT		PBYTE* ppbUnk,
		OUT		QWORD* pqwUnk
	);

	NTSYSAPI
	EXPORTNUM(429)
	VOID
	NTAPI
	XamLoaderSetSpindleSpeed(
		IN		int Speed
	);

	NTSYSAPI
	EXPORTNUM(430)
	HRESULT
	NTAPI
	XamTaskCreateQueue(
		IN		PHXAMTASKQUEUE hxamtask
	);

	NTSYSAPI
	EXPORTNUM(431)
	HRESULT
	NTAPI
	XamTaskSchedule(
		IN		PXAMTASKPROC pxamtaskproc,
		IN		PVOID pvParam,
		IN		PXAMTASKATTRIBUTES pxamtaskattribs,
		IN		PHXAMTASK phxamtask
	);

	NTSYSAPI
	EXPORTNUM(432)
	VOID
	NTAPI
	XamTaskReschedule(
		IN		HXAMTASK hxamtask
	);

	NTSYSAPI
	EXPORTNUM(433)
	VOID
	NTAPI
	XamTaskCloseHandle(
		IN		HXAMTASKOBJ hxamtask
	);

	NTSYSAPI
	EXPORTNUM(434)
	VOID
	NTAPI
	XamTaskCancel(
		IN		HXAMTASK hxamtask
	);

	NTSYSAPI
	EXPORTNUM(435)
	BOOL
	NTAPI
	XamTaskShouldExit(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(436)
	VOID
	NTAPI
	XamTaskWaitOnCompletion(
		IN		HXAMTASK hxamtask
	);
	
	NTSYSAPI
	EXPORTNUM(437)
	VOID
	NTAPI
	XamTaskModify(
		IN		HXAMTASK hxamtask,
		IN		DWORD dwFlags,
		IN		PXAMTASKPROC pxamtaskproc,
		IN		PVOID pvParam,
		IN		PXAMTASKATTRIBUTES pxamtaskattribs
	);

	NTSYSAPI
	EXPORTNUM(438)
	HXAMTASK
	NTAPI
	XamTaskGetCurrentTask(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(439)
	HRESULT
	NTAPI
	XamTaskGetAttributes(
		IN		HXAMTASK hxamtask,
		IN OUT	PXAMTASKATTRIBUTES pxamtaskattribs
	);
	
	NTSYSAPI
	EXPORTNUM(440)
	DWORD
	NTAPI
	XamExecutingOnBehalfOfTitle(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(444)
	BOOL
	NTAPI
	XamIsSystemTitleId(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(445)
	BOOL
	NTAPI
	XamLoaderIsTitleTerminatePending(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(446)
	DWORD
	NTAPI
	XamLoaderGetPriorTitleId(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(447)
	BOOL
	NTAPI
	XamIsXbox1TitleId(
		IN		DWORD dwTitleId
	);

	NTSYSAPI
	EXPORTNUM(450)
	HRESULT
	NTAPI
	XamTaskGetStatus(
		IN		HXAMTASK hxamtask
	);

	NTSYSAPI
	EXPORTNUM(453)
	NTSTATUS
	NTAPI
	XamGetDefaultSystemImage( // gets "defaultsystemimage.png"
		OUT		PVOID* imageSource,
		OUT		PDWORD imageLen
	);

	NTSYSAPI
	EXPORTNUM(457)
	NTSTATUS
	NTAPI
	XamGetDefaultImage(
		IN		XAM_DEFAULT_IMAGE_ID index,
		OUT		PVOID* imageSource,
		OUT		PDWORD imageLen
	);

	NTSYSAPI
	EXPORTNUM(458)
	NTSTATUS // always returns 0
	NTAPI
	XamMuteSound(
		IN		DWORD dwXuiElementMute
	);
	
	NTSYSAPI
	EXPORTNUM(460)
	VOID
	NTAPI
	XamSetDashContext(
		IN		DWORD dwDashContext
	);

	NTSYSAPI
	EXPORTNUM(461)
	DWORD
	NTAPI
	XamGetDashContext(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(462)
	BOOL
	NTAPI
	XamIsCurrentTitleDash(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(463)
	DWORD
	NTAPI
	XamGetCurrentTitleId(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(464)
	VOID
	NTAPI
	XamLoaderSetCurrentTitleIsDash(
		IN		DWORD titleIdToSet
	);
	
	NTSYSAPI
	EXPORTNUM(465)
	VOID
	NTAPI
	XamAllocHeapFreeSpace(
		IN		DWORD heapID // enum  XAMALLOC_HEAP_ID
	);

	NTSYSAPI
	EXPORTNUM(466)
	VOID
	NTAPI
	XamSetDashContextEx(
		IN		DWORD dwDashContext,
		IN		QWORD qwSessionId,
		IN		PBYTE pbSessionKey // 0x10 byte in
	);

	NTSYSAPI
	EXPORTNUM(467)
	DWORD // returns dwDashContext
	NTAPI
	XamGetDashContextEx(
		OUT		PQWORD pqwSessionId OPTIONAL,
		OUT		PBYTE pbSessionKey // 0x10 byte out
	);

	NTSYSAPI
	EXPORTNUM(468)
	VOID
	NTAPI
	XamSetHudContext(
		IN		DWORD dwHudContext
	);

	NTSYSAPI
	EXPORTNUM(469)
	DWORD
	NTAPI
	XamGetHudContext(
		VOID
	);
	
	NTSYSAPI
	EXPORTNUM(480)
	NTSTATUS
	NTAPI
	XamDbgPrint(
		IN		DWORD dwAppId,
		IN		const char* s,
		...
	);
	
	NTSYSAPI
	EXPORTNUM(481)
	VOID
	NTAPI
	XamDbgSetOutputLevel(
		IN		DWORD dwAppId, // 0xFE is the only one that sets XDebugOutLevel
		IN		BYTE level	// 0-0xF
	);
	
	NTSYSAPI
	EXPORTNUM(482)
	VOID
	NTAPI
	XamDbgSetBreakLevel(
		IN		DWORD dwAppId, // 0xFE is the only one that sets XDebugOutLevel
		IN		BYTE level	// 0-0xF
	);

	// already in xam.h
	// NTSYSAPI
	// EXPORTNUM(500)
	// HRESULT
	// NTAPI
	// XMsgInProcessCall(
		// IN		HXAMAPP hxamapp,
		// IN		DWORD dwMessage,
		// IN OUT	UINT_PTR param1,
		// IN OUT	UINT_PTR param2
		// );

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(501)
	// HRESULT
	// NTAPI
	// XMsgCompleteIORequest(
		// IN		PXOVERLAPPED pOverlapped,
		// IN		DWORD dwStatus,
		// IN		DWORD dwExtendedError,
		// IN		ULONG ulBytesTransferred
	// );

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(502)
	// HRESULT
	// NTAPI
	// XMsgSystemProcessCall(
		// IN		HXAMAPP hxamapp,
		// IN		DWORD dwMessage,
		// IN		PVOID pUserBuffer,
		// IN		DWORD cbUserBuffer
	// );

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(503)
	// HRESULT
	// NTAPI
	// XMsgStartIORequest(
		// IN		HXAMAPP hxamapp,
		// IN		DWORD dwMessage,
		// IN		PXOVERLAPPED pOverlapped,
		// IN		PVOID pUserBuffer,
		// IN		DWORD cbUserBuffer
	// );

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(504)
	// HRESULT
	// NTAPI
	// XMsgCancelIORequest(
		// IN		PXOVERLAPPED pOverlapped,
		// IN		BOOL fWait
	// );

	NTSYSAPI
	EXPORTNUM(512)
	BOOL
	NTAPI
	XamFeatureEnabled(
		IN		DWORD feature
	);
	
	NTSYSAPI
	EXPORTNUM(520)
	NTSTATUS
	NTAPI
	XamUserGetDeviceContext(
		IN		DWORD dwUserIndex,
		IN		DWORD category,
		IN OUT	PDWORD DeviceContext
	);

	NTSYSAPI
	EXPORTNUM(521)
	NTSTATUS
	NTAPI
	XamUserLookupDevice( 
		IN	DWORD dwDeviceContext,
		IN  DWORD dwCategory,
		OUT PDWORD pdwUserIndex
	);

	NTSYSAPI
	EXPORTNUM(522)
	HRESULT
	NTAPI
	XamUserGetXUID(
		IN		DWORD dwUserIndex,
		IN		DWORD unk,
		OUT		PXUID onlineOut
	);

	NTSYSAPI
	EXPORTNUM(523)
	HRESULT
	NTAPI
	XamUserLogon(
		IN		CONST PXUID rgUsers,
		IN		DWORD dwFlags,
		IN		PXOVERLAPPED pOverlapped OPTIONAL
	);

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(526)
	// DWORD
	// NTAPI
	// XamUserGetName(
		// IN		DWORD dwUserIndex,
		// IN		LPSTR pUserName,
		// IN		DWORD cchUserName
    // );

	NTSYSAPI
	EXPORTNUM(527)
	LPCWSTR
	NTAPI
	XamLookupCommonStringByIndex(
		IN		DWORD index
	);

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(528)
	// XAMUSER_SIGNIN_STATE
	// NTAPI
	// XamUserGetSigninState(
		// IN		DWORD dwUserIndex
    // );

	NTSYSAPI
	EXPORTNUM(530)
	DWORD
	NTAPI
	XamUserCheckPrivilege(
		IN		DWORD dwUserIndex,
		IN		DWORD PrivilegeType,
		OUT		PBOOL pfResult
	);

	NTSYSAPI
	EXPORTNUM(537)
	DWORD
	NTAPI
	XamUserReadProfileSettings(
        DWORD dwTitleId,
        DWORD dwUserIndexRequester,
        DWORD dwNumFor,
        const PXUID pxuidFor,
        DWORD dwNumSettingIds,
        const PDWORD pdwSettingIds,
        PDWORD pcbResults,
        PXUSER_READ_PROFILE_SETTING_RESULT pResults, // in xonline.h
        PXOVERLAPPED pXOverlapped OPTIONAL
    );

	NTSYSAPI
	EXPORTNUM(542)
	BOOL
	NTAPI
	XamUserIsGuest(
		IN		DWORD dwUserIndex
	);

	NTSYSAPI
	EXPORTNUM(545)
	BOOL
	NTAPI
	XamUserIsOnlineEnabled(
		IN		DWORD dwUserIndex
	);

	NTSYSAPI
	EXPORTNUM(551)
	NTSTATUS
	NTAPI
	XamUserGetSigninInfo(
		IN		DWORD userIndex,
		IN		DWORD flags,
		IN OUT	PXUSER_SIGNIN_INFO xSigningInfo
	);

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(556)
	// HRESULT
	// NTAPI
	// XamUserGetUsersMissingAvatars(
		// OUT		DWORD* pdwUserIndexMask,
		// IN OUT	PXOVERLAPPED pOverlapped OPTIONAL
    // );

	NTSYSAPI
	EXPORTNUM(561)
	DWORD
	NTAPI
	XamProfileCreateEnumerator(
		IN		XCONTENTDEVICEID DeviceID,
		OUT		PHANDLE phEnum
	);

	NTSYSAPI
	EXPORTNUM(562)
	DWORD
	NTAPI
	XamProfileEnumerate(
		IN		HANDLE hEnum,
		IN		DWORD dwFlags,
		OUT		PPROFILEENUMRESULT pProfileEnumResult,
		IN OUT	PXOVERLAPPED pOverlapped OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(580)
	HRESULT
	NTAPI
	XamAppLoad(
		IN		LPCSTR appName,
		IN		DWORD dwUserIndex,
		IN		PVOID params,
		IN		DWORD sz,
		IN OUT	PXOVERLAPPED xov,
		OUT		PVOID CSystemAppEntry
	);

	NTSYSAPI
	EXPORTNUM(581)
	VOID
	NTAPI
	XamAppUnloadSelf(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(587)
	HRESULT
	NTAPI
	XamRegisterSysApp(
		IN		HANDLE hInstDLL,
		IN		BYTE AppId,
		IN		PVOID MessageProc, // function
		IN		BOOL ignoreHudMessages
	);

	NTSYSAPI
	EXPORTNUM(588)
	HRESULT
	NTAPI
	XamUnregisterSysApp(
		IN		HANDLE hInstDLL,
		IN		BYTE AppId,
		IN		int xuiModuleId // used by XuiUnregisterModuleId
	);

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(590)
	// DWORD
	// NTAPI
	// XamCreateEnumeratorHandle(
		// IN		DWORD dwUserIndex,
		// IN		HXAMAPP hxamapp,
		// IN		DWORD dwMsgIDEnum,
		// IN		DWORD dwMsgIDCloseEnum,
		// IN		DWORD cbSizeOfPrivateEnumStructure,
		// IN		DWORD cItemsRequested,
		// IN		DWORD dwEnumFlags,
		// OUT		PHANDLE phEnum
    // );

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(591)
	// DWORD
	// NTAPI
	// XamGetPrivateEnumStructureFromHandle(
		// IN		HANDLE hEnum,
		// OUT		PVOID* ppvObj
    // );

	// NOT TESTED, same as XEnumerate with dwFlags added
	NTSYSAPI
	EXPORTNUM(592)
	DWORD
	NTAPI
	XamEnumerate(
		IN		HANDLE hEnum,
		IN		DWORD dwFlags,
		IN OUT  PVOID pvBuffer,
		IN		DWORD cbBuffer,
		OUT		PDWORD pcItemsReturned OPTIONAL,
		IN		PXOVERLAPPED pOverlapped OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(599)
	DWORD
	NTAPI
	XamContentLaunchImageFromFileInternal(
		IN		LPCSTR path,
		IN		LPCSTR xex
	);

	NTSYSAPI
	EXPORTNUM(602)
	DWORD
	NTAPI
	XamContentClose(
		IN		LPCSTR pszRootName, // pathtooriginalmapping
		IN OUT	PXOVERLAPPED pOverlapped OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(605)
	DWORD
	NTAPI
	XamContentCreateDeviceEnumerator(
		IN      DWORD dwContentTypes,
		IN      DWORD dwFlags,
		IN		DWORD cItem,
		OUT		PDWORD pcbBuffer,
		OUT		PHANDLE phEnum
	);

	NTSYSAPI
	EXPORTNUM(606)
	DWORD
	NTAPI
	XamContentGetDeviceData(
		IN		XCONTENTDEVICEID DeviceID,
		OUT		PXDEVICE_DATA pDeviceData
	);
	
	NTSYSAPI
	EXPORTNUM(607)
	DWORD
	NTAPI
	XamContentGetDeviceName(
		IN      XCONTENTDEVICEID            DeviceID,
		OUT     LPWSTR                      pwszName,
		IN      DWORD                       cchName
	);

	NTSYSAPI
	EXPORTNUM(615)
	HRESULT
	NTAPI
	XamContentFlush(
		IN      LPCSTR pszRootName,
		IN      PXOVERLAPPED pOverlapped OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(617)
	DWORD
	NTAPI
	XamContentOpenFile(
		IN		DWORD dwUserIndex,
		IN		LPCSTR pszRootName,
		IN		LPCSTR pszFileName,
		IN		DWORD dwContentFlags,
		IN		DWORD dwFileCacheSize,
		OUT		PDWORD pdwLicenseMask, OPTIONAL
		IN OUT	PXOVERLAPPED pOverlapped OPTIONAL
	);
	
	NTSYSAPI
	EXPORTNUM(619)
	DWORD
	NTAPI
	XamContentLockUnlockPackageHeaders(
		IN		PXCONTENT_MOUNTED_PACKAGE pMountedPackage,
		IN		BOOL fLock
	);

	NTSYSAPI
	EXPORTNUM(630)
	NTSTATUS
	NTAPI
	XamContentResolveInternal(
		PXCONTENT_DATA_INTERNAL pcdi,
		char *pszPath,
		DWORD cchPath,
		int fCreateDir,
		const char *pszRootName,
		PXOVERLAPPED overlapped
	);

	// untested
	NTSYSAPI
	EXPORTNUM(633)
	DWORD
	NTAPI
	XamContentAggregateCreateEnumerator(
		IN		XUID xuid,
		IN		XCONTENTDEVICEID DeviceId,
		IN		DWORD dwContentType,
		IN		DWORD dwTitleId,
		OUT		PHANDLE phEnum
	);
	
	NTSYSAPI
	EXPORTNUM(638)
	DWORD
	NTAPI
	XamContentClosePackageFile(
		IN		PXCONTENT_MOUNTED_PACKAGE pMountedPackage
	);

	NTSYSAPI
	EXPORTNUM(640)
	NTSTATUS
	NTAPI
	XamGetExecutionId(
		IN OUT	PXEX_EXECUTION_ID* xid
	);
	
	NTSYSAPI
	EXPORTNUM(642)
	DWORD
	NTAPI
	XamGetSystemVersion(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(648)
	DWORD
	NTAPI
	XamContentGetMountedPackageByRootName(
		IN		LPCSTR szRootName,
		IN		BOOL unk,
		IN OUT	PXCONTENT_MOUNTED_PACKAGE* ppMount
	);

	NTSYSAPI
	EXPORTNUM(650)
	DWORD
	NTAPI
	XamNotifyCreateListener(
		IN		QWORD qwAreas,
		IN		DWORD dwArea
	);

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(651)
	// BOOL
	// NTAPI
	// XNotifyGetNext(
		// IN		HANDLE hNotification,
		// IN		DWORD dwMsgFilter,
		// OUT		PDWORD pdwId,
		// OUT		PULONG_PTR pParam OPTIONAL
	// );

	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(652)
	// VOID
	// NTAPI
	// XNotifyPositionUI(
		// IN    DWORD dwPosition
	// );

	// already in sdk xam.h
	// returns 0 on success, 0x65B on error
	// NTSYSAPI
	// EXPORTNUM(653)
	// DWORD
	// NTAPI
	// XNotifyDelayUI(
		// IN		ULONG ulMilliSeconds
	// );

	NTSYSAPI
	EXPORTNUM(654)
	DWORD
	NTAPI
	XNotifyBroadcast(
		IN		DWORD dwNotificationType,
		IN		PVOID pXndata
	);
	
	NTSYSAPI
	EXPORTNUM(656)
	VOID
	NTAPI
	XNotifyQueueUI(
		IN		XNOTIFYQUEUEUI_TYPE exnq,
		IN		DWORD dwUserIndex,
		IN		ULONGLONG qwAreas,
		IN		PWCHAR displayText,
		IN		PVOID contextData
	);

	NTSYSAPI
	EXPORTNUM(657)
	HANDLE
	NTAPI
	XamNotifyCreateListenerInternal(
		IN		ULONGLONG qwAreas,
		IN		BOOL fReceiveInternalNotifications,
		IN		DWORD dwArea // area is 9 or less
	);

	NTSYSAPI
	EXPORTNUM(658)
	VOID
	NTAPI
	XNotifyUISetOptions(
		IN		BOOL pfShow,
		IN		BOOL pfShowMovie,
		IN		BOOL pfPlaySound,
		IN		BOOL pfShowIPTV
    );

	NTSYSAPI
	EXPORTNUM(659)
	VOID
	NTAPI
	XNotifyUIGetOptions(
		IN OUT	BOOL* pfShow OPTIONAL,
		IN OUT	BOOL* pfShowMovie OPTIONAL,
		IN OUT	BOOL* pfPlaySound OPTIONAL,
		IN OUT	BOOL* pfShowIPTV OPTIONAL
    );

	NTSYSAPI
	EXPORTNUM(679)
	DWORD
	NTAPI
	XamUpdateGetCurrentSystemVersion(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(695)
	BOOL
	NTAPI
	XamCacheReset(
		XAM_CACHE_FILE_TYPE FileType
	);

	NTSYSAPI
	EXPORTNUM(714)
	HRESULT
	NTAPI
	XamShowMessageBoxUI(
		IN		DWORD dwUserIndex,
		IN		LPCWSTR wszTitle,
		IN		LPCWSTR wszText,
		IN		DWORD cButtons,
		IN		LPCWSTR* pwszButtons OPTIONAL,
		IN		DWORD dwFocusButton,
		IN		DWORD dwFlags,
		OUT 	PMESSAGEBOX_RESULT pResult,
		IN OUT	PXOVERLAPPED pOverlapped OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(728)
	DWORD
	NTAPI
	XamShowPasscodeVerifyUI(
		IN		DWORD dwUserIndex,
		IN OUT	PDWORD pdwOutbuf,
		IN OUT	PXOVERLAPPED pOverlapped OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(745)
	HRESULT
	NTAPI
	XamShowMessageBox(
		IN		DWORD unk,
		IN		LPCWSTR wszTitle,
		IN		LPCWSTR wszText,
		IN		DWORD cButtons,
		IN		LPCWSTR* pwszButtons OPTIONAL,
		IN		DWORD dwFocusButton,
		IN		MBOXRESULT resFun,
		IN		DWORD dwFlags
	);

	NTSYSAPI
	EXPORTNUM(748)
	BOOL
	NTAPI
	XamIsUIActive(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(750)
	DWORD
	NTAPI
	XamUserCreateAchievementEnumerator(
		IN		DWORD dwTitleId,
		IN		DWORD dwUserIndex,
		IN		XUID xuidRequestee,
		IN		DWORD dwDetailFlags,
		IN		DWORD dwStartingIndex,
		IN		DWORD cItem,
		OUT		PDWORD pcbBuffer,
		OUT		PHANDLE ph
	);

	NTSYSAPI
	EXPORTNUM(754)
	DWORD
	NTAPI
	XamReadImage(
		IN		XTILETYPE eTileType,
		IN		DWORD dwTitleId,
		IN		DWORD dwIndex,				// Not 100% sure, needs to be verified
		IN		ULONGLONG ulImageId,		// Not 100% sure, needs to be verified
		OUT		HXUIBRUSH * hImageBrush
	);

	NTSYSAPI
	EXPORTNUM(755)
	DWORD
	NTAPI
	XamUserCreateTitlesPlayedEnumerator(
		IN		DWORD dwTitleId,
		IN		DWORD dwUserIndex,
		IN		XUID xuid,
		IN		DWORD dwStartingIndex,
		IN		DWORD cItem,
		OUT		PDWORD pcbBuffer,
		OUT		PHANDLE ph
	);

	NTSYSAPI
	EXPORTNUM(757)
	DWORD
	NTAPI
	XamReadTileToTexture(
		IN		XTILETYPE eTileType,
		IN		DWORD dwTitleId,
		IN		ULONGLONG qwImageId,
		IN		DWORD dwUserIndex,
		IN OUT	PBYTE pbTextureBuffer,
		IN		DWORD dwPitch,
		IN		DWORD dwHeight,
		IN OUT	PXOVERLAPPED pXOverlapped OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(969)
	DWORD
	NTAPI
	XamHudGetUserIndex(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(992)
	BOOL
	NTAPI
	XamIsMessageBoxActive(
		VOID
	);
	
	NTSYSAPI
	EXPORTNUM(1036)
	BOOL
	NTAPI
	XamIsNuiUIActive(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(1133)
	HRESULT
	NTAPI
	XamShowPasscodeVerifyUIEx(
		IN		DWORD dwUserIndex,
		OUT		PDWORD pdwOutbuf,
		IN		DWORD optMask,
		IN OUT	PXOVERLAPPED pOverlapped OPTIONAL
	);
	
	// already in sdk xam.h
	// NTSYSAPI
	// EXPORTNUM(1183)
	// HRESULT
	// NTAPI
	// XMsgProcessRequest(
		// IN		HXAMAPP hxamapp,
		// IN		DWORD dwMessage,
		// IN		PVOID pvBuffer,
		// IN		DWORD cbBuffer,
		// IN OUT	PXOVERLAPPED pOverlapped
	// );

	NTSYSAPI
	EXPORTNUM(1191)
	XamLoaderState
	NTAPI
	XamLoaderGetState(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(1192)
	DWORD
	NTAPI
	XamTaskQueryProperty(
		IN		HXAMTASK hxamtask,
		IN		DWORD dwPropertyMask
	);

	NTSYSAPI
	EXPORTNUM(1212)
	HRESULT
	NTAPI
	XampXAuthStartup(
		IN		PVOID xas // actually PXAUTH_SETTINGS in xauth.h
	);
	
	NTSYSAPI
	EXPORTNUM(1213)
	VOID
	NTAPI
	XampXAuthShutdown(
		IN OUT	PDWORD unk OPTIONAL // if return is nonzero should use XampXAuthGetTitleBuffer
	);
	
	NTSYSAPI
	EXPORTNUM(1262)
	PVOID
	NTAPI
	XampXAuthGetTitleBuffer(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(1264)
	HRESULT
	NTAPI
	XamUserGetOnlineXUIDFromOfflineXUID(
		IN		XUID offline,
		OUT		PXUID onlineOut
	);

	NTSYSAPI
	EXPORTNUM(1265)
	BOOL
	NTAPI
	XamIsUserUsingMetricSystem(
		IN		DWORD dwUserIndex
	);

	NTSYSAPI
	EXPORTNUM(1285)
	BOOL
	NTAPI
	XamIsUIThread(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(1286)
	VOID
	XamInputSendXenonButtonPress(
		IN		DWORD dwUserIndex
	);

	NTSYSAPI
	EXPORTNUM(1553)
	NTSTATUS
	NTAPI
	XamContentGetDeviceVolumePath(
		IN		DWORD deviceId,
		OUT		PCHAR szDest,
		IN		DWORD dwDestLen,
		IN		BOOL appendBackslash
	);

	NTSYSAPI
	EXPORTNUM(1590)
	BOOL
	NTAPI
	XamNetworkStorageShouldHideFromTitle(
		IN		DWORD dwTitleId
	);

	NTSYSAPI
	EXPORTNUM(1625)
	NTSTATUS
	NTAPI
	XdfLaunchNewImageFromCache(// this is how XNA titles are started, imageName "xna_titlelauncher.xex" and XLAUNCHDATA_CAFEBABE
		IN		PCHAR imageName,
		IN		PVOID launchData,
		IN		DWORD cbLaunchData,
		IN 		DWORD dwFlags
	);
	
	// returns 0 on success, 0x65B on error
	NTSYSAPI
	EXPORTNUM(1671)
	DWORD
	NTAPI
	XamNotifyDelayUIInternal(
		IN		ULONG ulMilliSeconds
	);

	NTSYSAPI
	EXPORTNUM(1800)
	HRESULT
	NTAPI
	XamGetLiveHiveValueA(
		IN		LPCSTR szName,
		IN		LPSTR szValue,
		IN		DWORD cchValue,
		IN		DWORD unk,
		IN OUT	PXOVERLAPPED xov
	);
	
	NTSYSAPI
	EXPORTNUM(1801)
	HRESULT
	NTAPI
	XamGetLiveHiveValueW(
		IN		LPCWSTR szwName,
		IN		LPWSTR wcsValue,
		IN		DWORD cwchValue,
		IN		DWORD unk,
		IN OUT	PXOVERLAPPED xov
	);

	NTSYSAPI
	EXPORTNUM(1802)
	HRESULT
	NTAPI
	XamGetLiveHiveValueDuringLogonAttemptA(
		IN		LPCSTR szName,
		IN		LPSTR szValue,
		IN		DWORD cchValue,
		IN		DWORD unk
	);

	NTSYSAPI
	EXPORTNUM(1804)
	HRESULT
	NTAPI
	XamQueryLiveHiveA(
		IN		LPCSTR szName,
		IN		LPSTR szValue,
		IN		DWORD cchValue,
		IN OUT	PXOVERLAPPED xov
	);

	NTSYSAPI
	EXPORTNUM(1805)
	HRESULT
	NTAPI
	XamQueryLiveHiveW(
		IN		LPCWSTR szwName,
		IN		LPWSTR wcsValue,
		IN		DWORD cwchValue,
		IN OUT	PXOVERLAPPED xov
	);

	NTSYSAPI
	EXPORTNUM(1902)
	VOID
	NTAPI
	XamSetStagingMode(
		STAGING_MODE mode
	);

	NTSYSAPI
	EXPORTNUM(1903)
	STAGING_MODE
	NTAPI
	XamGetStagingMode(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(2011)
	NTSTATUS
	NTAPI
	XamLaunchAvatarEditor(
		IN		DWORD dwUserIndex,
		OUT		DWORD dwFlags,
		OUT		LPCWSTR param
	);

	NTSYSAPI
	EXPORTNUM(2105)
	VOID
	NTAPI
	XamLoaderRebootToDash(
		IN		PXDASHLAUNCHDATA launchdata
	);

	NTSYSAPI
	EXPORTNUM(2275)
	NTSTATUS
	NTAPI
	XamKinectGetHardwareType(
		IN OUT	PDWORD dwUnk1,
		IN OUT	PDWORD dwUnk2
	);

	NTSYSAPI
	EXPORTNUM(2491)
	DWORD
	NTAPI
	XamMediaVerificationCreate(
		IN		DWORD dwHardwareThread
	);

	NTSYSAPI
	EXPORTNUM(2492)
	DWORD
	NTAPI
	XamMediaVerificationClose(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(2493)
	DWORD
	NTAPI
	XamMediaVerificationVerify(
		IN		DWORD dwMilliseconds,
		IN		LPOVERLAPPED lpOverlapped OPTIONAL,
		IN		LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(2494)
	DWORD
	NTAPI
	XamMediaVerificationFailedBlocks(
		IN OUT	PXSECURITY_FAILURE_INFORMATION pFailureInformation
	);

	NTSYSAPI
	EXPORTNUM(2505)
	HRESULT
	NTAPI
	XamBackgroundDownloadItemAdd(
		DWORD dwUserIdx,
		PBACKGROUND_DOWNLOAD_ITEM pbDlTask,
		PXOVERLAPPED overlapped
	);

	NTSYSAPI
	EXPORTNUM(2510)
	HRESULT
	NTAPI
	XamBackgroundDownloadItemToContentData(
		PBACKGROUND_DOWNLOAD_ITEM pbDlTask,
		DWORD unk,
		PXCONTENT_DATA_INTERNAL pxcData
	);

	NTSYSAPI
	EXPORTNUM(2524)
	BOOL
	NTAPI
	XamBackgroundDownloadSelectDevice(
		DWORD deviceType,
		ULONGLONG size,
		PDWORD deviceID
	);

	NTSYSAPI
	EXPORTNUM(2600)
	NTSTATUS
	NTAPI
	XamSwapDisc(
		IN		BYTE bDiscNum,
		IN		HANDLE hComplete,
		IN		CONST XSWAPDISC_ERROR_TEXT *pErrorStrings
	);

	NTSYSAPI
	EXPORTNUM(2803)
	DWORD
	NTAPI
	XamGetSecurityViolationsDetected(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(2851)
	HRESULT
	NTAPI
	XamTaskGetCompletionStatus(
		IN		HXAMTASK hxamtask
	);

	NTSYSAPI
	EXPORTNUM(2942)
	BOOL
	NTAPI
	XamIsSystemExperienceTitleId(
		IN		DWORD dwTitleId
	);
/* needs work
	NTSYSAPI
	EXPORTNUM(1560)
	NTSTATUS
	NTAPI
	XamNetworkStorageGetStatus(
		IN		DWORD unk1,
		IN OUT	PVOID unk2, // seems to require 0xC bytes
		IN		DWORD unk3,
	);*/
	
	// *** THESE NEED TO BE CHECKED!!! ***

	/* this needs to be added
	NTSYSAPI
	EXPORTNUM(UNKNOWN)
	DWORD
	NTAPI
	XOnlineFindMediaInstanceUrls(
		ULONGLONG qwUserId,
		const char *abMediaInstanceId,
		unsigned long cbResults,
		PFIND_MEDIA_INSTANCE_URLS_RESPONSE pResults,
		PXOVERLAPPED pXOverlapped
	);
	*/

#ifdef __cplusplus
}
#endif



#endif // __XAMEXT_DEFINES_H
