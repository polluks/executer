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
#include "objects.h"

#include "m68k.h"

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 200

typedef enum {
  WINDOW_MODE_LIST,
  WINDOW_MODE_EDIT
} window_mode;
struct ExecuterMainWindowData
{
    APTR menustrip;
    APTR GR_main;
    window_mode mode;
};


/* new */
DEFNEW(ExecuterMainWindow)
{
    APTR menustrip;
    APTR GR_main;

    obj = DoSuperNew(cl, obj,
        MUIA_Window_Title,   "Executer",
        MUIA_Window_ID,      MAKE_ID ('E', 'X', 'E', 'R'),
        MUIA_Window_Height,  DEFAULT_HEIGHT,
        MUIA_Window_Width,   DEFAULT_WIDTH,
        MUIA_Window_Menustrip, menustrip = ExecuterMainMenu(),
        MUIA_UserData, MO_Executer_MainWindow,
        WindowContents, VGroup, 
            Child, GR_main = NewObject (getexecutermaingroupclass(), NULL, TAG_DONE),
            TAG_END),
        TAG_MORE, (((struct opSet *)msg)->ops_AttrList),
        TAG_DONE);
    
    if (obj != NULL) {
        struct ExecuterMainWindowData *data = INST_DATA (cl, obj);
        data->menustrip = menustrip; 
        data->GR_main = GR_main; 
        data->mode = WINDOW_MODE_LIST; 
        //set (obj, MUIA_Window_Open, FALSE);
    }

    return (ULONG)obj;
}

DEFTMETHOD(ExecuterMainWindow_ToggleMode)
{
    struct ExecuterMainWindowData *data = INST_DATA (cl, obj);
    APTR edit = (APTR) DoMethod (_app(obj), MUIM_FindUData, MO_Executer_EditWindow);
    if (data->mode == WINDOW_MODE_LIST) {
        set (edit, MUIA_Window_Open, TRUE);
        data->mode = WINDOW_MODE_EDIT;
        set (obj, MUIA_Window_Sleep, TRUE);
    } else {
        set (obj, MUIA_Window_Sleep, FALSE);
        set (edit, MUIA_Window_Open, FALSE);
        data->mode = WINDOW_MODE_LIST;
    }
    return 0;
}

BEGINMTABLE2(executermainwindowclass)
DECNEW(ExecuterMainWindow)
DECTMETHOD(ExecuterMainWindow_ToggleMode)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Window, executermainwindowclass, ExecuterMainWindowData)
