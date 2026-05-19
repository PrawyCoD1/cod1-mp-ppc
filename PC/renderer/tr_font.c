#include "../qcommon/qcommon.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define GLYPH_START 0
#define GLYPH_END 255
#define GLYPH_CHARSTART 32
#define GLYPH_CHAREND 127
#define GLYPHS_PER_FONT (GLYPH_END - GLYPH_START + 1)

typedef int qhandle_t;

typedef struct {
	int height;     // number of scan lines
	int top;        // top of glyph in buffer
	int bottom;     // bottom of glyph in buffer
	int pitch;      // width for copying
	int xSkip;      // x adjustment
	int imageWidth; // width of actual image
	int imageHeight; // height of actual image
	float s;        // x offset in image where glyph starts
	float t;        // y offset in image where glyph starts
	float s2;
	float t2;
	qhandle_t glyph; // handle to the shader with the glyph
	char shaderName[32];
} glyphInfo_t;

typedef struct {
	glyphInfo_t glyphs [GLYPHS_PER_FONT];
	float glyphScale;
	char name[68];
} fontInfo_t;

// Declare external functions
extern int FS_ReadFile(const char *qpath, void **buffer);
extern qhandle_t RE_RegisterShader(const char *name, int imageTrack);
extern void R_SyncRenderThread(void);

// Local helpers for deserialization
static int readIntFromBuffer(const char **bufPtr) {
    int val;
    memcpy(&val, *bufPtr, 4);
    *bufPtr += 4;
    return val;
}

static float readFloatFromBuffer(const char **bufPtr) {
    float val;
    memcpy(&val, *bufPtr, 4);
    *bufPtr += 4;
    return val;
}

char * RE_GetFontLanguageDAT(char *fontPath) {
    static char buf[1024];
    int lang = Cvar_VariableIntegerValue("cl_language");
    
    const char *langSubfolder = NULL;
    if (lang == 6) langSubfolder = "russian";
    else if (lang == 7) langSubfolder = "polish";
    else if (lang == 8) langSubfolder = "korean";
    else if (lang == 9) langSubfolder = "taiwanese";
    else if (lang == 10) langSubfolder = "japanese";
    else if (lang == 11) langSubfolder = "chinese";
    else if (lang == 12) langSubfolder = "thai";
    
    if (langSubfolder && fontPath && !_strnicmp(fontPath, "fonts/", 6)) {
        int folderLen = strlen(langSubfolder);
        if (_strnicmp(fontPath + 6, langSubfolder, folderLen) != 0 || fontPath[6 + folderLen] != '/') {
            sprintf(buf, "fonts/%s/%s", langSubfolder, fontPath + 6);
            strcpy(fontPath, buf);
            return fontPath;
        }
    }
    return fontPath;
}

const char * RE_GetFontLanguageTGA(const char *fontPath) {
    static char buf[1024];
    int lang = Cvar_VariableIntegerValue("cl_language");
    
    const char *langSubfolder = NULL;
    if (lang == 6) langSubfolder = "russian";
    else if (lang == 7) langSubfolder = "polish";
    else if (lang == 8) langSubfolder = "korean";
    else if (lang == 9) langSubfolder = "taiwanese";
    else if (lang == 10) langSubfolder = "japanese";
    else if (lang == 11) langSubfolder = "chinese";
    else if (lang == 12) langSubfolder = "thai";
    
    if (langSubfolder && fontPath && !_strnicmp(fontPath, "fonts/", 6)) {
        int folderLen = strlen(langSubfolder);
        if (_strnicmp(fontPath + 6, langSubfolder, folderLen) != 0 || fontPath[6 + folderLen] != '/') {
            sprintf(buf, "fonts/%s/%s", langSubfolder, fontPath + 6);
            return buf;
        }
    }
    return fontPath;
}

#define MAX_FONTS 8
static fontInfo_t registeredFonts[MAX_FONTS];
static int registeredFontCount = 0;

void RE_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font, int imageTrack) {
    Com_Printf("DEBUG: RE_RegisterFont called for fontName: %s, pointSize: %d, imageTrack: %d\n", fontName ? fontName : "NULL", pointSize, imageTrack);
    int i;
    char datPath[256];
    void *buffer = NULL;
    int fileSize;

    if (pointSize <= 0)
        pointSize = 12;

    R_SyncRenderThread();

    // Cache full — caller gets nothing
    if (registeredFontCount >= MAX_FONTS) {
        return;
    }

    // Build .dat filename from point size
    Com_sprintf(datPath, sizeof(datPath), "fonts/fontImage_%i.dat", pointSize);

    // Cache lookup
    for (i = 0; i < registeredFontCount; i++) {
        if (!Q_stricmp(datPath, registeredFonts[i].name)) {
            memcpy(font, &registeredFonts[i], sizeof(fontInfo_t));
            return;
        }
    }

    // Redirect the datPath to Polish (or localized) if active
    char redirectedDatPath[512];
    strcpy(redirectedDatPath, datPath);
    RE_GetFontLanguageDAT(redirectedDatPath);

    // Load the .dat file
    Com_Printf("DEBUG: RE_RegisterFont loading file: %s\n", redirectedDatPath);
    fileSize = FS_ReadFile(redirectedDatPath, &buffer);
    if (fileSize <= 0 || !buffer) {
        // Fallback to original
        Com_Printf("DEBUG: RE_RegisterFont localized file not found, falling back to: %s\n", datPath);
        fileSize = FS_ReadFile(datPath, &buffer);
    }

    if (fileSize <= 0 || !buffer) {
        return;
    }

    if (fileSize != 20552) {
        free(buffer);
        return;
    }

    const char *ptr = (const char *)buffer;

    // Deserialize 256 glyphs
    for (i = 0; i < GLYPHS_PER_FONT; i++) {
        glyphInfo_t *glyph = &font->glyphs[i];

        glyph->height      = readIntFromBuffer(&ptr);
        glyph->top         = readIntFromBuffer(&ptr);
        glyph->bottom      = readIntFromBuffer(&ptr);
        glyph->pitch       = readIntFromBuffer(&ptr);
        glyph->xSkip       = readIntFromBuffer(&ptr);
        glyph->imageWidth  = readIntFromBuffer(&ptr);
        glyph->imageHeight = readIntFromBuffer(&ptr);
        glyph->s           = readFloatFromBuffer(&ptr);
        glyph->t           = readFloatFromBuffer(&ptr);
        glyph->s2          = readFloatFromBuffer(&ptr);
        glyph->t2          = readFloatFromBuffer(&ptr);
        glyph->glyph       = (qhandle_t)readIntFromBuffer(&ptr);

        memcpy(glyph->shaderName, ptr, sizeof(glyph->shaderName));
        ptr += sizeof(glyph->shaderName);
    }

    // Font-level metadata
    font->glyphScale = readFloatFromBuffer(&ptr);
    readFloatFromBuffer(&ptr); // ignored

    // Copy name block
    memcpy(font->name, ptr, sizeof(font->name));
    Q_strncpyz(font->name, datPath, sizeof(font->name));

    free(buffer);

    // Register a shader for each glyph (255 glyphs, matching original bound)
    for (i = 0; i < 255; i++) {
        const char *tgaPath = RE_GetFontLanguageTGA(font->glyphs[i].shaderName);
        font->glyphs[i].glyph = RE_RegisterShader(tgaPath, imageTrack);
    }

    // Store in cache
    memcpy(&registeredFonts[registeredFontCount++], font, sizeof(fontInfo_t));
}
