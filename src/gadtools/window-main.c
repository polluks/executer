#include <stdio.h>
#include <string.h>

#include <exec/exec.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include "window-main.h"
#include "window-edit.h"
#include "../notify.h"
#include "../prefs.h"

#define WINDOW_TITLE "Executer"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 160

/* common */
#define BUTTON_HEIGHT 14
/* file selectors */
#define FILE_BUTTON_WIDTH 48
#define ACTION_CYCLE_WIDTH 80
#define ADD_BUTTON_WIDTH 52
/* Save, Use, Cancel */
#define BUTTON_WIDTH 72

enum {
    GAD_ID_LIST = 0,
    GAD_ID_ADD_BUTTON,
    GAD_ID_EDIT_BUTTON,
    GAD_ID_REMOVE_BUTTON,
    GAD_ID_SAVE,
    GAD_ID_USE,
    GAD_ID_CANCEL,
    GAD_ID_LAST
};

struct gadget_item {
    struct Node node;
    char line[80];
    int index;
    struct notify_item *item;
};
static int _current_index = -1;

static BOOL _visible = FALSE;
static ULONG _signal = 0;

static struct TextAttr *_textattr = NULL;
static UWORD _topborder = 0;
static void *_visualinfo = NULL;

static struct Window *_window = NULL;
static struct Gadget *_glist = NULL;
static struct Gadget *_gads[GAD_ID_LAST] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static int _count = 0;
static struct List *_list = NULL;

static BOOL _handle_gadget_event (struct Gadget *gad, UWORD code);
static struct Window *_open_window (void);
static int _create_gadgets (void);

static struct gadget_item *_item_at_index (int index);
static int _remove_index_from_list (int index);
static int _clear_list (void);

int window_main_init (struct TextAttr *textattr, void *visualinfo, UWORD topborder)
{
    _textattr = textattr;
    _visualinfo = visualinfo;
    _topborder = topborder;
        
    _list = AllocMem (sizeof(struct List), MEMF_ANY|MEMF_CLEAR);
    if (_list == NULL) {
        fprintf (stderr, "Could not alloc list for list gadget.\n");
        return 1;
    }
    NewList (_list);

    return 0;
}

void window_main_free (void)
{
    _signal = 0;
    if (window_main_visibility (FALSE) != 0) {
        return;
    }
    if (_list != NULL) {
        _clear_list ();
        FreeMem (_list, sizeof (struct List));
    }
    _list = NULL;
}

ULONG window_main_signal (void)
{
    return _signal;
}

int window_main_visibility (BOOL visible)
{
    if (_visible == visible) return 0;

    _visible = visible;
    if (_visible) {
        if (_create_gadgets () != 0) {
            fprintf (stderr, "Could not create list gadgets\n");
            window_main_free ();
            return 1;
        }
        _window = _open_window ();
        if (_window == NULL) {
            _signal = 0;
            return 1;
        }
    } else {
        if (_window != NULL) {
            CloseWindow (_window);
            _window = NULL;
        }
        FreeGadgets (_glist);
        _glist = NULL;
        _signal = 0;
    }
    return 0;
}

BOOL window_main_is_visible (void)
{
    return _visible;
}

void window_main_dispose (BOOL *quit)
{
    struct IntuiMessage *imsg;
    ULONG imsgClass;
    UWORD imsgCode;
    struct Gadget *gad;

    BOOL v = _visible;
    if (_window == NULL) return;
    while ((imsg = GT_GetIMsg (_window->UserPort)) != NULL) {
        gad = (struct Gadget *)imsg->IAddress;

        imsgClass = imsg->Class;
        imsgCode = imsg->Code;

        GT_ReplyIMsg (imsg);

        switch (imsgClass) {
            /* case IDCMP_GADGETDOWN: */
            case IDCMP_MOUSEMOVE:
            case IDCMP_GADGETUP:
                v = _handle_gadget_event (gad, imsgCode);
                break;
            case IDCMP_VANILLAKEY:
                /*_handle_vanilla_key (imsgCode);*/
                break;
            case IDCMP_CLOSEWINDOW:
                v = FALSE;
                break;
            case IDCMP_REFRESHWINDOW:
                GT_BeginRefresh (_window);
                GT_EndRefresh (_window, TRUE);
                break;
        }
    }

    if (window_main_visibility (v) != 0) {
        fprintf (stderr, "Window change visibility failed. Quiting...\n");
        *quit = TRUE;
    }
}

