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
    APTR CM_create;
    APTR LB_modify;
    APTR CM_modify;
    APTR LB_delete;
    APTR CM_delete;
    APTR BT_ok;
    APTR BT_cancel;

    struct notify_item *item;
};

/* new */
DEFNEW(ExecuterEditGroup)
{
    APTR LB_path;
    APTR ST_path;
    APTR LB_command;
    APTR ST_command;
    APTR LB_create;
    APTR CM_create;
    APTR LB_modify;
    APTR CM_modify;
    APTR LB_delete;
    APTR CM_delete;
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
            Child, ST_path = ExecuterString (1024, ""),
            End,
          Child, HGroup,
            Child, LB_create = ExecuterLabel("Create"),
            Child, CM_create = ExecuterCheck(FALSE),
            End,
          Child, HGroup,
            Child, LB_modify = ExecuterLabel("Modify"),
            Child, CM_modify = ExecuterCheck (FALSE),
            End,
          Child, HGroup,
            Child, LB_delete = ExecuterLabel("Remove"),
            Child, CM_delete = ExecuterCheck (FALSE),
            End,
          Child, HGroup,
            Child, LB_command = ExecuterLabel("Command"),
            Child, ST_command = ExecuterString (1024, ""),
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
        data->CM_create = CM_create;
        data->LB_modify = LB_modify;
        data->CM_modify = CM_modify;
        data->LB_delete = LB_delete;
        data->CM_delete = CM_delete;
        data->BT_ok = BT_ok;
        data->BT_cancel = BT_cancel;
        
        DoMethod (data->BT_ok, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterEditGroup_Ok);
        DoMethod (data->BT_cancel, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MM_ExecuterEditGroup_Cancel);
    }

    return (ULONG)obj;
}

DEFGET(ExecuterEditGroup)
{
    struct ExecuterEditGroupData *data = INST_DATA(cl, obj);

    switch (msg->opg_AttrID) {
        case MA_Executer_EditItem: {
            *msg->opg_Storage = (ULONG)data->item;
        }
        return TRUE;
        default:
            break;
    }

    return DOSUPER;
}

static void reset(struct ExecuterEditGroupData *data)
{
    ULONG b = 0;
    fprintf (stderr, "reset() - 0\n");
    set (data->CM_create, MUIA_Selected, b);
    set (data->CM_modify, MUIA_Selected, b);
    set (data->CM_delete, MUIA_Selected, b);
    fprintf (stderr, "reset() - 1\n");
    set (data->ST_path, MUIA_String_Contents, (ULONG)"");
    set (data->ST_command, MUIA_String_Contents, (ULONG)"");
    fprintf (stderr, "reset() - 2\n");
    data->item = NULL;
    fprintf (stderr, "reset() - 3\n");
}

static void doset(APTR obj, struct ExecuterEditGroupData *data, struct TagItem *tags)
{
    FORTAG(tags)
    {
        case MA_Executer_EditItem: {
            data->item = (struct nofity_item *)tag->ti_Data;
            fprintf (stderr, "Trying to set0: item: %p path: '%s', command: '%s'\n", data->item, data->item->path, data->item->command);
            if  (data->item != NULL) {
                BOOL create = ((data->item->reason & NOTIFY_REASON_CREATE) != 0)?TRUE:FALSE;
                BOOL delete = ((data->item->reason & NOTIFY_REASON_DELETE) != 0)?TRUE:FALSE;
                BOOL modify = ((data->item->reason & NOTIFY_REASON_MODIFY) != 0)?TRUE:FALSE;

                set (data->CM_create, MUIA_Selected, create);
                set (data->CM_modify, MUIA_Selected, modify);
                set (data->CM_delete, MUIA_Selected, delete);

                fprintf (stderr, "Trying to set: path: '%s', command: '%s'\n", data->item->path, data->item->command);
                set (data->ST_path, MUIA_String_Contents, (ULONG)data->item->path);
                set (data->ST_command, MUIA_String_Contents, (ULONG)data->item->command);
            } else {
                reset (data);
            }
        }
        break;
    }
    NEXTTAG
}

/* set */
DEFSET(ExecuterEditGroup)
{
    struct ExecuterEditGroupData *data = INST_DATA(cl, obj);
    doset (obj, data, INITTAGS);
    return DOSUPER;
}


DEFTMETHOD(ExecuterEditGroup_Ok)
{
    struct ExecuterEditGroupData *data = INST_DATA (cl, obj);
    APTR list, window;
    char *path = NULL;
    char *command = NULL;
    ULONG create = 0;
    ULONG delete = 0;
    ULONG modify = 0;
    int reason = 0;

    get (data->ST_path, MUIA_String_Contents, &path);
    get (data->ST_command, MUIA_String_Contents, &command);

    get (data->CM_create, MUIA_String_Contents, &create);
    reason = (create!=0)?NOTIFY_REASON_CREATE:0;
    get (data->CM_delete, MUIA_String_Contents, &delete);
    reason |= (delete!=0)?NOTIFY_REASON_DELETE:0;
    get (data->CM_modify, MUIA_String_Contents, &modify);
    reason |= (modify!=0)?NOTIFY_REASON_MODIFY:0;

    if (data->item != NULL) {
        /* edit */
        data->item->reason = reason;
        CopyMem(path, data->item->path, strlen(path) + 1);
        CopyMem(command, data->item->command, strlen(path) + 1);
    } else {
       /* new */
        notify_add (path, command, reason, NULL);
    }
    reset (data);
    
    list = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_List);
    DoMethod (list, MM_ExecuterList_Update, NULL);

    window = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_MainWindow);
    DoMethod (window, MM_ExecuterMainWindow_ToggleMode, NULL);
    return 0;
}

DEFTMETHOD(ExecuterEditGroup_Cancel)
{
    struct ExecuterEditGroupData *data = INST_DATA (cl, obj);
    reset (data);
    APTR window = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_MainWindow);
    DoMethod (window, MM_ExecuterMainWindow_ToggleMode, NULL);
    return 0;
}

BEGINMTABLE2(executereditgroupclass)
DECNEW(ExecuterEditGroup)
DECGET(ExecuterEditGroup)
DECSET(ExecuterEditGroup)
DECTMETHOD(ExecuterEditGroup_Ok)
DECTMETHOD(ExecuterEditGroup_Cancel)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Group, executereditgroupclass, ExecuterEditGroupData)
