#include <fcntl.h>
#include <procfs.h>
#include <stdio.h>
#include <string.h>
#include <sys/swap.h>
#include <time.h>
#include <unistd.h>
#include <vm/anon.h>

#include "htop-sysdeps.h"
#include "utils.h"

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

bool sysdep_get_process_info(Process *process, pid_t pid)
{
    char filename[34];
    int fd;
    ssize_t r;
    psinfo_t info;
    prusage_t usage;

    snprintf(filename, sizeof(filename), "/proc/%d/psinfo", pid);
    fd = open(filename, O_RDONLY);
    if (-1 == fd) return false;
    r = xread(fd, &info, sizeof(info));
    (void) close(fd);
    if (!r) return false;

    snprintf(filename, sizeof(filename), "/proc/%d/usage", pid);
    fd = open(filename, O_RDONLY);
    if (-1 == fd) return false;
    r = xread(fd, &usage, sizeof(usage));
    (void) close(fd);
    if (!r) return false;

    process->state = info.pr_lwp.pr_sname;

    process->st_uid = info.pr_uid;

    process->ppid = info.pr_ppid;
    process->pgrp = info.pr_pgid;
    process->session = info.pr_sid;
    process->tty_nr = info.pr_ttydev;
    process->tpgid = -1; // FIXME
    process->flags = info.pr_flag;

    long tix = sysconf(_SC_CLK_TCK);
    process->utime = usage.pr_utime.tv_sec * tix;
    process->stime = usage.pr_stime.tv_sec * tix;

    process->priority = info.pr_lwp.pr_pri;
    process->nice = info.pr_lwp.pr_nice - 20;

    process->nlwp = info.pr_nlwp;

    // FIXME:
    process->exit_signal = SIGCHLD;

    process->processor = info.pr_lwp.pr_onpro;

    process->comm = strdup(info.pr_psargs);


    /* XXX root only: */
    snprintf(filename, sizeof(filename), "/proc/%d/status", pid);
    fd = open(filename, O_RDONLY);
    if (fd > 0) {
        pstatus_t status;
        if (xread(fd, &status, sizeof(status)) > 0) {
            process->cutime = status.pr_cutime.tv_sec * tix;
            process->cstime = status.pr_cstime.tv_sec * tix;
        }
        (void) close(fd);
    }


    return true;
}

long sysdep_uptime()
{
    long uptime = 0;
    int fd;
    if ((fd = open("/proc/0/psinfo", O_RDONLY)) > 0) {
        psinfo_t pid0;
        if (xread(fd, &pid0, sizeof(pid0)) > 0) {
            uptime = time(NULL) - pid0.pr_start.tv_sec;
        }
        (void) close(fd);
    }
    return uptime;
}
