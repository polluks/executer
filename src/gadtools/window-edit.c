#include <stdio.h>

#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include "window-edit.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 160

/* common */
#define BUTTON_HEIGHT 14
/* file selectors */
#define FILE_BUTTON_WIDTH 48
#define ADD_BUTTON_WIDTH 52
/* Ok Cancel */
#define BUTTON_WIDTH 72

typedef enum {
    FR_TYPE_FILE,
    FR_TYPE_CMD
} FR_TYPE;

enum {
    GAD_ID_FILE_STRING,
    GAD_ID_FILE_BUTTON,
    GAD_ID_ACTION1_TEXT,
    GAD_ID_ACTION_MODIFY_CHECK,
    GAD_ID_ACTION_REMOVE_CHECK,
    GAD_ID_ACTION_CREATE_CHECK,
    GAD_ID_ACTION2_TEXT,
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
static STRPTR _tmp_file = NULL;
static STRPTR _tmp_script = NULL;
static int _index = -1;
static char *_title = "";

static BOOL _handle_gadget_event (struct Gadget *gad, UWORD code);
static struct Window *_open_window (void);
static int _create_gadgets (void);

static void _open_filerequester (FR_TYPE type);

int window_edit_init (struct TextAttr *textattr, void *visualinfo, UWORD topborder)
{
    _textattr = textattr;
    _visualinfo = visualinfo;
    _topborder = topborder;
    if (_create_gadgets () != 0) {
        fprintf (stderr, "Could not create edit gadgets\n");
        window_free ();
        return 1;
    }

    _tmp_file = (STRPTR)AllocMem (TMP_SIZE, MEMF_ANY|MEMF_CLEAR);
    if (_tmp_file == NULL) {
        fprintf (stderr, "Could not alloc mem to tmp file string\n");
        window_free ();
        return 1;
    }
    _tmp_script = (STRPTR)AllocMem (TMP_SIZE, MEMF_ANY|MEMF_CLEAR);
    if (_tmp_script == NULL) {
        fprintf (stderr, "Could not alloc memory to tmp script string\n");
        window_free ();
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
    if (_tmp_file != NULL) FreeMem (_tmp_file, TMP_SIZE);
    if (_tmp_script != NULL) FreeMem (_tmp_script, TMP_SIZE);
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

    if (window_edit_visibility (v) != 0) {
        fprintf (stderr, "Window change visibility failed. Quiting...\n");
        *quit = TRUE;
    }
}

int window_edit_add (int index)
{
    _index = index;
    _tmp_file[0] = '\0';
    GT_SetGadgetAttrs (_gads[GAD_ID_FILE_STRING], _window, NULL, GTST_String, _tmp_file);
    _tmp_script[0] = '\0';
    GT_SetGadgetAttrs (_gads[GAD_ID_CMD_STRING], _window, NULL, GTST_String, _tmp_script);

    _title = "Add item";
    return window_edit_visibility (TRUE);
}

int window_edit_edit (int index, struct notify_item *item)
{
    _index = index;
    CopyMem (item->path, _tmp_file, strlen (item->path) + 1);
    GT_SetGadgetAttrs (_gads[GAD_ID_FILE_STRING], _window, NULL, GTST_String, _tmp_file);
    CopyMem (item->command, _tmp_script, strlen (item->command) + 1);
    GT_SetGadgetAttrs (_gads[GAD_ID_CMD_STRING], _window, NULL, GTST_String, _tmp_script);
    /* FIXME: item->reason */
    _title = "Edit item";
    return window_edit_visibility (TRUE);
}


static BOOL _handle_gadget_event (struct Gadget *gad, UWORD code)
{
    BOOL v = _visible;

    fprintf (stderr, "edit gadget id: :%lu\n", (ULONG)gad->GadgetID);
    switch (gad->GadgetID)
    {
    case GAD_ID_FILE_STRING: {
    }
    break;
    case GAD_ID_FILE_BUTTON: {
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
    case GAD_ID_ACTION_REMOVE_CHECK: {
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

    gad = CreateContext (&_glist);

    ng.ng_TextAttr   = _textattr;
    ng.ng_VisualInfo = _visualinfo;

    /* top */
    ng.ng_TopEdge   = top;
    ng.ng_LeftEdge   = 16;
    ng.ng_Width      = WINDOW_WIDTH - 32 - 4 - FILE_BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "";
    ng.ng_GadgetID   = GAD_ID_FILE_STRING;
    gad = CreateGadget (STRING_KIND, gad, &ng,
                    GT_Underscore, '_',
                    GTST_MaxChars, TMP_SIZE - 1,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_FILE_STRING] = gad;
    
    ng.ng_LeftEdge  += ng.ng_Width + 4;
    ng.ng_Width      = FILE_BUTTON_WIDTH;
    ng.ng_GadgetText = "_File";
    ng.ng_GadgetID   = GAD_ID_FILE_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_FILE_BUTTON] = gad;
    
    /* 2nd row */
    ng.ng_TopEdge   += BUTTON_HEIGHT + 4;
    ng.ng_LeftEdge   = 16;
    ng.ng_Width      = 40;
    ng.ng_GadgetText = "WHEN";
    ng.ng_GadgetID   = GAD_ID_ACTION1_TEXT;
    gad = CreateGadget (TEXT_KIND, gad, &ng,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION1_TEXT] = gad;
    
    /*FIXME: add checkboxs */ 

    ng.ng_LeftEdge   = WINDOW_WIDTH - 40 - 16;
    ng.ng_Width      = 40;
    ng.ng_GadgetText = "RUN";
    ng.ng_GadgetID   = GAD_ID_ACTION2_TEXT;
    gad = CreateGadget (TEXT_KIND, gad, &ng,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION2_TEXT] = gad;
    
    /* 3th row */
    ng.ng_TopEdge   += BUTTON_HEIGHT + 4;
    ng.ng_LeftEdge   = 16;
    ng.ng_GadgetText = "";
    ng.ng_Width      = WINDOW_WIDTH - 32 - 4 - FILE_BUTTON_WIDTH;
    ng.ng_GadgetID   = GAD_ID_CMD_STRING;
    gad = CreateGadget (STRING_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_CMD_STRING] = gad;

    ng.ng_LeftEdge   += ng.ng_Width + 4;
    ng.ng_Width      = FILE_BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "_REXX";
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
    ng.ng_GadgetText = "_OK";
    ng.ng_GadgetID   = GAD_ID_OK;
    ng.ng_Flags      = 0;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_OK] = gad;

    ng.ng_LeftEdge   = WINDOW_WIDTH - BUTTON_WIDTH - 16;
    ng.ng_GadgetText = "_Cancel";
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
    } else if (type == FR_TYPE_FILE) {
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
            CopyMem (fr->rf_Dir, _tmp_script, strlen (fr->rf_Dir) + 1);
            AddPart (_tmp_script, fr->rf_File, TMP_SIZE);
            GT_SetGadgetAttrs (_gads[GAD_ID_CMD_STRING], _window, NULL, GTST_String, _tmp_script);
        } else if (type == FR_TYPE_FILE) {
            CopyMem (fr->rf_Dir, _tmp_file, strlen (fr->rf_Dir) + 1);
            AddPart (_tmp_file, fr->rf_File, TMP_SIZE);
            GT_SetGadgetAttrs (_gads[GAD_ID_FILE_STRING], _window, NULL, GTST_String, _tmp_file);
        }
    }
    FreeAslRequest (fr);
}
