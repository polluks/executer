#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>

#include "libraries.h"

extern struct DosLibrary *DOSBase;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *RexxSysBase = NULL;

int libraries_open (void)
{
    DOSBase = (struct DosLibrary *)OpenLibrary ("dos.library", 37L); /* kick 2.04/5 required */
    if (DOSBase == NULL) {
        return 1;
    }
    IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", 0L);
    if (IntuitionBase == NULL) {
        libraries_close ();
        return 1;
    }
    RexxSysBase = OpenLibrary ("rexxsyslib.library", 0L);
    if (RexxSysBase == NULL) {
        libraries_close ();
        return 1;
    }
    return 0;
}

void libraries_close (void)
{
    if (RexxSysBase != NULL) {
        CloseLibrary (RexxSysBase);
    }
    if (IntuitionBase != NULL) {
        CloseLibrary ((struct Library *)IntuitionBase);
    }
    if (DOSBase != NULL) {
        CloseLibrary ((struct Library *)DOSBase);
    }
}
