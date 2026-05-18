#ifndef QCOMMON_H
#define QCOMMON_H

#include <stdio.h>

#define qboolean int
#define qtrue   1
#define qfalse  0

#define	MAX_RELIABLE_COMMANDS 64
#define MAX_INFO_STRING     1024
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	512	// max tokens resulting from Cmd_TokenizeString
#define MAX_TOKEN_CHARS     1024    // max length of an individual token
#define MAX_OSPATH          256
#define MAX_QPATH           64
#define MAX_ZPATH           256
#define MAX_MSGLEN          0x4000
#define MAX_HUDELEMENTS             31
#define MAX_HUDELEMS_ARCHIVAL       MAX_HUDELEMENTS
#define MAX_HUDELEMS_CURRENT        MAX_HUDELEMENTS
#define MAX_OBJECTIVES              16
#define MAX_WEAPONS                 64
#define MAX_OTHER_SERVERS           128
#define MAX_GLOBAL_SERVERS          2048
#define MAX_NAME_LENGTH     32      // max length of a client name
#define	MAX_FILE_HANDLES	64
#define	MAXPRINTMSG	4096
#define MAX_STRINGLENGTH            1024
#define MAX_DOWNLOAD_WINDOW         8
#define	MAX_FILE_HANDLES	64

#define PACKET_BACKUP   32

#define CVAR_NOFLAG     0
#define CVAR_ARCHIVE    1
#define CVAR_USERINFO       2   // sent to server on connect or change
#define CVAR_SERVERINFO 4
#define CVAR_SYSTEMINFO 8
#define CVAR_INIT       16
#define CVAR_LATCH      32
#define CVAR_ROM        64
#define CVAR_USER_CREATED   128 // created by a set command
#define CVAR_TEMP       256
#define CVAR_CHEAT      512
#define CVAR_NORESTART      1024    // do not clear when a cvar_restart is issued
#define CVAR_CODINFO    2048

typedef unsigned char byte;
typedef int fileHandle_t;
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

typedef enum
{
    ERR_FATAL = 0x0,
    ERR_VID_FATAL = 0x1,
    ERR_DROP = 0x2,
    ERR_SERVERDISCONNECT = 0x3,
    ERR_DISCONNECT = 0x4,
    ERR_NEED_CD = 0x5,
    ERR_AUTOUPDATE = 0x6,
} errorParm_t;

typedef enum
{
    NS_CLIENT,
    NS_SERVER
} netsrc_t;

typedef enum
{
    NA_BOT = 0,
    NA_BAD = 1,
    NA_LOOPBACK = 2,
    NA_BROADCAST = 3,
    NA_IP = 4,
    NA_IPX = 5,
    NA_BROADCAST_IPX = 6
} netadrtype_t;

typedef enum
{
    TEAM_FREE = 0,
    TEAM_AXIS = 1,
    TEAM_ALLIES = 2,
    TEAM_NUM_TEAMS = 4,
} team_t;

typedef enum
{
    PMSG_CONSOLE = 0,
    PMSG_GAME = 1,
    PMSG_BOLDGAME = 2,
    PMSG_SUBTITLE = 3,
    PMSG_LOGFILE = 4
} print_msg_type_t;

typedef struct
{
    netadrtype_t type;
    byte ip[4];
    byte ipx[10];
    unsigned short port;
} netadr_t;

typedef void netProfileInfo_t;

typedef struct
{
    netsrc_t sock;
    int dropped;
    netadr_t remoteAddress;
    int qport;
    int incomingSequence;
    int outgoingSequence;
    int fragmentSequence;
    int fragmentLength;
    byte fragmentBuffer[MAX_MSGLEN];
    qboolean unsentFragments;
    int unsentFragmentStart;
    int unsentLength;
    byte unsentBuffer[MAX_MSGLEN];
    netProfileInfo_t *netProfile;
} netchan_t;