static BOOL _handle_gadget_event (struct Gadget *gad, UWORD code)
{
    struct gadget_item *item = NULL;
    BOOL v = _visible;

    fprintf (stderr, "main gadget id: :%lu\n", (unsigned long int)gad->GadgetID);
    switch (gad->GadgetID)
    {
    case GAD_ID_LIST: {
        _current_index = (int)code;
        fprintf (stderr, "List code aka index: %d\n", (int)code);
    }
    break;
    case GAD_ID_REMOVE_BUTTON: {
        _remove_index_from_list (_current_index);
        notify_remove_index_from_list (_current_index);
    }
    break;
    case GAD_ID_EDIT_BUTTON: {
        if (_current_index >= 0 && _current_index < _count) {
            item = _item_at_index (_current_index);
            window_edit_edit (_current_index, item->item);
        }
    }
    break;
    case GAD_ID_ADD_BUTTON: {
        window_edit_add (_count);
    }
    break;
    case GAD_ID_SAVE: {
        prefs_save_envarc ();
    }
    case GAD_ID_USE: {
        prefs_save_env ();
    }
    case GAD_ID_CANCEL: {
        v = FALSE;
    }
    break;
    default:
    break;
    }
    return v;
}

static struct Window *_open_window (void)
{
    struct Window *w = (struct Window *)OpenWindowTags (NULL,
        WA_Title, (ULONG)WINDOW_TITLE,
        WA_Gadgets, _glist,
        WA_AutoAdjust, TRUE,
        WA_Width, WINDOW_WIDTH,
        WA_Height, WINDOW_HEIGHT,
        WA_MinWidth, WINDOW_WIDTH,
        WA_MinHeight, WINDOW_HEIGHT,
        WA_DepthGadget, TRUE,
        WA_CloseGadget, TRUE,
        WA_DragBar, TRUE,
        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_VANILLAKEY |
            STRINGIDCMP | BUTTONIDCMP | CYCLEIDCMP,
        TAG_END);
    if (w != NULL) {
        GT_RefreshWindow (w, NULL);
        _signal = 1L << w->UserPort->mp_SigBit;
    } else {
        fprintf (stderr, "Could not open window.\n");
    }
    SetAPen (w->RPort, 0);
    SetBPen (w->RPort, 1);

    return w;
}

static int _create_gadgets (void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    UWORD top = _topborder;

    gad = CreateContext (&_glist);

    ng.ng_TextAttr   = _textattr;
    ng.ng_VisualInfo = _visualinfo;

    /* list */
    ng.ng_TopEdge   = top;
    ng.ng_LeftEdge   = 16;
     
    ng.ng_Width      = WINDOW_WIDTH - 16 - 16;
    ng.ng_Height     = WINDOW_HEIGHT - BUTTON_HEIGHT - 4 - BUTTON_HEIGHT - 4 - top - 4;
    ng.ng_GadgetID   = GAD_ID_LIST;
    gad = CreateGadget (LISTVIEW_KIND, gad, &ng,
                    GTLV_ShowSelected, NULL,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_LIST] = gad;

    /* list actions */    
    ng.ng_TopEdge    = ng.ng_TopEdge + ng.ng_Height;
    ng.ng_Width      = BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = (UBYTE *)"_Add";
    ng.ng_GadgetID   = GAD_ID_ADD_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ADD_BUTTON] = gad;
    
    ng.ng_GadgetText = (UBYTE *)"_Edit";
    ng.ng_LeftEdge   = WINDOW_WIDTH/2 - BUTTON_WIDTH/2;
    ng.ng_GadgetID   = GAD_ID_EDIT_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_EDIT_BUTTON] = gad;

    ng.ng_GadgetText = (UBYTE *)"_Remove";
    ng.ng_LeftEdge   = WINDOW_WIDTH - BUTTON_WIDTH - 16;
    ng.ng_GadgetID   = GAD_ID_REMOVE_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_REMOVE_BUTTON] = gad;

    /* Bottom part */ 
    ng.ng_LeftEdge   = 16;
    ng.ng_TopEdge    = WINDOW_HEIGHT - 20;
    ng.ng_Width      = BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = (UBYTE *)"_Save";
    ng.ng_GadgetID   = GAD_ID_SAVE;
    ng.ng_Flags      = 0;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_SAVE] = gad;

    ng.ng_LeftEdge   = WINDOW_WIDTH/2 - BUTTON_WIDTH/2;
    ng.ng_GadgetText = (UBYTE *)"_Use";
    ng.ng_GadgetID   = GAD_ID_USE;
    ng.ng_Flags      = 0;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_USE] = gad;
    
    ng.ng_LeftEdge   = WINDOW_WIDTH - BUTTON_WIDTH - 16;
    ng.ng_GadgetText = (UBYTE *)"_Cancel";
    ng.ng_GadgetID   = GAD_ID_CANCEL;
    ng.ng_Flags      = 0;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_CANCEL] = gad;

    return 0;
}

static struct gadget_item *_item_at_index (int index)
{
    int cur = 0;
    struct gadget_item *item, *next;

