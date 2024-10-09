#include <stdio.h>

#include <exec/exec.h>
#include <dos/notify.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include "notify.h"

#define MAX_NOTIFIES 100
ULONG _signal;

static struct MsgPort *_port = NULL;
static struct List *_list = NULL;

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
    notify_clear ();
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

    if (IsListEmpty (_list)) {
        return;
    }

    while (msg = (struct NotifyMessage *)GetMsg (_port)) {
            item = (struct notify_item *)msg->nm_NReq->nr_UserData;
	    ReplyMsg ((struct Message *)msg);
            /* FIXME: handle script here. */
	    fprintf (stderr, "path: %s, script: %s\n", item->path, item->script);
    }
}

int notify_add (const char *path, const char *script, notify_reason_t reason)
{
    struct notify_item *item;
    if (path == NULL || script == NULL) {
        fprintf (stderr, "Notify add: invalid input\n");
        return 1;
    }

    if (FALSE == utility_exists (script)) {
        fprintf (stderr, "Notify add: invalid script. Does not exists or don't have access to file: %s.\n", script);
        return 1;
    }
    
    item = (struct notify_item *) AllocMem (sizeof (struct notify_item), MEMF_ANY|MEMF_CLEAR);
    if (item == NULL) {
        fprintf (stderr, "Notify add: could not alloc new item.\n");
        return 1;
    }

    item->initially_exists = utility_exists (path);
    item->reason = reason;
    CopyMem (path, item->path, strlen (path) + 1);
    CopyMem (script, item->script, strlen (script) + 1);

    item->node.ln_Name = (UBYTE *)item->path;

    item->request.nr_Name = (UBYTE *)item->path;
    item->request.nr_Flags = NRF_SEND_MESSAGE | NRF_WAIT_REPLY;
    item->request.nr_stuff.nr_Msg.nr_Port = _port;
    item->request.nr_UserData = (ULONG)item;

    if ((StartNotify (&(item->request))) != DOSTRUE) {
        FreeMem (item, sizeof (struct notify_item));
        fprintf (stderr, "Notify add: StartNotify() failed.\n");
        return 1;
    }
    
    AddTail (_list, (struct Node *)item);

    return 0;
}

int notify_remove (const char *path)
{
    struct notify_item *item;
    struct notify_item *next;

    if (IsListEmpty (_list)) {
        return 1;
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