typedef struct 
{
    int clientIndex;
    team_t team;
    int modelindex;
    int attachModelIndex[6];
    int attachTagIndex[6];
    char name[32];
} clientState_t;

typedef struct usercmd_s
{
    int serverTime; // 0x0  // pm + 4
    byte buttons;   // 0x4  // pm + 8   // console, chat, ads, attack, use
    byte wbuttons;  // 0x5  // pm + 9   // lean left, lean right, reload
    byte weapon;    // 0x6  // pm + 10
    byte gap_0x7;           // pm + 11
    int angles[3];  // 0x8  // pm + 12  [0] = 0x8, [1] = 0xC, [2] = 0x10
    signed char forwardmove;    // 0x14     // pm + 24
    signed char rightmove;      // 0x15     // pm + 25
    signed char upmove;         // 0x16     // pm + 26
    byte gap_0x17;                          // pm + 27
} usercmd_t;

typedef enum
{
    TR_STATIONARY = 0
} trType_t;

typedef struct
{
    trType_t trType;
    int trTime;
    int trDuration;
    vec3_t trBase;
    vec3_t trDelta;
} trajectory_t;

typedef enum
{
    ET_GENERAL = 0,
    ET_PLAYER = 1,
    ET_ITEM = 3,
    ET_MISSILE = 4,
    ET_MOVER = 5,
    ET_PORTAL = 6,
    ET_INVISIBLE = 7,
    ET_SCRIPTMOVER = 8
} entityType_t;

typedef enum
{
    WEAPON_READY = 0,
    WEAPON_RAISING = 1,
    WEAPON_DROPPING = 2,
    WEAPON_FIRING = 3,
    WEAPON_RECHAMBERING = 4,
    WEAPON_RELOADING = 5,
    WEAPON_RELOADING_INTERUPT = 6,
    WEAPON_RELOAD_START = 7,
    WEAPON_RELOAD_START_INTERUPT = 8,
    WEAPON_RELOAD_END = 9,
    WEAPON_MELEE_WINDUP = 10,
    WEAPON_MELEE_RELAX = 11
} weaponstate_t;

typedef enum
{
    PM_NORMAL = 0x0,
    PM_NORMAL_LINKED = 0x1,
    PM_NOCLIP = 0x2,
    PM_UFO = 0x3,
    PM_SPECTATOR = 0x4,
    PM_INTERMISSION = 0x5,
    PM_DEAD = 0x6,
    PM_DEAD_LINKED = 0x7,
} pmtype_t;

typedef enum
{
    LOCMSG_SAFE = 0,
    LOCMSG_NOERR = 1
} msgLocErrType_t;

typedef struct entityState_s
{
    int number;         // 0x0
    entityType_t eType; // 0x4
    int eFlags;         // 0x8
    trajectory_t pos;   // 0xC
    trajectory_t apos;  // 0x30
    int time;           // 0x54
    int time2;          // 0x58
    vec3_t origin2;     // 0x5c
    vec3_t angles2;     // 0x68
    int otherEntityNum; // 0x74
    int attackerEntityNum;  // 0x78
    int groundEntityNum;    // 0x7c
    int constantLight;
    int loopSound;
    int surfType;
    int index; // modelIndex
    int clientNum;
    int iHeadIcon;
    int iHeadIconTeam;
    int solid;
    int eventParm;
    int eventSequence;
    int events[4];
    unsigned int eventParms[4];
    int weapon;
    int legsAnim;
    int torsoAnim;
    int leanf;
    int scale; // used as loopfxid, hintstring, ... and doesn't actually scale a player's model size
    int dmgFlags;
    int animMovetype;
    float fTorsoHeight;
    float fTorsoPitch;
    float fWaistPitch;
} entityState_t;

typedef struct hudelem_s
{
    byte gap[112];
} hudelem_t;

typedef struct hudElemState_s
{
    hudelem_t current[MAX_HUDELEMS_CURRENT];
    hudelem_t archival[MAX_HUDELEMS_ARCHIVAL];
} hudElemState_t;

