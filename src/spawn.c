#include <stdio.h>

#include <exec/types.h>
#include <dos/dostags.h>

#include <clib/dos_protos.h>

#include "spawn.h"

static UBYTE *_con=(UBYTE *)"CON:0/40/640/150/Executer/auto/close/wait";

int spawn_start (const char *cmd)
{
    int ret = 0;
    BPTR con;
    LONG retst;

    if (cmd == NULL) {
        fprintf (stderr, "Cmd NULL.\n");
        return 1;
    }

    con = Open (_con, MODE_OLDFILE);
    if (!con) {
        fprintf (stderr, "Could not open CON:\n");
        return 1;
    }

    retst = SystemTags ((CONST_STRPTR)cmd,
        SYS_Asynch, TRUE,
        SYS_UserShell, TRUE,
        SYS_Input, con,
        SYS_Output, NULL,
        TAG_DONE);

    if (retst != 0) {
        fprintf (stderr, "Problem run cmd.\n");
        Close (con);
        ret = 1;
    }

    return ret;
}
