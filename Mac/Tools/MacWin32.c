#define _GNU_SOURCE
/* Converted to C from ASM: MacWin32.cp */
/* Original path: /Users/kevin/Development/i5works/COD2/Project/Mac/Tools/MacWin32.cp */
/* Linux replacement: Win32 API compatibility shims */

#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <pthread.h>
#include <sched.h>

#include "common_types.h"
#include "imports.h"
#include "stubs/gcc40_compat.h"

/* --- Timer functions --- */

MMRESULT timeBeginPeriod(void)
{
    return 0;
}

MMRESULT timeEndPeriod(void)
{
    return 0;
}

DWORD timeGetTime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (DWORD)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/* --- Performance counter --- */
/* Frequency = 1000000 (microseconds), matching the Mac original */

BOOL QueryPerformanceFrequency(void *lpFrequency)
{
    long long *freq = (long long *)lpFrequency;
    *freq = 1000000LL;
    return 1;
}

BOOL QueryPerformanceCounter(void *lpPerformanceCount)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long *counter = (long long *)lpPerformanceCount;
    *counter = (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
    return 1;
}

/* --- Message box --- */

int MessageBoxA(LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    int messageType = uType & 0xf;

    fprintf(stderr, "MessageBox: %s\n", lpText ? lpText : "(null)");
    if (lpCaption && lpCaption[0])
        fprintf(stderr, "  Caption: %s\n", lpCaption);

    if (messageType == 0) {
        /* MB_OK */
        return 1;
    } else if (messageType == 1) {
        /* MB_OKCANCEL - return OK */
        return 1;
    } else if (messageType == 4) {
        /* MB_YESNO - return IDYES=6 */
        return 6;
    }
    return 1;
}

/* --- File attributes --- */

BOOL SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes)
{
    return 0;
}

DWORD GetFileAttributesA(LPCSTR lpFileName)
{
    /* Returns 1 if file exists, -1 if not (matching Mac: access(file, R_OK)) */
    if (access(lpFileName, R_OK) == 0)
        return 1;
    return (DWORD)-1;
}

/* --- Thread functions --- */

BOOL SetThreadPriority(HANDLE hThread, int nPriority)
{
    /* Stub - thread priority rarely matters for game logic */
    return 1;
}

DWORD GetCurrentThreadId(void)
{
    return (DWORD)pthread_self();
}

/* --- Sleep --- */

void WinSleep(DWORD dwMilliseconds)
{
    usleep(dwMilliseconds * 1000);
}

/* --- Interlocked operations --- */
/* Use GCC builtins for atomics */

LONG InterlockedExchangeAdd(volatile LONG *Addend, LONG Value)
{
    return cod2_sync_fetch_and_add_i32((volatile int *)Addend, Value);
}

LONG InterlockedCompareExchange(volatile LONG *Destination, LONG Exchange, LONG Comperand)
{
    return cod2_sync_val_compare_and_swap_i32((volatile int *)Destination, Comperand, Exchange);
}

LONG InterlockedExchange(volatile LONG *Target, LONG Value)
{
    return cod2_sync_lock_test_and_set_i32((volatile int *)Target, Value);
}

/* --- Virtual key mapping --- */
/* Maps Mac virtual key codes to Win32 scan codes */

UINT MapVirtualKeyA(UINT uCode, UINT uMapType)
{
    /* Subset from original: numpad and special keys */
    switch (uCode) {
        case 0x92: return 0x3b; /* VK_NUMPAD2 -> scan */
        case 0x94: return 0x27; /* VK_NUMPAD4 */
        case 0x96: return 0x2c; /* VK_NUMPAD6 */
        case 0x97: return 0x2d; /* VK_NUMPAD7 */
        case 0x98: return 0x2e; /* VK_NUMPAD8 */
        case 0x99: return 0x2f; /* VK_NUMPAD9 */
        case 0xba: return 0x60; /* VK_OEM_1 -> backtick */
        case 0xbb: return 0x5b; /* VK_OEM_PLUS -> [ */
        case 0xbc: return 0x5c; /* VK_OEM_COMMA -> \ */
        case 0xbd: return 0x5d; /* VK_OEM_MINUS -> ] */
        case 0xde: return 0x3d; /* VK_OEM_7 -> = */
        default: return 0;
    }
}

/* --- Memory allocation (VirtualAlloc/VirtualFree) --- */
/* Original Mac version used calloc/free, so we do the same */

LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
    if (lpAddress != NULL)
        return lpAddress;
    return calloc(1, dwSize);
}

BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
    if (lpAddress == NULL)
        return 1;
    if (dwFreeType == 0x8000) /* MEM_RELEASE */
        free(lpAddress);
    return 1;
}

/* --- Display metrics --- */

int GetSystemMetrics(int nIndex)
{
    /* SM_CXSCREEN=0, SM_CYSCREEN=1 */
    /* Return reasonable defaults; the renderer will set actual resolution */
    if (nIndex == 0) return 1024;  /* width */
    if (nIndex == 1) return 768;   /* height */
    return 0;
}

/* --- FindFirstFile / FindNextFile --- */
/* Linux replacement using opendir/readdir + fnmatch */

BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);

/* Internal find-file state */
typedef struct {
    DIR *dir;
    char dirpath[512];
    char pattern[260];
    int find_dirs; /* 1 = find directories, 2 = find by extension */
} FFState;

HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    FFState *state;
    const char *slash;
    const char *wildcard;

    if (!lpFileName || !lpFindFileData)
        return (HANDLE)-1;

    memset(lpFindFileData, 0, sizeof(_WIN32_FIND_DATAA));

    state = (FFState *)calloc(1, sizeof(FFState));
    if (!state)
        return (HANDLE)-1;

    /* Split path into directory + pattern */
    slash = strrchr(lpFileName, '/');
    if (slash) {
        int dirlen = (int)(slash - lpFileName);
        if (dirlen >= (int)sizeof(state->dirpath))
            dirlen = (int)sizeof(state->dirpath) - 1;
        memcpy(state->dirpath, lpFileName, dirlen);
        state->dirpath[dirlen] = '\0';
        strncpy(state->pattern, slash + 1, sizeof(state->pattern) - 1);
    } else {
        strcpy(state->dirpath, ".");
        strncpy(state->pattern, lpFileName, sizeof(state->pattern) - 1);
    }

    /* Check if this is a wildcard pattern */
    wildcard = strstr(state->pattern, "*");
    if (!wildcard) {
        /* Direct file lookup (no wildcard) */
        struct stat st;
        if (stat(lpFileName, &st) != 0) {
            free(state);
            return (HANDLE)-1;
        }
        lpFindFileData->dwFileAttributes = S_ISDIR(st.st_mode) ? 0x10 : 0;
        slash = strrchr(lpFileName, '/');
        strncpy(lpFindFileData->cFileName, slash ? slash + 1 : lpFileName, 259);
        lpFindFileData->nFileSizeLow = (DWORD)(st.st_size & 0xFFFFFFFF);
        lpFindFileData->nFileSizeHigh = (DWORD)(st.st_size >> 32);
        /* Return state with dir=NULL to signal single-result mode */
        return (HANDLE)state;
    }

    state->dir = opendir(state->dirpath);
    if (!state->dir) {
        free(state);
        return (HANDLE)-1;
    }

    /* Find the first matching entry */
    if (FindNextFileA((HANDLE)state, lpFindFileData))
        return (HANDLE)state;

    closedir(state->dir);
    free(state);
    return (HANDLE)-1;
}

BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
    FFState *state = (FFState *)hFindFile;
    struct dirent *entry;
    struct stat st;
    char fullpath[1024];

    if (!state || !lpFindFileData || !state->dir)
        return 0;

    while ((entry = readdir(state->dir)) != NULL) {
        /* Skip . and .. */
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' ||
             (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            continue;

        /* Match against pattern */
        if (fnmatch(state->pattern, entry->d_name, FNM_CASEFOLD) != 0)
            continue;

        /* Fill in find data */
        snprintf(fullpath, sizeof(fullpath), "%s/%s", state->dirpath, entry->d_name);
        memset(lpFindFileData, 0, sizeof(_WIN32_FIND_DATAA));

        if (stat(fullpath, &st) == 0) {
            lpFindFileData->dwFileAttributes = S_ISDIR(st.st_mode) ? 0x10 : 0;
            lpFindFileData->nFileSizeLow = (DWORD)(st.st_size & 0xFFFFFFFF);
            lpFindFileData->nFileSizeHigh = (DWORD)(st.st_size >> 32);
        }
        strncpy(lpFindFileData->cFileName, entry->d_name, 259);
        return 1;
    }
    return 0;
}

/* The C++ std::vector<FFItem>::_M_insert_aux is only used internally by
   FindFirstFileA's Mac implementation. Our Linux version doesn't need it,
   but provide a stub to satisfy the linker. */
void ZNSt6vectorI6FFItemSaIS0_EE13_M_insert_auxEN9__gnu_cxx17__normal_iteratorIPS0_S2_EERKS0_(void)
{
}
