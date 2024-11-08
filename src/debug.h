#ifndef _EXECUTER_DEBUG_
#define _EXECUTER_DEBUG_

#ifdef __MORPHOS_
#include <clib/debug_protos.h>

#define BUG kprintf

#ifdef DEBUG
# define D(x) (x)
#else
# define D(x)
#endif

#else

#include <stdio.h>

#define BUG printf

#ifdef DEBUG
# define D(x) (x)
#else
# define D(x)
#endif

#endif

#endif
