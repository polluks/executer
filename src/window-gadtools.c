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

#include "window.h"

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

typedef enum {
    FR_TYPE_FILE,
    FR_TYPE_SCRIPT
} FR_TYPE;

enum {
    GAD_ID_LIST = 0,
    GAD_ID_DELETE_BUTTON,
    GAD_ID_FILE_STRING,
    GAD_ID_FILE_BUTTON,
    GAD_ID_ACTION1_TEXT,
    GAD_ID_ACTION_CYCLE,
    GAD_ID_ACTION2_TEXT,
    GAD_ID_SCRIPT_STRING,
    GAD_ID_SCRIPT_BUTTON,
    GAD_ID_ADD_BUTTON,
    GAD_ID_SAVE,
    GAD_ID_USE,
    GAD_ID_CANCEL,
    GAD_ID_LAST
};

static BOOL _visible = FALSE;
static ULONG _signal = 0;

static struct Screen *_pubscreen = NULL;
static struct TextFont *_font = NULL;
static void *_visualinfo = NULL;

static struct Window *_window = NULL;
static struct Gadget *_glist;
static struct Gadget *_gads[GAD_ID_LAST];

#define TMP_SIZE 1024
static STRPTR _tmp_file = NULL;
static STRPTR _tmp_script = NULL;

static void _handle_gadget_event (struct Gadget *gad, UWORD code);
static struct Window *_open_window (void);
static int _create_gadgets (void);

static void _open_filerequester (FR_TYPE type);

struct TextAttr topaz8 = {
	"topaz.font", 8, 0, 0
};

static STRPTR labels[] = { "Modified", "Removed", "Created", NULL };

