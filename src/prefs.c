#include <stdio.h>
#include <exec/exec.h>
#include <dos/dos.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include "prefs.h"
#include "notify.h"

/*
 * FIXME: check errors better.
 */
int prefs_load (void)
{
    int ret = 0;
    BPTR fh;
    char *buf = NULL;
    char *buf2;
    char *file = NULL;
    notify_reason_t r;
    BYTE i = 0;
    size_t len;

    buf = (char *)AllocMem (1025, MEMF_ANY);
    if (buf == NULL) {
        fprintf (stderr, "prefs_load: could not alloc memory for tmp buffer\n");
        return 1;
    }
    file = (char *)AllocMem (1025, MEMF_ANY);
    if (file == NULL) {
        fprintf (stderr, "prefs_load: could not alloc memory for tmp file string\n");
        FreeMem (buf, 1025);
        return 1;
    }

    if (fh = Open("ENV:executer.prefs", MODE_OLDFILE)) {
        while ((buf2 = (char *)FGets (fh, (STRPTR)buf, 1024)) != NULL) {
            len = strlen (buf2);
            if (buf2[len-1] == '\n') buf2[len-1] = '\0';
            fprintf (stderr, "Could not add notify. Buf2: %s, len: %lu\n", buf2, len);
            if (i == 0) {
                CopyMem (buf2, file, len);
                i++;
            } else if (i == 1) {
                r = (notify_reason_t)atoi (buf2);
                i++;
            } else if (i == 2) {
                if (notify_add (file, buf2, r) != 0) {
                    fprintf (stderr, "Could not add notify. File: %s Cmd: %s\n", file, buf2);
                }
                i = 0;
            }
        }
        Close (fh);
    } else {
        fprintf (stderr, "prefs_load: could not open prefs file\n");
    }

    if (file != NULL) FreeMem (file, 1025);
    if (buf != NULL) FreeMem (buf, 1025);

    return ret;
}

void prefs_use (void)
{
}

void prefs_save (void)
{
}

