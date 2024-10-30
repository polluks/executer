#ifndef _EXECUTOR_MUI_CLASSES_H_
#define _EXECUTOR_MUI_CLASSES_H_

#include <proto/alib.h>

#include "vapor.h"

BOOL classes_init(void);
void classes_cleanup(void);

#define DEFCLASS(s) \
    ULONG create_##s##class(void); \
    APTR get##s##class(void); \
    APTR get##s##classroot(void); \
    void delete_##s##class(void)

DEFCLASS(executerapplication);
DEFCLASS(executermainwindow);
DEFCLASS(executerlist);

/* All methods, attributes and findable objects */
enum {
    ME_Executer_Project = 1, /* Menu items */
    ME_Executer_About,
    ME_Executer_Quit,
    MO_Executer_MainWindow,
    MO_Executer_List,
    MM_ExecuterApplication_About,
    MM_ExecuterApplication_ReallyQuit,
    MM_ExecuterList_RemoveSelected,
    MM_ExecuterList_EditSelected,
    MM_ExecuterList_Add,
    MM_ExecuterList_Clear,
    MA_Executer_Quit
};

#endif