int window_init (void)
{
    UWORD topborder;
    _font = OpenFont (&topaz8);
    if (_font == NULL) {
        fprintf (stderr, "Could not open font.\n");
        return 1;
    }
    _pubscreen = LockPubScreen (NULL);
    if (_pubscreen == NULL) {
        fprintf (stderr, "Could not lock pubscreen\n");
        window_free ();
        return 1;
    }
    _visualinfo = GetVisualInfo (_pubscreen, TAG_END);
    if (_visualinfo == NULL) {
        fprintf (stderr, "Could not get visual info.\n");
        window_free ();
        return 1;
    }
    if (_create_gadgets () != 0) {
        fprintf (stderr, "Could not create gadgets\n");
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

void window_free (void)
{
    _signal = 0;
    if (window_visibility (FALSE) != 0) {
        return;
    }
    if (_glist != NULL) FreeGadgets (_glist);
    if (_visualinfo != NULL) FreeVisualInfo (_visualinfo);
    if (_pubscreen != NULL) UnlockPubScreen (NULL, _pubscreen);
    if (_font != NULL) CloseFont (_font);
    if (_tmp_file != NULL) FreeMem (_tmp_file, TMP_SIZE);
    if (_tmp_script != NULL) FreeMem (_tmp_script, TMP_SIZE);
}

ULONG window_signal (void)
{
    return _signal;
}

int window_visibility (BOOL visible)
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

BOOL window_is_visible (void)
{
    return _visible;
}

void window_dispose (BOOL *quit)
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
                _handle_gadget_event (gad, imsgCode);
                break;
            case IDCMP_VANILLAKEY:
                /*handleVanillaKey(mywin, imsgCode, slider_level, my_gads);*/
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

    if (window_visibility (v) != 0) {
        fprintf (stderr, "Window change visibility failed. Quiting...\n");
        *quit = TRUE;
    }
}

static void _handle_gadget_event (struct Gadget *gad, UWORD code)
{
    switch (gad->GadgetID)
    {
    case GAD_ID_LIST: {
    }
    break;
    case GAD_ID_FILE_STRING: {
    }
    break;
    case GAD_ID_FILE_BUTTON: {
         _open_filerequester (FR_TYPE_FILE);
    }
    break;
    case GAD_ID_SCRIPT_STRING: {
    }
    break;
    case GAD_ID_SCRIPT_BUTTON: {
         _open_filerequester (FR_TYPE_SCRIPT);
    }
    break;
    case GAD_ID_ACTION_CYCLE: {
    }
    break;
    case GAD_ID_ADD_BUTTON: {
    }
    break;
    case GAD_ID_SAVE: {
    }
    break;
    case GAD_ID_USE: {
    }
    case GAD_ID_CANCEL: {
        if (window_visibility (FALSE) != 0) {
        }
    }
    break;
    default:
    break;
    }
}

static struct Window *_open_window (void)
{
    struct Window *w = (struct Window *)OpenWindowTags (NULL,
	WA_Title, (ULONG)WINDOW_TITLE,
        WA_AutoAdjust, TRUE,
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
    return w;
}

static int _create_gadgets (void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    UWORD top = _pubscreen->WBorTop + (_pubscreen->Font->ta_YSize + 1) + 4;
    UWORD rightleft = 0;

    gad = CreateContext (&_glist);

    ng.ng_TextAttr   = &topaz8;
    ng.ng_VisualInfo = _visualinfo;

    /* Top left */
    ng.ng_TopEdge   = top;
    ng.ng_LeftEdge   = 10;
    ng.ng_Width      = WINDOW_WIDTH/2 - 10 - 4 - FILE_BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "";
    ng.ng_GadgetID   = GAD_ID_FILE_STRING;
    gad = CreateGadget (STRING_KIND, gad, &ng,
                    GT_Underscore, '_',
                    GTST_MaxChars, TMP_SIZE - 1,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_FILE_STRING] = gad;
    
    ng.ng_LeftEdge   += ng.ng_Width + 4;
    ng.ng_Width      = FILE_BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "_File";
    ng.ng_GadgetID   = GAD_ID_FILE_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_FILE_BUTTON] = gad;
    
    rightleft = ng.ng_LeftEdge + ng.ng_Width + 8; 
    
    /* 2nd row */
    ng.ng_TopEdge    = top + BUTTON_HEIGHT + 4;
    ng.ng_LeftEdge   = 10;
    ng.ng_Width      = 56;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "WHEN";
    ng.ng_GadgetID   = GAD_ID_ACTION1_TEXT;
    gad = CreateGadget (TEXT_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION1_TEXT] = gad;
    
    ng.ng_LeftEdge   +=  ng.ng_Width + 4;
    ng.ng_Width      = WINDOW_WIDTH/2 - 114 - 8;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "";
    ng.ng_GadgetID   = GAD_ID_ACTION_CYCLE;
    gad = CreateGadget (CYCLE_KIND, gad, &ng,
                    GT_Underscore, '_',
                    GTCY_Labels, labels,
                    GTCY_Active, 0,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION_CYCLE] = gad;
    
    ng.ng_LeftEdge   += ng.ng_Width + 4;
    ng.ng_Width      = 48;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "RUN";
    ng.ng_GadgetID   = GAD_ID_ACTION2_TEXT;
    gad = CreateGadget (TEXT_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ACTION2_TEXT] = gad;
    
    /* 3th row */
    ng.ng_TopEdge    = top + BUTTON_HEIGHT + 4 + BUTTON_HEIGHT + 4;
    ng.ng_LeftEdge   = 10;
    ng.ng_GadgetText = "";
    ng.ng_Width      = WINDOW_WIDTH/2 - 10 - 4 - FILE_BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetID   = GAD_ID_SCRIPT_STRING;
    gad = CreateGadget (STRING_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_SCRIPT_STRING] = gad;

    ng.ng_LeftEdge   += ng.ng_Width + 4;
    ng.ng_Width      = FILE_BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "_REXX";
    ng.ng_GadgetID   = GAD_ID_SCRIPT_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    GTST_MaxChars, TMP_SIZE - 1,
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_SCRIPT_BUTTON] = gad;
    
    /* 4th row */
    ng.ng_TopEdge    = top +  WINDOW_HEIGHT - BUTTON_HEIGHT - 4 - BUTTON_HEIGHT - 4 - BUTTON_HEIGHT - 4;
    ng.ng_LeftEdge   = 10;
    ng.ng_Width      = WINDOW_WIDTH/2 - 10;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "_Add";
    ng.ng_GadgetID   = GAD_ID_ADD_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_ADD_BUTTON] = gad;
     
    /* Top right */
    ng.ng_TopEdge    = top;
    ng.ng_LeftEdge   = rightleft;
    ng.ng_Width      = WINDOW_WIDTH/2 - 10 - 8;
    ng.ng_Height     = WINDOW_HEIGHT - BUTTON_HEIGHT - 4 - BUTTON_HEIGHT - 4 - BUTTON_HEIGHT - 4;
    ng.ng_GadgetID   = GAD_ID_LIST;
    gad = CreateGadget (LISTVIEW_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_LIST] = gad;
    
    ng.ng_TopEdge    = top + ng.ng_Height;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "_Delete";
    ng.ng_GadgetID   = GAD_ID_DELETE_BUTTON;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_DELETE_BUTTON] = gad;

   
    /* Bottom part */ 
    ng.ng_LeftEdge   = 10;
    ng.ng_TopEdge    = WINDOW_HEIGHT - 20;
    ng.ng_Width      = BUTTON_WIDTH;
    ng.ng_Height     = BUTTON_HEIGHT;
    ng.ng_GadgetText = "_Save";
    ng.ng_GadgetID   = GAD_ID_SAVE;
    ng.ng_Flags      = 0;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_SAVE] = gad;

    ng.ng_LeftEdge   = WINDOW_WIDTH/2 - BUTTON_WIDTH/2;
    ng.ng_GadgetText = "_Use";
    ng.ng_GadgetID   = GAD_ID_USE;
    ng.ng_Flags      = 0;
    gad = CreateGadget (BUTTON_KIND, gad, &ng,
                    GT_Underscore, '_',
                    TAG_END);
    if (gad == NULL) return 1;
    _gads[GAD_ID_USE] = gad;
    
    ng.ng_LeftEdge   = WINDOW_WIDTH - BUTTON_WIDTH - 10;
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
    if (type == FR_TYPE_SCRIPT) {
        title = "Select AREXX script";
        pattern = "#?.rexx";
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
        if (type == FR_TYPE_SCRIPT) {
            CopyMem (fr->rf_Dir, _tmp_script, strlen (fr->rf_Dir) + 1);
            AddPart (_tmp_script, fr->rf_File, TMP_SIZE);
            GT_SetGadgetAttrs (_gads[GAD_ID_SCRIPT_STRING], _window, NULL, GTST_String, _tmp_script);
        } else if (type == FR_TYPE_FILE) {
            CopyMem (fr->rf_Dir, _tmp_file, strlen (fr->rf_Dir) + 1);
            AddPart (_tmp_file, fr->rf_File, TMP_SIZE);
            GT_SetGadgetAttrs (_gads[GAD_ID_FILE_STRING], _window, NULL, GTST_String, _tmp_file);
        }
    }
    FreeAslRequest (fr);
}
