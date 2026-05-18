/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include "../qcommon/qcommon.h"

#include <stdarg.h>
#include <stddef.h>

static char *rd_buffer; /* rd_buffer */
static int rd_buffersize; /* rd_buffersize */
static void	(*rd_flush)( char *buffer );

extern cvar_t* com_dedicated;
extern cvar_t* com_logfile;

qboolean	com_errorEntered;
char	com_errorMessage[MAXPRINTMSG];
/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/common.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
void Com_Printf(const char *fmt, ...)
{
    char msg[4096];
    va_list argptr;

    va_start(argptr, fmt);
    vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Com_PrintMessage(0, msg);
}

static fileHandle_t logfile;
static qboolean opening_qconsole;
void Com_PrintMessage(print_msg_type_t type, const char *msg)
{
    extern void I_strncat(char *dest, int destsize, const char *src);
    extern void Sys_Print(const char *msg);
    extern int FS_Initialized(void);
    extern void FS_Write(const void *buffer, int len, int h);
    extern void FS_Flush(int f);
    extern int FS_FOpenTextFileWrite(const char *filename);
    extern void CL_ConsolePrint(int type, const char *msg, int unknown1, int unknown2);

    if (rd_buffer) {
        if (type == 4)
            return;
        if (strlen(msg) + strlen(rd_buffer) - 1 > (unsigned)(rd_buffersize - 1)) {
            rd_flush(rd_buffer);
            *rd_buffer = '\0';
        }
        I_strncat(rd_buffer, rd_buffersize, msg);
        return;
    }

    if (type != 4 && (!com_dedicated || !com_dedicated->integer)) {
        CL_ConsolePrint(type, msg, 0, 0);
    }

    /* strip color codes */
    if (msg[0] == '^' && msg[1] != '\0')
        msg += 2;

    if (type != 4)
        Sys_Print(msg);

    if (!com_logfile || !com_logfile->integer)
        return;
    if (!FS_Initialized())
        return;

    if (!logfile) {
        if (opening_qconsole)
            return;
        opening_qconsole = 1;
        {
            long aclock;
            time(&aclock);
            logfile = FS_FOpenTextFileWrite("qconsole_mp.log");
            Com_Printf("logfile opened on %s\n", asctime(localtime(&aclock)));
        }
        opening_qconsole = 0;
        if (!logfile)
            return;
    }

    FS_Write(msg, strlen(msg), logfile);
    if (com_logfile->integer > 1) {
        FS_Flush(logfile);
    }
}

