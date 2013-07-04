#include "htop-sysdeps.h"

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

IOPriority sysdep_get_ioprio(pid_t p)
{
    IOPriority ioprio = syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, p);
    return ioprio;
}

void sysdep_set_ioprio(pid_t p, IOPriority ioprio)
{
    syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, p);
}

