#include <unistd.h>
#include <string.h>
#include <sys/swap.h>
#include <vm/anon.h>

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

void sysdep_get_meminfo(ProcessList *this)
{
    long pageSize;

    pageSize = sysconf(_SC_PAGESIZE) / 1024; // KiB

    this->totalMem = sysconf(_SC_PHYS_PAGES) * pageSize;
    this->freeMem  = sysconf(_SC_AVPHYS_PAGES) * pageSize;
    this->usedMem  = this->totalMem - this->freeMem;

    struct anoninfo ai;
    memset(&ai, 0, sizeof(ai));
    (void) swapctl(SC_AINFO, &ai);
    this->totalSwap = ai.ani_max * pageSize;
    this->usedSwap  = (ai.ani_max - ai.ani_free) * pageSize;

    /*TODO*/
    this->sharedMem = 0;
    this->cachedMem = 0;
    this->buffersMem = 0;
}

