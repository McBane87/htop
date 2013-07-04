#ifndef HTOP_SYSDEPS_H
#define HTOP_SYSDEPS_H

#include <sys/types.h>
#include "IOPriority.h"

IOPriority sysdep_get_ioprio(pid_t);
void sysdep_set_ioprio(pid_t, IOPriority);

#endif

