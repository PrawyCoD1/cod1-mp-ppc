// struct clientConnection_t {
//     int connectTime;
//     int connectPacketCount;
//     char serverMessage[256];
//     int challenge;
//     int checksumFeed;
//     int reliableSequence;
//     int reliableAcknowledge;
//     char reliableCommands[128][1024];
//     int serverMessageSequence;
//     int serverCommandSequence;
//     int lastExecutedServerCommand;
//     char serverCommands[128][1024];
//     fileHandle_t download;
//     char downloadTempName[256];
//     char downloadName[256];
//     int downloadNumber;
//     int downloadBlock;
//     int downloadCount;
//     int downloadSize;
//     char downloadList[1024];
//     qboolean downloadRestart;
//     char demoName[64];
//     qboolean demorecording;
//     qboolean demoplaying;
//     qboolean isTimeDemo;
//     qboolean demowaiting;
//     qboolean firstDemoFrameSkipped;
//     fileHandle_t demofile;
//     fileHandle_t timeDemoLog;
//     int timeDemoFrames;
//     int timeDemoStart;
//     int timeDemoPrev;
//     int timeDemoBaseTime;
//     struct netchan_t netchan;
//     netProfileInfo_t *pOOBProf;
// };

#include "../qcommon/qcommon.h"

typedef enum {
	CA_DISCONNECTED = 0,    // not talking to a server
	CA_CONNECTING = 1,      // sending request packets to the server
	CA_CHALLENGING = 2,     // sending challenge packets to the server
	CA_CONNECTED = 3,       // netchan_t established, getting gamestate
	CA_LOADING = 4,         // only during cgame initialization, never during main loop
	CA_PRIMED = 5,          // got gamestate, waiting for first frame
	CA_ACTIVE = 6,          // game views should be displayed
	CA_CINEMATIC = 7        // playing a cinematic or a static pic, not connected to a server
} connstate_t;

typedef enum {
	GLDRV_ICD,                  // driver is integrated with window system
								// WARNING: there are tests that check for
								// > GLDRV_ICD for minidriverness, so this
								// should always be the lowest value in this
								// enum set
	GLDRV_STANDALONE,           // driver is a non-3Dfx standalone driver
	GLDRV_VOODOO                // driver is a 3Dfx standalone driver
} glDriverType_t;

typedef enum {
	GLHW_GENERIC,           // where everthing works the way it should
	GLHW_3DFX_2D3D,         // Voodoo Banshee or Voodoo3, relevant since if this is
							// the hardware type then there can NOT exist a secondary
							// display adapter
	GLHW_RIVA128,           // where you can't interpolate alpha
	GLHW_RAGEPRO,           // where you can't modulate alpha on alpha textures
	GLHW_PERMEDIA2          // where you don't have src*dst
} glHardwareType_t;

