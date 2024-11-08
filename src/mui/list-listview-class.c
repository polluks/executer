#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libraries/mui.h>

#include <proto/muimaster.h>
#include <proto/alib.h> /* DoMetmod */
#include <proto/exec.h> /* Wait */
#include <proto/intuition.h> /* NewObject */
#include <proto/gadtools.h> /* NewMenu */

#include "vapor.h" /* Class macros */

#include "common.h"
#include "objects.h"

/* classes */
#include "classes.h"

#include "m68k.h"

/* FIXME: TODO: Check leaks carefully this is one of the component where this can leak a lot */

struct ExecuterListviewData
{
    APTR LI_list;
    APTR ME_contextmenu;
    BOOL menuenabled;
    struct MUI_EventHandlerNode ehnode;
};

enum {
    CTXMEN_PLAYLIST_ADDSELECTED = 100,
    CTXMEN_PLAYLIST_REPLACESELECTED,
    CTXMEN_PLAYLIST_ADDURL,
};

static struct NewMenu ListviewContextMenuNM[] =
{
    {NM_TITLE, (STRPTR)"Listview", (STRPTR)0, 0, 0, (APTR)0},
    {NM_ITEM, (STRPTR)"Add selected",  (STRPTR)"A", 0, 0, (APTR)CTXMEN_PLAYLIST_ADDSELECTED},
    {NM_ITEM, (STRPTR)"Replace with selected",  (STRPTR)"R", 0, 0, (APTR)CTXMEN_PLAYLIST_REPLACESELECTED},
    {NM_ITEM, (STRPTR)"Add URL",  (STRPTR)"U", 0, 0, (APTR)CTXMEN_PLAYLIST_ADDURL},
    {NM_END, (STRPTR)NULL, (STRPTR)0, 0, 0, (APTR)0}
};

static Object *ExecuterListviewContextMenu (BOOL enabled)
{
    int i;

    /* FIXME: JVJV: is there any better way to disable menu in this case ??? */
    for (i = 1; i < (int)sizeof(ListviewContextMenuNM)/sizeof(struct NewMenu)-1; i++) {
        struct NewMenu *nmitem = &ListviewContextMenuNM[i];
        if (enabled == FALSE) {
             nmitem->nm_Flags = NM_ITEMDISABLED;
        } else {
             nmitem->nm_Flags = 0;
        }
    }

    return MUI_MakeObject (MUIO_MenustripNM, ListviewContextMenuNM, MUIA_ContextMenu, TRUE);
}

