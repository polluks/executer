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

//#include "m68k.h"

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 200

struct ExecuterMainWindowData
{
    APTR menustrip;
};


/* new */
DEFNEW(ExecuterMainWindow)
{
    APTR menustrip; /*, group;*/

    obj = DoSuperNew(cl, obj,
        MUIA_Window_Title,   "Executer",
        MUIA_Window_ID,      MAKE_ID ('E', 'X', 'E', 'R'),
        MUIA_Window_Height,  DEFAULT_HEIGHT,
        MUIA_Window_Width,   DEFAULT_WIDTH,
        MUIA_Window_Menustrip, menustrip = ExecuterMainMenu(),
        MUIA_UserData, MO_Executer_MainWindow,
        WindowContents, VGroup, 
            /*Child, group = NewObject (getzmpcgroupclass(), NULL, TAG_DONE),*/
            TAG_END),
        TAG_MORE, (((struct opSet *)msg)->ops_AttrList),
        TAG_DONE);
    
    if (obj != NULL) {
        struct ExecuterMainWindowData *data = INST_DATA (cl, obj);
        data->menustrip = menustrip; 
        set (obj, MUIA_Window_Open, TRUE);
    }

    return (ULONG)obj;
}

BEGINMTABLE2(executermainwindowclass)
DECNEW(ExecuterMainWindow)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Window, executermainwindowclass, ExecuterMainWindowData)
