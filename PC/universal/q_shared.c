/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../qcommon/qcommon.h"
/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/q_shared.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
extern int Sys_GetValue(int);

char * va(const char *format, ...)
{
    va_list argptr;
    va_info_t *info;
    char *buf;
    int len;

    va_start(argptr, format);
    info = (va_info_t *)Sys_GetValue(1);
    buf = info->va_string[info->index];
    info->index = (info->index + 1) & 1;

    len = vsnprintf(buf, 1024, format, argptr);
    va_end(argptr);
    buf[1023] = '\0';

    if ((unsigned)len > 1023) {
        Com_Error(1, "va string too long");
    }

    return buf;
}

void I_strncpyz(char *dest, const char *src, int destsize)
{
    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = '\0';
}

void I_strncat(char *dest, int size, const char *src)
{
    int destLen;

    destLen = strlen(dest);
    if (destLen >= size) {
        Com_Error(0, "I_strncat: already overflowed");
    }

    I_strncpyz(dest + destLen, src, size - destLen);
}

char I_CleanChar(int character)
{
    if ((unsigned char)character == 0x92)
        return '\'';
    return (char)character;
}

int Q_stricmp(const char *s1, const char *s2) {
    return _stricmp(s1, s2);
}

void Com_Memset(void *dest, int val, int count) {
    memset(dest, val, count);
}

void Com_Memcpy(void *dest, const void *src, int count) {
    memcpy(dest, src, count);
}

int Com_sprintf(char *dest, int size, const char *fmt, ...) {
    va_list argptr;
    int len;
    va_start(argptr, fmt);
    len = vsnprintf(dest, size, fmt, argptr);
    va_end(argptr);
    return len;
}

char *CopyString(const char *in) {
    char *out = (char *)malloc(strlen(in) + 1);
    if (out) {
        strcpy(out, in);
    }
    return out;
}

void Z_Free(void *ptr) {
    free(ptr);
}

// Globals and structures for FS_InitFilesystem
int dword_1407450 = 0;
char fs_cdpath_copy[256] = {0};
char fs_basepath_copy[256] = {0};

extern cvar_t *fs_cdpath;
extern cvar_t *fs_basepath;

extern void Com_StartupVariable(const char *var);
extern void SEH_InitLanguage(void);
extern void FS_Startup(const char *path);
extern void SEH_Init_StringEd(int *val, int val2);
extern void SEH_UpdateLanguageInfo(void);
extern void FS_SetRestrictions(void);
extern int FS_ReadFile(const char *qpath, void **buffer);

char *FS_InitFilesystem(void)
{
    char *result;

    Com_StartupVariable("fs_cdpath");
    Com_StartupVariable("fs_basepath");
    Com_StartupVariable("fs_homepath");
    Com_StartupVariable("fs_game");
    Com_StartupVariable("fs_copyfiles");
    Com_StartupVariable("fs_restrict");
    Com_StartupVariable("fs_usewolf");
    Com_StartupVariable("cl_language");
    
    SEH_InitLanguage();
    FS_Startup("main");
    Com_Printf("DEBUG: FS_Startup main completed successfully.\n");
    
    Com_Printf("DEBUG: Calling SEH_Init_StringEd...\n");
    SEH_Init_StringEd(&dword_1407450, 0);
    Com_Printf("DEBUG: SEH_Init_StringEd completed successfully.\n");
    
    Com_Printf("DEBUG: Calling SEH_UpdateLanguageInfo...\n");
    SEH_UpdateLanguageInfo();
    Com_Printf("DEBUG: SEH_UpdateLanguageInfo completed successfully.\n");
    
    Com_Printf("DEBUG: Calling FS_SetRestrictions...\n");
    FS_SetRestrictions();
    Com_Printf("DEBUG: FS_SetRestrictions completed successfully.\n");
    
    Com_Printf("DEBUG: Calling FS_ReadFile for default_mp.cfg...\n");
    int readResult = FS_ReadFile("default_mp.cfg", NULL);
    Com_Printf("DEBUG: FS_ReadFile returned %d\n", readResult);
    if (readResult <= 0) {
        Com_Printf("DEBUG: Couldn't load default_mp.cfg! Triggering error...\n");
        Com_Error(0, "Couldn't load %s.  Make sure Call of Duty is run from the correct folder.", "default_mp.cfg");
    }
    
    if (fs_cdpath && fs_cdpath->string) {
        strncpy(fs_cdpath_copy, fs_cdpath->string, 0xFF);
        fs_cdpath_copy[0xFF] = '\0';
    }
    
    if (fs_basepath && fs_basepath->string) {
        result = strncpy(fs_basepath_copy, fs_basepath->string, 0xFF);
        fs_basepath_copy[0xFF] = '\0';
    } else {
        result = fs_basepath_copy;
    }
    
    return result;
}