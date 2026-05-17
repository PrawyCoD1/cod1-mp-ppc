#include "../qcommon/qcommon.h"
#include <stddef.h>
#include <stdarg.h>
#include <math.h>
#include <GL/gl.h>
#include <ctype.h>
#include <stdio.h>

// VM-related variables
typedef struct vm_s {
    int (*entryPoint)( int callnum, ... );
    int compiled;
} vm_t;

vm_t *currentVM = NULL;
vm_t *lastVM = NULL;
int vm_debugLevel = 0;

// Sub-system stub functions
void Com_InitPushEvent(void) {}
void Com_ParseCommandLine(char *commandLine) {}
void Swap_Init(void) {}
void Cmd_Init(void) {}
void Com_StartupVariable(const char *var) {}
void CL_InitKeyCommands(void) {}
void Com_InitJournaling(void) {}

// Authentic FS_InitFilesystem helper stubs
void SEH_InitLanguage(void) {}
void FS_Startup(const char *path) {}
void SEH_Init_StringEd(int *val, int val2) {}
void FS_SetRestrictions(void) {}

#pragma pack(push, 1)
typedef struct {
    unsigned int signature;
    unsigned short version;
    unsigned short flags;
    unsigned short compression;
    unsigned short modTime;
    unsigned short modDate;
    unsigned int crc32;
    unsigned int compressedSize;
    unsigned int uncompressedSize;
    unsigned short filenameLength;
    unsigned short extraFieldLength;
} localHeader_t;
#pragma pack(pop)

typedef struct z_stream_s {
    const unsigned char *next_in;
    unsigned int     avail_in;
    unsigned long    total_in;

    unsigned char    *next_out;
    unsigned int     avail_out;
    unsigned long    total_out;

    const char       *msg;
    struct internal_state *state;

    void * (*zalloc) (void *, unsigned int, unsigned int);
    void   (*zfree)  (void *, void *);
    void *     opaque;

    int     data_type;
    unsigned long   adler;
    unsigned long   reserved;
} z_stream;

static int inflate_raw(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen) {
    z_stream strm;
    strm.next_in = source;
    strm.avail_in = sourceLen;
    strm.next_out = dest;
    strm.avail_out = *destLen;
    strm.zalloc = NULL;
    strm.zfree = NULL;
    strm.opaque = NULL;

    extern const char * zlibVersion(void);
    extern int inflateInit2_(z_stream *strm, int windowBits, const char *version, int stream_size);
    extern int inflate(z_stream *strm, int flush);
    extern int inflateEnd(z_stream *strm);

    int err = inflateInit2_(&strm, -15, zlibVersion(), sizeof(z_stream));
    if (err != 0) return err;

    err = inflate(&strm, 4); // Z_FINISH = 4
    inflateEnd(&strm);

    if (err == 1) { // Z_STREAM_END = 1
        *destLen = strm.total_out;
        return 0; // Success
    }
    return err;
}

#include <setjmp.h>

#define NIL ((unsigned char *)0)
#define MAXBITS 15
#define MAXLCODES 286
#define MAXDCODES 30
#define MAXCODES (MAXLCODES+MAXDCODES)
#define FIXLCODES 288

struct state {
    unsigned char *out;
    unsigned long outlen;
    unsigned long outcnt;
    const unsigned char *in;
    unsigned long inlen;
    unsigned long incnt;
    int bitbuf;
    int bitcnt;
    int err;
};

static int bits(struct state *s, int need) {
    Com_Printf("DEBUG: bits entering - need: %d, incnt: %u, inlen: %u, err: %d\n", need, s->incnt, s->inlen, s->err);
    if (s->err) return 0;
    long val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->incnt == s->inlen) {
            s->err = 2;
            return 0;
        }
        val |= (long)(s->in[s->incnt++]) << s->bitcnt;
        s->bitcnt += 8;
    }
    s->bitbuf = (int)(val >> need);
    s->bitcnt -= need;
    return (int)(val & ((1L << need) - 1));
}

static int stored(struct state *s) {
    unsigned len;
    s->bitbuf = 0;
    s->bitcnt = 0;
    if (s->incnt + 4 > s->inlen) {
        s->err = 2;
        return 2;
    }
    len = s->in[s->incnt++];
    len |= s->in[s->incnt++] << 8;
    if (s->in[s->incnt++] != (~len & 0xff) ||
        s->in[s->incnt++] != ((~len >> 8) & 0xff))
        return -2;
    if (s->incnt + len > s->inlen) {
        s->err = 2;
        return 2;
    }
    if (s->out != NIL) {
        if (s->outcnt + len > s->outlen) return 1;
        while (len--) s->out[s->outcnt++] = s->in[s->incnt++];
    } else {
        s->outcnt += len;
        s->incnt += len;
    }
    return 0;
}

struct huffman {
    short *count;
    short *symbol;
};

static int decode(struct state *s, const struct huffman *h) {
    if (s->err) return -10;
    int len = 1;
    int code = 0;
    int first = 0;
    int index = 0;
    int bitbuf = s->bitbuf;
    int left = s->bitcnt;
    short *next = h->count + 1;
    while (1) {
        while (left--) {
            code |= bitbuf & 1;
            bitbuf >>= 1;
            int count = *next++;
            if (code - count < first) {
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                return h->symbol[index + (code - first)];
            }
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
            len++;
        }
        left = (MAXBITS + 1) - len;
        if (left == 0) break;
        if (s->incnt == s->inlen) {
            s->err = 2;
            return -10;
        }
        bitbuf = s->in[s->incnt++];
        if (left > 8) left = 8;
    }
    return -10;
}

static int construct(struct huffman *h, const short *length, int n) {
    int symbol, len, left;
    short offs[MAXBITS + 1];
    for (len = 0; len <= MAXBITS; len++) h->count[len] = 0;
    for (symbol = 0; symbol < n; symbol++) (h->count[length[symbol]])++;
    if (h->count[0] == n) return 0;
    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= h->count[len];
        if (left < 0) return left;
    }
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++) offs[len + 1] = offs[len] + h->count[len];
    for (symbol = 0; symbol < n; symbol++) {
        if (length[symbol] != 0) {
            h->symbol[offs[length[symbol]]++] = symbol;
        }
    }
    return left;
}

