#ifdef __M68K__
# include <libraries/gadtools.h> /* NM_...*/
#else
# include <proto/gadtools.h> /* NM_...*/
#endif
#include <proto/muimaster.h>
#include <libraries/mui.h>
#include <mui/BetterString_mcc.h>

#include "objects.h"
#include "classes.h"

static struct NewMenu _ExecuterMainMenu[] =
{
    { NM_TITLE, (STRPTR)"Project",   0, 0, 0, (APTR)ME_Executer_Project },
    { NM_ITEM,  (STRPTR)"About...",  (STRPTR)"?", 0, 0, (APTR)ME_Executer_About },
    { NM_ITEM,  (STRPTR)"Quit",      (STRPTR)"Q", 0, 0, (APTR)ME_Executer_Quit },

    { NM_END,   NULL,              0, 0, 0,   (APTR)0 }
};

Object *ExecuterMainMenu (void)
{
    return MUI_MakeObject (MUIO_MenustripNM, (ULONG)_ExecuterMainMenu, MUIO_MenustripNM_CommandKeyCheck);
}


Object *ExecuterLabel (const char *text)
{
    return (Object *)MUI_NewObject((CONST_STRPTR)MUIC_Text,
        NoFrame,
        MUIA_Text_Contents, text,
        MUIA_Text_SetMax, FALSE,
        TAG_END);
}

Object *ExecuterButton (const char *text, char key)
{
    return (Object *)TextObject,
        ButtonFrame,
        MUIA_Font, MUIV_Font_Button,
        MUIA_Text_Contents, text,
        MUIA_Text_PreParse, "\33c",
        MUIA_InputMode, MUIV_InputMode_RelVerify,
        MUIA_Background, MUII_ButtonBack,
        MUIA_CycleChain, TRUE,
        MUIA_Text_HiChar, key,
        MUIA_ControlChar, key,
        TAG_END);
}

Object *ExecuterCheck (BOOL check)
{
    return (Object *)ImageObject,
        ImageButtonFrame,
        MUIA_InputMode, MUIV_InputMode_Toggle,
        MUIA_Image_Spec, MUII_CheckMark,
        MUIA_Background, MUII_ButtonBack,
        MUIA_Selected, check,
        MUIA_ShowSelState, FALSE,
        MUIA_CycleChain, TRUE,
        TAG_END);
}

Object *ExecuterString (int maxlen, const char *text)
{
    return (Object *)BetterStringObject,
        StringFrame,
        MUIA_String_AdvanceOnCR, TRUE,
        MUIA_String_MaxLen, maxlen,
        MUIA_String_Contents, text,
        MUIA_CycleChain, TRUE,
        MUIA_Text_SetMax, TRUE,
        TAG_END);
}

Object *ExecuterCycle (char **strings)
{
    return (Object *)CycleObject,
        MUIA_Font, MUIV_Font_Button,
        MUIA_Cycle_Entries, strings,
        MUIA_CycleChain, TRUE, 
        TAG_END);
}

