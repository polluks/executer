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

struct ExecuterApplyGroupData
{
    APTR BT_save;
    APTR BT_use;
    APTR BT_cancel;
 
    struct MUI_InputHandlerNode ihnode;
};

/* new */
DEFNEW(ExecuterApplyGroup)
{
    APTR BT_save;
    APTR BT_use;
    APTR BT_cancel;

    obj = DoSuperNew(cl, obj,
        MUIA_UserData, MO_Executer_Apply_Group,
        MUIA_Group_Horiz, TRUE,
        InnerSpacing(0,0),
        Child, HGroup,
          Child, BT_save = ExecuterButton ("Save", 's'),
          Child, BT_use = ExecuterButton ("Use", 'u'),
          Child, BT_cancel = ExecuterButton ("Cancel", 'c'),
        End,
        TAG_DONE);

    if (obj) {
        struct ExecuterApplyGroupData *data = INST_DATA (cl, obj);

        data->BT_save = BT_save;
        data->BT_use = BT_use;
        data->BT_cancel = BT_cancel;
        
        DoMethod (data->BT_save, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterApplyGroup_Save);
        DoMethod (data->BT_use, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterApplyGroup_Use);
        DoMethod (data->BT_cancel, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterApplyGroup_Cancel);
    }

    return (ULONG)obj;
}

/* Own methods */
DEFTMETHOD(ExecuterApplyGroup_Save)
{
//    struct ExecuterApplyGroupData *data = INST_DATA (cl, obj);
    DoMethod (_app(obj), MM_ExecuterApplication_Quit);
    return 0;
}

DEFTMETHOD(ExecuterApplyGroup_Use)
{
//    struct ExecuterApplyGroupData *data = INST_DATA (cl, obj);
    DoMethod (_app(obj), MM_ExecuterApplication_Quit);
    return 0;
}

DEFTMETHOD(ExecuterApplyGroup_Cancel)
{
//    struct ExecuterApplyGroupData *data = INST_DATA (cl, obj);
    DoMethod (_app(obj), MM_ExecuterApplication_Quit);
    return 0;
}

BEGINMTABLE2(executerapplygroupclass)
DECNEW(ExecuterApplyGroup)
DECTMETHOD(ExecuterApplyGroup_Save)
DECTMETHOD(ExecuterApplyGroup_Use)
DECTMETHOD(ExecuterApplyGroup_Cancel)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Group, executerapplygroupclass, ExecuterApplyGroupData)
