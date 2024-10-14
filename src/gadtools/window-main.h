#ifndef EXECUTER_GADTOOLS_WINDOW_MAIN_H
#define EXECUTER_GADTOOLS_WINDOW_MAIN_H

#include <exec/types.h>

int window_main_init (struct TextAttr *textattr, void *visualinfo, UWORD topborder);
void window_main_free (void);
ULONG window_main_signal (void);
int window_main_visibility (BOOL visible);
BOOL window_main_is_visible (void);
void window_main_dispose (BOOL *quit);

int window_main_setup_list (struct List *nitems);

#endif /* EXECUTER_GADTOOLS_WINDOW_MAIN_H */
