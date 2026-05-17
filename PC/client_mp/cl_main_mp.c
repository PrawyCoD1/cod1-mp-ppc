#include "../qcommon/qcommon.h"
#include "../client_mp/client.h"

#include <stdlib.h>
#include <string.h>

/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif
clientConnection_t	clc;
clientStatic_t		cls;

extern void MSG_WriteReliableCommandToBuffer(const char *pszCommand, char *pszBuffer, int iBufferSize);
/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/client_mp/cl_main_mp.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */

void CL_AddReliableCommand(const char *cmd)
{
  int index; // r0

  if ( clc.reliableSequence - clc.reliableAcknowledge > MAX_RELIABLE_COMMANDS )
    Com_Error(ERR_DROP, "EXE_ERR_CLIENT_CMD_OVERFLOW");

  clc.reliableSequence++;
  index = clc.reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 );
  MSG_WriteReliableCommandToBuffer(cmd, clc.reliableCommands[ index ], 1024);
}

void CL_StopRecord_f(void)
{
    int len;

    if ( !clc.demorecording ) {
        Com_Printf ("Not recording a demo.\n");
        return;
    }

    len = -1;
    FS_Write(len, 4, clc.demofile);
    FS_Write(len, 4, clc.demofile);
    FS_FCloseFile(clc.demofile);
    clc.demofile = 0;
    clc.demorecording = 0;
    Com_Printf("Stopped demo.\n");
}

clientActive_t		cl;
void CL_ClearState(void)
{
    memset(&cl, 0, 0x12CE78);
}

extern int Cmd_Argc(void);
extern char *Cmd_Argv(int arg);
extern void I_strncpyz(char *dest, const char *src, int destsize);
extern void I_strncat(char *dest, int destsize, const char *src);
void CL_Setenv_f(void)
{
    char buffer[1024];
    int argc, i;
    const char *env;

    argc = Cmd_Argc();

    if (argc > 2) {
        /* Set environment variable: "setenv VAR value1 value2 ..." */
        I_strncpyz(buffer, Cmd_Argv(1), sizeof(buffer));
        I_strncat(buffer, sizeof(buffer), "="); /* "=" */
        for (i = 2; i < argc; i++) {
            I_strncat(buffer, sizeof(buffer), Cmd_Argv(i));
            I_strncat(buffer, sizeof(buffer), " "); /* " " */
        }
        putenv(buffer);
    } else if (argc == 2) {
        /* Print environment variable */
        env = getenv(Cmd_Argv(1));
        if (env)
            Com_Printf("%s=%s\n", Cmd_Argv(1), env);
        else
            Com_Printf("%s undefined\n", Cmd_Argv(1));
    }
}

extern void Cbuf_AddText(const char *text);
extern const char *va(const char *fmt, ...);
void CL_Reconnect_f(void)
{
    char *server = cls.servername;

    /* Don't reconnect to empty or localhost */
    if (!*server || !memcmp(server, "localhost", 10)) {
        Com_Printf("Can't reconnect to localhost.\n");
        return;
    }

    Cbuf_AddText(va("connect %s\n", server));
}

void CL_Shutdown(void)
{
    static qboolean recursive = qfalse;

    Com_Printf("----- CL_Shutdown -----\n");

    if (recursive)
    {
        printf("Recursive shutdown\n");
        return;
    }

    recursive = qtrue;
/*TODO*/
    // CL_ShutdownDebugData();

    // CL_Disconnect(qtrue);
    // CL_ShutdownCGame();

    // MSS_Shutdown();
    // CL_ShutdownRef();
    // CL_ShutdownUI();
    // CL_ShutdownInput();

    Cmd_RemoveCommand("cmd");
    Cmd_RemoveCommand("configstrings");
    Cmd_RemoveCommand("clientinfo");
    Cmd_RemoveCommand("snd_restart");
    Cmd_RemoveCommand("vid_restart");
    Cmd_RemoveCommand("disconnect");
    Cmd_RemoveCommand("record");
    Cmd_RemoveCommand("demo");
    Cmd_RemoveCommand("cinematic");
    Cmd_RemoveCommand("stoprecord");
    Cmd_RemoveCommand("connect");
    Cmd_RemoveCommand("reconnect");
    Cmd_RemoveCommand("localservers");
    Cmd_RemoveCommand("globalservers");
    Cmd_RemoveCommand("rcon");
    Cmd_RemoveCommand("setenv");
    Cmd_RemoveCommand("ping");
    Cmd_RemoveCommand("serverstatus");
    Cmd_RemoveCommand("showip");
    Cmd_RemoveCommand("fs_openedList");
    Cmd_RemoveCommand("fs_referencedList");
    Cmd_RemoveCommand("updatehunkusage");
    Cmd_RemoveCommand("updatescreen");
    Cmd_RemoveCommand("startSingleplayer");
    Cmd_RemoveCommand("buyNow");
    Cmd_RemoveCommand("singlePlayLink");
    Cmd_RemoveCommand("setRecommended");
    Cmd_RemoveCommand("cubemapShot");

    Cvar_Set("cl_running", "0");

    recursive = qfalse;

    memset(&cls, 0, sizeof(cls));

    Com_Printf("-----------------------\n");
}
