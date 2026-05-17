#include <stdio.h>
#include <time.h>

typedef unsigned int uInt;
typedef unsigned long uLong; /* 32 bits or more */

#define UNZ_OK                                  (0)
#define UNZ_PARAMERROR                  (-102)

typedef struct tm_unz_s
{
    uInt tm_sec;            /* seconds after the minute - [0,59] */
    uInt tm_min;            /* minutes after the hour - [0,59] */
    uInt tm_hour;           /* hours since midnight - [0,23] */
    uInt tm_mday;           /* day of the month - [1,31] */
    uInt tm_mon;            /* months since January - [0,11] */
    uInt tm_year;           /* years - [1980..2044] */
} tm_unz;

typedef void* unzFile;
typedef void* (*alloc_func) (void* opaque, unsigned int items, unsigned int size);
typedef void   (*free_func) (void* opaque, void* address);

typedef struct z_stream_s {
    unsigned char    *next_in;  /* next input unsigned char */
    unsigned int     avail_in;  /* number of unsigned chars available at next_in */
    unsigned long    total_in;  /* total nb of input unsigned chars read so */

    unsigned char    *next_out; /* next output unsigned char should be put there */
    unsigned int     avail_out; /* remaining free space at next_out */
    unsigned long    total_out; /* total nb of unsigned chars output so */

    char     *msg;      /* last error message, NULL if no error */
    struct internal_state *state; /* not visible by applications */

    alloc_func zalloc;  /* used to allocate the internal state */
    free_func  zfree;   /* used to free the internal state */
    unsigned char*     opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: ascii or binary */
    unsigned long   adler;      /* adler32 value of the uncompressed data */
    unsigned long   reserved;   /* reserved for future use */
} z_stream;

typedef struct 
{
    char *read_buffer;
    z_stream stream;
    uLong pos_in_zipfile;
    uLong stream_initialised;
    uLong offset_local_extrafield;
    uInt size_local_extrafield;
    uLong pos_local_extrafield;
    uLong rest_read_compressed;
    uLong rest_read_uncompressed;
    FILE *file;
    uLong compression_method;
    uLong byte_before_the_zipfile;
} file_in_zip_read_info_s;

typedef struct 
{
    uLong offset_curfile;
} unz_file_info_internal_s;

typedef struct
{
    uLong version;
    uLong version_needed;
    uLong flag;
    uLong compression_method;
    uLong dosDate;
    uLong crc;
    uLong compressed_size;
    uLong uncompressed_size;
    uLong size_filename;
    uLong size_file_extra;
    uLong size_file_comment;
    uLong disk_num_start;
    uLong internal_fa;
    uLong external_fa;
    tm_unz tmu_date;
} unz_file_info_s;

typedef struct
{
    uLong number_entry;
    uLong size_comment;
} unz_global_info_s;

typedef struct
{
    FILE *file;
    unz_global_info_s gi;
    uLong byte_before_the_zipfile;
    uLong num_file;
    uLong pos_in_central_dir;
    uLong current_file_ok;
    uLong central_pos;
    uLong size_central_dir;
    uLong offset_central_dir;
    unz_file_info_s cur_file_info;
    unz_file_info_internal_s cur_file_info_internal;
    file_in_zip_read_info_s *pfile_in_zip_read;
} unz_s;
