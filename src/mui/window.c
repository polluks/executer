#include <stdio.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <clib/exec_protos.h>
#include <proto/muimaster.h>
#include <libraries/mui.h>

#include "window.h"
#include "classes.h"
#include "common.h"

static BOOL _visible = FALSE;
static ULONG _signal = 0;
static APTR _app = NULL, _window = NULL, _menuitem = NULL;

int window_init (void)
{
    classes_init();
    
    _app = NewObject (getexecuterapplicationclass(), NULL, TAG_DONE);
    if (!_app) {
        fprintf (stderr, "Could not create application!\n");
        return 1;
    }

    /* collect objects needed later */
    _window = (APTR) DoMethod (_app, MUIM_FindUData, MO_Executer_MainWindow);

    /* menuitem notifies */
    _menuitem = (APTR) DoMethod (_app, MUIM_FindUData, ME_Executer_About);
    DoMethod (_menuitem, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, _app, 1, MM_ExecuterApplication_About);
    _menuitem = (APTR) DoMethod (_app, MUIM_FindUData, ME_Executer_Quit);
    DoMethod (_menuitem, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, _app, 1, MM_ExecuterApplication_ReallyQuit);

    (void)DoMethod (_app, MUIM_Application_Input, &_signal);

    /* open window */
    window_visibility (TRUE);

    return 0;
}

void window_free (void)
{
    fprintf (stderr, "Window free\n");
    _signal = 0;
    if (window_visibility (FALSE) != 0) {
        return;
    }
    MUI_DisposeObject (_app);
    classes_cleanup();
}

ULONG window_signal (void)
{
#if 0
    if (_visible) {
	(void)DoMethod (_app, MUIM_Application_Input, &_signal);
    } else {
        _signal = 0;
    }
#endif
    return _signal;
}

int window_visibility (BOOL visible)
{
    if (_visible == visible) return 0;
    if (visible == TRUE) {
        set (_app, MA_Executer_Quit, 0);
    }

    _visible = visible;
    set (_window, MUIA_Window_Open, visible);

    return 0;
}

BOOL window_is_visible (void)
{
    return _visible;
}

void window_dispose (BOOL *quit)
{
    BOOL v = _visible;
    ULONG winquit = 0;
    _signal = 0;
    (void)DoMethod (_app, MUIM_Application_Input, &_signal);
    
    get (_app, MA_Executer_Quit, &winquit);
    if (winquit != 0) {
        fprintf (stderr, "Window quit -> hide\n");
        v = FALSE;
    }

    if (window_visibility (v) != 0) {
        fprintf (stderr, "Window change visibility failed. Quiting...\n");
        *quit = TRUE;
    }
}

int window_setup_list (struct List *nitems)
{
    return 0;
}
