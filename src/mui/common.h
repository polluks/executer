#ifndef _EXECUTER_MUI_COMMON_
#define _EXECUTER_MUI_COMMON_

#include <proto/intuition.h> /* NewObject */
#include <proto/utility.h> /* NextTagItem */

#include <exec/types.h>

#include "vapor.h"

struct ExecuterApp
{
    APTR app;

};

#define executer_print_error(app, msg) \
    do { \
        if (app != NULL) { \
            MUI_Request (app, NULL, 0, "Executer error", "_OK", (char *)(msg)); \
        } else { \
            fprintf (stderr, "%s\n", (char *)(msg)); \
        } \
    } while(0)

#define FORTAG(_tagp) \
    { \
        struct TagItem *tag, *_tags = (struct TagItem *)(_tagp); \
        while ((tag = NextTagItem(&_tags))) switch ((int)tag->ti_Tag)
#define NEXTTAG }
#define INITTAGS (((struct opSet *)msg)->ops_AttrList)

#ifndef IPTR
# define IPTR LONG
#endif

#endif
