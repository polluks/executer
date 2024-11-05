#include <stdio.h>

#include "vapor.h"
#include "classes.h"

/* Classes management */

struct classdesc {
    char * name;
    APTR initfunc;
    APTR cleanupfunc;
};

#define CLASSENT(s) {#s, create_##s##class, delete_##s##class}

/* classes declaration */

static const struct classdesc cd[] = {
    CLASSENT(executerapplication),
    CLASSENT(executermainwindow),
    CLASSENT(executermaingroup),
    CLASSENT(executereditgroup),
    CLASSENT(executerlistgroup),
    CLASSENT(executerlistview),
    CLASSENT(executerapplygroup),
    CLASSENT(executerlist),
    { 0, 0, 0 }
};

BOOL classes_init(void)
{
    ULONG i;

    for (i = 0; cd[i].name; i++)
    {
        if (!(*(int(*)(void))cd[i].initfunc)())
        {
            fprintf(stderr, "Couldn't create class %s.\n", cd[i].name);
            return (FALSE);
        }
    }
    return (TRUE);
}


void classes_cleanup(void)
{
    LONG i;

    for (i = sizeof(cd) / sizeof(struct classdesc) - 2; i >= 0; i--)
    {
        (*(void(*)(void))cd[i].cleanupfunc)();
    }
}

