#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libraries/mui.h>

#include <proto/muimaster.h>
#include <proto/alib.h> /* DoMetmod */
#include <proto/exec.h> /* Wait */
#include <proto/intuition.h> /* NewObject */
#include <proto/dos.h> /* SplitName */

#include "vapor.h" /* Class macros */

#include "common.h"
#include "objects.h"
#include "classes.h"

#include "m68k.h"
#include "../notify.h"
#include "../debug.h"

struct ExecuterListEntry
{
    struct notify_item *item;
    int index;
    char line[80];
};

struct ExecuterListData
{
};

static BOOL _build_entry (struct MP_ExecuterListview_Add *data, struct ExecuterListEntry *entry)
{
    size_t pos = 0;
    char *file_part;

    if (entry == NULL || data == NULL) return FALSE;

    entry->item = data->item;
    entry->index = data->index;

#if 1
    file_part = data->item->path;
    pos = strlen (file_part);
#else
    file_part = (char *)FilePart ((STRPTR)data->item->path);
    pos = strlen (file_part);
    if (pos == 0) {
        file_part = nitem->path;
        pos = strlen (file_part);
    }
#endif
    if (pos > 79) {
        pos = 79;
    }
    CopyMem (file_part, entry->line, pos);
    entry->line[pos] = '\0';
    return TRUE;
}

#ifndef __MORPHOS__
DEFHOOKFUNC2(APTR, List_Construct, APTR pool, struct MP_ExecuterListview_Add  *data)
{
    struct ExecuterListEntry *entry = (struct ExecuterListEntry *)AllocPooled (pool, sizeof (struct ExecuterListEntry));

    if (_build_entry (data, entry) == FALSE) {
        if (entry != NULL) FreePooled (pool, entry, sizeof (struct ExecuterListEntry));
        return (APTR)0;
    }

    return (APTR)entry;
}

DEFHOOKFUNC2(void, List_Destruct, APTR pool, struct ExecuterListEntry *e)
{
    if (e == NULL) {
        return;
    }
    FreePooled (pool, e, sizeof (struct ExecuterListEntry));
}

DEFHOOKFUNC2(APTR, List_Display, char **array, struct ExecuterListEntry *e)
{
    *array = (unsigned char *)e->line;

    return 0;
}

DEFHOOKFUNC2(LONG, List_Compare, struct ExecuterListEntry *e1, struct ExecuterListEntry *e2)
{
    //struct ExecuterListData *data = e1->data;
    LONG result = 0;

    if (e1->index < e2->index) {
        result = 1;
    } else if (e1->index > e2->index) {
        result = -1; 
    }

    return result;
}
#endif

/* new */
DEFNEW(ExecuterList)
{
#ifdef __MORPHOS__
    obj = DoSuperNew(cl, obj,
#else // M68K
    DEFHOOK(List_Construct);
    DEFHOOK(List_Display);
    DEFHOOK(List_Destruct);
    DEFHOOK(List_Compare);
    obj = DoSuperNew(cl, obj,
        MUIA_List_ConstructHook, &List_Construct_hook,
        MUIA_List_DisplayHook, &List_Display_hook,
        MUIA_List_DestructHook, &List_Destruct_hook,
        MUIA_List_CompareHook, &List_Compare_hook,
#endif
        MUIA_UserData, MO_Executer_List,
        MUIA_CycleChain, 1,
        MUIA_Dropable, FALSE,
        MUIA_List_DragSortable, FALSE,
        TAG_MORE, (((struct opSet *)msg)->ops_AttrList),
        TAG_END);

#if 0
    if (obj) {
        struct ExecuterListData *data = INST_DATA (cl, obj);
    }
#endif

    return (ULONG)obj;
}

/* dispose */
DEFDISP(ExecuterList)
{
    return DOSUPER;
}

/* get */
DEFGET(ExecuterList)
{
    return DOSUPER;
}

/* set */
DEFSET(ExecuterList)
{
    return DOSUPER;
}


DEFTMETHOD(ExecuterList_RemoveSelected)
{
    struct ExecuterListEntry *e = NULL;
    LONG pos = MUIV_List_NextSelected_Start;

    while (1) {
        DoMethod (obj, MUIM_List_NextSelected, &pos);
        if (pos == MUIV_List_NextSelected_End) {
            break;
        }

        DoMethod (obj, MUIM_List_GetEntry, pos, &e);

        if (e != NULL && e->item != NULL) {
            D(BUG("remove[%d] - %s\n", e->index, e->item->path));
            if (notify_remove_item_from_list (e->item) != 0) {
               fprintf (stderr, "Could not remove notify item.\n");
            }
        }
    }

//    DoMethod (obj, MUIM_List_Remove, MUIV_List_Remove_Active);
//    DoMethod (obj, MUIM_List_Remove, MUIV_List_Remove_Selected);
    DoMethod (obj, MM_ExecuterList_Update);

    return 0;
}

DEFTMETHOD(ExecuterList_EditSelected)
{
    ULONG tmp = 0;
    struct ExecuterListEntry *e = NULL;
    LONG pos = MUIV_List_Active_Off;
    APTR window;
    APTR edit;

    get (obj, MUIA_List_Active, &pos);
    if (pos != MUIV_List_Active_Off) {
	DoMethod (obj, MUIM_List_GetEntry, pos, &tmp);
        e = (struct ExecuterListEntry *)tmp;
        if (e != NULL && e->item != NULL) {
            D(BUG("Edit %p - %s\n", e->item, e->item->path));
            edit = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_Edit_Group);
            set (edit, MA_Executer_EditItem, (ULONG)e->item);
            window = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_MainWindow);
            DoMethod (window, MM_ExecuterMainWindow_ToggleMode);
        }
    }
    return 0;
}

