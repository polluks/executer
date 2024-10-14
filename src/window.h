#ifndef EXECUTER_WINDOW_H
#define EXECUTER_WINDOW_H

#include <exec/types.h>

int window_init (void);
void window_free (void);
ULONG window_signal (void);
int window_visibility (BOOL visible);
BOOL window_is_visible (void);
void window_dispose (BOOL *quit);

int window_setup_list (struct List *nitems);

#endif /* EXECUTER_WINDOW_H */
