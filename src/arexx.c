#include <stdio.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/arexx.h>
#include <proto/rexxsyslib.h>

#include <exec/ports.h>
#include <rexx/errors.h>
#include <rexx/storage.h>

#include <clib/alib_protos.h>

#include "arexx.h"
#include "debug.h"

static ULONG _signal = 0;
static struct MsgPort *_port = NULL;
static struct rx_command *_list = NULL;

/*
 * Init AREXX port and signal bit
 *
 * @param: name, name of the AREXX port
 * @param: list, command list. Can be NULL
 */
int arexx_init (const char *name, struct rx_command  *list)
{
    int retval = 1;
    Forbid ();
    if (FindPort ((CONST_STRPTR)name) == NULL) {
        _port = (struct MsgPort *)CreatePort ((CONST_STRPTR)name, 0L);
    }
    Permit ();
    if (_port != NULL) {
        _signal = 1L << _port->mp_SigBit;
        _list = list;
        retval = 0;
    } else {
        retval = -1;
    }
    return retval;
}

ULONG arexx_signal (void)
{
    return _signal;
}

void arexx_free (void)
{
    if (_port) {
        RemPort (_port);
        _port = NULL;
#if 0
        if (oRexxMsg) {¬
            oRexxMsg->rm_Result1 = RXERRORIMGONE ;¬
            ReplyMsg((struct Message *)oRexxMsg) ;¬
            oRexxMsg = NULL ;¬
        }
#endif
    }
}

int arexx_dispose (void)
{
    struct rx_command *rcmd;
    char *args;
    size_t len = 0;
    struct RexxMsg *msg;
    BOOL reply = FALSE;

    if (_port == NULL) return 1;

    while ((msg = (struct RexxMsg *)GetMsg(_port))) {
        if (msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG) { /* got reply. we sent arexx */
        } else {
            /* receive arexx command */
            D(BUG("AREXX in: %s\n", (char *)msg->rm_Args[0]));
            args = (char *)msg->rm_Args[0];
            while (*args != '\0' && *args != ' ') args++;
            if (*args == ' ') args++;
            rcmd = _list;
            if (rcmd != NULL) {
                for (; rcmd->command != NULL; rcmd++) {
                    len = strlen(rcmd->command);
                    if ( 0 == strncmp (rcmd->command, (const char *)msg->rm_Args[0], strlen(rcmd->command)) && 
                        (((char *)msg->rm_Args[0])[len] == ' ' || ((char *)msg->rm_Args[0])[len] == '\0')) {
                        D(BUG("call command: %s\n", rcmd->command));
                        rcmd->func (msg, args);
                        break;
                    }
                }
            }
            if (rcmd == NULL || rcmd->command == NULL) {
                msg->rm_Result1 = ERR10_011; /* command string error */
            }
            reply = TRUE;
        }

        if (reply == TRUE) {
            ReplyMsg((struct Message *)msg);
        }
    }
    return 0;
}