static int codes(struct state *s, const struct huffman *lencode, const struct huffman *distcode) {
    int symbol, len;
    unsigned dist;
    static const short lens[29] = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dext[30] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
    do {
        symbol = decode(s, lencode);
        if (symbol < 0) return symbol;
        if (symbol < 256) {
            if (s->out != NIL) {
                if (s->outcnt == s->outlen) return 1;
                s->out[s->outcnt] = symbol;
            }
            s->outcnt++;
        } else if (symbol > 256) {
            symbol -= 257;
            if (symbol >= 29) return -10;
            len = lens[symbol] + bits(s, lext[symbol]);
            symbol = decode(s, distcode);
            if (symbol < 0) return symbol;
            dist = dists[symbol] + bits(s, dext[symbol]);
            if (s->out != NIL) {
                if (s->outcnt + len > s->outlen) return 1;
                while (len--) {
                    s->out[s->outcnt] = dist > s->outcnt ? 0 : s->out[s->outcnt - dist];
                    s->outcnt++;
                }
            } else {
                s->outcnt += len;
            }
        }
    } while (symbol != 256);
    return 0;
}

static int fixed(struct state *s) {
    static int virgin = 1;
    static short lencnt[MAXBITS + 1], lensym[FIXLCODES];
    static short distcnt[MAXBITS + 1], distsym[MAXDCODES];
    static struct huffman lencode, distcode;
    if (virgin) {
        int symbol;
        short lengths[FIXLCODES];
        lencode.count = lencnt;
        lencode.symbol = lensym;
        distcode.count = distcnt;
        distcode.symbol = distsym;
        for (symbol = 0; symbol < 144; symbol++) lengths[symbol] = 8;
        for (; symbol < 256; symbol++) lengths[symbol] = 9;
        for (; symbol < 280; symbol++) lengths[symbol] = 7;
        for (; symbol < FIXLCODES; symbol++) lengths[symbol] = 8;
        construct(&lencode, lengths, FIXLCODES);
        for (symbol = 0; symbol < MAXDCODES; symbol++) lengths[symbol] = 5;
        construct(&distcode, lengths, MAXDCODES);
        virgin = 0;
    }
    return codes(s, &lencode, &distcode);
}

static int dynamic(struct state *s) {
    int nlen, ndist, ncode, index, err;
    short lengths[MAXCODES];
    short lencnt[MAXBITS + 1], lensym[MAXLCODES];
    short distcnt[MAXBITS + 1], distsym[MAXDCODES];
    struct huffman lencode, distcode;
    static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    lencode.count = lencnt;
    lencode.symbol = lensym;
    distcode.count = distsym;
    nlen = bits(s, 5) + 257;
    ndist = bits(s, 5) + 1;
    ncode = bits(s, 4) + 4;
    if (s->err) return s->err;
    if (nlen > MAXLCODES || ndist > MAXDCODES) return -3;
    for (index = 0; index < ncode; index++) lengths[order[index]] = bits(s, 3);
    for (; index < 19; index++) lengths[order[index]] = 0;
    if (s->err) return s->err;
    err = construct(&lencode, lengths, 19);
    if (err != 0) return -4;
    index = 0;
    while (index < nlen + ndist) {
        int symbol = decode(s, &lencode);
        if (symbol < 0) return symbol;
        if (symbol < 16) {
            lengths[index++] = symbol;
        } else {
            int len = 0;
            if (symbol == 16) {
                if (index == 0) return -5;
                len = lengths[index - 1];
                symbol = 3 + bits(s, 2);
            } else if (symbol == 17) {
                symbol = 3 + bits(s, 3);
            } else {
                symbol = 11 + bits(s, 7);
            }
            if (s->err) return s->err;
            if (index + symbol > nlen + ndist) return -6;
            while (symbol--) lengths[index++] = len;
        }
    }
    if (lengths[256] == 0) return -9;
    err = construct(&lencode, lengths, nlen);
    if (err && (err < 0 || nlen != lencode.count[0] + lencode.count[1])) return -7;
    err = construct(&distcode, lengths + nlen, ndist);
    if (err && (err < 0 || ndist != distcode.count[0] + distcode.count[1])) return -8;
    return codes(s, &lencode, &distcode);
}

static int puff(unsigned char *dest, unsigned long *destlen, const unsigned char *source, unsigned long *sourcelen) {
    Com_Printf("DEBUG: Entered puff - dest: %p, destlen_val: %u, source: %p, sourcelen_val: %u\n", dest, destlen ? *destlen : 0, source, sourcelen ? *sourcelen : 0);
    struct state s;
    int last, type, err;
    s.out = dest;
    s.outlen = *destlen;
    s.outcnt = 0;
    s.in = source;
    s.inlen = *sourcelen;
    s.incnt = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;
    s.err = 0;
    Com_Printf("DEBUG: puff state initialized. Calling bits for last...\n");
    do {
        last = bits(&s, 1);
        Com_Printf("DEBUG: puff last returned %d\n", last);
        type = bits(&s, 2);
        Com_Printf("DEBUG: puff type returned %d\n", type);
        if (s.err) {
            err = s.err;
            break;
        }
        err = type == 0 ? stored(&s) : (type == 1 ? fixed(&s) : (type == 2 ? dynamic(&s) : -1));
        if (err != 0) break;
    } while (!last);
    if (s.err && err == 0) {
        err = s.err;
    }
    if (err <= 0) {
        *destlen = s.outcnt;
        *sourcelen = s.incnt;
    }
    return err;
}

