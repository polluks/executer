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

struct ExecuterListEntry
{
    struct notify_item *item;
    int index;
    char line[80];
};

struct ExecuterListData
{
};

#ifndef __MORPHOS__
DEFHOOKFUNC2(APTR, List_Construct, APTR pool, struct MP_ExecuterListview_Add  *data)
{
    struct ExecuterListEntry *nentry = (struct ExecuterListEntry *)AllocPooled (pool, sizeof (struct ExecuterListEntry));
    size_t pos = 0;
    char *file_part;

    nentry->item = data->item;
    nentry->index = data->index;

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
    CopyMem (file_part, nentry->line, pos);
    nentry->line[pos] = '\0';

    return (APTR)nentry;
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
    LONG result = 0;
    //struct ExecuterListData *data = e1->data;

#if 0
    if (e1->info->pos < e2->info->pos) {
        result = 1;
    } else if (e1->info->pos > e2->info->pos) {
        result = -1; 
    }
#endif

    //result *= data->order;

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
            fprintf (stderr, "remove - %s\n", e->item->path);
            //notifier_remove_index (e->index);
        }
    }

    DoMethod (obj, MUIM_List_Remove, MUIV_List_Remove_Active);
    DoMethod (obj, MUIM_List_Remove, MUIV_List_Remove_Selected);

    return 0;
}

DEFTMETHOD(ExecuterList_EditSelected)
{
    struct ExecuterListEntry *e = NULL;
    LONG pos = -1;

    get (obj, MUIA_List_Active, &pos);
    if (pos != MUIV_List_Active_Off) {
	DoMethod (obj, MUIM_List_GetEntry, pos, &e);
        if (e != NULL && e->item != NULL) {
            //APTR window = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_MainWindow);
            //DoMethod (window, MM_ExecuterMainWindow_ToggleMode, e->item);
            fprintf (stderr, "Edit - %s\n", e->item->path);
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

#ifdef __MORPHOS__
DEFMMETHOD(List_Construct)
{
    struct ExecuterListEntry *nentry = (struct ExecuterListEntry *)calloc (sizeof (struct ExecuterListEntry), 1);

    nentry->line = "test";

    return (ULONG)nentry;
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
    struct ExecuterListEntry *e1 = (struct ExecuterListEntry *)msg->entry1;
    struct ExecuterListEntry *e2 = (struct ExecuterListEntry *)msg->entry2;
#endif
    LONG result = 0;

#if 0
    if (e1->info->pos > e2->info->pos) {
        result = 1;
    } else if (e1->info->pos < e2->info->pos) {
        result = -1; 
    }

    result *= data->order;
#endif

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
#ifdef __MORPHOS__
DECMMETHOD(List_Construct)
DECMMETHOD(List_Destruct)
DECMMETHOD(List_Display)
DECMMETHOD(List_Compare)
#endif
ENDMTABLE

DECSUBCLASS_NC(MUIC_List, executerlistclass, ExecuterListData)
