#include <stdio.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>

#include "window.h"

#define WINDOW_WIDTH 160
#define WINDOW_HEIGHT 100
#define WINDOW_TITLE "Executer"

static BOOL _visible = FALSE;
static ULONG _signal = 0;
static struct Window *_window = NULL;

static struct Window *_open_window (void);

int window_init (void)
{
    return 0;
}

void window_free (void)
{
    _signal = 0;
    if (window_visibility (FALSE) != 0) {
        return;
    }
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
    struct IntuiMessage *msg;
    BOOL v = _visible;
    if (_window == NULL) return; 
    while ((msg = (struct IntuiMessage *)GetMsg(_window->UserPort)) != NULL) {
        switch (msg->Class) {
            case IDCMP_CLOSEWINDOW:
                v = FALSE;
                break;
            default:
                break;
        }
        ReplyMsg ((struct Message *)msg);
    }

    if (window_visibility (v) != 0) {
        fprintf (stderr, "Window change visibility failed. Quiting...\n");
        *quit = TRUE;
    }    
}

static struct Window *_open_window (void)
{
    struct Window *w = (struct Window *)OpenWindowTags (NULL,
        WA_Left, 25,
        WA_Top, 25,
        WA_Width, WINDOW_WIDTH,
        WA_Height, WINDOW_HEIGHT,
		WA_Title, (ULONG)WINDOW_TITLE,
		WA_DepthGadget, TRUE,
		WA_CloseGadget, TRUE,
		WA_DragBar, TRUE,
		WA_IDCMP, IDCMP_CLOSEWINDOW,
	        TAG_END, NULL);
    if (w != NULL) _signal = 1L << w->UserPort->mp_SigBit;
    return w;
}

