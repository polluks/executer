#include <stdio.h>
#include <string.h>

#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>

#include "window-edit.h"
#include "../notify.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 116

/* common */
#define BUTTON_HEIGHT 16
/* file selectors */
#define REQUESTER_BUTTON_WIDTH 52
/* Ok Cancel */
#define BUTTON_WIDTH 72

typedef enum {
    FR_TYPE_FILE,
    FR_TYPE_CMD
} FR_TYPE;

enum {
    GAD_ID_PATH_STRING,
    GAD_ID_REQUESTER_BUTTON,
    GAD_ID_ACTION_CREATE_CHECK,
    GAD_ID_ACTION_DELETE_CHECK,
    GAD_ID_ACTION_MODIFY_CHECK,
    GAD_ID_ACTION_CREATE_TEXT,
    GAD_ID_ACTION_DELETE_TEXT,
    GAD_ID_ACTION_MODIFY_TEXT,
    GAD_ID_CMD_STRING,
    GAD_ID_CMD_BUTTON,
    GAD_ID_OK,
    GAD_ID_CANCEL,
    GAD_ID_LAST
};

static BOOL _visible = FALSE;
static ULONG _signal = 0;

static struct TextAttr *_textattr = NULL;
static void *_visualinfo = NULL;
static UWORD _topborder = 0;

static struct Window *_window = NULL;
static struct Gadget *_glist = NULL;
static struct Gadget *_gads[GAD_ID_LAST];

#define TMP_SIZE 1024
static STRPTR _path_str = NULL;
static STRPTR _command_str = NULL;
static int _reason = 0;
static int _index = -1;
static char *_title = "";

static BOOL _handle_gadget_event (struct Gadget *gad, UWORD code);
static struct Window *_open_window (void);
static int _create_gadgets (void);

static void _open_filerequester (FR_TYPE type);

extern struct Library *GadToolsBase;

int window_edit_init (struct TextAttr *textattr, void *visualinfo, UWORD topborder)
{
    _textattr = textattr;
    _visualinfo = visualinfo;
    _topborder = topborder;
    if (_create_gadgets () != 0) {
        fprintf (stderr, "Could not create edit gadgets\n");
        window_edit_free ();
        return 1;
    }

    _path_str = (STRPTR)AllocMem (TMP_SIZE, MEMF_ANY|MEMF_CLEAR);
    if (_path_str == NULL) {
        fprintf (stderr, "Could not alloc mem to tmp file string\n");
        window_edit_free ();
        return 1;
    }
    _command_str = (STRPTR)AllocMem (TMP_SIZE, MEMF_ANY|MEMF_CLEAR);
    if (_command_str == NULL) {
        fprintf (stderr, "Could not alloc memory to tmp script string\n");
        window_edit_free ();
        return 1;
    }
    return 0;
}

void window_edit_free (void)
{
    _signal = 0;
    if (window_edit_visibility (FALSE) != 0) {
        return;
    }
    if (_path_str != NULL) FreeMem (_path_str, TMP_SIZE);
    if (_command_str != NULL) FreeMem (_command_str, TMP_SIZE);
}

ULONG window_edit_signal (void)
{
    return _signal;
}

int window_edit_visibility (BOOL visible)
{
    if (_visible == visible) return 0;

    _visible = visible;
    if (_visible) {
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
        _signal = 0;
    }
    return 0;
}

BOOL window_edit_is_visible (void)
{
    return _visible;
}

