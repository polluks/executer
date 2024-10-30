#include <stdio.h>

#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#ifdef ENABLE_MUI
# include <proto/muimaster.h>
#endif

#include "libraries.h"

extern struct DosLibrary *DOSBase;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *RexxSysBase = NULL;
#ifdef ENABLE_MUI
struct Library *MUIMasterBase = NULL;
#else
struct Library *AslBase = NULL;
struct Library *GfxBase = NULL;
struct Library *GadToolsBase = NULL;
#endif

int libraries_open (void)
{
    DOSBase = (struct DosLibrary *)OpenLibrary ((CONST_STRPTR)"dos.library", 37L); /* kick 2.04/5 required */
    if (DOSBase == NULL) {
        return 1;
    }
    IntuitionBase = (struct IntuitionBase *)OpenLibrary ((CONST_STRPTR)"intuition.library", 0L);
    if (IntuitionBase == NULL) {
        libraries_close ();
        return 1;
    }
    RexxSysBase = OpenLibrary ((CONST_STRPTR)"rexxsyslib.library", 0L);
    if (RexxSysBase == NULL) {
        libraries_close ();
        return 1;
    }
#ifdef ENABLE_MUI
    MUIMasterBase = OpenLibrary ((STRPTR)MUIMASTER_NAME, MUIMASTER_VMIN);
    if (!MUIMasterBase) {
        libraries_close ();
        return 1;
    }
#else
    GfxBase = OpenLibrary ((CONST_STRPTR)"graphics.library", 0L);
    if (GfxBase == NULL) {
        libraries_close ();
        return 1;
    }
    GadToolsBase = OpenLibrary ((CONST_STRPTR)"gadtools.library", 0L);
    if (GadToolsBase == NULL) {
        libraries_close ();
        return 1;
    }
    AslBase = OpenLibrary ((CONST_STRPTR)"asl.library", 0L);
    if (AslBase == NULL) {
        libraries_close ();
        return 1;
    }
#endif
    return 0;
}

void libraries_close (void)
{
#ifdef ENABLE_MUI
    if (MUIMasterBase) {
        CloseLibrary (MUIMasterBase);
    }
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