int FS_ReadFile(const char *qpath, void **buffer) {
    Com_Printf("DEBUG: FS_ReadFile entering for qpath: %s (buffer is %s)\n", qpath, buffer ? "NOT NULL" : "NULL");
    char ospath[512];
    sprintf(ospath, "main/%s", qpath);
    for (int i = 0; ospath[i]; i++) {
        if (ospath[i] == '/') ospath[i] = '\\';
    }
    
    Com_Printf("DEBUG: FS_ReadFile checking disk ospath: %s\n", ospath);
    FILE *f = fopen(ospath, "rb");
    if (!f) {
        f = fopen(qpath, "rb");
    }
    if (f) {
        Com_Printf("DEBUG: FS_ReadFile found on disk!\n");
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        void *buf = malloc(len + 1);
        if (buf) {
            fread(buf, 1, len, f);
            ((char*)buf)[len] = 0;
        }
        fclose(f);
        if (buffer) *buffer = buf;
        else free(buf);
        Com_Printf("DEBUG: FS_ReadFile returning disk len %d\n", len);
        return len;
    }

    Com_Printf("DEBUG: FS_ReadFile searching PK3s...\n");
    const char *pk3Names[] = {
        "localized_english_pak0.pk3", "localized_english_pak1.pk3", "localized_english_pak2.pk3",
        "localized_english_pak3.pk3", "localized_english_pak5.pk3",
        "pak0.pk3", "pak1.pk3", "pak2.pk3", "pak3.pk3", "pak4.pk3", "pak5.pk3",
        "pak6.pk3", "pak8.pk3", "pak9.pk3", "paka.pk3", "pakb.pk3"
    };

    for (int p = 0; p < 16; p++) {
        char pk3Path[512];
        sprintf(pk3Path, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty\\main\\%s", pk3Names[p]);
        FILE *pf = fopen(pk3Path, "rb");
        if (!pf) continue;

        while (1) {
            localHeader_t header;
            if (fread(&header, 1, sizeof(header), pf) != sizeof(header)) break;
            if (header.signature != 0x04034b50) break;

            char filename[512];
            int len = header.filenameLength < 511 ? header.filenameLength : 511;
            fread(filename, 1, len, pf);
            filename[len] = 0;
            
            if (header.filenameLength > len) {
                fseek(pf, header.filenameLength - len, SEEK_CUR);
            }

            fseek(pf, header.extraFieldLength, SEEK_CUR);

            char normFilename[512];
            strcpy(normFilename, filename);
            for (int i = 0; normFilename[i]; i++) {
                if (normFilename[i] == '\\') normFilename[i] = '/';
            }

            char normQpath[512];
            strcpy(normQpath, qpath);
            for (int i = 0; normQpath[i]; i++) {
                if (normQpath[i] == '\\') normQpath[i] = '/';
            }

            if (_stricmp(normFilename, normQpath) == 0) {
                Com_Printf("DEBUG:   Found file in PK3: %s (PK3: %s)\n", normFilename, pk3Names[p]);
                void *compBuf = malloc(header.compressedSize);
                if (!compBuf) {
                    fclose(pf);
                    if (buffer) *buffer = NULL;
                    return -1;
                }
                fread(compBuf, 1, header.compressedSize, pf);

                if (header.compression == 0) {
                    Com_Printf("DEBUG:   Compression: Store (uncompressed size: %d)\n", header.uncompressedSize);
                    fclose(pf);
                    if (buffer) {
                        *buffer = compBuf;
                    } else {
                        free(compBuf);
                    }
                    return header.uncompressedSize;
                } else if (header.compression == 8) {
                    Com_Printf("DEBUG:   Compression: Deflate (compressed: %d, uncompressed: %d)\n", header.compressedSize, header.uncompressedSize);
                    void *decompBuf = malloc(header.uncompressedSize + 1);
                    if (!decompBuf) {
                        free(compBuf);
                        fclose(pf);
                        if (buffer) *buffer = NULL;
                        return -1;
                    }
                    
                    unsigned long decompLen = header.uncompressedSize;
                    unsigned long compLen = header.compressedSize;
                    
                    Com_Printf("DEBUG:   Invoking zlib raw inflate...\n");
                    int err = inflate_raw((unsigned char *)decompBuf, &decompLen, (const unsigned char *)compBuf, compLen);
                    Com_Printf("DEBUG:   inflate_raw returned: %d (bytes decoded: %d)\n", err, decompLen);
                    if (err != 0) {
                        free(decompBuf);
                        free(compBuf);
                        fclose(pf);
                        if (buffer) *buffer = NULL;
                        return -1;
                    }

                    ((char*)decompBuf)[header.uncompressedSize] = 0;
                    free(compBuf);
                    fclose(pf);
                    if (buffer) {
                        *buffer = decompBuf;
                    } else {
                        free(decompBuf);
                    }
                    return header.uncompressedSize;
                }
                free(compBuf);
            } else {
                fseek(pf, header.compressedSize, SEEK_CUR);
            }
        }
        fclose(pf);
    }

    Com_Printf("DEBUG: FS_ReadFile did NOT find file: %s\n", qpath);
    if (buffer) *buffer = NULL;
    return -1;
}

// Authentic R_Init helper stubs
void Com_NoiseInit(void) {}
void R_Register(void) {}
void R_InitAllocators(void) {}
void R_InitImages(void) {}
void R_InitVertexPrograms(void) {}
void R_InitShaders(const char *shaderPath) {}
void R_ModelInit(void) {}
void R_InitFreeType(void) {}
void R_SetHwLightGlobals(void) {}
void R_ClearLightVisCache(void) {}
void R_InitDebug(void) {}
void R_InitWater(void) {}
int Com_SafeMode(int autoExec) { return 0; }
// void Cbuf_Execute(void) {}
int Com_ConfigureFileChanged(void) { return 0; }
void Com_SetRecommended(int val) {}
int Sys_InfoChanged(void) { return 0; }
void Sys_HideSplashWindow(void) {}
void Sys_ShowConsole(int val1, int val2) {}
void Sys_NormalExit(void) {}
void Com_InitHunkMemory(void) {}
void AskForPBEULA(void) {}
void Scr_Abort(void) {}
void Hunk_ClearToStart(void) {}
void Com_Close(void) {}
// void Cmd_TokenizeString(const char *text) {}
void Com_DPrintf(const char *fmt, ...) {}
void FS_Restart(void) {}
int VM_CallCompiled(vm_t *vm, int *args) { return 0; }
int VM_CallInterpreted(vm_t *vm, int *args) { return 0; }
void SV_FinalMessage(const char *msg) {}
void SV_RemoveOperatorCommands(void) {}
void SV_MasterShutdown(void) {}
void SV_ShutdownGameProgs(void) {}
void SV_ClearServer(void) {}
void SV_FreeClients(void) {}
void SV_FreeArchivedSnapshot(void) {}
void CL_Disconnect(void) {}
const char *SE_GetString(const char *msg) {
    if (!msg) return "";
    if (!_stricmp(msg, "@MENU_BACKTOGAME")) return "Back to Game";
    if (!_stricmp(msg, "@MENU_JOIN_GAME")) return "Join Game";
    if (!_stricmp(msg, "@MENU_DISCONNECT")) return "Disconnect";
    if (!_stricmp(msg, "@MENU_START_NEW_SERVER")) return "Start New Server";
    if (!_stricmp(msg, "@MENU_MULTIPLAYER_OPTIONS")) return "Multiplayer Options";
    if (!_stricmp(msg, "@MENU_OPTIONS")) return "Options";
    if (!_stricmp(msg, "@MENU_MODS")) return "Mods";
    if (!_stricmp(msg, "@MENU_SINGLE_PLAYER")) return "Single Player";
    if (!_stricmp(msg, "@MENU_QUIT")) return "Quit";
    return msg;
}
void *FS_FileForHandle(int h) { return NULL; }
void Sys_EndStreamedFile(void) {}
const char *Sys_DefaultCDPath(void) { return "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty"; }
const char *Sys_DefaultInstallPath(void) { return "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty"; }
const char *Sys_DefaultHomePath(void) { return "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty"; }
void FS_CreatePath(const char *path) {}
void FS_Printf(int h, const char *fmt, ...) {}
int Com_Filter(const char *filter, const char *name, int casesensitive) { return 0; }
void Info_SetValueForKey(char *s, const char *key, const char *value) {}
void Info_SetValueForKey_Big(char *s, const char *key, const char *value) {}
void SEH_UpdateLanguageInfo(void) {}

#include <setjmp.h>
#include <windows.h>

// Globals and stubs for decompiled OpenGL/input/frame engine
int vidWidth = 0;
char *dword_16C3A60 = "GL_EXTENSIONS";
void dword_16C405C(int p1, int *p2) { if (p2) *p2 = 0; }
int dword_16C3A74 = 0;
char dword_16C388C[1024] = {0};
int dword_16C3A7C = 0;
void GfxInfo_f(void) {}
int dword_16C3980 = 0;
void GL_SetDefaultState(void) {}
int dword_16D8888 = 0;
char dword_16C3A40[1024] = {0};
int dword_16D8884 = 0;
char dword_16C3B0C[1024] = {0};
int dword_16D888C = 0;
int dword_16C3890 = 0;
void sub_52E7B9(void *p) {}
unsigned int _security_cookie = 0;

void sub_461D00(void) {}
int mouseInitialized = 0;
int cls_keyCatchers = 0;
int dword_16BDD98 = 0;
void sub_461730(void) {}
int sub_461850(void) { return 0; }
int mouseActive = 0;
int sub_4616B0(void) { return 0; }

jmp_buf Buf;
int Com_ErrorCleanup(char p1) { return 0; }
char dword_163A208[1024] = {0};
char dword_163A210[1024] = {0};
int dword_1631780 = 0;
void StatMon_Warning(int p1, int p2) {}
char dword_163A21C[1024] = {0};
char dword_163A200[1024] = {0};
void sub_466750(int p1) {}
char dword_163B3C4[1024] = {0};
int dword_14073B4 = 0;
int dword_14073B8 = 0;
char dword_163B3D8[1024] = {0};
int dword_16C35C0 = 0;
int dword_163B3EC[1024] = {0};
int dword_14073F0 = 0;
void MSG_InitHuffmann(void) {}
int dword_14073FC = 0;
int dword_14073F8 = 0;

static int dummy_event[6] = {0, 0, 0, 0, 0, 0};
static char byte_8AB330[24 * 256] = {0};

const void* Com_GetRealEvent(void) {
    MSG msg;
    while ( PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ) ) {
        TranslateMessage( &msg );
        DispatchMessageA( &msg );
    }
    dummy_event[0] = Sys_Milliseconds();
    dummy_event[1] = 0; // SE_NONE
    return dummy_event;
}

