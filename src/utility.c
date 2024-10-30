#include <stdio.h>

#include <exec/exec.h>
#include <clib/dos_protos.h>

#include "utility.h"

BOOL utility_exists (const char *path)
{
#ifdef DEBUG_UTILITY
    LONG error;
    STRPTR buf;
#endif
    BOOL ret = FALSE;
    BPTR l = Lock ((CONST_STRPTR)path, ACCESS_READ);
    if (l != 0) {
        ret = TRUE;
    }
#ifdef DEBUG_UTILITY
    else {
        error = IoErr();
        buf = (STRPTR)AllocMem (80, MEMF_ANY);
        if (buf != NULL) {
            Fault (error, "", buf, 80);
            fprintf (stderr, "Utility Lock error (%ld): %s\n", error, buf);
            FreeMem (buf, 80);
        }
    }
#endif
    UnLock (l);
    return ret;
}