/*
=============
Com_Quit_f

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Quit_f( void ) {
	// don't try to shutdown if we are in a recursive error
	if ( !com_errorEntered ) {
		SV_Shutdown ("Server quit\n");
		CL_Shutdown ();
		Com_Shutdown ();
		//FS_Shutdown(qtrue); //todo
	}
	Sys_Quit ();
}

#include <setjmp.h>
cvar_t* com_recommendedSet;
cvar_t* com_dedicated;
cvar_t* com_maxfps;
cvar_t* developer;
cvar_t* developer_script;
cvar_t* com_logfile;
cvar_t* com_statmon;
cvar_t* com_timescale;
cvar_t* com_fixedtime;
cvar_t* com_viewlog;
cvar_t* com_speeds;
cvar_t* sv_paused;
cvar_t* cl_paused;
cvar_t* sv_running;
cvar_t* com_cl_running;
cvar_t* com_introplayed;
cvar_t* com_animCheck;
void Com_Init(char *commandLine){
    Com_Printf("%s %s build %s %s\n", "COD MP", "1.5", "win-x86", __DATE__);
    
    jmp_buf *abortframe = (jmp_buf *)Sys_GetValue(2);

    if (setjmp(*abortframe)) {
        Sys_Error("Error during initialization");
        return;
    }

    Com_InitPushEvent();

    Cvar_Init();

    // prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_ParseCommandLine( commandLine );

	Swap_Init();
	Cbuf_Init();

    Cmd_Init();

	// override anything from the config files with command line args
	Com_StartupVariable( "developer" );

    // done early so bind command exists
	CL_InitKeyCommands();

    FS_InitFilesystem();

	Com_InitJournaling();

    Cbuf_AddText( "exec default_mp.cfg\n" );

	Cbuf_AddText( "exec language.cfg\n" ); //----(SA)	added

    Cbuf_AddText( "exec config_mp.cfg\n" );

	int autoExec = Cbuf_AddText( "exec autoexec_mp.cfg\n" );

    if ( Com_SafeMode(autoExec) )
    {
        Cbuf_AddText("exec safemode_mp.cfg\n");
    }

    Cbuf_Execute();

    com_recommendedSet = Cvar_Get("com_recommendedSet", "0", CVAR_ARCHIVE);

    if ( !com_recommendedSet->integer || Com_ConfigureFileChanged() )
    {
        Com_SetRecommended(0);
        Cvar_Set("com_recommendedSet", "1");
    }

    if ( Sys_InfoChanged() )
        Com_SetRecommended(0);

    Com_StartupVariable(NULL);  
    SEH_UpdateLanguageInfo(); 

    com_dedicated = Cvar_Get("dedicated", "0", CVAR_LATCH);

    /* If dedicated, show console and exit splash */
    if (com_dedicated->integer) {
        Sys_HideSplashWindow();
        Sys_ShowConsole(1, 1);
        Sys_NormalExit();
    }

    Com_InitHunkMemory();

    cvar_modifiedFlags &= ~CVAR_ARCHIVE;
    com_maxfps = Cvar_Get("com_maxfps", "85", CVAR_ARCHIVE);
    developer = Cvar_Get("developer", "0", CVAR_TEMP);
    developer_script = Cvar_Get("developer_script", "0", CVAR_TEMP);
    com_logfile = Cvar_Get("logfile", "0", CVAR_NOFLAG);
    com_statmon = Cvar_Get("com_statmon", "0", CVAR_NOFLAG);
    com_timescale = Cvar_Get("timescale", "1.0", CVAR_CODINFO|CVAR_CHEAT);
    com_fixedtime = Cvar_Get("com_fixedtime", "0", CVAR_CHEAT);
    com_viewlog = Cvar_Get("com_viewlog", "0", CVAR_CHEAT);
    com_speeds = Cvar_Get("com_speeds", "0", CVAR_NOFLAG);
    sv_paused = Cvar_Get("sv_paused", "0", CVAR_ROM);
    cl_paused = Cvar_Get("cl_paused", "0", CVAR_ROM);
    sv_running = Cvar_Get("sv_running", "0", CVAR_ROM);
    com_cl_running = Cvar_Get("com_cl_running", "0", CVAR_ROM);
    com_introplayed = Cvar_Get("com_introplayed", "0", CVAR_ARCHIVE);
    com_animCheck = Cvar_Get("com_animCheck", "0", CVAR_NOFLAG);
    AskForPBEULA();

    if (com_dedicated->integer && !com_viewlog->integer )
        Cvar_Set("com_viewlog", "0");

    if ( com_developer && com_developer->integer ) {
		Cmd_AddCommand ("error", Com_Error_f);
		Cmd_AddCommand ("crash", Com_Crash_f );
		Cmd_AddCommand ("freeze", Com_Freeze_f);
	}

    Cmd_AddCommand ("quit", Com_Quit_f);

    //TODO
}
static int timeStamp = 1;
void Com_ResetSkeletonCache(void)
{
    timeStamp++;

    if (!timeStamp)
    {
        timeStamp++;
    }
}

void Com_ClearTempMemory(void)
{
    Com_ResetSkeletonCache();

    Hunk_ClearTempMemory();
    Hunk_ClearTempMemoryHigh();
}
    extern const char * SEH_LocalizeTextMessage(const char *pszInputBuffer, const char *pszMessageType, msgLocErrType_t errType);
