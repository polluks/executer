#ifndef EXECUTER_GADTOOLS_WINDOW_EDIT_H
#define EXECUTER_GADTOOLS_WINDOW_EDIT_H

#include <exec/types.h>
#include "../notify.h"

int window_edit_init (struct TextAttr *textattr, void *visualinfo, UWORD topborder);
void window_edit_free (void);
ULONG window_edit_signal (void);
int window_edit_visibility (BOOL visible);
BOOL window_edit_is_visible (void);
void window_edit_dispose (BOOL *quit);

int window_edit_add (int index);
int window_edit_edit (int index, struct notify_item *item);

#endif /* EXECUTER_GADTOOLS_WINDOW_EDIT_H */
