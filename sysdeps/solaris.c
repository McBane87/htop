#include "htop-sysdeps.h"

/* dummy */
IOPriority sysdep_get_ioprio(pid_t p)
{
    return 1;
}

/* dummy */
void sysdep_set_ioprio(pid_t p, IOPriority ioprio)
{
}

