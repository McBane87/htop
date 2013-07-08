#ifndef HTOP_SYSDEPS_H
#define HTOP_SYSDEPS_H

#include <sys/types.h>

#include "IOPriority.h"
#include "ProcessList.h"

IOPriority sysdep_get_ioprio(pid_t);
void sysdep_set_ioprio(pid_t, IOPriority);
void sysdep_get_meminfo(ProcessList*);
void sysdep_update_cpu_data(ProcessList *);

#endif

