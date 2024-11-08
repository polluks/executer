#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libraries/mui.h>

#include <clib/utility_protos.h>

#include <proto/muimaster.h>
#include <proto/alib.h> /* DoMetmod */
#include <proto/exec.h> /* Wait */
#include <proto/intuition.h> /* SetAttrs */

#include "vapor.h" /* Class macros */

#include "common.h"
#include "classes.h"
#include "objects.h"

#include "m68k.h"

struct ExecuterEditWindowData
{
    APTR GR_edit;
};

/* new */
DEFNEW(ExecuterEditWindow)
{
    APTR GR_edit;

    obj = DoSuperNew(cl, obj,
        MUIA_Window_Title,   "Executer - Edit",
        MUIA_Window_ID,      MAKE_ID ('E', 'X', 'E', 'D'),
        MUIA_UserData, MO_Executer_EditWindow,
        WindowContents, VGroup, 
            Child, GR_edit = NewObject (getexecutereditgroupclass(), NULL, TAG_DONE),
            TAG_END),
        TAG_MORE, (((struct opSet *)msg)->ops_AttrList),
        TAG_DONE);
    
    if (obj != NULL) {
        struct ExecuterEditWindowData *data = INST_DATA (cl, obj);
        data->GR_edit = GR_edit;
        //set (obj, MUIA_Window_Open, FALSE);
    }

    return (ULONG)obj;
}

BEGINMTABLE2(executereditwindowclass)
DECNEW(ExecuterEditWindow)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Window, executereditwindowclass, ExecuterEditWindowData)
