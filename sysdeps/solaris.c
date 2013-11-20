#include <fcntl.h>
#include <procfs.h>
#include <stdio.h>
#include <string.h>
#include <sys/swap.h>
#include <time.h>
#include <unistd.h>
#include <vm/anon.h>

#include <kstat.h>
#include <sys/cpuvar.h>
#include <stdlib.h>

#include "htop-sysdeps.h"
#include "utils.h"

static kstat_ctl_t *kc = NULL;
static kid_t kcid = 0;
static unsigned kstat_initialized = 0;

/*
 * int kstat_safe_namematch(int num, kstat_t *ksp, char *name, void *buf)
 *
 * Safe scan of kstat chain for names starting with "name".  Matches
 * are copied in to "ksp", and kstat_read is called on each match using
 * "buf" as a buffer of length "size".  The actual number of records
 * found is returned.  Up to "num" kstats are copied in to "ksp", but
 * no more.  If any kstat_read indicates that the chain has changed, then
 * the whole process is restarted.
 */
int kstat_safe_namematch(int num, kstat_t **ksparg, char *name, void *buf, int size){
   kstat_t *ks;
   kstat_t **ksp;
   kid_t new_kcid;
   int namelen;
   int count;
   int changed;
   char *cbuf;

   namelen = strlen(name);

   do {
      /* initialize before the scan */
      cbuf = (char *)buf;
      ksp = ksparg;
      count = 0;
      changed = 0;

      /* scan the chain for matching kstats */
      for (ks = kc->kc_chain; ks != NULL; ks = ks->ks_next) {
         if (strncmp(ks->ks_name, name, namelen) == 0) {
            /* this kstat matches: save it if there is room */
            if (count++ < num) {
               /* read the kstat */
               new_kcid = kstat_read(kc, ks, cbuf);

               /* if the chain changed, update it */
               if (new_kcid != kcid) {
                  changed = 1;
                  kcid = kstat_chain_update(kc);

                  /* there's no sense in continuing the scan */
                  /* so break out of the for loop */
                  break;
               }

               /* move to the next buffers */
               cbuf += size;
               *ksp++ = ks;
            }
         }
      }
   } while(changed);

   return count;
}

static kstat_t *ks_system_misc = NULL;

int kstat_safe_retrieve(kstat_t **ksp, char *module, int instance, char *name, void *buf){
   kstat_t *ks;
   kid_t new_kcid;
   int changed;

   ks = *ksp;
   do {
      changed = 0;
      /* if we dont already have the kstat, retrieve it */
      if (ks == NULL) {
         if ((ks = kstat_lookup(kc, module, instance, name)) == NULL) {
            return (-1);
         }
         *ksp = ks;
      }

      /* attempt to read it */
      new_kcid = kstat_read(kc, ks, buf);
      /* chance for an infinite loop here if kstat_read keeps 
         returning -1 */

      /* if the chain changed, update it */
      if (new_kcid != kcid) {
         changed = 1;
         kcid = kstat_chain_update(kc);
      }
   } while (changed);

   return (0);
}

int get_ncpus(){
   kstat_named_t *kn;
   int ret = -1;

   if ((kn = kstat_data_lookup(ks_system_misc, "ncpus")) != NULL) {
      ret = (int)(kn->value.ui32);
   }

   return ret;
}

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
   static kstat_t **cpu_ks = NULL;
   static cpu_stat_t *cpu_stat = NULL;
   static unsigned int nelems = 0;
   cpu_stat_t *cpu_stat_p;
   int i, cpu_num;

   if(!kstat_initialized) {
      /* open kstat */
      if ((kc = kstat_open()) == NULL) {
         fprintf(stderr, "Unable to open kstat.\n");
         return;
      }
      kcid = kc->kc_chain_id;
      kstat_safe_retrieve(&ks_system_misc, "unix", 0, "system_misc", NULL);
      kstat_initialized = 1;
   }

   while (nelems > 0 ?
         (cpu_num = kstat_safe_namematch(nelems,
                                         cpu_ks,
                                         "cpu_stat",
                                         cpu_stat,
                                         sizeof(cpu_stat_t))) > nelems :
         (cpu_num = get_ncpus()) > 0)
   {
      /* reallocate the arrays */
      nelems = cpu_num;
      if (cpu_ks != NULL) {
         free(cpu_ks);
      }
      cpu_ks = (kstat_t **)calloc(nelems, sizeof(kstat_t *));
      if (cpu_stat != NULL) {
         free(cpu_stat);
      }
      cpu_stat = (cpu_stat_t *)malloc(nelems * sizeof(cpu_stat_t));
   }
   cpu_stat_p = cpu_stat;

   unsigned long long int
       usertime = 0, nicetime = 0, systemtime = 0,
       systemalltime = 0, idlealltime = 0, idletime = 0,
       totaltime = 0, virtalltime = 0;

   for (int i = 0; i <= this->cpuCount; i++) {
      unsigned long long int ioWait, irq, softIrq, steal, guest;
      ioWait = irq = softIrq = steal = guest = 0;

      usertime = cpu_stat_p->cpu_sysinfo.cpu[CPU_USER];
      systemtime = cpu_stat_p->cpu_sysinfo.cpu[CPU_KERNEL];
      idletime = cpu_stat_p->cpu_sysinfo.cpu[CPU_IDLE];
      ioWait = cpu_stat_p->cpu_sysinfo.wait[W_IO] + cpu_stat_p->cpu_sysinfo.wait[W_PIO];

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

      cpu_stat_p++;
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
