#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libraries/mui.h>

#include <clib/utility_protos.h>

#include <proto/muimaster.h>
#include <proto/alib.h> /* DoMetmod */
#include <proto/exec.h> /* Wait */
#include <proto/intuition.h> /* SetAttrs */

#include "vapor.h" /* Class macros */

#include "common.h"
#include "classes.h"

#include "m68k.h"

#define ABOUT_TEXT MUIX_C MUIX_B"Executer\n" \
MUIX_N "©2024 Joni Valtanen\n" \
"GPLv2\n\n" \
"Ver 0.1 (01.11.24)\n" \
"jvaltane at kapsi fi\n" \
"http://jvaltane.kapsi.fi/executer/\n"

#define QUIT_TEXT MUIX_C "Do you really want to quit?"

struct ExecuterApplicationData
{
    APTR window;
    ULONG quit;
};


/* new */
DEFNEW(ExecuterApplication)
{
    APTR main_win;

    obj = DoSuperNew(cl, obj,
        MUIA_Application_Title,       "Executer",
        MUIA_Application_Version,     "$VER: Executer 0.1 (01.11.24)",
        MUIA_Application_Copyright,   "©2024 Joni Valtanen",
        MUIA_Application_Author,      "Joni Valtanen",
        MUIA_Application_Base,        "EXER",
        MUIA_Application_Description, "Run commands when file is modified, removed or created",
        MUIA_Application_Window,       main_win = NewObject (getexecutermainwindowclass(), NULL, TAG_DONE),
        TAG_MORE, (((struct opSet *)msg)->ops_AttrList),
        TAG_DONE);

    if (obj != NULL) {
        struct ExecuterApplicationData *data = INST_DATA (cl, obj);
        data->window = main_win;
        data->quit = 0;

        DoMethod (obj, MUIM_Notify, MUIV_Application_ReturnID_Quit, obj, 1, MM_ExecuterApplication_ReallyQuit);
        DoMethod (data->window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 1, MM_ExecuterApplication_ReallyQuit);
    }

    return (ULONG)obj;
}

DEFGET(ExecuterApplication)
{
    struct ExecuterApplicationData *data = INST_DATA(cl, obj);

    switch (msg->opg_AttrID) {
        case MA_Executer_Quit: {
            *msg->opg_Storage = (ULONG)data->quit;
        }
        return TRUE;
        default:
            break;
    }

    return DOSUPER;
}

/* set */
static void doset(APTR obj, struct ExecuterApplicationData *data, struct TagItem *tags)
{
    FORTAG(tags)
    {
        case MA_Executer_Quit:
            data->quit = (ULONG)tag->ti_Data;
            break;
    }
    NEXTTAG
}

DEFSET(ExecuterApplication)
{
    struct ExecuterApplicationData *data = INST_DATA(cl, obj);
    doset (obj, data, INITTAGS);
    return DOSUPER;
}

DEFTMETHOD(ExecuterApplication_About)
{
    struct ExecuterApplicationData *data = INST_DATA(cl, obj);
    MUI_Request (obj, data->window, 0, NULL, "_OK", ABOUT_TEXT);
    return 0;
}

DEFTMETHOD(ExecuterApplication_ReallyQuit)
{
    struct ExecuterApplicationData *data = INST_DATA(cl, obj);
#if 0
    int ret = MUI_Request (obj, data->window, 0, "Quit", "_Quit|_Cancel", QUIT_TEXT);
    if (ret == 1) {
        data->quit = 1;
    }
#endif
    fprintf (stderr, "Application quit -> hide\n");
    data->quit = 1;
    return 0;
}

BEGINMTABLE2(executerapplicationclass)
DECNEW(ExecuterApplication)
DECGET(ExecuterApplication)
DECSET(ExecuterApplication)
DECTMETHOD(ExecuterApplication_About)
DECTMETHOD(ExecuterApplication_ReallyQuit)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Application, executerapplicationclass, ExecuterApplicationData)