typedef struct objective_s
{
    int state;
    vec3_t origin;
    int entNum;
    int teamNum;
    int icon;
} objective_t;

typedef struct trace_s
{
    float fraction;     // 0x0
    vec3_t endpos;      // 0x4
    vec3_t normal;      // 0x10
    int surfaceFlags;   // 0x1C
    byte gap0x20[8];
    unsigned short entityNum; // 0x28
    unsigned short partName;  // 0x2A
    byte gap0x2C[2];
    byte allsolid;      // 0x2E
    byte startsolid;    // 0x2F
} trace_t;

typedef struct playerState_s
{
    int commandTime;        // 0x0
    pmtype_t pm_type;       // 0x4
    int bobCycle;           // 0x8
    int pm_flags;           // 0xC
    int pm_time;            // 0x10
    vec3_t origin;          // [0] = 0x14, [1] = 0x18, [2] = 0x1C
    vec3_t velocity;        // [0] = 0x20, [1] = 0x24, [2] = 0x28
    int weaponTime;         // 0x2c
    int weaponDelay;        // 0x30
    int grenadeTimeLeft;    // 0x34
    int iFoliageSoundTime;  // 0x38
    int gravity;            // 0x3C
    float leanf;            // 0x40
    int speed;              // 0x44
    vec3_t delta_angles;    // [0] = 0x48, [1] = 0x4C, [2] = 0x50
    int groundEntityNum;    // 0x54
    vec3_t vLadderVec;      // [0] = 0x58, [1] = 0x5C, [2] = 0x60
    int jumpTime;           // 0x64
    float fJumpOriginZ;     // 0x68
    int legsTimer;          // 0x6C
    int legsAnim;           // 0x70
    int torsoTimer;         // 0x74
    int torsoAnim;          // 0x78
    int movementDir;        // 0x7C
    int eFlags;             // 0x80
    int eventSequence;      // 0x84
    int events[4];          // 0x88
    unsigned int eventParms[4]; // 0x98
    int oldEventSequence;       // 0xA8
    int clientNum;              // 0xAC
    unsigned int weapon;        // 0xB0
    weaponstate_t weaponstate;  // 0xB4
    float fWeaponPosFrac;       // 0xB8
    int viewmodelIndex;         // 0xBC
    vec3_t viewangles;          // 0xC0
    int viewHeightTarget;       // 0xCC
    float viewHeightCurrent;    // 0xD0
    int viewHeightLerpTime;     // 0xD4
    int viewHeightLerpTarget;   // 0xD8
    int viewHeightLerpDown;     // 0xDC
    int viewHeightLerpPosAdj;   // 0xE0
    int damageEvent;            // 0xe4
    int damageYaw;              // 0xe8
    int damagePitch;            // 0xec
    int damageCount;            // 0xf0
    int stats[6];               // 0xf4
    int ammo[MAX_WEAPONS];      // 0x10c
    int ammoclip[MAX_WEAPONS];  // 0x20c
    unsigned int weapons[2];    // 0x30c
    byte weaponslots[8];        // 0x314
    unsigned int weaponrechamber[2]; // 0x31c
    vec3_t mins;                // 0x324
    vec3_t maxs;                // 0x330
    int proneViewHeight;        // 0x33C
    int crouchViewHeight;       // 0x340
    int standViewHeight;        // 0x344
    int deadViewHeight;         // 0x348
    float walkSpeedScale;       // 0x34C // ADS
    float runSpeedScale;        // 0x350
    float proneSpeedScale;      // 0x354
    float crouchSpeedScale;     // 0x358
    float strafeSpeedScale;     // 0x35C
    float backSpeedScale;       // 0x360
    float leanSpeedScale;       // 0x364
    float proneDirection;       // 0x368
    float proneDirectionPitch;  // 0x36c
    float proneTorsoPitch;      // 0x370
    int viewlocked;             // 0x374
    int viewlocked_entNum;      // 0x378
    float friction;             // 0x37C
    int gunfx;                  // 0x380
    int serverCursorHint;       // 0x384
    int serverCursorHintVal;    // 0x388
    trace_t serverCursorHintTrace; // 0x38C
    byte gap_0x3BC[4];
    int iCompassFriendInfo;     // 0x3C0
    float fTorsoHeight;         // 0x3c4
    float fTorsoPitch;          // 0x3c8
    float fWaistPitch;          // 0x3cc
    int entityEventSequence;    // 0x3D0
    int weapAnim;               // 0x3d4
    float aimSpreadScale;       // 0x3d8
    int shellshockIndex;        // 0x3dc
    int shellshockTime;         // 0x3e0
    int shellshockDuration;     // 0x3e4
    objective_t objective[MAX_OBJECTIVES]; // 0x3E8
    hudElemState_t hud;         // 0x5A8
    int deltaTime;              // 0x20C8
} playerState_t;

