#include <stdio.h>

extern void Sys_Print(const char *msg);
extern void Sys_AddConsoleLine(const char *msg);

void Conbuf_AppendText(const char *pMsg)
{
    if (!pMsg)
        return;

    fputs(pMsg, stdout);
    fflush(stdout);

    Sys_AddConsoleLine(pMsg);
}