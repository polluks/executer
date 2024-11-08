#ifndef _EXECUTOR_MUI_CLASSES_H_
#define _EXECUTOR_MUI_CLASSES_H_

#include <proto/alib.h>

#include "vapor.h"
#include "../notify.h"

BOOL classes_init(void);
void classes_cleanup(void);

#define DEFCLASS(s) \
    ULONG create_##s##class(void); \
    APTR get##s##class(void); \
    APTR get##s##classroot(void); \
    void delete_##s##class(void)

DEFCLASS(executerapplication);
DEFCLASS(executermainwindow);
DEFCLASS(executermaingroup);
DEFCLASS(executereditgroup);
DEFCLASS(executerlistgroup);
DEFCLASS(executerlistview);
DEFCLASS(executerlist);
DEFCLASS(executerapplygroup);

/* All methods, attributes and findable objects */
enum {
    ME_Executer_Project = 1, /* Menu items */
    ME_Executer_About,
    ME_Executer_Quit,

    MO_Executer_MainWindow,
    MO_Executer_Main_Group,
    MO_Executer_Edit_Group,
    MO_Executer_List_Group,
    MO_Executer_List_Listview,
    MO_Executer_List,
    MO_Executer_Apply_Group,

    MM_ExecuterApplication_About,
    MM_ExecuterApplication_Quit,
    MM_ExecuterMainWindow_ToggleMode,
    MM_ExecuterListGroup_Add,
    MM_ExecuterListGroup_Edit,
    MM_ExecuterListGroup_Remove,
    MM_ExecuterListGroup_SelectChange,
    MM_ExecuterListview_Add,
    MM_ExecuterListview_Clear,
    MM_ExecuterListview_Sort,
    MM_ExecuterListview_DoubleClick,
    MM_ExecuterList_RemoveSelected,
    MM_ExecuterList_EditSelected,
    MM_ExecuterList_Clear,
    MM_ExecuterList_Update,
    MM_ExecuterList_DoubleClick,
    MM_ExecuterApplyGroup_Save,
    MM_ExecuterApplyGroup_Use,
    MM_ExecuterApplyGroup_Cancel,    
    MM_ExecuterEditGroup_Ok,
    MM_ExecuterEditGroup_Cancel,
    MM_ExecuterEditGroup_CheckValidy,

    MA_Executer_EditItem,
    MA_Executer_ContextMenuEnabled,
    MA_Executer_Quit
};

struct MP_ExecuterListview_Add
{
    ULONG MethodID;
    struct notify_item *item;
    int index;
};

#endif
