/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include "../server_mp/server.h"

extern cvar_t *sv_running;

/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/server_mp/sv_init_mp.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
void SV_Shutdown(const char *finalmsg)
{
    if (!sv_running || !sv_running->integer)
    {
        return;
    }

    Com_Printf("----- Server Shutdown -----\n");

    if (svs.clients)
    {
        SV_FinalMessage(finalmsg);
    }

    SV_RemoveOperatorCommands();
    SV_MasterShutdown();
    SV_ShutdownGameProgs();
    SV_ClearServer();

    if (svs.clients)
    {
        SV_FreeClients();
    }

    SV_FreeArchivedSnapshot();

    memset(&svs, 0, sizeof(svs));

    Cvar_Set("sv_running", "0");

    Com_Printf("---------------------------\n");

    CL_Disconnect();
}
