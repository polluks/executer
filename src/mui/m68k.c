#ifndef __MORPHOS__
#include <stdarg.h>
#include <clib/alib_protos.h>

#include "m68k.h"

APTR DoSuperNew( struct IClass *cl, APTR obj, Tag tags, ...)
{
    struct opSet MyopSet;
    MyopSet.MethodID = OM_NEW;
    MyopSet.ops_AttrList = (struct TagItem*)&tags;
    MyopSet.ops_GInfo = NULL;
    return (APTR)DoSuperMethodA(cl, obj, (APTR) &MyopSet);
}

#endif