void CL_KeyEvent(int p1, int p2) {}
void CL_CharEvent(int p1) {}
void CL_MouseEvent(int p1, int p2) {}
int dword_143A95C[16] = {0};
// Cbuf_AddText is defined in PC/qcommon/cmd.c
char byte_5685A4[] = "Com_EventLoop: %i";
char byte_562D38[] = "Com_EventLoop: bad event type %i";
cvar_t *com_sv_running = NULL;
void Com_RunAndTimeServerPacket(void *p) {}
void CL_PacketEvent(char p) {}
int NET_GetLoopPacket(void *p) { return 0; }

void Com_Eventloop(void)
{
  unsigned __int8 v0; // al
  const void *RealEvent; // esi
  void *v2; // esi
  void *v3[5]; // [esp-14h] [ebp-4080h] BYREF
  int v4[12]; // [esp+Ch] [ebp-4060h] BYREF
  char v5[20]; // [esp+3Ch] [ebp-4030h] BYREF
  char v6; // [esp+68h] [ebp-4004h] BYREF
  unsigned int v7; // [esp+4068h] [ebp-4h]
  unsigned int retaddr = 0; // [esp+406Ch] [ebp+0h]

  v7 = retaddr ^ _security_cookie;
  if ( !dword_14073F0 )
    MSG_InitHuffmann();
  memset(&v4[6], 0, 0x18u);
  v4[7] = (int)&v6;
  v4[8] = 0x4000;
  while ( 1 )
  {
    v0 = dword_14073FC;
    if ( dword_14073F8 <= dword_14073FC )
    {
      RealEvent = (const void *)Com_GetRealEvent();
    }
    else
    {
      ++dword_14073FC;
      RealEvent = (const void *)(24 * v0 + (int)byte_8AB330);
    }
    memcpy(v4, RealEvent, 0x18u);
    if ( !v4[1] )
      break;
    switch ( v4[1] )
    {
      case 1:
        CL_KeyEvent(v4[2], v4[0]);
        break;
      case 2:
        CL_CharEvent(v4[2]);
        break;
      case 3:
        v3[4] = (void *)v4[0];
        CL_MouseEvent(v4[3], v4[2]);
        break;
      case 4:
        if ( v4[2] >= 6u )
          Com_Error(1, (char *)&byte_5685A4, v4[2]);
        dword_143A95C[v4[2]] = v4[3];
        break;
      case 5:
        v2 = (void *)v4[5];
        Cbuf_AddText();
        free(v2);
        Cbuf_AddText();
        break;
      case 6:
        memcpy(v5, (const void *)v4[5], sizeof(v5));
        v4[9] = v4[4] - 20;
        v3[4] = (void *)v4[5];
        if ( (unsigned int)(v4[4] - 20) <= v4[8] )
        {
          memcpy((void *)v4[7], (const void *)(v4[5] + 20), v4[4] - 20);
          free(v3[4]);
          if ( com_sv_running && com_sv_running->integer )
          {
            Com_RunAndTimeServerPacket(v5);
          }
          else
          {
            memcpy(v3, v5, sizeof(v3));
            CL_PacketEvent((char)v3[0]);
          }
        }
        else
        {
          free(v3[4]);
          Com_Printf("Com_EventLoop: oversize packet\n");
        }
        break;
      default:
        Com_Error(0, (char *)&byte_562D38, v4[1]);
    }
  }
  while ( NET_GetLoopPacket(v5) )
  {
    memcpy(v3, v5, sizeof(v3));
    CL_PacketEvent((char)v3[0]);
  }
  while ( NET_GetLoopPacket(v5) )
  {
    if ( com_sv_running && com_sv_running->integer )
      Com_RunAndTimeServerPacket(v5);
  }
  sub_52E7B9((void *)(retaddr ^ v7));
}
int dword_163B3F0 = 0;
int dword_8AAB60 = 0;
int Com_ModifyMsec(int p1) { return p1; }
void SV_Frame(int p1) {}
void CL_Shutdown(void) {}
int dword_8E58AC = 0;
int dword_8E58A8 = 0;
int dword_8E5274 = 0;
char FileName[256] = "";
int sub_463F60(void) { return 0; }
void CL_Init(void) {}
void CL_StartHunkUsers(void) {}
int dword_8AC7B0 = 0;
char dword_163B400[1024] = {0};
int dword_163B3CC = 0;
int dword_163B3D0 = 0;
int dword_163A238 = 0;
void Com_WriteConfigToFile(void) {}
void CL_Frame(int p1) {}

