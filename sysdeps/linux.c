#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "CRT.h"
#include "String.h"
#include "htop-sysdeps.h"

IOPriority sysdep_get_ioprio(pid_t p)
{
    IOPriority ioprio = syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, p);
    return ioprio;
}

void sysdep_set_ioprio(pid_t p, IOPriority ioprio)
{
    syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, p);
}

void sysdep_get_meminfo(ProcessList *this)
{
   unsigned long long int swapFree = 0;

   FILE* file = fopen(PROCMEMINFOFILE, "r");
   if (file == NULL) {
      CRT_fatalError("Cannot open " PROCMEMINFOFILE);
   }
   {
      char buffer[128];
      while (fgets(buffer, 128, file)) {

         switch (buffer[0]) {
         case 'M':
            if (String_startsWith(buffer, "MemTotal:"))
               sscanf(buffer, "MemTotal: %llu kB", &this->totalMem);
            else if (String_startsWith(buffer, "MemFree:"))
               sscanf(buffer, "MemFree: %llu kB", &this->freeMem);
            else if (String_startsWith(buffer, "MemShared:"))
               sscanf(buffer, "MemShared: %llu kB", &this->sharedMem);
            break;
         case 'B':
            if (String_startsWith(buffer, "Buffers:"))
               sscanf(buffer, "Buffers: %llu kB", &this->buffersMem);
            break;
         case 'C':
            if (String_startsWith(buffer, "Cached:"))
               sscanf(buffer, "Cached: %llu kB", &this->cachedMem);
            break;
         case 'S':
            if (String_startsWith(buffer, "SwapTotal:"))
               sscanf(buffer, "SwapTotal: %llu kB", &this->totalSwap);
            if (String_startsWith(buffer, "SwapFree:"))
               sscanf(buffer, "SwapFree: %llu kB", &swapFree);
            break;
         }
      }
   }

   this->usedMem = this->totalMem - this->freeMem;
   this->usedSwap = this->totalSwap - swapFree;
   fclose(file);
}


void sysdep_update_cpu_data(ProcessList *this)
{
   unsigned long long int usertime, nicetime, systemtime, systemalltime, idlealltime, idletime, totaltime, virtalltime;

   FILE *file = fopen(PROCSTATFILE, "r");
   if (file == NULL) {
      CRT_fatalError("Cannot open " PROCSTATFILE);
   }
   for (int i = 0; i <= this->cpuCount; i++) {
      char buffer[256];
      int cpuid;
      unsigned long long int ioWait, irq, softIrq, steal, guest;
      ioWait = irq = softIrq = steal = guest = 0;
      // Dependending on your kernel version,
      // 5, 7 or 8 of these fields will be set.
      // The rest will remain at zero.
      fgets(buffer, 255, file);
      if (i == 0)
         sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                 &usertime, &nicetime, &systemtime, &idletime, &ioWait, &irq, &softIrq, &steal, &guest);
      else {
         sscanf(buffer, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu",
         &cpuid, &usertime, &nicetime, &systemtime, &idletime, &ioWait, &irq, &softIrq, &steal, &guest);
         assert(cpuid == i - 1);
      }
      // Fields existing on kernels >= 2.6
      // (and RHEL's patched kernel 2.4...)
      idlealltime = idletime + ioWait;
      systemalltime = systemtime + irq + softIrq;
      virtalltime = steal + guest;
      totaltime = usertime + nicetime + systemalltime + idlealltime + virtalltime;
      CPUData* cpuData = &(this->cpus[i]);
      assert (usertime >= cpuData->userTime);
      assert (nicetime >= cpuData->niceTime);
      assert (systemtime >= cpuData->systemTime);
      assert (idletime >= cpuData->idleTime);
      assert (totaltime >= cpuData->totalTime);
      assert (systemalltime >= cpuData->systemAllTime);
      assert (idlealltime >= cpuData->idleAllTime);
      assert (ioWait >= cpuData->ioWaitTime);
      assert (irq >= cpuData->irqTime);
      assert (softIrq >= cpuData->softIrqTime);
      assert (steal >= cpuData->stealTime);
      assert (guest >= cpuData->guestTime);
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
   fclose(file);
}

int sysdep_max_pid()
{
   int maxPid = 4194303;
   FILE* file = fopen(PROCDIR "/sys/kernel/pid_max", "r");
   if (file) {
       fscanf(file, "%32d", &maxPid);
       fclose(file);
   }
   return maxPid;
}
