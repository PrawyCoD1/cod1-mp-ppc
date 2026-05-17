/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/msg_mp.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
extern void Com_Printf(const char *fmt, ...);
extern char I_CleanChar(int character);

void MSG_WriteReliableCommandToBuffer(const char *pszCommand, char *pszBuffer, int iBufferSize) {
    int len = strlen(pszCommand);
    int i;

    if (len >= iBufferSize) {
        Com_Printf((const char *)"WARNING: Reliable command is too long (%i/%i) and will be truncated: '%s'\n", len, iBufferSize, pszCommand);
    }

    if (len == 0) {
        Com_Printf((const char *)"WARNING: Empty reliable command\n");
    }

    if (iBufferSize <= 0) {
        pszBuffer[iBufferSize - 1] = '\0';
        return;
    }

    for (i = 0; i < iBufferSize; i++) {
        char ch = pszCommand[i];
        if (!ch) {
            pszBuffer[i] = '\0';
            return;
        }
        ch = (char)I_CleanChar((int)(signed char)ch);
        if (ch == '%')
            ch = '.';
        pszBuffer[i] = ch;
    }

    pszBuffer[iBufferSize - 1] = '\0';
}