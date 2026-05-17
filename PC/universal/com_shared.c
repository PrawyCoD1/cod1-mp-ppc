/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif
#include "../qcommon/qcommon.h"

static byte *s_hunkData; /* s_hunkData */
static hunkUsed_t hunk_low; /* hunk_low */
static hunkUsed_t hunk_high; /* hunk_high */
/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/com_shared.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
void Hunk_ClearTempMemory(void)
{
    if (s_hunkData) {
        hunk_low.temp = hunk_low.permanent;
    }
}

void Hunk_ClearTempMemoryHigh(void)
{
    hunk_high.temp = hunk_high.permanent;
}