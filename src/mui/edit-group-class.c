#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libraries/mui.h>


#include <proto/muimaster.h>
#include <proto/alib.h> /* DoMetmod */
#include <proto/exec.h> /* Wait */
#include <proto/intuition.h> /* NewObject */

#ifdef __MORPHOS__
#include <devices/rawkeycodes.h>
#endif

#include "vapor.h" /* Class macros */

#include "common.h"
#include "objects.h"

/* executer mui classes */
#include "classes.h"

#include "m68k.h"

struct ExecuterEditGroupData
{
    //APTR GR_list;
    APTR GR_apply;
 
    struct MUI_InputHandlerNode ihnode;
};

/* new */
DEFNEW(ExecuterEditGroup)
{
    //APTR GR_list;
    APTR GR_apply;

    obj = DoSuperNew(cl, obj,
        MUIA_UserData, MO_Executer_Edit_Group,
        MUIA_Group_Horiz, FALSE,
        InnerSpacing(0,0),
          //Child, GR_list = (APTR)NewObject (getexecuterlistgroupclass(), NULL, TAG_DONE),
          //Child, GR_apply = (APTR)NewObject (getexecuterapplygroupclass(), NULL, TAG_DONE),
          Child, GR_apply = ExecuterButton ("OK", 'o'),
        TAG_DONE);

    if (obj) {
        struct ExecuterEditGroupData *data = INST_DATA (cl, obj);
        //data->GR_list = GR_list;
        data->GR_apply = GR_apply;
    }

    return (ULONG)obj;
}

BEGINMTABLE2(executereditgroupclass)
DECNEW(ExecuterEditGroup)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Group, executereditgroupclass, ExecuterEditGroupData)
