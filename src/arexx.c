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

void arexx_send_simple (const char *name, const char *command)
{
    struct MsgPort *rp = NULL;
    struct MsgPort *rxport = NULL;
    struct RexxMsg *msg = NULL;
    struct RexxMsg *rm;

    D(BUG("AREXX - Sending command %s to port %s\n", command, name));
    rp = CreateMsgPort();
    if (rp == NULL) {
        fprintf (stderr, "Could not create reply port\n");
        return;
    }

    msg = CreateRexxMsg (rp, NULL, name);
    if (msg == NULL) {
        DeleteMsgPort (rp);
        fprintf (stderr, "Could not create RexxMsg\n");
        return;
    }
    msg->rm_Args[0] = (STRPTR)CreateArgstring (command, strlen (command)+1);
    msg->rm_Action = RXCOMM;
    msg->rm_Node.mn_Node.ln_Name="REXX";

    Forbid();
    rxport = FindPort (name);
    if (rxport != NULL) {
        D(BUG("AREXX - send  message: %p to port: %p\n", msg, rxport));
        PutMsg (rxport, (struct Message *)msg);
    }
    Permit();

    if (rxport != NULL) {
        rm = (struct RexxMsg *)WaitPort (rp);
        while ((rm = (struct RexxMsg *)GetMsg (rp))) {
            D(BUG("AREXX - got reply: %p\n", rm));
        }
    }
    DeleteMsgPort (rp);
    DeleteRexxMsg (msg);
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
            D(BUG("AREXX dispose reply: %s\n", (char *)msg->rm_Args[0]));
        } else {
            /* receive arexx command */
            D(BUG("AREXX dispose got message: %s\n", (char *)msg->rm_Args[0]));
            args = (char *)msg->rm_Args[0];
            while (*args != '\0' && *args != ' ') args++;
            if (*args == ' ') args++;
            rcmd = _list;
            if (rcmd != NULL) {
                for (; rcmd->command != NULL; rcmd++) {
                    len = strlen(rcmd->command);
                    if ( 0 == strncmp (rcmd->command, (const char *)msg->rm_Args[0], strlen(rcmd->command)) && 
                        (((char *)msg->rm_Args[0])[len] == ' ' || ((char *)msg->rm_Args[0])[len] == '\0')) {
                        D(BUG("AREXX dispose - call command: %s\n", rcmd->command));
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

