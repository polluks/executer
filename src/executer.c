#include <stdio.h>

#include <exec/types.h>
#include <dos/dos.h>

#include "libraries.h"
#include "arexx.h"
#include "notify.h"
#include "window.h"

#define AREXX_PORTNAME "EXECUTER"

void _rx_show (struct RexxMsg *msg, const char *args);
void _rx_hide (struct RexxMsg *msg, const char *args);
void _rx_quit (struct RexxMsg *msg, const char *args);

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
        BOOL send_window_show_hide = FALSE;
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
        
        retval = prefs_load ();
        if (retval != 0) {
            notify_free ();
            arexx_free ();
	    libraries_close ();
            return 1;
        }

        if (window_init() != 0) {
            arexx_free ();
	    libraries_close ();
            return 1;
        }

        _rx_signal = arexx_signal ();
        _notify_signal = notify_signal ();
        _win_signal = window_signal ();
	/* Open window if requested */
        while (_quit == FALSE) {
            signals = Wait (_rx_signal | _notify_signal | _win_signal | SIGBREAKF_CTRL_C);
            if (signals & _rx_signal) {
                arexx_dispose ();
            }
            if (signals & _notify_signal) {
                notify_dispose ();
            }
            if (signals & _win_signal) {
                window_dispose (&_quit);
                _win_signal = window_signal ();
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

void _rx_show (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "SHOW. args:%s\n", args);
    if (window_visibility (TRUE) != 0) {
        fprintf (stderr, "SHOW failed.\n");
    }
    _win_signal = window_signal ();
    fprintf (stderr, "%s: New signal %lu.\n", "SHOW", _win_signal);
}

void _rx_hide (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "HIDE. args:%s\n", args);
    if (window_visibility (FALSE) != 0) {
        fprintf (stderr, "HIDE failed.\n");
    }
    _win_signal = window_signal ();
    fprintf (stderr, "%s: New signal %lu.\n", "HIDE", _win_signal);
}

void _rx_quit (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "QUIT. args:%s\n...leaving.\n", args);
    _quit = TRUE;
}
