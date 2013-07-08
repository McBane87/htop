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

/* TODO */
void sysdep_update_cpu_data(ProcessList *this)
{
   unsigned long long int
       usertime = 0, nicetime = 0, systemtime = 0,
       systemalltime = 0, idlealltime = 0, idletime = 0,
       totaltime = 0, virtalltime = 0;

   for (int i = 0; i <= this->cpuCount; i++) {
      unsigned long long int ioWait, irq, softIrq, steal, guest;
      ioWait = irq = softIrq = steal = guest = 0;
      idlealltime = idletime + ioWait;
      systemalltime = systemtime + irq + softIrq;
      virtalltime = steal + guest;
      totaltime = usertime + nicetime + systemalltime + idlealltime + virtalltime;
      CPUData* cpuData = &(this->cpus[i]);
      cpuData->userPeriod = usertime - cpuData->userTime;
      cpuData->nicePeriod = nicetime - cpuData->niceTime;
      cpuData->systemPeriod = systemtime - cpuData->systemTime;
      cpuData->systemAllPeriod = systemalltime - cpuData->systemAllTime;
      cpuData->idleAllPeriod = idlealltime - cpuData->idleAllTime;
      cpuData->idlePeriod = idletime - cpuData->idleTime;
      cpuData->ioWaitPeriod = ioWait - cpuData->ioWaitTime;
      cpuData->irqPeriod = irq - cpuData->irqTime;
      cpuData->softIrqPeriod = softIrq - cpuData->softIrqTime;
      cpuData->stealPeriod = steal - cpuData->stealTime;
      cpuData->guestPeriod = guest - cpuData->guestTime;
      cpuData->totalPeriod = totaltime - cpuData->totalTime;
      cpuData->userTime = usertime;
      cpuData->niceTime = nicetime;
      cpuData->systemTime = systemtime;
      cpuData->systemAllTime = systemalltime;
      cpuData->idleAllTime = idlealltime;
      cpuData->idleTime = idletime;
      cpuData->ioWaitTime = ioWait;
      cpuData->irqTime = irq;
      cpuData->softIrqTime = softIrq;
      cpuData->stealTime = steal;
      cpuData->guestTime = guest;
      cpuData->totalTime = totaltime;
   }
}

int sysdep_max_pid()
{
    return sysconf(_SC_MAXPID);
}