void Com_SetErrorMessage(const char *errMsg)
{
    const char *translation;

    Cvar_Get("com_errorMessage", "", CVAR_ROM);

    if (!errMsg || !errMsg[0])
    {
        Cvar_Set("com_errorMessage", "");
        return;
    }

    translation = SEH_LocalizeTextMessage(errMsg, "error message", 1);

    if (!translation)
    {
        Cvar_Set("com_errorMessage", errMsg);
        return;
    }

    Cvar_Set("com_errorMessage", translation);
    Q_strncpyz(com_errorMessage, translation, 4096);
}
static int iWeaponInfoSource; /* iWeaponInfoSource */
extern int bg_iNumWeapons;
void Com_FreeWeaponInfoMemory(int iSource, qboolean force)
{
    if (iSource == iWeaponInfoSource)
    {
        if (!force)
        {
            bg_iNumWeapons = 0;
        }

        iWeaponInfoSource = 0;
    }
}

void Com_Error(errorParm_t code, const char *fmt, ...)
{
    va_list argptr;
    char msg[4104];
    int now;
    static int	lastErrorTime;
    static int	errorCount;

    #ifndef QDECL
    #if defined(_MSC_VER)
    #define QDECL __cdecl
    #else
    #define QDECL
    #endif
    #endif

    extern void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames );
    extern void I_strncpyz(char *dest, const char *src, int destsize);
    extern int	QDECL VM_Call( vm_t *vm, int callnum, ... );

    if (com_errorEntered)
    {
        Sys_Error("recursive error after: %s", com_errorMessage);
    }

    Com_ClearTempMemory();

    va_start(argptr, fmt);
    vsprintf(com_errorMessage, fmt, argptr);
    va_end(argptr);

    strcpy(msg, com_errorMessage);

    if ((unsigned int)(code - 6) <= 1)
    {
        code = ERR_FATAL; // 1
    }

    com_errorEntered = 1;

    FS_PureServerSetLoadedPaks("", "");

    SEH_UpdateLanguageInfo();

    if (code == ERR_DROP || code == ERR_SERVERDISCONNECT || code == ERR_DISCONNECT)
    {
        if (com_errorMessage[0])
        {
            const char *localized;

            localized = SEH_LocalizeTextMessage(
                com_errorMessage,
                "error message",
                1
            );

            if (localized)
            {
                I_strncpyz(com_errorMessage, localized, 4096);
            }
        }
    }
    else
    {
        if (sv_running)
        {
            VM_Call(sv_running, 7);
        }

        Com_SetErrorMessage(com_errorMessage);
    }

    if (code != ERR_DROP)
    {
        Scr_Abort();
    }

    //MSS_ErrorCleanup(); //TODO
    //Com_CleanupSkeletons(); //todo
    Com_ResetParseSessions();

    // if (com_unknownSkeletonFlag)
    // {
    //     Com_ShutdownSkeletons(&com_unknownSkeletonData);
    // }

    FS_ResetFiles();

    if (code == ERR_FATAL)
    {
        Cbuf_Init();
    }

    Com_FreeWeaponInfoMemory(1, 0);
    Com_FreeWeaponInfoMemory(2, 0);

    now = Sys_Milliseconds();

    if (now - lastErrorTime >= 100)
    {
        errorCount = 0;
    }
    else
    {
        errorCount++;

        if (errorCount > 3)
        {
            code = ERR_FATAL;
        }
    }

    lastErrorTime = now;

    // if ((unsigned int)(code - 1) <= 4)
    // {
    //     com_errorCode = code;
    //     com_sv_running = 0;
    //     Com_PrintMessage(CON_CHANNEL_ERROR, com_errorMessage, -1);
    // }

    CL_Shutdown();

    SV_Shutdown(va("EXE_SERVER_FATAL_CRASHED %s", msg));

    Hunk_ClearToStart();

    Com_Close();

    Sys_Error("%s", com_errorMessage);
}
