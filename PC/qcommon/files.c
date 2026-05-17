/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include "../qcommon/qcommon.h"

#include <stdio.h>

#define MAX_ZPATH			256
#define	MAX_SEARCH_PATHS	4096

static int		fs_numServerPaks;
static int		fs_serverPaks[MAX_SEARCH_PATHS];				// checksums
static char		*fs_serverPakNames[MAX_SEARCH_PATHS];			// pk3 names

static qboolean fs_reordered;
static int fs_checksumFeed;

/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/files.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
fileHandleData_t	fsh[MAX_FILE_HANDLES];

qboolean FS_HandleSync( fileHandle_t h ) {
	return fsh[h].handleSync;
}

qboolean FS_HandleIsStreamed( fileHandle_t h ) {
	return fsh[h].streamed;
}

qboolean FS_HandleIsZipFile( fileHandle_t h ) {
	return fsh[h].zipFile;
}

qboolean FS_HandleIsUnique( fileHandle_t h ) {
	return fsh[h].handleFiles.unique;
}

void *FS_HandleZipFile( fileHandle_t h ) {
	return fsh[h].handleFiles.file.z;
}

void *FS_HandleOSFile( fileHandle_t h ) {
	return fsh[h].handleFiles.file.o;
}

extern void Com_Memset(void *dest, int val, int count);
void FS_ClearHandle( fileHandle_t h ) {
	Com_Memset( &fsh[h], 0, sizeof( fsh[h] ) );
}

/*
=====================
FS_PureServerSetLoadedPaks

If the string is empty, all data sources will be allowed.
If not empty, only pk3 files that match one of the space
separated checksums will be checked for files, with the
exception of .cfg and .dat files.
=====================
*/
void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames ) {
	int		i, c, d;

	Cmd_TokenizeString( pakSums );

	c = Cmd_Argc();
	if ( c > MAX_SEARCH_PATHS ) {
		c = MAX_SEARCH_PATHS;
	}

	fs_numServerPaks = c;

	for ( i = 0 ; i < c ; i++ ) {
		fs_serverPaks[i] = atoi( Cmd_Argv( i ) );
	}

	if (fs_numServerPaks) {
		Com_DPrintf( "Connected to a pure server.\n" );
	}
	else
	{
		if (fs_reordered)
		{
			// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=540
			// force a restart to make sure the search order will be correct
			Com_DPrintf( "FS search reorder is required\n" );
			FS_Restart(fs_checksumFeed);
			return;
		}
	}

	for ( i = 0 ; i < c ; i++ ) {
		if (fs_serverPakNames[i]) {
			Z_Free(fs_serverPakNames[i]);
		}
		fs_serverPakNames[i] = NULL;
	}
	if ( pakNames && *pakNames ) {
		Cmd_TokenizeString( pakNames );

		d = Cmd_Argc();
		if ( d > MAX_SEARCH_PATHS ) {
			d = MAX_SEARCH_PATHS;
		}

		for ( i = 0 ; i < d ; i++ ) {
			fs_serverPakNames[i] = CopyString( Cmd_Argv( i ) );
		}
	}
}