void window_edit_dispose (BOOL *quit)
{
    char *buf;
    struct IntuiMessage *imsg;
    ULONG imsgClass;
    UWORD imsgCode;
    struct Gadget *gad;
    BOOL v = _visible;
    fprintf (stderr, "edit dispose\n");

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

    if (v == FALSE) {
       buf = (char *)((struct StringInfo *)_gads[GAD_ID_PATH_STRING]->SpecialInfo)->Buffer;
       CopyMem (buf, _path_str, strlen (buf) + 1);
       buf = (char *)((struct StringInfo *)_gads[GAD_ID_CMD_STRING]->SpecialInfo)->Buffer;
       CopyMem (buf, _command_str, strlen (buf) + 1);
       fprintf (stderr, "path:%s, command:%s, reason:%s\n", _path_str, _command_str, notify_reason_bitfield_to_string (_reason) );
    }
    if (window_edit_visibility (v) != 0) {
        fprintf (stderr, "Window change visibility failed. Quiting...\n");
        *quit = TRUE;
    }
}

int window_edit_add (int index)
{
    _index = index;
    _path_str[0] = '\0';
    _command_str[0] = '\0';
    GT_SetGadgetAttrs (_gads[GAD_ID_PATH_STRING], _window, NULL, GTST_String, _path_str);
    GT_SetGadgetAttrs (_gads[GAD_ID_CMD_STRING], _window, NULL, GTST_String, _command_str);
    _reason = _reason | NOTIFY_REASON_MODIFY;
    GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_MODIFY_CHECK], _window, NULL, GTCB_Checked, TRUE);
    GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_CREATE_CHECK], _window, NULL, GTCB_Checked, FALSE);
    GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_DELETE_CHECK], _window, NULL, GTCB_Checked, FALSE);

    _title = "Add item";
    return window_edit_visibility (TRUE);
}

int window_edit_edit (int index, struct notify_item *item)
{
    _index = index;
    CopyMem (item->path, _path_str, strlen (item->path) + 1);
    GT_SetGadgetAttrs (_gads[GAD_ID_PATH_STRING], _window, NULL, GTST_String, _path_str);
    CopyMem (item->command, _command_str, strlen (item->command) + 1);
    GT_SetGadgetAttrs (_gads[GAD_ID_CMD_STRING], _window, NULL, GTST_String, _command_str);
    _reason = item->reason;
    if (item->reason & NOTIFY_REASON_CREATE) {
        GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_CREATE_CHECK], _window, NULL, GTCB_Checked, TRUE);
    } else {
        GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_CREATE_CHECK], _window, NULL, GTCB_Checked, FALSE);
    }
    if (item->reason & NOTIFY_REASON_DELETE) {
        GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_DELETE_CHECK], _window, NULL, GTCB_Checked, TRUE);
    } else {
        GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_DELETE_CHECK], _window, NULL, GTCB_Checked, FALSE);
    }
    if (item->reason & NOTIFY_REASON_MODIFY) {
        GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_MODIFY_CHECK], _window, NULL, GTCB_Checked, TRUE);
    } else {
        GT_SetGadgetAttrs (_gads[GAD_ID_ACTION_MODIFY_CHECK], _window, NULL, GTCB_Checked, FALSE);
    }

    /* FIXME: item->reason */
    _title = "Edit item";
    return window_edit_visibility (TRUE);
}


