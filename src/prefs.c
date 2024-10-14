#include <stdio.h>
#include <exec/exec.h>
#include <dos/dos.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include "prefs.h"
#include "notify.h"

static int _save_to (const char *path);

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
    int reason;
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

    notify_clear ();
    fh = Open (PREFS_PATH_ENV, MODE_OLDFILE);
    if (fh != 0) {
        while ((buf2 = (char *)FGets (fh, (STRPTR)buf, 1024)) != NULL) {
            len = strlen (buf2);
            if (buf2[len-1] == '\n') buf2[len-1] = '\0';
            if (i == 0) {
                CopyMem (buf2, file, len);
                i++;
            } else if (i == 1) {
                reason = atoi (buf2);
                i++;
            } else if (i == 2) {
                if (notify_add (file, buf2, reason, NULL) != 0) {
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

int prefs_save_env (void)
{
    return _save_to (PREFS_PATH_ENV);
}

int prefs_save_envarc (void)
{
    return _save_to (PREFS_PATH_ENVARC);
}

static int _save_to (const char *path)
{
    char c;
    LONG err;
    int ret = 0;
    BPTR fh;
    struct notify_item *item, *next;
    struct List *list;

    if (path == NULL) {
        fprintf (stderr, "Prefs: save path is NULL.\n");
        return 1;
    }

    list = notify_list ();
    if (list == NULL) {
        fprintf (stderr, "Prefs: list to save is NULL.\n");
        return 1;
    }

    if ((fh = Open (path, MODE_NEWFILE)) == 0) {
        fprintf (stderr, "Prefs: could not write to save path: %s.\n", path);
        return 1;
    }
    
    if (IsListEmpty (list)) {
        Close (fh);    
        return 0;
    }

    item = (struct notify_item *)list->lh_Head;
    while ((next = (struct notify_item *) item->node.ln_Succ) != NULL) {
        if (item->cb != NULL) {
            item = next;
            continue;
        }
        /* path */
        err = FPuts (fh, (STRPTR)item->path);
        if (err != 0) break;
        err = FPutC (fh, (LONG)'\n');
        if (err == EOF) break;
        /* reason */
        c = '0' + (char)item->reason; /* value should be 1-7 ( bitfield ) */
        err = FPutC (fh, (LONG)c);
        if (err == EOF) break;
        err = FPutC (fh, (LONG)'\n');
        if (err == EOF) break;
        /* command */
        err = FPuts (fh, (STRPTR)item->command);
        if (err != 0) break;
        err = FPutC (fh, (LONG)'\n');
        if (err == EOF) break;

        item = next;
    }

    Close (fh);
    return ret;
}

