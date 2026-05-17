#include <stdlib.h>
#include <setjmp.h>

typedef struct
{
    char va_string[2][1024];
    int index;
} va_info_t;

static void *g_threadValues[5]; /* g_threadValues */

void *Sys_GetValue(int valueIndex)
{
    if (valueIndex == 1 && g_threadValues[1] == NULL) {
        g_threadValues[1] = calloc(1, sizeof(va_info_t));
    }
    if (valueIndex == 2 && g_threadValues[2] == NULL) {
        g_threadValues[2] = calloc(1, sizeof(jmp_buf));
    }
    return g_threadValues[valueIndex];
}