static BOOL _handle_gadget_event (struct Gadget *gad, UWORD code)
{
    BOOL v = _visible;

    fprintf (stderr, "edit gadget id: :%lu\n", (unsigned long int)gad->GadgetID);
    switch (gad->GadgetID)
    {
    case GAD_ID_PATH_STRING: {
    }
    break;
    case GAD_ID_REQUESTER_BUTTON: {
         _open_filerequester (FR_TYPE_FILE);
    }
    break;
    case GAD_ID_CMD_STRING: {
    }
    break;
    case GAD_ID_CMD_BUTTON: {
         _open_filerequester (FR_TYPE_CMD);
    }
    break;
    case GAD_ID_ACTION_CREATE_CHECK: {
        if (gad->Flags & GFLG_SELECTED) {
            _reason = _reason | NOTIFY_REASON_CREATE;
        } else {
            _reason = _reason & ~NOTIFY_REASON_CREATE;
        }
    }
    break;
    case GAD_ID_ACTION_DELETE_CHECK: {
        if (gad->Flags & GFLG_SELECTED) {
            _reason = _reason | NOTIFY_REASON_DELETE;
        } else {
            _reason = _reason & ~NOTIFY_REASON_DELETE;
        }
    }
    break;
    case GAD_ID_ACTION_MODIFY_CHECK: {
        if (gad->Flags & GFLG_SELECTED) {
            _reason = _reason | NOTIFY_REASON_MODIFY;
        } else {
            _reason = _reason & ~NOTIFY_REASON_MODIFY;
        }
    }
    break;
    case GAD_ID_OK: {
        v = FALSE;
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
        WA_Title, (ULONG)_title,
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
    UWORD top2 = 0;
    UWORD top3 = 0;

    gad = CreateContext (&_glist);

    ng.ng_TextAttr   = _textattr;
    ng.ng_VisualInfo = _visualinfo;

    /* 1st row - File */
    ng.ng_TopEdge    = top;
    ng.ng_LeftEdge   = 16;
    ng.ng_Width      = WINDOW_WIDTH - 32 - 4 - 8 - REQUESTER_BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = (UBYTE *)"";
    ng.ng_GadgetID   = GAD_ID_PATH_STRING;
    gad = CreateGadget (STRING_KIND, gad, &ng,
                    GTST_MaxChars, TMP_SIZE - 1,
                    GA_Immediate, TRUE,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_PATH_STRING] = gad;
    if (GadToolsBase->lib_Version == 37) {
        gad->Activation |= GACT_IMMEDIATE;
    }
 
    ng.ng_LeftEdge  += ng.ng_Width + 4;
    ng.ng_Width      = REQUESTER_BUTTON_WIDTH;
    ng.ng_GadgetText = (UBYTE *)"_Path";
    ng.ng_GadgetID   = GAD_ID_REQUESTER_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_REQUESTER_BUTTON] = gad;
    
    /* 2nd row - Action check boxes */
    ng.ng_TopEdge   += BUTTON_HEIGHT + 4 + 4;
    top2             = ng.ng_TopEdge;
    ng.ng_LeftEdge   = 16;
    ng.ng_Width      = 26;
    ng.ng_Height     = 11;
    ng.ng_GadgetText = NULL;
    ng.ng_GadgetID   = GAD_ID_ACTION_CREATE_CHECK;
    gad = CreateGadget (CHECKBOX_KIND, gad, &ng,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION_CREATE_CHECK] = gad;
    
    top3             = top2;
    ng.ng_TopEdge    = top3;
    ng.ng_LeftEdge  += ng.ng_Width + 4;
    ng.ng_Width      = 64;
    ng.ng_GadgetText = (UBYTE *)"Create";
    ng.ng_GadgetID   = GAD_ID_ACTION_CREATE_TEXT;
    gad = CreateGadget (TEXT_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION_CREATE_TEXT] = gad;
    
    ng.ng_TopEdge     = top2;
    ng.ng_LeftEdge   += ng.ng_Width + 8;
    ng.ng_GadgetText = NULL;
    ng.ng_Width      = 26;
    ng.ng_GadgetID   = GAD_ID_ACTION_DELETE_CHECK;
    gad = CreateGadget (CHECKBOX_KIND, gad, &ng,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION_DELETE_CHECK] = gad;
    
    ng.ng_TopEdge     = top3;
    ng.ng_LeftEdge   += ng.ng_Width + 4;
    ng.ng_Width      = 64;
    ng.ng_GadgetText = (UBYTE *)"Delete";
    ng.ng_GadgetID   = GAD_ID_ACTION_DELETE_TEXT;
    gad = CreateGadget (TEXT_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION_DELETE_TEXT] = gad;
    
    ng.ng_TopEdge     = top2;
    ng.ng_LeftEdge   += ng.ng_Width + 8;
    ng.ng_GadgetText = NULL;
    ng.ng_Width      = 26;
    ng.ng_GadgetID   = GAD_ID_ACTION_MODIFY_CHECK;
    gad = CreateGadget (CHECKBOX_KIND, gad, &ng,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION_MODIFY_CHECK] = gad;
    
    ng.ng_TopEdge     = top3;
    ng.ng_LeftEdge   += ng.ng_Width + 4;
    ng.ng_Width      = 64;
    ng.ng_GadgetText = (UBYTE *)"Modify";
    ng.ng_GadgetID   = GAD_ID_ACTION_MODIFY_TEXT;
    gad = CreateGadget (TEXT_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION_MODIFY_TEXT] = gad;
    
    /* 3th row - command */
    ng.ng_TopEdge   += BUTTON_HEIGHT + 4;
    ng.ng_LeftEdge   = 16;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = (UBYTE *)"";
    ng.ng_Width      = WINDOW_WIDTH - 32 - 4 - 8 - REQUESTER_BUTTON_WIDTH;
    ng.ng_GadgetID   = GAD_ID_CMD_STRING;
    gad = CreateGadget (STRING_KIND, gad, &ng,
                    GA_Immediate, TRUE,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_CMD_STRING] = gad;
    if (GadToolsBase->lib_Version == 37) {
        gad->Activation |= GACT_IMMEDIATE;
    }

    ng.ng_LeftEdge   += ng.ng_Width + 4;
    ng.ng_Width      = REQUESTER_BUTTON_WIDTH;
    ng.ng_GadgetText = (UBYTE *)"_Cmd";
    ng.ng_GadgetID   = GAD_ID_CMD_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    GTST_MaxChars, TMP_SIZE - 1,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_CMD_BUTTON] = gad;
    
    /* Bottom */
    ng.ng_LeftEdge   = 16;
    ng.ng_TopEdge    = WINDOW_HEIGHT - 20;
    ng.ng_Width      = BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = (UBYTE *)"_OK";
    ng.ng_GadgetID   = GAD_ID_OK;
    ng.ng_Flags      = 0;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_OK] = gad;

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

