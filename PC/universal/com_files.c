/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include "../qcommon/qcommon.h"

#include <stdio.h>
#include <windows.h>
#include <errno.h>

extern searchpath_t *fs_searchpaths; /* 0x0 */

qboolean FS_Initialized(void);
char *FS_ReplaceSeparators(char *path);
/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/com_files.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
qboolean FS_Initialized(void)
{
    return fs_searchpaths != 0;
}

/*
=================
FS_Write

Properly handles partial writes
=================
*/
int FS_Write( const void *buffer, int len, fileHandle_t h ) {
	int block, remaining;
	int written;
	byte    *buf;
	int tries;
	FILE    *f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !h ) {
		return 0;
	}

	f = FS_FileForHandle( h );
	buf = (byte *)buffer;

	remaining = len;
	tries = 0;
	while ( remaining ) {
		block = remaining;
		written = fwrite( buf, 1, block, f );
		if ( written == 0 ) {
			if ( !tries ) {
				tries = 1;
			} else {
				Com_Printf( "FS_Write: 0 bytes written\n" );
				return 0;
			}
		}

		if ( written == -1 ) {
			Com_Printf( "FS_Write: -1 bytes written\n" );
			return 0;
		}

		remaining -= written;
		buf += written;
	}
	if ( FS_HandleSync( h ) ) {
		fflush( f );
	}
	return len;
}

qboolean FS_LanguageHasAssets(int iLanguage)
{
    // Always consider primary languages (0: English to 7: Polish) present
    // to prevent cl_language from being reset to 0 by the engine.
    if (iLanguage >= 0 && iLanguage <= 7) {
        return 1;
    }

    searchpath_t *sp;
    for (sp = fs_searchpaths; sp; sp = sp->next) {
        if (sp->bLocalized && sp->language == iLanguage) {
            return 1;
        }
    }
    return 0;
}
static	int			fs_loadStack;			// total files in memory
float FS_ResetFiles(void)
{
    fs_loadStack = 0;
}

extern void Sys_EndStreamedFile(fileHandle_t h);
extern void unzCloseCurrentFile(void *file);
extern void unzClose(void *file);
void FS_FCloseFile( fileHandle_t f )
{
	if (FS_HandleIsStreamed(f)) {
		Sys_EndStreamedFile(f);
	}
	if (FS_HandleIsZipFile(f) == qtrue) {
		void *zip = FS_HandleZipFile(f);
		unzCloseCurrentFile(zip);
		if (FS_HandleIsUnique(f)) {
			unzClose(zip);
		}
		FS_ClearHandle(f);
		return;
	}

	// we didn't find it as a pak, so close it as a unique file
	if (FS_HandleOSFile(f)) {
		fclose((FILE *)FS_HandleOSFile(f));
	}
	FS_ClearHandle(f);
}

fileHandle_t FS_HandleForFile(void) {
    int i;

    for (i = 1; i <= MAX_FILE_HANDLES; i++) {
        if (!fsh[i].handleFiles.file.o) {
            return i;
        }
    }

    // No free handle found, print all open files and error out
    for (i = 1; i < MAX_FILE_HANDLES; i++) {
        Com_Printf("FILE %2i: '%s'\n", i, fsh[i].name);
    }

    Com_Error(ERR_DROP, "FS_HandleForFile: none free");
    return -1;
}

cvar_t *fs_debug;
cvar_t *fs_copyfiles;
cvar_t *fs_cdpath;
cvar_t *fs_basepath;
cvar_t *fs_homepath;
cvar_t *fs_basegame;
cvar_t *fs_game;
cvar_t *fs_restrict;
cvar_t *fs_ignoreLocalized;
char *fs_gamedir;
searchpath_t *fs_searchpaths;
static int		fs_numServerPaks;