/* new */
DEFNEW(ExecuterListview)
{
    APTR LI_list;

    obj = DoSuperNew(cl, obj, 
        MUIA_UserData, MO_Executer_List_Listview,
        MUIA_Listview_Input, TRUE,
        MUIA_Listview_MultiSelect, MUIV_Listview_MultiSelect_Default,
        MUIA_Listview_List, LI_list = NewObject(getexecuterlistclass(), NULL, TAG_DONE),
        MUIA_ContextMenu, TRUE,
        TAG_MORE, (((struct opSet *)msg)->ops_AttrList),
        End;
        
    if (obj) {
        struct ExecuterListviewData *data = INST_DATA (cl, obj);

        data->LI_list = LI_list;
        data->ME_contextmenu = NULL;
        data->menuenabled = FALSE;

        DoMethod (data->LI_list, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime, obj, 1, MM_ExecuterListview_DoubleClick);
        //DoMethod (data->LI_list, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom); 
    }

    return (ULONG)obj;
}

/* dispose */
DEFDISP(ExecuterListview)
{
    struct ExecuterListviewData *data = INST_DATA(cl, obj);

    if (data != NULL && data->ME_contextmenu != NULL) {
        MUI_DisposeObject (data->ME_contextmenu);
    }

    return DOSUPER;
}

/* setup */
DEFMMETHOD(Setup)
{
    struct ExecuterListviewData *data = INST_DATA (cl, obj);

    if (!DOSUPER) {
        return FALSE;
    }

    data->ehnode.ehn_Object = obj;
    data->ehnode.ehn_Class = cl;
    data->ehnode.ehn_Events = IDCMP_RAWKEY;

    DoMethod (_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);

    return TRUE;
};

DEFMMETHOD(Cleanup)
{
    struct ExecuterListviewData *data = INST_DATA (cl, obj);

    DoMethod (_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);

    return DOSUPER;
};

DEFMMETHOD(HandleEvent)
{
    struct ExecuterListviewData *data = INST_DATA (cl, obj);

    if (data->menuenabled == FALSE) {
        return DOSUPER;
    }


    if (msg->imsg) {
        UWORD code = msg->imsg->Code;
        ULONG class = msg->imsg->Class;
        UWORD qualifier = msg->imsg->Qualifier;

        switch (class) {
        case IDCMP_RAWKEY:
            if (qualifier & IEQUALIFIER_LCOMMAND) {
               if (0x20 == code) { /* a */
                   //DoMethod (data->LI_list, MM_ExecuterList_AddSelected);
               } else if (0x13 == code) { /* r */
                   //DoMethod (obj, MM_ExecuterListview_ClearQueueAndAddSelected);
               } else {
                   //D(BUG((CONST_STRPTR)"%s: IEQUALIFIER_LCOMMAND qualifier: %x code: %x\n", __FUNCTION__, qualifier, code));
               }
            }
        break;
        default:
        break;
        }
    }

    return DOSUPER;
}

/* get */
DEFGET(ExecuterListview)
{
    struct ExecuterListviewData *data = INST_DATA(cl, obj);

    switch (msg->opg_AttrID) {
        case MA_Executer_ContextMenuEnabled: {
            *msg->opg_Storage = (LONG)data->menuenabled;
        }
        default:
            break;
    }

    return DOSUPER;
}

static void doset(APTR obj, struct ExecuterListviewData *data, struct TagItem *tags)
{
    FORTAG(tags)
    {
        case MA_Executer_ContextMenuEnabled:
            data->menuenabled = (BOOL)tag->ti_Data;
            break;
    }
    NEXTTAG
}

/* set */
DEFSET(ExecuterListview)
{
    struct ExecuterListviewData *data = INST_DATA(cl, obj);
    doset (obj, data, INITTAGS);
    return DOSUPER;
};

DEFMMETHOD(ContextMenuBuild)
{
    struct ExecuterListviewData *data = INST_DATA(cl, obj);

    if(data->ME_contextmenu != NULL) {
        MUI_DisposeObject (data->ME_contextmenu);
        data->ME_contextmenu = NULL;
    }

    data->ME_contextmenu = ExecuterListviewContextMenu (data->menuenabled);

    return (IPTR)data->ME_contextmenu;
}

DEFMMETHOD(ContextMenuChoice)
{
    struct MUIP_ContextMenuChoice *m;
    struct ExecuterListviewData *data = INST_DATA (cl, obj);
    IPTR user_data=0;
    
    if (data == NULL || data->menuenabled == FALSE) {
        return 0;
    }

    m = (struct MUIP_ContextMenuChoice *)msg;
    if (m == NULL) {
        return 0;
    }

    GetAttr (MUIA_UserData, m->item, (ULONG *)&user_data);

    switch (user_data) {
        case CTXMEN_PLAYLIST_ADDSELECTED:
            //DoMethod (data->LI_list, MM_ExecuterListview_AddSelected);
            break;
        case CTXMEN_PLAYLIST_ADDURL:
            break;
        case CTXMEN_PLAYLIST_REPLACESELECTED:
            //DoMethod (obj, MM_ExecuterListview_ClearQueueAndAddSelected);
            break;
        default:
            break;
    }

    return 0;
}

/* Own methods */
DEFSMETHOD(ExecuterListview_Add)
{
#if 0
    struct ExecuterListviewData *data = INST_DATA (cl, obj);
    //struct ExecuterListItem *item = NULL;
    const char *item = "TST";

    if (data == NULL || data->LI_list == NULL) {
        return 0;
    }

    //item = (struct ExecuterLibraryInfo *)msg->item;

    DoMethod (data->LI_list,
        MUIM_List_InsertSingle, item,
        MUIV_List_Insert_Bottom);
#endif
    return 0;
}

DEFTMETHOD(ExecuterListview_Clear)
{
    struct ExecuterListviewData *data = INST_DATA (cl, obj);

    DoMethod (data->LI_list, MUIM_List_Clear);

    return 0;
}

DEFTMETHOD(ExecuterListview_Sort)
{
    struct ExecuterListviewData *data = INST_DATA (cl, obj);

    DoMethod (data->LI_list, MUIM_List_Sort);

    return 0;
}

DEFTMETHOD(ExecuterListview_DoubleClick)
{
    struct ExecuterListviewData *data = INST_DATA (cl, obj);

    DoMethod (data->LI_list, MM_ExecuterList_DoubleClick);

    return 0;
}

#if 0
static BOOL _is_something_selected (APTR list)
{
    LONG id = MUIV_List_NextSelected_Start;
    BOOL retval = FALSE;

    DoMethod (list, MUIM_List_NextSelected, &id);
    if (id != MUIV_List_NextSelected_End) {
        retval = TRUE;
    }

    return retval;
}
#endif

BEGINMTABLE2(executerlistviewclass)
DECMMETHOD(Setup)
DECMMETHOD(Cleanup)
DECMMETHOD(HandleEvent)
DECNEW(ExecuterListview)
DECGET(ExecuterListview)
DECSET(ExecuterListview)
DECDISP(ExecuterListview)
DECMMETHOD(ContextMenuBuild)
DECMMETHOD(ContextMenuChoice)
DECSMETHOD(ExecuterListview_Add)
DECTMETHOD(ExecuterListview_Clear)
DECTMETHOD(ExecuterListview_Sort)
DECTMETHOD(ExecuterListview_DoubleClick)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Listview, executerlistviewclass, ExecuterListviewData)