int GLW_CheckOSVersion(void) { return 1; }
char String = '\0';
int dword_19BFFE0 = 0;
cvar_t *dword_19BFFC8 = NULL;
extern HWND hWndParent;
HDC g_hdc = NULL;

int consoleScrollOffset = 0;
int consoleVisible = 0;

#define MAX_CONSOLE_LINES 100
static char consoleLines[MAX_CONSOLE_LINES][256] = {0};
int consoleLineCount = 0;
static int consoleLineWritePtr = 0;

void Sys_AddConsoleLine(const char *msg) {
    if (!msg) return;
    
    const char *p = msg;
    while (*p) {
        char line[256];
        int len = 0;
        while (*p && *p != '\n' && *p != '\r' && len < 255) {
            line[len++] = *p++;
        }
        line[len] = '\0';
        
        if (*p == '\n' || *p == '\r') {
            p++;
        }
        
        if (len == 0) continue;
        
        strncpy(consoleLines[consoleLineWritePtr], line, 255);
        consoleLines[consoleLineWritePtr][255] = '\0';
        consoleLineWritePtr = (consoleLineWritePtr + 1) % MAX_CONSOLE_LINES;
        if (consoleLineCount < MAX_CONSOLE_LINES) {
            consoleLineCount++;
        }
    }
}

static char consoleInput[128] = "";
static int consoleInputLen = 0;
static int wasKeyPressed[256] = {0};

void UpdateConsoleInput(void) {
    if (GetForegroundWindow() != hWndParent) {
        memset(wasKeyPressed, 0, sizeof(wasKeyPressed));
        return;
    }
    // Check backspace
    if (GetAsyncKeyState(VK_BACK) & 0x8000) {
        if (!wasKeyPressed[VK_BACK]) {
            if (consoleInputLen > 0) {
                consoleInputLen--;
                consoleInput[consoleInputLen] = '\0';
            }
            wasKeyPressed[VK_BACK] = 1;
        }
    } else {
        wasKeyPressed[VK_BACK] = 0;
    }

    // Check enter / return
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        if (!wasKeyPressed[VK_RETURN]) {
            if (consoleInputLen > 0) {
                // Print command to console log
                char logMsg[160];
                sprintf(logMsg, "] %s", consoleInput);
                Sys_AddConsoleLine(logMsg);

                // Execute console command
                extern void Cbuf_AddText(const char *text);
                char cmdStr[160];
                sprintf(cmdStr, "%s\n", consoleInput);
                Cbuf_AddText(cmdStr);

                // Clear input
                consoleInput[0] = '\0';
                consoleInputLen = 0;
            }
            wasKeyPressed[VK_RETURN] = 1;
        }
    } else {
        wasKeyPressed[VK_RETURN] = 0;
    }

    // Check space
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (!wasKeyPressed[VK_SPACE]) {
            if (consoleInputLen < 127) {
                consoleInput[consoleInputLen++] = ' ';
                consoleInput[consoleInputLen] = '\0';
            }
            wasKeyPressed[VK_SPACE] = 1;
        }
    } else {
        wasKeyPressed[VK_SPACE] = 0;
    }

    // Check under-score / minus
    if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) {
        if (!wasKeyPressed[VK_OEM_MINUS]) {
            if (consoleInputLen < 127) {
                char c = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? '_' : '-';
                consoleInput[consoleInputLen++] = c;
                consoleInput[consoleInputLen] = '\0';
            }
            wasKeyPressed[VK_OEM_MINUS] = 1;
        }
    } else {
        wasKeyPressed[VK_OEM_MINUS] = 0;
    }

    // Check period
    if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000) {
        if (!wasKeyPressed[VK_OEM_PERIOD]) {
            if (consoleInputLen < 127) {
                consoleInput[consoleInputLen++] = '.';
                consoleInput[consoleInputLen] = '\0';
            }
            wasKeyPressed[VK_OEM_PERIOD] = 1;
        }
    } else {
        wasKeyPressed[VK_OEM_PERIOD] = 0;
    }

    // Check slash
    if (GetAsyncKeyState(VK_OEM_2) & 0x8000) {
        if (!wasKeyPressed[VK_OEM_2]) {
            if (consoleInputLen < 127) {
                consoleInput[consoleInputLen++] = '/';
                consoleInput[consoleInputLen] = '\0';
            }
            wasKeyPressed[VK_OEM_2] = 1;
        }
    } else {
        wasKeyPressed[VK_OEM_2] = 0;
    }

    // Check characters A to Z
    for (int k = 'A'; k <= 'Z'; k++) {
        if (GetAsyncKeyState(k) & 0x8000) {
            if (!wasKeyPressed[k]) {
                if (consoleInputLen < 127) {
                    char c = k;
                    if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
                        c = tolower(c);
                    }
                    consoleInput[consoleInputLen++] = c;
                    consoleInput[consoleInputLen] = '\0';
                }
                wasKeyPressed[k] = 1;
            }
        } else {
            wasKeyPressed[k] = 0;
        }
    }

    // Check numbers 0 to 9
    for (int k = '0'; k <= '9'; k++) {
        if (GetAsyncKeyState(k) & 0x8000) {
            if (!wasKeyPressed[k]) {
                if (consoleInputLen < 127) {
                    consoleInput[consoleInputLen++] = k;
                    consoleInput[consoleInputLen] = '\0';
                }
                wasKeyPressed[k] = 1;
            }
        } else {
            wasKeyPressed[k] = 0;
        }
    }
}