static void _open_filerequester (FR_TYPE type)
{
    struct FileRequester *fr;
    char *title;
    char *pattern;
    char *path;

    /* FIXME: paths */
    if (type == FR_TYPE_CMD) {
        title = "Select command";
        pattern = "#?";
        path = "S:";
    } else {  /* if (type == FR_TYPE_FILE) { */
        title = "Select FILE to inspect";
        pattern = "#?";
        path = "SYS:";
    }
    fr = (struct FileRequester *)AllocAslRequestTags(ASL_FileRequest,
                            ASL_Hail, (ULONG)title,
                            ASL_Dir,  (ULONG)path,
                            ASL_File, (ULONG)"",
                            ASL_Pattern, (ULONG)pattern,
                            ASL_FuncFlags, FILF_PATGAD,
                            ASL_Window, _window,
                            TAG_DONE);
    if (fr == NULL) {
        fprintf (stderr, "Could not open ASL filerequest\n");
        return;
    }
    if (AslRequest(fr, 0L)) {
        if (type == FR_TYPE_CMD) {
            CopyMem (fr->rf_Dir, _command_str, strlen ((char *)fr->rf_Dir) + 1);
            AddPart (_command_str, fr->rf_File, TMP_SIZE);
            GT_SetGadgetAttrs (_gads[GAD_ID_CMD_STRING], _window, NULL, GTST_String, _command_str);
        } else if (type == FR_TYPE_FILE) {
            CopyMem (fr->rf_Dir, _path_str, strlen ((char *)fr->rf_Dir) + 1);
            AddPart (_path_str, fr->rf_File, TMP_SIZE);
            GT_SetGadgetAttrs (_gads[GAD_ID_PATH_STRING], _window, NULL, GTST_String, _path_str);
        }
    }
    FreeAslRequest (fr);
}
