#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>

#include "libraries.h"

extern struct DosLibrary *DOSBase;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *RexxSysBase = NULL;
#ifdef USE_MUI
#else
struct Library *AslBase = NULL;
struct Library *GfxBase = NULL;
struct Library *GadToolsBase = NULL;
#endif

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
#ifdef USE_MUI
#else
    GfxBase = OpenLibrary ("graphics.library", 0L);
    if (GfxBase == NULL) {
        libraries_close ();
        return 1;
    }
    GadToolsBase = OpenLibrary ("gadtools.library", 0L);
    if (GadToolsBase == NULL) {
        libraries_close ();
        return 1;
    }
    AslBase = OpenLibrary ("asl.library", 0L);
    if (AslBase == NULL) {
        libraries_close ();
        return 1;
    }
#endif
    return 0;
}

void libraries_close (void)
{
#ifdef USE_MUI
#else
    if (AslBase != NULL) {
        CloseLibrary (AslBase);
    }
    if (GadToolsBase != NULL) {
        CloseLibrary (GadToolsBase);
    }
    if (GfxBase != NULL) {
        CloseLibrary (GfxBase);
    }
#endif
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
