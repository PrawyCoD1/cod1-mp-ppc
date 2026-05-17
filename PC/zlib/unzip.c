/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include "unzip.h"
#include <stdlib.h>

#ifndef TRYFREE
#define TRYFREE(p) {if (p) free(p);}
#endif

/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/zlib/unzip.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
int unzCloseCurrentFile(unzFile file)
{
    unz_s *s;
    file_in_zip_read_info_s *readInfo;

    if (file == NULL)
        return UNZ_PARAMERROR;

    s = (unz_s *)file;
    readInfo = s->pfile_in_zip_read;
    if (readInfo == NULL)
        return UNZ_PARAMERROR;

    free(readInfo->read_buffer);
    readInfo->read_buffer = NULL;

    if (readInfo->stream_initialised)
        inflateEnd(&readInfo->stream);

    readInfo->stream_initialised = 0;
    free(readInfo);
    s->pfile_in_zip_read = NULL;
    return UNZ_OK;
}

/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */
int unzClose (unzFile file)
{
	unz_s* s;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;

    if (s->pfile_in_zip_read!=NULL)
        unzCloseCurrentFile(file);

	fclose(s->file);
	TRYFREE(s);
	return UNZ_OK;
}
