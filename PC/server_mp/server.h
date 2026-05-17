#include "../qcommon/qcommon.h"

#define	MAX_CHALLENGES	1024

typedef struct gentity_s gentity_s;

typedef enum
{
    CS_FREE,
    CS_ZOMBIE,
    CS_CONNECTED,
    CS_PRIMED,
    CS_ACTIVE
} clientConnectState_t;

typedef struct
{
    char command[MAX_STRINGLENGTH];
    int cmdTime;
    int cmdType;
} reliableCommands_t;

typedef struct
{
    byte linked;            // 0x0  (ent + 240)
    byte gap_0x1[3];
    byte svFlags;           // 0x4  (... + 244)
    byte gap_0x5[0xF];      //      (... + 245)
    vec3_t mins;            // 0x14 (... + 260)
    vec3_t maxs;            // 0x20 (... + 272)
    /* singleClient moved to 0x2C since 0x29 overlaps maxs when vec3_t is 12 bytes */
    int  singleClient;      // 0x2C
    byte gap_0x30[0x8];     // fills to preserve size/offsets up to original layout
    int  contents;          // 0x38 (... + 284)
    byte gap_0x3C[0x30];
} entityShared_t;

typedef struct gentity_s
{
    entityState_t s;        // 0x0
    entityShared_t r;       // 0xF0
    struct gclient_s *client;   // 0x15C
    byte gap_0x160[0x13];
    byte watertype;         // 0x173
    byte waterlevel;        // 0x174
    byte takedamage;        // 0x175
    byte gap_0x176[0x6];
    unsigned short classname;     // 0x17c
    byte gap_0x17E[0x6];
    int flags;              // 0x184
    byte gap_0x188[0x10];
    int clipmask;           // 0x198
    byte gap_0x19C[0x84];
    void (*die)(gentity_s *self, gentity_s *inflictor, gentity_s *attacker, int damage, int meansOfDeath, int iWeapon, const float *vDir, int hitLoc); // 0x220
    byte gap_0x224[0xF8];
} gentity_t;

typedef struct
{
    playerState_t ps;
    int num_entities;
    int num_clients;
    int first_entity;
    int first_client;
    unsigned int messageSent;
    unsigned int messageAcked;
    int messageSize;
} clientSnapshot_t;

typedef struct client_s
{
    clientConnectState_t state;
    qboolean sendAsActive;
    const char *dropReason;
    char userinfo[MAX_INFO_STRING];
    reliableCommands_t reliableCommands[MAX_RELIABLE_COMMANDS];
    int reliableSequence;
    int reliableAcknowledge;
    int reliableSent;
    int messageAcknowledge;
    int gamestateMessageNum;
    int challenge;
    usercmd_t lastUsercmd;
    int lastClientCommand;
    char lastClientCommandString[MAX_STRINGLENGTH];
    gentity_t *gentity;
    char name[MAX_NAME_LENGTH];
    char downloadName[MAX_QPATH];
    fileHandle_t download;
    int downloadSize;
    int downloadCount;
    int downloadClientBlock;
    int downloadCurrentBlock;
    int downloadXmitBlock;
    unsigned char *downloadBlocks[MAX_DOWNLOAD_WINDOW];
    int downloadBlockSize[MAX_DOWNLOAD_WINDOW];
    qboolean downloadEOF;
    int downloadSendTime;
    char downloadURL[MAX_OSPATH];
    qboolean wwwOk;
    qboolean downloadingWWW;
    qboolean clientDownloadingWWW;
    qboolean wwwFallback;
    int deltaMessage;
    int nextReliableTime;
    int lastPacketTime;
    int lastConnectTime;
    int nextSnapshotTime;
    qboolean rateDelayed;
    int timeoutCount;
    clientSnapshot_t frames[PACKET_BACKUP];
    int ping;
    int rate;
    int snapshotMsec;
    int pureAuthentic;
    netchan_t netchan;
    int guid;
    unsigned short clscriptid;
    int bIsTestClient;
    int serverId;
    char PBGuid[33];
} client_t;

typedef struct
{
    netadr_t adr;
    int challenge;
    int time;
    int pingTime;
    int firstTime;
    int firstPing;
    qboolean connected;
    int guid;
    char PBGuid[33];
} challenge_t;

typedef struct
{
    qboolean initialized;
    int time;
    int time2;
    int snapFlagServerBit;
    client_t *clients;
    byte gap_0x83CCD94[0x4C];
    challenge_t challenges[MAX_CHALLENGES];
    //...
} serverStatic_t;

extern	serverStatic_t	svs;
