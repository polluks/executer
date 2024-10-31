#include <stdio.h>

#include <exec/types.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>

#include "libraries.h"
#include "arexx.h"
#include "notify.h"
#include "prefs.h"
#include "window.h"

#define AREXX_PORTNAME "EXECUTER"

static void _rx_show (struct RexxMsg *msg, const char *args);
static void _rx_hide (struct RexxMsg *msg, const char *args);
static void _rx_quit (struct RexxMsg *msg, const char *args);

static void _prefs_modified (const char *path);

static struct rx_command _commands[] =
{
    { "SHOW", NULL, NULL, _rx_show },
    { "HIDE", NULL, NULL, _rx_hide },
    { "QUIT", NULL, NULL, _rx_quit },
    { NULL, NULL, NULL, NULL }
};

static BOOL _quit = FALSE;
static ULONG _rx_signal, _notify_signal, _win_signal;

int main (int argc, char **argv)
{
        int retval;
        ULONG signals;
	/* Open libraries */
	if (libraries_open () != 0) {
		return 1;
	}

	/* Setup AREXX */
        retval = arexx_init (AREXX_PORTNAME, _commands);
        if (retval > 0) { /* fail */
	    libraries_close ();
            return 1;
        } else if (retval < 0) { /* already open */
            /* FIXME: send arexx show window */
            arexx_free ();
	    libraries_close ();
            return 0;
        }
	
        /* Setup notifies */
        retval = notify_init ();
        if (retval != 0) {
            arexx_free ();
	    libraries_close ();
            return 1;
        }
        
        if (window_init() != 0) {
            notify_free ();
            arexx_free ();
	    libraries_close ();
            return 1;
        }
        
        retval = prefs_load ();
        if (retval != 0) {
            window_free ();
            notify_free ();
            arexx_free ();
	    libraries_close ();
            return 1;
        }
        (void)notify_add (PREFS_PATH_ENV, "", NOTIFY_REASON_CREATE|NOTIFY_REASON_DELETE|NOTIFY_REASON_MODIFY, _prefs_modified);
        window_setup_list (notify_list ());

	fprintf (stderr, "1\n");
        _rx_signal = arexx_signal ();
        _notify_signal = notify_signal ();
//        _win_signal = window_signal ();
	fprintf (stderr, "2 _win_Signal %du\n", _win_signal);
	/* Open window if requested */
#ifdef ENABLE_MUI
        window_dispose (&_quit);
#endif
        _win_signal = window_signal ();
        while (_quit == FALSE) {
             window_dispose (&_quit);
            signals = Wait (_rx_signal | _notify_signal | _win_signal | SIGBREAKF_CTRL_C);
            if (signals & _rx_signal) {
                arexx_dispose ();
            }
            if (signals & _notify_signal) {
                notify_dispose ();
            }
            if (signals & _win_signal) {
                fprintf (stderr, "call window_dispose()\n");
                window_dispose (&_quit);
#ifndef ENABLE_MUI
                _win_signal = window_signal ();
                fprintf (stderr, "win signal:%lu\n", (long unsigned int)_win_signal);
#endif
            }
            if (signals & SIGBREAKF_CTRL_C) {
                _quit = TRUE;
            }
        }

        window_free ();
        notify_free ();
        arexx_free ();
	libraries_close ();

	return 0;
}

static void _rx_show (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "SHOW. args:%s\n", args);
    if (window_visibility (TRUE) != 0) {
        fprintf (stderr, "SHOW failed.\n");
    }
    _win_signal = window_signal ();
    fprintf (stderr, "%s: New signal %lu.\n", "SHOW", (long unsigned int)_win_signal);
}

static void _rx_hide (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "HIDE. args:%s\n", args);
    if (window_visibility (FALSE) != 0) {
        fprintf (stderr, "HIDE failed.\n");
    }
    _win_signal = window_signal ();
    fprintf (stderr, "%s: New signal %lu.\n", "HIDE", (long unsigned int)_win_signal);
}

static void _rx_quit (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "QUIT. args:%s\n...leaving.\n", args);
    _quit = TRUE;
}

static void _prefs_modified (const char *path)
{
    if (prefs_load () != 0) {
        fprintf (stderr, "Reloading prefs failed");
    }
    (void)notify_add (path, "", NOTIFY_REASON_CREATE|NOTIFY_REASON_DELETE|NOTIFY_REASON_MODIFY, _prefs_modified);
    window_setup_list (notify_list ());
}
