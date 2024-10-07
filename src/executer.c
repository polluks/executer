#include <stdio.h>

#include <exec/types.h>
#include <dos/dos.h>

#include "libraries.h"
#include "arexx.h"
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

int main (int argc, char **argv)
{
        BOOL send_window_show_hide = FALSE;
        int retval;
        ULONG rx_signal, win_signal, signals;
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
            /* send arexx show window */
            return 0;
        }

        if (window_init() != 0) {
            arexx_free ();
	    libraries_close ();
            return 1;
        }

        rx_signal = arexx_signal ();
        win_signal = window_signal ();
	/* Open window if requested */
        while (_quit == FALSE) {
            signals = Wait (rx_signal | win_signal | SIGBREAKF_CTRL_C);
            if (signals & SIGBREAKF_CTRL_C) {
                _quit = TRUE;
            }
            if (signals & rx_signal) {
                arexx_dispose ();
            }
            if (signals & win_signal) {
                window_dispose (&_quit);
            }
        }

        window_free ();
        arexx_free ();
	libraries_close ();

	return 0;
}

void _rx_show (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "SHOW. args:%s\n", args);
}

void _rx_hide (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "HIDE. args:%s\n", args);
}

void _rx_quit (struct RexxMsg *msg, const char *args)
{
    fprintf (stderr, "QUIT. args:%s\n...leaving.\n", args);
    _quit = TRUE;
}
