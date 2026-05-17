/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif
#include "../qcommon/qcommon.h"

#include <stdio.h>
#include <string.h>

extern cvar_t *Cvar_Get( const char *var_name, const char *var_value, int flags );
extern void Cvar_Set( const char *var_name, const char *value);
extern void Com_Printf(const char *fmt, ...);
extern const char *va(const char *fmt, ...);
extern int FS_LanguageHasAssets(int language);
/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/stringed/stringed_hooks.cpp */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */

char * g_languages[] = {
    "english", NULL,
    "french", NULL,
    "german", NULL,
    "italian", NULL,
    "spanish", NULL,
    "british", NULL,
    "russian", NULL,
    "polish", NULL,
    "korean", NULL,
    "taiwanese", NULL,
    "japanese", NULL,
    "chinese", NULL,
    "thai", NULL,
    "leet", NULL,
    NULL, NULL, NULL, NULL,
};
#define g_languages ((languageInfo_t *)g_languages)

static qboolean g_currentAsian;
int Language_IsAsian()
{
  return g_currentAsian;
}
cvar_t *loc_language;
cvar_t *loc_translate;
cvar_t *loc_warnings;
cvar_t *loc_warningsAsErrors;
int SEH_UpdateCurrentLanguage(void)
{
    loc_language = Cvar_Get("cl_language", "0", CVAR_LATCH | CVAR_ARCHIVE);
    loc_translate = Cvar_Get("loc_translate", "1", CVAR_LATCH | CVAR_ARCHIVE);
    loc_warnings = Cvar_Get("loc_warnings", "0", CVAR_LATCH | CVAR_ARCHIVE);
    loc_warningsAsErrors = Cvar_Get("loc_warningsAsErrors", "0", CVAR_LATCH | CVAR_ARCHIVE);

    if (loc_language) {
        g_currentAsian =
            loc_language->integer >= 8 &&
            loc_language->integer < 13;
    } else {
        g_currentAsian = qfalse;
    }

    return (int)loc_language;
}

static qboolean SEH_StringEd_SetLanguageStrings(int iLanguage) {
    /* Bounds-check iLanguage to avoid reading off g_languages[] when
     * loc_language->current.integer returns garbage (dvar struct offsets
     * are still being reconciled with the original Mach-O binary). */
    if ((unsigned int)iLanguage >= 14) {
        return 0;
    }
    if (!g_languages[iLanguage].bPresent) {
        return 0;
    }

    /* SE_LoadLanguage is an unresolved Mach-O symbol in this build (the
     * StringEd back-end has not been ported yet). Calling it would
     * NULL-call. If the language's assets are present we can safely
     * report success; the engine will fall back to passing string keys
     * through unchanged at lookup time. */
    return 1;
}

int SEH_UpdateLanguageInfo(void)
{
    int i;
    int numLanguages;
    int result = 0;

    SEH_UpdateCurrentLanguage();

    numLanguages = 0;

    for (i = 0; i < 14; i++)
    {
        if (FS_LanguageHasAssets(i))
        {
            g_languages[i].bPresent = 1;
            numLanguages++;
        }
        else
        {
            g_languages[i].bPresent = 0;
        }
    }

    if (numLanguages < 1)
    {
        Com_Printf("^1ERROR: No languages available because no localized assets were found\n");
    }

    Cvar_Set("cl_languagesAvailable", va("%i", numLanguages));

    if (loc_language) {
        result = SEH_StringEd_SetLanguageStrings(loc_language->integer);
    }

    if (!result)
    {
        for (i = 0; i < 14; i++)
        {
            if (g_languages[i].bPresent)
            {
                Cvar_Set("cl_language", va("%i", i));
                SEH_UpdateCurrentLanguage();

                result = SEH_StringEd_SetLanguageStrings(i);

                if (result)
                    break;
            }
        }

        if (i == 14)
        {
            Cvar_Set("cl_language", "0");
            return SEH_UpdateCurrentLanguage();
        }
    }

    return result;
}
#if defined(_MSC_VER)
__declspec(align(4)) char __DefaultRuneLocale[64] = {0};
#else
char __DefaultRuneLocale[64] __attribute__((aligned(4))) = {0};
#endif
static int SEH_IsDigit(int c) {
    if ((unsigned int)c > 0xFF) {
        return 0;
    }
    return (((unsigned long *)((byte *)__DefaultRuneLocale + 0x34))[(unsigned char)c] >> 0) & 4;
}

