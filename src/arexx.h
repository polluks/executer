#ifndef EXECUTER_AREXX_H
#define EXECUTER_AREXX_H

#include <exec/types.h>

struct RexxMsg;

struct rx_command {
  const char *command;
  const char *args;
  const char *results;
  void (*func)(struct RexxMsg *, const char *args);
};

int arexx_init (const char *name, struct rx_command  *list);
ULONG arexx_sigbit (void);
void arexx_free (void);
int arexx_dispose (void);

#endif /* EXECUTER_AREXX_H */
