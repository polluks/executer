#ifndef EXECUTER_NOTIFY_H
#define EXECUTER_NOTIFY_H

#include <exec/types.h>
#include <exec/lists.h>
#include <dos/notify.h>

enum {
    NOTIFY_REASON_NONE = 0,
    NOTIFY_REASON_CREATE = 1,
    NOTIFY_REASON_DELETE = 2,
    NOTIFY_REASON_MODIFY = 4
};

typedef void (*notify_cb_t)(const char *path);

struct notify_item
{
    struct Node node;
    struct NotifyRequest request;
    char path[1024];
    char command[1024];
    int reason;
    BOOL exists;
    notify_cb_t cb;
};

int notify_init (void);
void notify_free (void);

ULONG *notify_signals (void);
void notify_dispose (void);

int notify_add (const char *path, const char *command, int reason, notify_cb_t cb);
int notify_remove (const char *path);
int notify_clear (void);

struct List *notify_list (void);

const char *notify_reason_bitfield_to_string (int reason);

#endif /* EXECUTER_NOTIFY_H */