typedef struct
{
    qboolean valid;
    int snapFlags;
    int serverTime;
    int messageNum;
    int deltaNum;
    int ping;
    int cmdNum;
    playerState_t ps;
    int numEntities;
    int numClients;
    int parseEntitiesNum;
    int parseClientsNum;
    int serverCommandNum;
} clSnapshot_t;

typedef struct 
{
    int p_cmdNumber;
    int p_serverTime;
    int p_realtime;
} outPacket_t;

typedef struct 
{
    int stringOffsets[2048];
    char stringData[16000];
    int dataCount;
} gameState_t;

typedef struct
{
    byte active;
    int keyCatchers;
    byte displayHUDWithKeycatchUI;
    byte cgameInitialized;
    byte cgameInitCalled;
    byte usingAds;
    int frameActiveClientCount;
    int frameClientIndex;
    int timeoutcount;
    clSnapshot_t snap;
    int serverTime;
    int oldServerTime;
    int oldFrameServerTime;
    int serverTimeDelta;
    int oldSnapServerTime;
    qboolean extrapolatedSnapshot;
    qboolean newSnapshots;
    gameState_t gameState;
    char mapname[64];
    int parseEntitiesNum;
    int parseClientsNum;
    int mouseDx[2];
    int mouseDy[2];
    int mouseIndex;
    byte stanceHeld;
    int stancePosition;
    int stanceTime;
    int cgameUserCmdValue;
    int cgameUserHoldableValue;
    qboolean cgameInShellshock;
    float cgameSensitivity;
    float cgameMaxPitchSpeed;
    float cgameMaxYawSpeed;
    vec3_t cgameKickAngles;
    vec3_t viewangles;
    int serverId;
    vec4_t color_allies;
    vec4_t color_axis;
    usercmd_t cmds[128];
    int cmdNumber;
    outPacket_t outPackets[32];
    clSnapshot_t snapshots[32];
    entityState_t entityBaselines[1024];
    entityState_t parseEntities[2048];
    clientState_t parseClients[2048];
    qboolean corruptedTranslationFile;
    char translationVersion[256];
} clientActive_t;

extern clientActive_t cl;

typedef struct
{
    char va_string[2][1024];
    int index;
} va_info_t;

typedef struct {
	byte    *data;
	int maxsize;
	int cursize;        //DAJ renamed from cursize
} cmd_t;

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char        *name;
	char        *string;
	char        *resetString;       // cvar_restart will reset to this value
	char        *latchedString;     // for CVAR_LATCH vars
	int flags;
	qboolean modified;              // set each time the cvar is changed
	int modificationCount;          // incremented each time the cvar is changed
	float value;                    // atof( string )
	int integer;                    // atoi( string )
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

extern int cvar_modifiedFlags;
cvar_t *Cvar_FindVar( const char *var_name );