    if (IsListEmpty (_list)) {
        return NULL;
    }
    if (index < 0 || index > _count-1) {
        return NULL;
    }

    item = (struct gadget_item *)_list->lh_Head;
    while ((next = (struct gadget_item *)item->node.ln_Succ) != NULL) {
        if (index == cur++) {
            return item;
        }
        item = next;
    }
    return NULL;
}

static int _remove_index_from_list (int index)
{
    struct gadget_item *item = _item_at_index (index);
    if (item == NULL) {
        return 1;
    }

    GT_SetGadgetAttrs (_gads[GAD_ID_LIST], _window, NULL, GTLV_Labels, "~0");
    Remove ((struct Node *) item);
    _count--;
    if (_count > 0) {
        index--;
        if (index < 0) index = 0;
        GT_SetGadgetAttrs (_gads[GAD_ID_LIST], _window, NULL, GTLV_Selected, index);
        _current_index = index;
        GT_SetGadgetAttrs (_gads[GAD_ID_LIST], _window, NULL, GTLV_Labels, _list);
        GT_SetGadgetAttrs (_gads[GAD_ID_EDIT_BUTTON], _window, NULL, GA_Disabled, FALSE);
        GT_SetGadgetAttrs (_gads[GAD_ID_REMOVE_BUTTON], _window, NULL, GA_Disabled, FALSE);
    } else {
        GT_SetGadgetAttrs (_gads[GAD_ID_EDIT_BUTTON], _window, NULL, GA_Disabled, TRUE);
        GT_SetGadgetAttrs (_gads[GAD_ID_REMOVE_BUTTON], _window, NULL, GA_Disabled, TRUE);
    }
    return 0;
}


static int _clear_list (void)
{
    struct gadget_item *item, *next;

    if (_visible == TRUE) GT_SetGadgetAttrs (_gads[GAD_ID_LIST], _window, NULL, GTLV_Labels, "~0");
    _current_index = -1;

    _count = 0;
    if (IsListEmpty (_list)) {
        return 0; /* already empty */
    }

    item = (struct gadget_item *)_list->lh_Head;
    while ((next = (struct gadget_item *)item->node.ln_Succ) != NULL) {
        Remove ((struct Node *) item);
        FreeMem (item, sizeof (struct gadget_item));
        item = next;
    }

    return 0;
}

int window_main_setup_list (struct List *nlist)
{
    int ret = 0;
    int i = 0;
    struct notify_item *nitem, *nnext;
    struct gadget_item *gitem;
    size_t pos = 0;
    char *file_part;

    _clear_list ();

    nitem = (struct notify_item *)nlist->lh_Head;
    while ((nnext = (struct notify_item *)nitem->node.ln_Succ) != NULL) {
        if (nitem->cb != NULL) { /* skip internals */
            nitem = nnext;
            continue;
        }
#if 1
        file_part = nitem->path;
        pos = strlen (file_part);
#else
        file_part = (char *)FilePart ((STRPTR)nitem->path);
        pos = strlen (file_part);
        if (pos == 0) {
            file_part = nitem->path;
            pos = strlen (file_part);
        }
#endif
        if (pos == 0) {
            nitem = nnext;
            continue;
        }
        gitem = AllocMem (sizeof (struct gadget_item), MEMF_ANY|MEMF_CLEAR);
        if (gitem == NULL) {
            ret = 1;
            break;
        }
        if (pos > 78) pos = 78;
        CopyMem (file_part, gitem->line, pos++);
        gitem->line[pos] = '\0';
        gitem->item = nitem;
        gitem->index = i++;
        gitem->node.ln_Name = gitem->line;

        AddTail (_list, (struct Node *)gitem);

        nitem = nnext;
    }
    _count = i;
    GT_SetGadgetAttrs (_gads[GAD_ID_LIST], _window, NULL, GTLV_Labels, _list);
    if (_count > 0) {
        GT_SetGadgetAttrs (_gads[GAD_ID_LIST], _window, NULL, GTLV_Selected, 0);
        _current_index = 0;
        GT_SetGadgetAttrs (_gads[GAD_ID_EDIT_BUTTON], _window, NULL, GA_Disabled, FALSE);
        GT_SetGadgetAttrs (_gads[GAD_ID_REMOVE_BUTTON], _window, NULL, GA_Disabled, FALSE);
    } else {
        GT_SetGadgetAttrs (_gads[GAD_ID_EDIT_BUTTON], _window, NULL, GA_Disabled, TRUE);
        GT_SetGadgetAttrs (_gads[GAD_ID_REMOVE_BUTTON], _window, NULL, GA_Disabled, TRUE);
    }
    fprintf (stderr, "window_main_setup_list() count:%d ret: %d\n", _count, ret);
    return ret;
}
