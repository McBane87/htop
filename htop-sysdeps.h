#ifndef HTOP_SYSDEPS_H
#define HTOP_SYSDEPS_H

#include <stdbool.h>
#include <sys/types.h>

#include "IOPriority.h"
#include "ProcessList.h"
#include "Process.h"

IOPriority sysdep_get_ioprio(pid_t);
void sysdep_set_ioprio(pid_t, IOPriority);
void sysdep_get_meminfo(ProcessList*);
void sysdep_update_cpu_data(ProcessList *);
int sysdep_max_pid();
bool sysdep_get_process_info(Process *, pid_t);
long sysdep_uptime(); // seconds since boot
#endif

