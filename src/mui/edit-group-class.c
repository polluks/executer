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
    APTR LB_path;
    APTR ST_path;
    APTR LB_command;
    APTR ST_command;
    APTR LB_create;
    APTR CH_create;
    APTR LB_modify;
    APTR CH_modify;
    APTR LB_remove;
    APTR CH_remove;
    APTR BT_ok;
    APTR BT_cancel;
};

/* new */
DEFNEW(ExecuterEditGroup)
{
    APTR LB_path;
    APTR ST_path;
    APTR LB_command;
    APTR ST_command;
    APTR LB_create;
    APTR CH_create;
    APTR LB_modify;
    APTR CH_modify;
    APTR LB_remove;
    APTR CH_remove;
    APTR BT_ok;
    APTR BT_cancel;

    obj = DoSuperNew(cl, obj,
        MUIA_UserData, MO_Executer_Edit_Group,
        MUIA_Group_Horiz, FALSE,
        MUIA_Group_SameHeight, TRUE,
        MUIA_Group_SameSize, FALSE,
        InnerSpacing(0,0),
        Child, VGroup,
          Child, HGroup,
            Child, LB_path = ExecuterLabel("Path"),
            Child, ST_path = ExecuterString (255, ""),
            End,
          Child, HGroup,
            Child, LB_create = ExecuterLabel("Create"),
            Child, CH_create = ExecuterCheck(FALSE),
            End,
          Child, HGroup,
            Child, LB_modify = ExecuterLabel("Modify"),
            Child, CH_modify = ExecuterCheck (FALSE),
            End,
          Child, HGroup,
            Child, LB_remove = ExecuterLabel("Remove"),
            Child, CH_remove = ExecuterCheck (FALSE),
            End,
          Child, HGroup,
            Child, LB_command = ExecuterLabel("Command"),
            Child, ST_command = ExecuterString (255, ""),
            End,
          End,
        Child, HGroup,
          MUIA_Group_SameSize, TRUE,
          Child, BT_ok = ExecuterButton ("OK", 'o'),
          Child, BT_cancel = ExecuterButton ("Cancel", 'c'),
          End,
        TAG_DONE);

    if (obj) {
        struct ExecuterEditGroupData *data = INST_DATA (cl, obj);
        data->LB_path = LB_path;
        data->ST_path = ST_path;
        data->LB_command = LB_command;
        data->ST_command = ST_command;
        data->LB_create = LB_create;
        data->CH_create = CH_create;
        data->LB_modify = LB_modify;
        data->CH_modify = CH_modify;
        data->LB_remove = LB_remove;
        data->CH_remove = CH_remove;
        data->BT_ok = BT_ok;
        data->BT_cancel = BT_cancel;
        
        DoMethod (data->BT_ok, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterEditGroup_Ok);
        DoMethod (data->BT_cancel, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterEditGroup_Cancel);
    }

    return (ULONG)obj;
}

DEFTMETHOD(ExecuterEditGroup_Ok)
{
//    struct ExecuterApplyGroupData *data = INST_DATA (cl, obj);
    APTR window = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_MainWindow);
    DoMethod (window, MM_ExecuterMainWindow_ToggleMode, NULL);
    return 0;
}

DEFTMETHOD(ExecuterEditGroup_Cancel)
{
//    struct ExecuterApplyGroupData *data = INST_DATA (cl, obj);
    APTR window = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_MainWindow);
    DoMethod (window, MM_ExecuterMainWindow_ToggleMode, NULL);
    return 0;
}


BEGINMTABLE2(executereditgroupclass)
DECNEW(ExecuterEditGroup)
DECTMETHOD(ExecuterEditGroup_Ok)
DECTMETHOD(ExecuterEditGroup_Cancel)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Group, executereditgroupclass, ExecuterEditGroupData)