typedef void *unzFile;
typedef struct
{
    char pakFilename[MAX_OSPATH];
    char pakBasename[MAX_OSPATH];
    char pakGamename[MAX_OSPATH];
    unzFile handle;
    int checksum;
    int pure_checksum;
    int numFiles;
    int referenced;
    int hashSize;
    //...
} pack_t;

typedef struct
{
    char path[MAX_OSPATH];
    char gamedir[MAX_OSPATH];
} directory_t;

typedef struct searchpath_s searchpath_t;

typedef struct searchpath_s
{
    searchpath_t *next;
    pack_t *pak;
    directory_t *dir;
    qboolean bLocalized;
    int language;
} searchpath_t;

typedef union qfile_gus {
	FILE*		o;
	unzFile		z;
} qfile_gut;

typedef struct qfile_us {
	qfile_gut	file;
	qboolean	unique;
} qfile_ut;

typedef struct {
	qfile_ut	handleFiles;
	qboolean	handleSync;
	int			baseOffset;
	int			fileSize;
	int			zipFilePos;
	qboolean	zipFile;
	qboolean	streamed;
	char		name[MAX_ZPATH];
} fileHandleData_t;

extern fileHandleData_t fsh[MAX_FILE_HANDLES];

qboolean FS_HandleSync( fileHandle_t h );
qboolean FS_HandleIsStreamed( fileHandle_t h );
qboolean FS_HandleIsZipFile( fileHandle_t h );
qboolean FS_HandleIsUnique( fileHandle_t h );
void *FS_HandleZipFile( fileHandle_t h );
void *FS_HandleOSFile( fileHandle_t h );
void FS_ClearHandle( fileHandle_t h );
void FS_FCloseFile( fileHandle_t f );

typedef struct languageInfo_t {
    const char *pszName;
    qboolean bPresent;
} languageInfo_t;

typedef struct
 {
    int permanent;
    int temp;
} hunkUsed_t;

typedef struct vm_s vm_t;

typedef struct
 {
    const char *fieldBuffer;
    int mark;
    short unsigned int canonicalStrCount;
    byte developer;
    byte developer_script;
    byte evaluate;
    const char *error_message;
    int error_index;
    int time;
    unsigned int timeArrayId;
    unsigned int pauseArrayId;
    unsigned int levelId;
    unsigned int gameId;
    unsigned int animId;
    unsigned int freeEntList;
    unsigned int tempVariable;
} scrVarPub_t;

extern scrVarPub_t scrVarPub;

typedef struct
{
    char token[1024];
    int lines;
    byte ungetToken;
    byte spaceDelimited;
    byte keepStringQuotes;
    byte csv;
    byte negativeNumbers;
    const char *errorPrefix;
    const char *warningPrefix;
    int backup_lines;
    const char *backup_text;
    char parseFile[64];
} parseInfo_t;

typedef struct
{
    parseInfo_t parseInfo[16];
    int parseInfoNum;
    const char *tokenPos;
    const char *prevTokenPos;
    char line[1024];
} ParseThreadInfo;
typedef void (*xcommand_t)();
typedef struct cmd_function_s {
    int next;
    char *name;
    const char *autoCompleteDir;
    const char *autoCompleteExt;
    xcommand_t function;
} cmd_function_t;

void I_strncpyz(char *dest, const char *src, int destsize);
#define Q_strncpyz I_strncpyz

extern cvar_t *com_developer;
void Com_Error_f(void);
void Com_Crash_f(void);
void Com_Freeze_f(void);
void Com_Quit_f(void);
int Com_Shutdown(void);
void Sys_Quit(void);

extern int se_localized;
const char *SE_GetString(const char *reference, qboolean wantTranslation);
void SE_LoadLanguage(const char *languageName, int force);
void SE_Load(const char *qpath, int force);
void SE_Init(void);
void SE_ShutDown(void);

void SEH_Init_StringEd(int *val, int val2);
void SEH_Shutdown_StringEd(void);
void SEH_InitLanguage(void);
int SEH_UpdateLanguageInfo(void);

#endif /* QCOMMON_H */