static int iCurrString; /* iCurrString */
static char szStrings[2][1024]; /* szStrings */
extern void I_strncpyz(char *dest, const char *src, int destsize);
extern void Com_Error(int level, const char *fmt, ...);
const char * SEH_LocalizeTextMessage(const char *pszInputBuffer, const char *pszMessageType, msgLocErrType_t errType) {
    char szTokenBuf[1024];
    char szInsertBuf[1024];
    char *pszString;
    const char *pszIn;
    const char *pszScanStart;
    int bLocOn;
    int bInsertEnabled;
    int iInsertLevel;
    int insertIndex;
    int bLocSkipped;
    int outputLen;
    int iTokenLen;
    int iLen;
    int i;
    const char *pszTranslated;

    iCurrString = (iCurrString + 1) % 2;
    pszString = szStrings[iCurrString];
    memset(pszString, 0, 1024);

    pszIn = pszInputBuffer;
    outputLen = 0;
    bLocOn = 1;
    bInsertEnabled = 1;
    iInsertLevel = 0;
    insertIndex = 1;
    bLocSkipped = 0;

    while (*pszIn) {
        /* Scan forward to find next control character */
        pszScanStart = pszIn;
        while (*pszIn && *pszIn != 0x14 && *pszIn != 0x15 && *pszIn != 0x16) {
            pszIn++;
        }

        /* Process token if non-empty */
        if (pszScanStart < pszIn) {
            iTokenLen = pszIn - pszScanStart;
            I_strncpyz(szTokenBuf, pszScanStart, iTokenLen + 1);

            /* Translate if localization is on */
            if (bLocOn) {
                /* Inline SEH_StringEd_GetString logic */
                pszTranslated = NULL;
                if (loc_translate && loc_translate->integer &&
                    szTokenBuf[0] != '\0' && szTokenBuf[1] != '\0') {
                    pszTranslated = SE_GetString(szTokenBuf);
                }

                if (!pszTranslated) {
                    if (!loc_translate || !loc_translate->integer ||
                        szTokenBuf[0] == '\0') {
                        pszTranslated = szTokenBuf;
                    } else {
                        /* Translation failed */
                        if (loc_warnings && loc_warnings->integer) {
                            if (loc_warningsAsErrors && loc_warningsAsErrors->integer) {
                                if (errType != LOCMSG_NOERR) {
                                    Com_Error(6, "Could not translate part of %s: \"%s\"", pszMessageType, szTokenBuf);
                                }
                            }
                            Com_Printf("^3WARNING: Could not translate part of %s: \"%s\"\n", pszMessageType, szTokenBuf);
                        }

                        if (!loc_warnings || !loc_warnings->integer) {
                            pszTranslated = va("%s", szTokenBuf);
                        } else {
                            pszTranslated = va("^1UNLOCALIZED(^7%s^1)^7", szTokenBuf);
                        }

                        if (errType == LOCMSG_NOERR) {
                            return NULL;
                        }
                    }
                }

                /* Copy translated string back to token buffer and recompute length */
                strcpy(szTokenBuf, pszTranslated);
                iTokenLen = strlen(szTokenBuf);
            }

            /* Check output length */
            iLen = iTokenLen + outputLen;
            if (iLen > 1023) {
                if (loc_warnings && loc_warnings->integer &&
                    loc_warningsAsErrors && loc_warningsAsErrors->integer) {
                    if (errType != LOCMSG_NOERR) {
                        Com_Error(1, "%s too long when translated: \"%s\"", pszMessageType, pszInputBuffer);
                    }
                }
                Com_Printf("%s too long when translated: \"%s\"\n", pszMessageType, pszInputBuffer);
            }

            /* Search for "&&" insert markers in the token */
            if (iTokenLen - 2 > 0) {
                if (bInsertEnabled) {
                    /* Count insert markers */
                    for (i = 0; i < iTokenLen - 2; i++) {
                        if (szTokenBuf[i] == '&' && szTokenBuf[i + 1] == '&' &&
                            SEH_IsDigit((signed char)szTokenBuf[i + 2])) {
                            iInsertLevel++;
                        }
                    }
                } else {
                    /* Replace "&&[digit]" with 0x16 marker */
                    for (i = 0; i < iTokenLen - 2; i++) {
                        if (szTokenBuf[i] == '&' && szTokenBuf[i + 1] == '&' &&
                            SEH_IsDigit((signed char)szTokenBuf[i + 2])) {
                            szTokenBuf[i] = 0x16;
                            bLocSkipped = 1;
                        }
                    }
                }
            }

            /* Perform insert replacement if needed */
            if (iInsertLevel > 0 && outputLen > 0) {
                int searchLen = outputLen - 2;
                char *insertPoint;
                int searchIdx;

                if (searchLen <= 0) {
                    insertPoint = pszString;
                    searchLen = 0;
                } else {
                    /* Search output string for "&&[digit]" matching insertIndex */
                    insertPoint = NULL;
                    for (searchIdx = 0; searchIdx < searchLen; searchIdx++) {
                        insertPoint = &pszString[searchIdx];
                        if (pszString[searchIdx] == '&' && pszString[searchIdx + 1] == '&' &&
                            SEH_IsDigit((signed char)pszString[searchIdx + 2])) {
                            int digit = (signed char)pszString[searchIdx + 2] - '0';
                            if (digit == 0) {
                                Com_Printf("%s cannot have &&0 as conversion format: \"%s\"\n", pszMessageType, pszInputBuffer);
                            }
                            if (digit == insertIndex) {
                                /* Found matching insert marker */
                                strcpy(szInsertBuf, &pszString[searchIdx + 3]);
                                pszString[searchIdx] = '\0';
                                insertIndex = digit + 1;
                                searchLen = searchIdx;
                                insertPoint = &pszString[searchIdx];
                                goto do_insert;
                            }
                        }
                    }
                    /* No matching insert found */
                    insertPoint = &pszString[searchIdx];
                }

            do_insert:
                strcpy(insertPoint, szTokenBuf);
                strcpy(&pszString[searchLen + iTokenLen], szInsertBuf);
                iInsertLevel--;
                outputLen = outputLen + iTokenLen - 3;
            } else {
                /* Simple append */
                strcpy(&pszString[outputLen], szTokenBuf);
                outputLen = iLen;
            }
        }

        /* Process control characters */
        if (*pszIn == 0x14) {
            pszIn++;
            bLocOn = 1;
        } else if (*pszIn == 0x15) {
            pszIn++;
            bLocOn = 0;
        }

        if (*pszIn == 0x16) {
            pszIn++;
            bInsertEnabled = 0;
        }
    }

    /* Post-processing: replace 0x16 markers with '%' */
    if (bLocSkipped && outputLen > 0) {
        for (i = 0; i < outputLen; i++) {
            if (pszString[i] == 0x16) {
                pszString[i] = '%';
            }
        }
    }

    return pszString;
}
