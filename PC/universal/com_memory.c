#include "../qcommon/qcommon.h"
#include <stdlib.h>

void Z_FreeInternal(void *ptr)
{
    free(ptr);
}