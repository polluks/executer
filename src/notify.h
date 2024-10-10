#ifndef EXECUTER_NOTIFY_H
#define EXECUTER_NOTIFY_H

#include <exec/types.h>
#include <exec/lists.h>
#include <dos/notify.h>

typedef enum {
    NOTIFY_REASON_CREATE = 0,
    NOTIFY_REASON_DELETE, 
    NOTIFY_REASON_MODIFY
} notify_reason_t;

typedef void (*notify_cb_t)(const char *path);

struct notify_item
{
    struct Node node;
    struct NotifyRequest request;
    char path[1024];
    char command[1024];
    notify_reason_t reason;
    BOOL initially_exists;
    notify_cb_t cb;
};

int notify_init (void);
void notify_free (void);

ULONG *notify_signals (void);
void notify_dispose (void);

int notify_add (const char *path, const char *command, notify_reason_t reason, notify_cb_t cb);
int notify_remove (const char *path);
int notify_clear (void);

int notify_get_items (struct notify_item **items, int *count);

#endif /* EXECUTER_NOTIFY_H */
