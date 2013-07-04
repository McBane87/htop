#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

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

