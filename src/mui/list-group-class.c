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

struct ExecuterListGroupData
{
    APTR LV_list;
    APTR LI_list;
    APTR BT_add;
    APTR BT_edit;
    APTR BT_remove;
};

/* new */
DEFNEW(ExecuterListGroup)
{
    APTR LV_list;
    APTR BT_add;
    APTR BT_edit;
    APTR BT_remove;

    obj = DoSuperNew(cl, obj,
        MUIA_UserData, MO_Executer_List_Group,
        MUIA_Group_Horiz, FALSE,
        InnerSpacing(0,0),
        Child, VGroup,
          Child, LV_list = (APTR)NewObject (getexecuterlistviewclass(), NULL, TAG_DONE),
          End,
        Child, HGroup,
          Child, BT_add = ExecuterButton ("Add", 'a'),
          Child, BT_edit = ExecuterButton ("Edit", 'e'),
          Child, BT_remove = ExecuterButton ("Remove", 'r'),
          End,
        TAG_DONE);

    if (obj) {
        struct ExecuterListGroupData *data = INST_DATA (cl, obj);

        data->LV_list = LV_list;
        data->BT_add = BT_add;
        data->BT_edit = BT_edit;
        data->BT_remove = BT_remove;
        data->LI_list = (APTR) DoMethod (obj, MUIM_FindUData, MO_Executer_List);

        set (data->BT_add, MUIA_Disabled, FALSE);
        set (data->BT_edit, MUIA_Disabled, TRUE);
        set (data->BT_remove, MUIA_Disabled, TRUE);

        DoMethod (data->BT_add, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterListGroup_Add);
        DoMethod (data->BT_edit, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterListGroup_Edit);
        DoMethod (data->BT_remove, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterListGroup_Remove);

        DoMethod (data->LI_list, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MM_ExecuterListGroup_SelectChange);
    }


    return (ULONG)obj;
}

/* Own methods */
DEFTMETHOD(ExecuterListGroup_Add)
{
//    struct ExecuterListGroupData *data = INST_DATA (cl, obj);
    APTR window = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_MainWindow);
    DoMethod (window, MM_ExecuterMainWindow_ToggleMode, NULL);
    return 0;
}

DEFTMETHOD(ExecuterListGroup_Edit)
{
    struct ExecuterListGroupData *data = INST_DATA (cl, obj);
    DoMethod (data->LI_list, MM_ExecuterList_EditSelected);
    return 0;
}

DEFTMETHOD(ExecuterListGroup_Remove)
{
    struct ExecuterListGroupData *data = INST_DATA (cl, obj);
    DoMethod (data->LI_list, MM_ExecuterList_RemoveSelected);
    return 0;
}
        
DEFTMETHOD(ExecuterListGroup_SelectChange)
{
    struct ExecuterListGroupData *data = INST_DATA (cl, obj);
    LONG active = -1;
    get (data->LI_list, MUIA_List_Active, &active);
    if (active == MUIV_List_Active_Off) {
        set (data->BT_edit, MUIA_Disabled, TRUE);
        set (data->BT_remove, MUIA_Disabled, TRUE);
    } else {
        set (data->BT_edit, MUIA_Disabled, FALSE);
        set (data->BT_remove, MUIA_Disabled, FALSE);
    }
    return 0;
}

BEGINMTABLE2(executerlistgroupclass)
DECNEW(ExecuterListGroup)
DECTMETHOD(ExecuterListGroup_Add)
DECTMETHOD(ExecuterListGroup_Edit)
DECTMETHOD(ExecuterListGroup_Remove)
DECTMETHOD(ExecuterListGroup_SelectChange)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Group, executerlistgroupclass, ExecuterListGroupData)
