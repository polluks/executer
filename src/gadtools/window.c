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

#include "../window.h"
#include "window-main.h"

static struct Screen *_pubscreen = NULL;
static struct TextFont *_font = NULL;
static void *_visualinfo = NULL;

static struct TextAttr topaz8 = {
	"topaz.font", 8, 0, 0
};

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
    topborder = _pubscreen->WBorTop + (_pubscreen->Font->ta_YSize + 1);
    if (window_main_init (&topaz8, _visualinfo, topborder + 4) != 0) {
        fprintf (stderr, "Could not init main window\n");
        window_free ();
        return 1;
    }
    if (window_edit_init (&topaz8, _visualinfo, topborder + 4) != 0) {
        fprintf (stderr, "Could not init main window\n");
        window_main_free ();
        return 1;
    }
    return 0;
}

void window_free (void)
{
    window_edit_free ();
    window_main_free ();
    if (_visualinfo != NULL) FreeVisualInfo (_visualinfo);
    if (_pubscreen != NULL) UnlockPubScreen (NULL, _pubscreen);
    if (_font != NULL) CloseFont (_font);
}

ULONG window_signal (void)
{
    if (window_edit_is_visible () == TRUE) return window_edit_signal ();
    return window_main_signal ();
}

int window_visibility (BOOL visible)
{
    if (visible == FALSE && window_edit_is_visible () == TRUE) {
        (void)window_edit_visibility (visible);
    }
    window_main_visibility (visible);
    return 0;
}

BOOL window_is_visible (void)
{
    return window_main_is_visible ();
}

void window_dispose (BOOL *quit)
{
    if (window_edit_is_visible () == TRUE) {
        window_edit_dispose (quit);
    } else {
        window_main_dispose (quit);
    }
}
