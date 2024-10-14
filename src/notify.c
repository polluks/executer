#include <stdio.h>

#include <exec/exec.h>
#include <dos/notify.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include "notify.h"
#include "utility.h"

#define MAX_NOTIFIES 100
ULONG _signal;

static struct MsgPort *_port = NULL;
static struct List *_list = NULL;

static const char *_reason_to_string (int reason); 

int notify_init (void)
{
    _port = CreateMsgPort ();
    if (_port == NULL) {
         fprintf (stderr, "Notify: Could not creeate msg port.\n");
         return 1;
    }
    _list = AllocMem (sizeof(struct List), MEMF_ANY|MEMF_CLEAR);
    if (_list == NULL) {
        fprintf (stderr, "Could not alloc notify list.\n");
        notify_free ();
        return 1;
    }
    NewList (_list);
    return 0;
}

void notify_free (void)
{
    _signal = 0;
    (void)notify_clear ();
    if (_list != NULL) {
        FreeMem (_list, sizeof (struct List));
    }
    if (_port != NULL) {
        DeleteMsgPort (_port);
    }
}

ULONG notify_signal (void)
{
    if (_port == NULL) return 0;
    return (1L << _port->mp_SigBit);
}

void notify_dispose (void)
{
    struct NotifyMessage *msg;
    struct notify_item *item;
    BOOL exists;
    int reason;

    if (_list == NULL) {
        fprintf (stderr, "Notify add: invalid list.\n");
        return; /* error */
    }

    if (IsListEmpty (_list)) {
        return;
    }

    while (msg = (struct NotifyMessage *)GetMsg (_port)) {
            reason = NOTIFY_REASON_MODIFY;
            item = (struct notify_item *)msg->nm_NReq->nr_UserData;
	    ReplyMsg ((struct Message *)msg);
            /* FIXME: handle command here. */

            exists = utility_exists (item->path);
            fprintf (stderr, "\n\nExists: %s. Item exists: %s\n", exists?"TRUE":"FALSE", item->exists?"TRUE":"FALSE");
            if (exists != item->exists) {
                if (exists == TRUE) reason = NOTIFY_REASON_CREATE;
                else reason = NOTIFY_REASON_DELETE;
            }
            fprintf (stderr, "Triggered path: %s with reason %s. Item reasons: %s \n",
                item->path, _reason_to_string (reason), notify_reason_bitfield_to_string (item->reason));
            item->exists = exists;

            if (item->reason & reason) {
                fprintf (stderr, "Reasons match. Try to spawn cmd: %s\n", item->command);
                if (item->cb != NULL) {
                    item->cb (item->path);
                } else {
                    (void)spawn_start (item->command);
                }
            } else {
                fprintf (stderr, "Reasons did not match. Trigger: %s != Item: %s \n", _reason_to_string (reason), notify_reason_bitfield_to_string (item->reason));
            }
    }
}

int notify_add (const char *path, const char *command, int reason, notify_cb_t cb)
{
    struct notify_item *item;
    
    if (_list == NULL) {
        fprintf (stderr, "Notify add: invalid list.\n");
        return 1; /* error */
    }

    if (path == NULL || (command == NULL && cb == NULL) || (cb == NULL && !strcmp(command,""))) {
        fprintf (stderr, "Notify add: invalid input\n");
        return 1;
    }

/*
    if (FALSE == utility_exists (command)) {
        fprintf (stderr, "Notify add: invalid command. Does not exists or don't have access to file: %s.\n", command);
        return 1;
    }
*/  
    item = (struct notify_item *) AllocMem (sizeof (struct notify_item), MEMF_ANY|MEMF_CLEAR);
    if (item == NULL) {
        fprintf (stderr, "Notify add: could not alloc new item.\n");
        return 1;
    }

    item->exists = utility_exists (path);
    item->reason = reason;
    item->cb = cb;
    CopyMem (path, item->path, strlen (path) + 1);
    CopyMem (command, item->command, strlen (command) + 1);

    item->node.ln_Name = (UBYTE *)item->path;

    item->request.nr_Name = (UBYTE *)item->path;
    item->request.nr_Flags = NRF_SEND_MESSAGE | NRF_WAIT_REPLY;
    item->request.nr_stuff.nr_Msg.nr_Port = _port;
    item->request.nr_UserData = (ULONG)item;
 
    AddTail (_list, (struct Node *)item);

    if ((StartNotify (&(item->request))) != DOSTRUE) {
        Remove ((struct Node *)item);
        FreeMem (item, sizeof (struct notify_item));
        fprintf (stderr, "Notify add: StartNotify() failed.\n");
        return 1;
    }
    fprintf (stderr, "Item added path: %s, cmd: %s, reason: %s\n",
                item->path, item->command, notify_reason_bitfield_to_string (item->reason));

    return 0;
}

int notify_remove (const char *path)
{
    struct notify_item *item;
    struct notify_item *next;
    
    if (_list == NULL) {
        return 1; /* error */
    }

    if (IsListEmpty (_list)) {
        return 0;
    }

    item = (struct notify_item *)_list->lh_Head;
    while ((next = (struct notify_item *) item->node.ln_Succ) != NULL) {
        if (strncmp (item->path, path, strlen (path) + 1) == 0) {
            EndNotify (&item->request);
            Remove ((struct Node *) item);
            FreeMem (item, sizeof (struct notify_item));
            break;
        }
        item = next;
    }

    return 0;
}

int notify_clear (void)
{
    struct notify_item *item;
    struct notify_item *next;
    
    if (_list == NULL) {
        return 1; /* error */
    }

    if (IsListEmpty (_list)) {
        return 0; /* already empty */
    }

    item = (struct notify_item *)_list->lh_Head;
    while (next = (struct notify_item *)item->node.ln_Succ) {
        EndNotify (&(item->request));
        Remove ((struct Node *) item);
        FreeMem (item, sizeof (struct notify_item));
        item = next;
    }

    return 0;
}

struct List *notify_list (void)
{
    return _list;
}

static const char *_reason_to_string (int reason)
{
    if (reason == NOTIFY_REASON_CREATE) return "C";
    else if (reason == NOTIFY_REASON_DELETE) return "D";
    return "M";
}

static char _rbfstr[7];
const char *notify_reason_bitfield_to_string (int reason)
{
    int pos = 0;
    BOOL added = FALSE;
    for (pos = 0; pos < 7; pos++) _rbfstr[pos] = '\0';
    pos = 0;
    if (reason & NOTIFY_REASON_CREATE) {
        _rbfstr[pos++] = 'C';
        added = TRUE;
    }
    if (reason & NOTIFY_REASON_DELETE) {
        if (added == TRUE) _rbfstr[pos++] = '|';
        _rbfstr[pos++] = 'D';
        added = TRUE;
    }
    if (reason & NOTIFY_REASON_MODIFY) {
        if (added == TRUE) _rbfstr[pos++] = '|';
        _rbfstr[pos++] = 'M';
    }
    return _rbfstr;
}
