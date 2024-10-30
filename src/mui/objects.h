#ifndef _EXECUTER_OBJECTS_H_
#define _EXECUTER_OBJECTS_H_

#include <libraries/mui.h>

Object *ExecuterMainMenu (void);
Object *ExecuterLabel (const char *text);
Object *ExecuterButton (const char *text, char key);
Object *ExecuterCheck (BOOL check);
Object *ExecuterString (int maxlen, const char *text);
Object *ExecuterCycle (char **strings);

#endif
