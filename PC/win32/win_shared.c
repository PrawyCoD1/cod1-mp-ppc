#include "../qcommon/qcommon.h"
#include <windows.h>

int sys_timeBase = 0;
static qboolean initialized = qfalse;

int Sys_Milliseconds(void)
{
    if (!initialized) {
        sys_timeBase = timeGetTime();
        initialized = qtrue;
    }
    return timeGetTime() - sys_timeBase;
}