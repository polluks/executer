#include <clib/dos_protos.h>

#include "utility.h"

BOOL utility_exists (const char *path)
{
    BOOL ret = FALSE;
    BPTR l = Lock (path, ACCESS_READ);
    if (l) {
        UnLock (l);
        ret = TRUE;
    }
    return ret;
}