void FS_AddLocalizedGameDirectory(const char *path, const char *gameName) {
    char searchPath[MAX_OSPATH];
    WIN32_FIND_DATAA findData;
    HANDLE hFind;
    
    // First, register the loose directory as a searchpath so we can read unzipped/loose files
    searchpath_t *sp_dir = malloc(sizeof(searchpath_t));
    if (sp_dir) {
        sp_dir->dir = malloc(sizeof(directory_t));
        if (sp_dir->dir) {
            strncpy(sp_dir->dir->path, path, MAX_OSPATH - 1);
            sp_dir->dir->path[MAX_OSPATH - 1] = '\0';
            strncpy(sp_dir->dir->gamedir, gameName, MAX_OSPATH - 1);
            sp_dir->dir->gamedir[MAX_OSPATH - 1] = '\0';
        }
        sp_dir->pak = NULL;
        sp_dir->bLocalized = qfalse;
        sp_dir->language = 0;
        sp_dir->next = fs_searchpaths;
        fs_searchpaths = sp_dir;
    }

    // Now, scan for .pk3 files in this directory
    sprintf(searchPath, "%s/%s/*.pk3", path, gameName);
    FS_ReplaceSeparators(searchPath);
    
    Com_Printf("DEBUG: FS_AddLocalizedGameDirectory scanning path: %s\n", searchPath);
    
    hFind = FindFirstFileA(searchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            Com_Printf("DEBUG:   Found pk3 file: %s\n", findData.cFileName);
            
            // Found a pk3 file! Create a searchpath and register its localized metadata
            searchpath_t *sp_pak = malloc(sizeof(searchpath_t));
            if (sp_pak) {
                sp_pak->pak = NULL;
                sp_pak->dir = NULL;
                
                // Identify localized pack files and tag their corresponding languages
                const char *langNames[] = {
                    "english", "french", "german", "italian", "spanish", "british",
                    "russian", "polish", "korean", "taiwanese", "japanese", "chinese",
                    "thai", "leet"
                };
                qboolean foundLang = qfalse;
                for (int l = 0; l < 14; l++) {
                    char targetStr[64];
                    sprintf(targetStr, "localized_%s", langNames[l]);
                    if (strstr(findData.cFileName, targetStr)) {
                        sp_pak->bLocalized = qtrue;
                        sp_pak->language = l;
                        foundLang = qtrue;
                        break;
                    }
                }
                if (!foundLang) {
                    sp_pak->bLocalized = qfalse;
                    sp_pak->language = 0;
                }
                
                sp_pak->next = fs_searchpaths;
                fs_searchpaths = sp_pak;
                
                fs_numServerPaks++;
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    } else {
        Com_Printf("DEBUG:   FindFirstFileA failed for %s! Error: %d\n", searchPath, GetLastError());
    }
}

void FS_Startup(const char *gameName) {
    const char *homePath;

    Com_Printf("--- FS_Startup ---\n");

    // Register cvars
    fs_debug    = Cvar_Get("fs_debug",          "0",    CVAR_NOFLAG);
    fs_copyfiles = Cvar_Get("fs_copyfiles",     "0",    CVAR_INIT);
    fs_cdpath   = Cvar_Get("fs_cdpath",         Sys_DefaultCDPath(),      CVAR_INIT);
    fs_basepath = Cvar_Get("fs_basepath",       Sys_DefaultInstallPath(), CVAR_INIT);
    fs_basegame = Cvar_Get("fs_basegame",       "",     CVAR_INIT);

    // homepath falls back to basepath if Sys_DefaultHomePath returns NULL or empty
    homePath = Sys_DefaultHomePath();
    if (!homePath || !*homePath) {
        homePath = fs_basepath->string;
    }
    fs_homepath  = Cvar_Get("fs_homepath",  homePath, CVAR_INIT);
    fs_game      = Cvar_Get("fs_game",      "",       CVAR_INIT | CVAR_SYSTEMINFO);
    if (fs_game && fs_game->string && fs_game->string[0]) {
        fs_gamedir = fs_game->string;
    } else {
        fs_gamedir = (char *)gameName;
    }
    fs_restrict  = Cvar_Get("fs_restrict",  "",       CVAR_INIT);
    fs_ignoreLocalized = Cvar_Get("fs_ignoreLocalized", "0", CVAR_CHEAT | CVAR_LATCH);

    // Add base game directories (cdpath, basepath, homepath if different)
    if (*fs_cdpath->string)
        FS_AddLocalizedGameDirectory(fs_cdpath->string, gameName);

    if (*fs_basepath->string)
        FS_AddLocalizedGameDirectory(fs_basepath->string, gameName);

    if (*fs_basepath->string && Q_stricmp(fs_homepath->string, fs_basepath->string))
        FS_AddLocalizedGameDirectory(fs_homepath->string, gameName);

    // Add fs_basegame directories if set and different from gameName
    if (*fs_basegame->string && !Q_stricmp(gameName, "main") && Q_stricmp(fs_basegame->string, gameName)) {
        if (*fs_cdpath->string)
            FS_AddLocalizedGameDirectory(fs_cdpath->string,   fs_basegame->string);
        if (*fs_basepath->string)
            FS_AddLocalizedGameDirectory(fs_basepath->string, fs_basegame->string);
        if (*fs_homepath->string && Q_stricmp(fs_homepath->string, fs_basepath->string))
            FS_AddLocalizedGameDirectory(fs_homepath->string, fs_basegame->string);
    }

    // Add fs_game directories if set and different from gameName
    if (*fs_game->string && !Q_stricmp(gameName, "main") && Q_stricmp(fs_game->string, gameName)) {
        if (*fs_cdpath->string)
            FS_AddLocalizedGameDirectory(fs_cdpath->string,   fs_game->string);
        if (*fs_basepath->string)
            FS_AddLocalizedGameDirectory(fs_basepath->string, fs_game->string);
        if (*fs_homepath->string && Q_stricmp(fs_homepath->string, fs_basepath->string))
            FS_AddLocalizedGameDirectory(fs_homepath->string, fs_game->string);
    }

    // Add non-pack (loose file) directories for asset types
    // FS_AddNonPackFileDirectory("xanim",      "");
    // FS_AddNonPackFileDirectory("xmodel",     "");
    // FS_AddNonPackFileDirectory("xmodelparts","");
    // FS_AddNonPackFileDirectory("xmodelsurfs","");
    // FS_AddNonPackFileDirectory("weapons",    "");
    // FS_AddNonPackFileDirectory("animtrees",  "atr");

    // CD key handling
    // Com_ReadCDKey("main");
    // {
    //     cvar_t *game = Cvar_Get("fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO);
    //     if (game && *game->string) {
    //         Com_AppendCDKey();
    //     }
    // }

    // FS_AddCommands();
    // FS_Path_f();

    // // Clear modified flag on fs_game
    // fs_game->modificationCount = 0;

    Com_Printf("----------------------\n");
    Com_Printf("%d files in pk3 files\n", fs_numServerPaks);
}

char *FS_ReplaceSeparators(char *path) {
    char *s = path;

    while (*s) {
        if (*s == '/') {
            *s = '\\';
        }
        s++;
    }

    return path;
}

static void FS_BuildOSPath_Internal(const char *base, const char *game, const char *qpath, char *ospath, qboolean streamThread) {
    int baseLen, gameLen, qpathLen, totalLen;

    // Fall back to fs_gamedir if game is NULL or empty
    if (!game || !*game) {
        game = fs_gamedir;
    }

    baseLen   = strlen(base);
    gameLen   = strlen(game);
    qpathLen  = strlen(qpath);
    totalLen  = baseLen + gameLen + qpathLen;

    if (totalLen + 2 >= MAX_OSPATH) {
        if (streamThread) {
            ospath[0] = '\0';
            return;
        }
        Com_Error(ERR_FATAL, "FS_BuildOSPath: path length exceeded for: %s / %s / %s", base, game, qpath);
    }

    // Assemble: base/game/qpath
    memcpy(ospath, base, baseLen);
    ospath[baseLen] = '/';
    memcpy(ospath + baseLen + 1, game, gameLen);
    ospath[baseLen + 1 + gameLen] = '/';
    memcpy(ospath + baseLen + 2 + gameLen, qpath, qpathLen + 1); // +1 for null terminator

    FS_ReplaceSeparators(ospath + baseLen);
}

void FS_BuildOSPath(const char *base, const char *game, const char *qpath, char *ospath) {
    FS_BuildOSPath_Internal(base, game, qpath, ospath, qfalse);
}

fileHandle_t FS_FOpenTextFileWrite(const char *filename) {
    fileHandle_t h;
    char ospath[MAX_OSPATH];

    h = FS_HandleForFile();

    fsh[h].zipFile = qfalse;

    FS_BuildOSPath(fs_homepath->string, fs_gamedir, filename, ospath);

    Com_Printf("DEBUG: FS_FOpenTextFileWrite built ospath: %s\n", ospath);

    if (FS_CreatePath(ospath)) {
        Com_Printf("DEBUG: FS_CreatePath failed for: %s\n", ospath);
        return 0;
    }

    fsh[h].handleFiles.file.o = fopen(ospath, "wt");
    I_strncpyz(fsh[h].name, filename, sizeof(fsh[h].name));
    fsh[h].zipFile = qfalse;

    if (!fsh[h].handleFiles.file.o) {
        Com_Printf("DEBUG: FS_FOpenTextFileWrite failed to open: %s, errno: %d\n", ospath, errno);
        return 0;
    } else {
        Com_Printf("DEBUG: FS_FOpenTextFileWrite successfully opened: %s for writing!\n", ospath);
    }

    return h;
}

void FS_Flush(fileHandle_t h) {
    fflush(fsh[h].handleFiles.file.o);
}