DEFTMETHOD(ExecuterList_DoubleClick)
{
    DoMethod (obj, MM_ExecuterList_EditSelected);
    return 0;
}

DEFTMETHOD(ExecuterList_Clear)
{
    DoMethod (obj, MUIM_List_Clear);
    return 0;
}

DEFTMETHOD(ExecuterList_Update)
{
    int i = 0;
    int j = 0;
    struct notify_item *nitem, *nnext;
    struct List *nlist = notify_list();

    DoMethod (obj, MUIM_List_Clear);

    if (nlist == NULL) return 0;

    nitem = (struct notify_item *)nlist->lh_Head;
    while ((nnext = (struct notify_item *)nitem->node.ln_Succ) != NULL) {
        if (nitem->cb != NULL) { /* skip internals */
            nitem = nnext;
            i++;
            continue;
        }

        {
            struct MP_ExecuterListview_Add item;
            item.item = nitem;
            item.index = i++;
            DoMethod (obj, MUIM_List_InsertSingle, &item, MUIV_List_Insert_Bottom);
            j++;
        }
        nitem = nnext;
    }

    if (j>0) { 
        set (obj, MUIA_List_Active, MUIV_List_Active_Top);
        DoMethod(obj, MUIM_List_Select, MUIV_List_Select_Active, MUIV_List_Select_Toggle, NULL);
    }
    return 0;
}


#ifdef __MORPHOS__
DEFMMETHOD(List_Construct)
{
    struct MP_ExecuterListview_Add *data = (struct MP_ExecuterListview_Add *) msg->entry;
    struct ExecuterListEntry *entry = (struct ExecuterListEntry *)calloc (sizeof (struct ExecuterListEntry), 1);

    if (_build_entry (data, entry) == FALSE) {
        if (entry != NULL) free (entry);
        return (ULONG)0;
    }

    return (ULONG)entry;
}

DEFMMETHOD(List_Destruct)
{
    struct ExecuterListEntry *data = (struct ExecuterListEntry *)msg->entry;

    if (data == NULL) {
        return 0;
    }

    free (data);

    return 0;
}

DEFMMETHOD(List_Display)
{
    struct ExecuterListEntry *e = (struct ExecuterListEntry *)msg->entry;

    msg->array[0] = (char *)e->line;

    return 0;
}

DEFMMETHOD(List_Compare)
{
#if 0
    struct ExecuterListData *data = INST_DATA(cl, obj);
#endif
    struct ExecuterListEntry *e1 = (struct ExecuterListEntry *)msg->entry1;
    struct ExecuterListEntry *e2 = (struct ExecuterListEntry *)msg->entry2;
    LONG result = 0;

    if (e1->index < e2->index) {
        result = 1;
    } else if (e1->index > e2->index) {
        result = -1; 
    }

    return result;
}
#endif


BEGINMTABLE2(executerlistclass)
DECNEW(ExecuterList)
DECDISP(ExecuterList)
DECGET(ExecuterList)
DECSET(ExecuterList)
DECTMETHOD(ExecuterList_RemoveSelected)
DECTMETHOD(ExecuterList_EditSelected)
DECTMETHOD(ExecuterList_DoubleClick)
DECTMETHOD(ExecuterList_Clear)
DECTMETHOD(ExecuterList_Update)
#ifdef __MORPHOS__
DECMMETHOD(List_Construct)
DECMMETHOD(List_Destruct)
DECMMETHOD(List_Display)
DECMMETHOD(List_Compare)
#endif
ENDMTABLE

DECSUBCLASS_NC(MUIC_List, executerlistclass, ExecuterListData)