void RenderDemoFrame(void) {
    if (!g_hdc) return;

    extern int RE_RegisterShader(const char *name, int a2);
    extern void R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int shaderHandle);

    static int backTop = -1;
    static int backBottom = -1;
    static int iwLogo = -1;
    static int initialized = 0;

    if (!initialized) {
        backTop = RE_RegisterShader("ui_mp/assets/main_back_top_mp.tga", 0);
        backBottom = RE_RegisterShader("ui_mp/assets/main_back_bottom_mp.tga", 0);
        iwLogo = RE_RegisterShader("video/iw_logo1.tga", 0);
        initialized = 1;
    }

    glViewport(0, 0, 800, 600);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (backTop > 0) {
        R_DrawStretchPic(0, 0, 640, 320, 0, 0, 1, 1, backTop);
    }
    if (backBottom > 0) {
        R_DrawStretchPic(0, 320, 640, 160, 0, 0, 1, 1, backBottom);
    }

    /*
    if (iwLogo > 0) {
        R_DrawStretchPic(35, 370, 140, 70, 0, 0, 1, 1, iwLogo);
    }
    */

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize Win32 hardware-accelerated OpenGL Georgia fonts
    static GLuint fontListBase = 0;
    static GLuint verFontListBase = 0;
    if (!fontListBase) {
        fontListBase = glGenLists(96);
        HFONT hFont = CreateFontA(
            -20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Georgia"
        );
        SelectObject(g_hdc, hFont);
        wglUseFontBitmaps(g_hdc, 32, 96, fontListBase);
        DeleteObject(hFont);

        verFontListBase = glGenLists(96);
        HFONT hVerFont = CreateFontA(
            -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Georgia"
        );
        SelectObject(g_hdc, hVerFont);
        wglUseFontBitmaps(g_hdc, 32, 96, verFontListBase);
        DeleteObject(hVerFont);
    }

    // Toggle console on tilde key release
    static int wasConsoleKeyPressed = 0;
    extern int consoleVisible;
    static float consoleHeight = 0.0f; // animation height (0 to 240)
    static int wasPageUpPressed = 0;
    static int wasPageDownPressed = 0;
    extern int consoleScrollOffset;
    static int lastLineWritePtr = 0;

    if (GetForegroundWindow() == hWndParent) {
        if (GetAsyncKeyState(0xC0) & 0x8000) { // VK_OEM_3 is 0xC0
            if (!wasConsoleKeyPressed) {
                consoleVisible = !consoleVisible;
                wasConsoleKeyPressed = 1;
            }
        } else {
            wasConsoleKeyPressed = 0;
        }
    } else {
        wasConsoleKeyPressed = 0;
    }

    // Slide animation
    if (consoleVisible) {
        if (consoleHeight < 240.0f) {
            consoleHeight += 20.0f;
            if (consoleHeight > 240.0f) consoleHeight = 240.0f;
        }
    } else {
        if (consoleHeight > 0.0f) {
            consoleHeight -= 20.0f;
            if (consoleHeight < 0.0f) consoleHeight = 0.0f;
        }
    }

    // Scroll handling (PageUp / PageDown)
    if (consoleVisible) {
        if (GetForegroundWindow() == hWndParent && (GetAsyncKeyState(VK_PRIOR) & 0x8000)) { // PageUp
            if (!wasPageUpPressed) {
                consoleScrollOffset += 3;
                if (consoleScrollOffset > consoleLineCount - 15) {
                    consoleScrollOffset = consoleLineCount - 15;
                }
                if (consoleScrollOffset < 0) consoleScrollOffset = 0;
                wasPageUpPressed = 1;
            }
        } else {
            wasPageUpPressed = 0;
        }

        if (GetForegroundWindow() == hWndParent && (GetAsyncKeyState(VK_NEXT) & 0x8000)) { // PageDown
            if (!wasPageDownPressed) {
                consoleScrollOffset -= 3;
                if (consoleScrollOffset < 0) consoleScrollOffset = 0;
                wasPageDownPressed = 1;
            }
        } else {
            wasPageDownPressed = 0;
        }
    } else {
        consoleScrollOffset = 0;
    }

    // Snap to bottom on new log print
    if (consoleLineWritePtr != lastLineWritePtr) {
        consoleScrollOffset = 0;
        lastLineWritePtr = consoleLineWritePtr;
    }

    // Capture mouse positions and scale to virtual 640x480 ortho dimensions
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hWndParent, &pt);

    RECT clientRect;
    GetClientRect(hWndParent, &clientRect);
    float mouseX = 0;
    float mouseY = 0;
    if (clientRect.right > 0 && clientRect.bottom > 0) {
        mouseX = pt.x * (640.0f / clientRect.right);
        mouseY = pt.y * (480.0f / clientRect.bottom);
    }

    // Track button click state (detect transitions from release to press)
    static int wasClicked = 0;
    int clickTriggered = 0;
    if (GetForegroundWindow() == hWndParent && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
        if (!wasClicked) {
            clickTriggered = 1;
            wasClicked = 1;
        }
    } else {
        wasClicked = 0;
    }

    float btnY[] = {190, 220, 250, 280, 310, 340, 370};
    const char *btnText[] = {
        "Join a Game",
        "Start New Server",
        "Multiplayer Options",
        "Options",
        "Mods",
        "Single Player",
        "Quit"
    };

    static int showQuitPopup = 0;

    int hoveredBtn = -1;
    if (consoleHeight < 5.0f && !showQuitPopup) {
        for (int b = 0; b < 7; b++) {
            if (mouseX >= 385 && mouseX <= 580 && mouseY >= btnY[b] && mouseY <= btnY[b] + 20) {
                hoveredBtn = b;
            }
        }
    } else if (consoleHeight >= 5.0f) {
        UpdateConsoleInput();
    }

    // Render each button dynamically as floating, soft-shadowed Georgia text
    for (int b = 0; b < 7; b++) {
        if (fontListBase) {
            glListBase(fontListBase - 32);

            // 1. Draw soft drop-shadow offset to bottom-right (black)
            glColor3f(0.02f, 0.02f, 0.02f);
            glRasterPos2f(385.0f + 1.5f, btnY[b] + 14.0f + 1.5f);
            glCallLists(strlen(btnText[b]), GL_UNSIGNED_BYTE, btnText[b]);

            // 2. Draw foreground text
            if (hoveredBtn == b) {
                glColor3f(0.95f, 0.82f, 0.45f); // Beautiful authentic gold color on hover!
            } else {
                glColor3f(0.82f, 0.82f, 0.82f); // Authentic soft white normally!
            }
            
            glRasterPos2f(385.0f, btnY[b] + 14.0f);
            glCallLists(strlen(btnText[b]), GL_UNSIGNED_BYTE, btnText[b]);
        }
    }

    // Render the authentic version string "1.5" at the bottom right
    if (verFontListBase) {
        const char *verStr = "1.5";
        glListBase(verFontListBase - 32);

        // 1. Draw black shadow
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(565.0f + 1.0f, 455.0f + 1.0f);
        glCallLists(strlen(verStr), GL_UNSIGNED_BYTE, verStr);

        // 2. Draw foreground text
        glColor3f(0.6f, 0.6f, 0.6f);
        glRasterPos2f(565.0f, 455.0f);
        glCallLists(strlen(verStr), GL_UNSIGNED_BYTE, verStr);
    }

    // Process button actions
    if (consoleHeight < 5.0f && clickTriggered && hoveredBtn >= 0) {
        extern void Cbuf_AddText(const char *text);
        if (hoveredBtn == 0) { // Join Game
            Com_Printf("DEBUG: Connecting to local test server...\n");
            Cbuf_AddText("connect 127.0.0.1\n");
        } else if (hoveredBtn == 1) { // Start New Server
            Com_Printf("DEBUG: Loading game map mp_depot...\n");
            Cbuf_AddText("map mp_depot\n");
        } else if (hoveredBtn == 2) { // Multiplayer Options
            Com_Printf("DEBUG: Multiplayer options selected\n");
        } else if (hoveredBtn == 3) { // Options
            Com_Printf("DEBUG: Options selected\n");
        } else if (hoveredBtn == 4) { // Mods
            Com_Printf("DEBUG: Mods selected\n");
        } else if (hoveredBtn == 5) { // Single Player
            Com_Printf("DEBUG: Single player selected\n");
        } else if (hoveredBtn == 6) { // Quit
            Com_Printf("DEBUG: Triggering quit confirmation popup...\n");
            showQuitPopup = 1;
        }
    }

    // Render Quit Confirmation Popup overlay in virtual 640x480 space
    if (showQuitPopup) {
        int hoveredYes = (mouseX >= 230 && mouseX <= 280 && mouseY >= 225 && mouseY <= 255);
        int hoveredNo = (mouseX >= 330 && mouseX <= 380 && mouseY >= 225 && mouseY <= 255);
        
        if (clickTriggered) {
            extern void Cbuf_AddText(const char *text);
            if (hoveredYes) {
                Cbuf_AddText("quit\n");
            } else if (hoveredNo) {
                showQuitPopup = 0;
            }
        }
        
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // 1. Draw popmenu background panel body (below gold bar)
        glColor4f(0.06f, 0.06f, 0.06f, 0.88f);
        glBegin(GL_QUADS);
            glVertex2f(190, 185);
            glVertex2f(450, 185);
            glVertex2f(450, 300);
            glVertex2f(190, 300);
        glEnd();

        // 2. Draw the golden/olive header strip
        glColor4f(0.36f, 0.40f, 0.16f, 0.90f);
        glBegin(GL_QUADS);
            glVertex2f(190, 160);
            glVertex2f(450, 160);
            glVertex2f(450, 185);
            glVertex2f(190, 185);
        glEnd();
        
        // Complete card outer border
        glColor4f(0.30f, 0.30f, 0.30f, 1.0f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(190, 160);
            glVertex2f(450, 160);
            glVertex2f(450, 300);
            glVertex2f(190, 300);
        glEnd();
        
        // 3. Render exact prompt text in the header bar
        if (fontListBase) {
            glListBase(fontListBase - 32);
            const char *promptStr = "Are you sure you want to quit?";
            
            // Text shadow
            glColor3f(0.0f, 0.0f, 0.0f);
            glRasterPos2f(225.0f + 1.0f, 177.0f + 1.0f);
            glCallLists(strlen(promptStr), GL_UNSIGNED_BYTE, promptStr);
            
            // Text foreground (soft white)
            glColor3f(0.92f, 0.92f, 0.92f);
            glRasterPos2f(225.0f, 177.0f);
            glCallLists(strlen(promptStr), GL_UNSIGNED_BYTE, promptStr);
        }
        
        // 4. Render "Yes" Button
        if (hoveredYes) {
            glColor4f(0.24f, 0.20f, 0.10f, 0.90f); // Gold highlight
        } else {
            glColor4f(0.18f, 0.20f, 0.25f, 0.90f); // Slate charcoal
        }
        glBegin(GL_QUADS);
            glVertex2f(230, 225);
            glVertex2f(280, 225);
            glVertex2f(280, 255);
            glVertex2f(230, 255);
        glEnd();
        
        glColor4f(0.40f, 0.40f, 0.40f, 1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(230, 225);
            glVertex2f(280, 225);
            glVertex2f(280, 255);
            glVertex2f(230, 255);
        glEnd();
        
        if (fontListBase) {
            glListBase(fontListBase - 32);
            const char *yesStr = "Yes";
            glColor3f(0.0f, 0.0f, 0.0f);
            glRasterPos2f(243.0f + 1.0f, 246.0f + 1.0f);
            glCallLists(strlen(yesStr), GL_UNSIGNED_BYTE, yesStr);
            
            if (hoveredYes) {
                glColor3f(0.95f, 0.82f, 0.45f);
            } else {
                glColor3f(0.85f, 0.85f, 0.85f);
            }
            glRasterPos2f(243.0f, 246.0f);
            glCallLists(strlen(yesStr), GL_UNSIGNED_BYTE, yesStr);
        }
        
        // 5. Render "No" Button
        if (hoveredNo) {
            glColor4f(0.24f, 0.20f, 0.10f, 0.90f);
        } else {
            glColor4f(0.18f, 0.20f, 0.25f, 0.90f);
        }
        glBegin(GL_QUADS);
            glVertex2f(330, 225);
            glVertex2f(380, 225);
            glVertex2f(380, 255);
            glVertex2f(330, 255);
        glEnd();
        
        glColor4f(0.40f, 0.40f, 0.40f, 1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(330, 225);
            glVertex2f(380, 225);
            glVertex2f(380, 255);
            glVertex2f(330, 255);
        glEnd();
        
        if (fontListBase) {
            glListBase(fontListBase - 32);
            const char *noStr = "No";
            glColor3f(0.0f, 0.0f, 0.0f);
            glRasterPos2f(347.0f + 1.0f, 246.0f + 1.0f);
            glCallLists(strlen(noStr), GL_UNSIGNED_BYTE, noStr);
            
            if (hoveredNo) {
                glColor3f(0.95f, 0.82f, 0.45f);
            } else {
                glColor3f(0.85f, 0.85f, 0.85f);
            }
            glRasterPos2f(347.0f, 246.0f);
            glCallLists(strlen(noStr), GL_UNSIGNED_BYTE, noStr);
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // --- Render Dropdown Console ---
    if (consoleHeight > 0.0f) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 640, 480, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 1. Semi-transparent charcoal background
        glColor4f(0.06f, 0.06f, 0.06f, 0.90f);
        glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f(640, 0);
            glVertex2f(640, consoleHeight);
            glVertex2f(0, consoleHeight);
        glEnd();

        // 2. Divider line at the bottom
        glColor4f(0.35f, 0.35f, 0.35f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
            glVertex2f(0, consoleHeight);
            glVertex2f(640, consoleHeight);
        glEnd();

        // Initialize and bind the Consolas font
        static GLuint consoleFontListBase = 0;
        if (!consoleFontListBase) {
            consoleFontListBase = glGenLists(96);
            HFONT hFont = CreateFontA(
                -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY, FF_DONTCARE | FIXED_PITCH, "Consolas"
            );
            SelectObject(g_hdc, hFont);
            wglUseFontBitmaps(g_hdc, 32, 96, consoleFontListBase);
            DeleteObject(hFont);
        }

        // Draw log lines inside the console
        if (consoleHeight > 15.0f && consoleFontListBase) {
            glListBase(consoleFontListBase - 32);

            // Draw the interactive command input line at the bottom of the console
            char inputLine[160];
            int showCursor = (Sys_Milliseconds() / 500) % 2;
            sprintf(inputLine, "] %s%s", consoleInput, showCursor ? "_" : "");
            
            // Input line shadow
            glColor3f(0.0f, 0.0f, 0.0f);
            glRasterPos2f(10.0f + 1.0f, consoleHeight - 8.0f + 1.0f);
            glCallLists(strlen(inputLine), GL_UNSIGNED_BYTE, inputLine);
            
            // Input line foreground (bright green input text!)
            glColor3f(0.3f, 1.0f, 0.3f);
            glRasterPos2f(10.0f, consoleHeight - 8.0f);
            glCallLists(strlen(inputLine), GL_UNSIGNED_BYTE, inputLine);

            // Draw historical output lines (with scroll offset!)
            int drawY = (int)consoleHeight - 22;
            int idx = (consoleLineWritePtr - 1 - consoleScrollOffset + MAX_CONSOLE_LINES) % MAX_CONSOLE_LINES;
            int limit = consoleLineCount - consoleScrollOffset;
            if (limit > MAX_CONSOLE_LINES) limit = MAX_CONSOLE_LINES;

            for (int i = 0; i < limit && drawY > 15; i++) {
                // Draw shadow
                glColor3f(0.0f, 0.0f, 0.0f);
                glRasterPos2f(10.0f + 1.0f, (float)drawY + 1.0f);
                glCallLists(strlen(consoleLines[idx]), GL_UNSIGNED_BYTE, consoleLines[idx]);

                // Foreground color depending on text contents
                if (strstr(consoleLines[idx], "WARNING") || strstr(consoleLines[idx], "warning")) {
                    glColor3f(1.0f, 0.8f, 0.2f); // Yellow warnings
                } else if (strstr(consoleLines[idx], "ERROR") || strstr(consoleLines[idx], "error")) {
                    glColor3f(1.0f, 0.3f, 0.3f); // Red errors
                } else if (strstr(consoleLines[idx], "DEBUG") || strstr(consoleLines[idx], "debug")) {
                    glColor3f(0.4f, 0.8f, 1.0f); // Light blue debug statements
                } else {
                    glColor3f(0.85f, 0.85f, 0.85f); // Soft white normal output
                }
                glRasterPos2f(10.0f, (float)drawY);
                glCallLists(strlen(consoleLines[idx]), GL_UNSIGNED_BYTE, consoleLines[idx]);

                drawY -= 13;
                idx = (idx - 1 + MAX_CONSOLE_LINES) % MAX_CONSOLE_LINES;
            }
        }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    SwapBuffers(g_hdc);
}

LRESULT CALLBACK CoD1_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_MOUSEWHEEL) {
        if (consoleVisible) {
            short zDelta = (short)HIWORD(wParam);
            if (zDelta > 0) {
                consoleScrollOffset += 3;
                if (consoleScrollOffset > consoleLineCount - 15) {
                    consoleScrollOffset = consoleLineCount - 15;
                }
                if (consoleScrollOffset < 0) consoleScrollOffset = 0;
            } else if (zDelta < 0) {
                consoleScrollOffset -= 3;
                if (consoleScrollOffset < 0) consoleScrollOffset = 0;
            }
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void sub_50A7A0(void) {
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = CoD1_WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "CoD1_Window_Class";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "CoD1_Window_Class",
        "Call of Duty MP (Decompiled)",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL,
        wc.hInstance,
        NULL
    );

    HDC hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);

    HGLRC hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);

    hWndParent = hwnd;
    g_hdc = hdc;
    Com_Printf("GLW_StartOpenGL() - Created active hardware-accelerated OpenGL window!\n");
}
int dword_16C3A64 = 0;
char *dword_16C3A68 = NULL;
int dword_16C3A6C = 0;
int dword_16C43F0 = 0;
int dword_16C45A0 = 0;
int dword_16C3A70 = 0;
void sub_50A870(void) {}
void GLW_InitExtensions(void) {}
int dword_16C3A8C = 0;
char dword_16C3A14[1024] = {0};
char byte_1406CE8[2048] = {0};
char byte_1406EE7 = 0;
char byte_1406CE9 = 0;
char byte_14070E7 = 0;
char byte_1406EE9 = 0;
char byte_14072E7 = 0;
char byte_14070E9 = 0;
char byte_1406E53 = 0;
char byte_1406EE8[2048] = {0};
char byte_14070E8[2048] = {0};
int nullsub_7(void) { return 0; }
int sub_1D94DC(int p1) { return 0; }