/*
** glconfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum {
	TC_NONE,
	TC_S3TC,
	TC_EXT_COMP_S3TC
} textureCompression_t;

typedef struct {

	int			clientNum;
	int			lastPacketSentTime;			// for retransmits during connection
	int			lastPacketTime;				// for timeouts

	netadr_t	serverAddress;
	int			connectTime;				// for connection retransmits
	int			connectPacketCount;			// for display on connection dialog
	char		serverMessage[MAX_STRING_TOKENS];	// for display on connection dialog

	int			challenge;					// from the server to use for connecting
	int			checksumFeed;				// from the server for checksum calculations

	// these are our reliable messages that go to the server
	int			reliableSequence;
	int			reliableAcknowledge;		// the last one the server has executed
	char		reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];

	// server message (unreliable) and command (reliable) sequence
	// numbers are NOT cleared at level changes, but continue to
	// increase as long as the connection is valid

	// message sequence is used by both the network layer and the
	// delta compression layer
	int			serverMessageSequence;

	// reliable messages received from server
	int			serverCommandSequence;
	int			lastExecutedServerCommand;		// last server command grabbed or executed with CL_GetServerCommand
	char		serverCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];

	// file transfer from server
	fileHandle_t download;
	char		downloadTempName[MAX_OSPATH];
	char		downloadName[MAX_OSPATH];
	int			downloadNumber;
	int			downloadBlock;	// block we are waiting for
	int			downloadCount;	// how many bytes we got
	int			downloadSize;	// how many bytes we got
	char		downloadList[MAX_INFO_STRING]; // list of paks we need to download
	qboolean	downloadRestart;	// if true, we need to do another FS_Restart because we downloaded a pak

	// demo information
	char		demoName[MAX_QPATH];
	qboolean	demorecording;
	qboolean	demoplaying;
	qboolean	demowaiting;	// don't record until a non-delta message is received
	qboolean	firstDemoFrameSkipped;
	fileHandle_t	demofile;

	int			timeDemoFrames;		// counter of rendered frames
	int			timeDemoStart;		// cls.realtime before first frame
	int			timeDemoBaseTime;	// each frame will be at this time + frameNum * 50

	// big stuff at end of structure so most offsets are 15 bits or less
	netchan_t	netchan;
} clientConnection_t;

extern	clientConnection_t clc;

typedef struct {
	netadr_t adr;
	char hostName[MAX_NAME_LENGTH];
	char mapName[MAX_NAME_LENGTH];
	char game[MAX_NAME_LENGTH];
	int netType;
	int gameType;
	int clients;
	int maxClients;
	int minPing;
	int maxPing;
	int ping;
	qboolean visible;
	int allowAnonymous;
} serverInfo_t;

typedef struct {
	byte ip[4];
	unsigned short port;
} serverAddress_t;

typedef struct {
	char renderer_string[MAX_STRING_CHARS];
	char vendor_string[MAX_STRING_CHARS];
	char version_string[MAX_STRING_CHARS];
	char extensions_string[4 * MAX_STRING_CHARS];                       // this is actually too short for many current cards/drivers  // (SA) doubled from 2x to 4x MAX_STRING_CHARS

	int maxTextureSize;                             // queried from GL
	int maxActiveTextures;                          // multitexture ability

	int colorBits, depthBits, stencilBits;

	glDriverType_t driverType;
	glHardwareType_t hardwareType;

	qboolean deviceSupportsGamma;
	textureCompression_t textureCompression;
	qboolean textureEnvAddAvailable;
	qboolean anisotropicAvailable;                  //----(SA)	added
	float maxAnisotropy;                            //----(SA)	added

	// vendor-specific support
	// NVidia
	qboolean NVFogAvailable;                    //----(SA)	added
	int NVFogMode;                                  //----(SA)	added
	// ATI
	int ATIMaxTruformTess;                          // for truform support
	int ATINormalMode;                          // for truform support
	int ATIPointMode;                           // for truform support


	int vidWidth, vidHeight;
	// aspect is the screen's physical width / height, which may be different
	// than scrWidth / scrHeight if the pixels are non-square
	// normal screens should be 4/3, but wide aspect monitors may be 16/9
	float windowAspect;

	int displayFrequency;

	// synonymous with "does rendering consume the entire screen?", therefore
	// a Voodoo or Voodoo2 will have this set to TRUE, as will a Win32 ICD that
	// used CDS.
	qboolean isFullscreen;
	qboolean stereoEnabled;
	qboolean smpActive;                     // dual processor

	qboolean textureFilterAnisotropicAvailable;                 //DAJ
} glconfig_t;

typedef int qhandle_t;
typedef struct {
	connstate_t state;              // connection status
	int keyCatchers;                // bit flags

	qboolean cddialog;              // bring up the cd needed dialog next frame
	qboolean endgamemenu;           // bring up the end game credits menu next frame

	char servername[MAX_OSPATH];            // name of server from original connect (used by reconnect)

	// when the server clears the hunk, all of these must be restarted
	qboolean rendererStarted;
	qboolean soundStarted;
	qboolean soundRegistered;
	qboolean uiStarted;
	qboolean cgameStarted;

	int framecount;
	int frametime;                  // msec since last frame

	int realtime;                   // ignores pause
	int realFrametime;              // ignoring pause, so console always works

	int numlocalservers;
	serverInfo_t localServers[MAX_OTHER_SERVERS];

	int numglobalservers;
	serverInfo_t globalServers[MAX_GLOBAL_SERVERS];
	// additional global servers
	int numGlobalServerAddresses;
	serverAddress_t globalServerAddresses[MAX_GLOBAL_SERVERS];

	int numfavoriteservers;
	serverInfo_t favoriteServers[MAX_OTHER_SERVERS];

	int nummplayerservers;
	serverInfo_t mplayerServers[MAX_OTHER_SERVERS];

	int pingUpdateSource;       // source currently pinging or updating

	int masterNum;

	// update server info
	netadr_t updateServer;
	char updateChallenge[MAX_TOKEN_CHARS];
	char updateInfoString[MAX_INFO_STRING];

	netadr_t authorizeServer;

	// rendering info
	glconfig_t glconfig;
	qhandle_t charSetShader;
	qhandle_t whiteShader;
	qhandle_t consoleShader;
	qhandle_t consoleShader2;   //----(SA)	added

} clientStatic_t;

extern clientStatic_t cls;