#ifndef EXECUTER_PREFS_H
#define EXECUTER_PREFS_H

#define PREFS_FILE "executer.prefs"
#define PREFS_PATH_ENV ("ENV:" PREFS_FILE)
#define PREFS_PATH_ENVARC ("ENVARC:" PREFS_FILE)

int prefs_load (void);
int prefs_save_env ();
int prefs_save_envarc ();

#endif /* EXECUTER_PREFS_H */
