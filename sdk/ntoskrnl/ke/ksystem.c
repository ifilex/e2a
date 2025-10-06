/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "winebox.h"
#include "ksystem.h"
#include "karray.h"
#include "log.h"
#include "kalloc.h"
//#include "pbl.h"
#include "khashmap.h"
#include "kprocess.h"
#include "bufferaccess.h"
#include "kstat.h"
#include "kio.h"
#include "fsapi.h"
#include "kscheduler.h"

#include <time.h>

#ifdef BOXEDWINE_VM

SDL_mutex *mutexProcess;
SDL_cond *condProcessPid;

#endif

U32 screenCx = 800;
U32 screenCy = 600;

static struct KArray processes;
//static PblMap* mappedFileCache;
static struct KHashmap mappedFileCache;

void initSystem() {
    initArray(&processes, 100);		
    //mappedFileCache = pblMapNewHashMap();
    initHashmap(&mappedFileCache);

#ifdef BOXEDWINE_VM
    mutexProcess = SDL_CreateMutex();
    condProcessPid = SDL_CreateCond();
#endif
}

#ifndef BOXEDWINE_64BIT_MMU
struct MappedFileCache* getMappedFileInCache(const char* name) {
    return (struct MappedFileCache*)getHashmapValue(&mappedFileCache, name);
    //struct MappedFileCache** result = pblMapGet(mappedFileCache, (void*)name, strlen(name), NULL);
    //if (result)
    //	return *result;
    //return NULL;
}

void putMappedFileInCache(struct MappedFileCache* file) {
    putHashmapValue(&mappedFileCache, file->name, file);
    //pblMapAdd(mappedFileCache, file->name, strlen(file->name), &file, sizeof(struct MappedFileCache*));
}

void removeMappedFileInCache(struct MappedFileCache* file) {
    removeHashmapKey(&mappedFileCache, file->name);
    //pblMapRemove(mappedFileCache, file->name, strlen(file->name), NULL);
}
#endif
U32 addProcess(struct KThread* thread, struct KProcess* process) {
    BOOL result;
    BOXEDWINE_LOCK(thread, mutexProcess);
    result = addObjecToArray(&processes, process);
    BOXEDWINE_UNLOCK(thread, mutexProcess);
    return result;
}

void removeProcess(struct KThread* thread, struct KProcess* process) {
    BOXEDWINE_LOCK(thread, mutexProcess);
    removeObjectFromArray(&processes, process->id);
    BOXEDWINE_UNLOCK(thread, mutexProcess);
}

struct KProcess* getProcessById(struct KThread* thread, U32 pid) {
    struct KProcess* result;
    BOXEDWINE_LOCK(thread, mutexProcess);
    result = (struct KProcess*)getObjectFromArray(&processes, pid);
    BOXEDWINE_UNLOCK(thread, mutexProcess);
    return result;
}

BOOL getNextProcess(U32* index, struct KProcess** process) {
    return getNextObjectFromArray(&processes, index, (void**)process);
}

U32 getProcessCount() {
    return getArrayCount((&processes));
}


U32 syscall_uname(struct KThread* thread, U32 address) {
    writeNativeString(thread, address, "Linux");
    writeNativeString(thread, address + 65, "Linux");
    writeNativeString(thread, address + 130, "4.4.0-21-generic");
    writeNativeString(thread, address + 260, "i686");
    return 0;
}

U32 syscall_ugetrlimit(struct KThread* thread, U32 resource, U32 rlim) {
    switch (resource) {
        case 2: // RLIMIT_DATA
            writed(thread, rlim, MAX_DATA_SIZE);
            writed(thread, rlim + 4, MAX_DATA_SIZE);
            break;
        case 3: // RLIMIT_STACK
            writed(thread, rlim, MAX_STACK_SIZE);
            writed(thread, rlim + 4, MAX_STACK_SIZE);
            break;
        case 4: // RLIMIT_CORE
            writed(thread, rlim, 1024 * 1024 * 4);
            writed(thread, rlim + 4, 1024 * 1024 * 4);
            break;
        case 5: // RLIMIT_DATA
            writed(thread, rlim, MAX_DATA_SIZE);
            writed(thread, rlim + 4, MAX_DATA_SIZE);
            break;
        case 6: // RLIMIT_MEMLOCK
            writed(thread, rlim, 64 * 1024 * 1024);
            writed(thread, rlim + 4, 64 * 1024 * 1024);
            break;
        case 7: // RLIMIT_NOFILE
            writed(thread, rlim, MAX_NUMBER_OF_FILES);
            writed(thread, rlim + 4, MAX_NUMBER_OF_FILES);
            break;
        case 9: // RLIMIT_AS
            writed(thread, rlim, MAX_ADDRESS_SPACE);
            writed(thread, rlim + 4, MAX_ADDRESS_SPACE);
            break;
        default:
            kpanic("sys call __NR_ugetrlimit resource %d not implemented", resource);
    }
    return 0;
}

U32 syscall_getrusuage(struct KThread* thread, U32 who, U32 usuage) {
    if (who==0) { // RUSAGE_SELF
        // user time
        writed(thread, usuage, (U32)(thread->userTime / 1000000l));
        writed(thread, usuage + 4, (U32)(thread->userTime % 1000000l));
        // system time
        writed(thread, usuage + 8, (U32)(thread->kernelTime / 1000000l));
        writed(thread, usuage + 12, (U32)(thread->kernelTime % 1000000l));
    }
    return 0;
}

U32 syscall_times(struct KThread* thread, U32 buf) {
    if (buf) {
        writed(thread, buf, (U32)thread->userTime * 10); // user time
        writed(thread, buf + 4, (U32)thread->kernelTime * 10); // system time
        writed(thread, buf + 8, 0); // user time of children
        writed(thread, buf + 12, 0); // system time of children
    }
    return (U32)getMicroCounter()*10;
}

U32 syscall_clock_gettime(struct KThread* thread, U32 clock_id, U32 tp) {    
    if (clock_id==0) { // CLOCK_REALTIME
        U64 m = getSystemTimeAsMicroSeconds();
        writed(thread, tp, (U32)(m / 1000000l));
        writed(thread, tp + 4, (U32)(m % 1000000l) * 1000);
    } else if (clock_id==1 || clock_id==2 || clock_id==4 || clock_id==6) { // CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID , CLOCK_MONOTONIC_COARSE
        U64 diff = getMicroCounter();
        writed(thread, tp, (U32)(diff / 1000000l));
        writed(thread, tp + 4, (U32)(diff % 1000000l) * 1000);
    } else {
        kpanic("Unknown clock id for clock_gettime: %d",clock_id);
    }
    return 0;
}

U32 syscall_gettimeofday(struct KThread* thread, U32 tv, U32 tz) {
    U64 m = getSystemTimeAsMicroSeconds();
    
    writed(thread, tv, (U32)(m / 1000000l));
    writed(thread, tv + 4, (U32)(m % 1000000l));
    return 0;
}

U32 syscall_mincore(struct KThread* thread, U32 address, U32 length, U32 vec) {
    U32 i;
    U32 pages = (length+PAGE_SIZE+1)/PAGE_SIZE;
    U32 page = address >> PAGE_SHIFT;

    for (i=0;i<pages;i++) {
        if (isPageInMemory(thread->process->memory, page+i))
            writeb(thread, vec, 1);
        else
            writeb(thread, vec, 0);
        vec++;
    }
    return 0;
}

/*
 struct sysinfo {
    long uptime;             // Seconds since boot
    unsigned long loads[3];  // 1, 5, and 15 minute load averages
    unsigned long totalram;  // Total usable main memory size
    unsigned long freeram;   // Available memory size
    unsigned long sharedram; // Amount of shared memory
    unsigned long bufferram; // Memory used by buffers
    unsigned long totalswap; // Total swap space size
    unsigned long freeswap;  // Swap space still available
    unsigned short procs;    // Number of current processes
    unsigned long totalhigh; // Total high memory size
    unsigned long freehigh;  // Available high memory size
    unsigned int mem_unit;   // Memory unit size in bytes
    char _f[20-2*sizeof(long)-sizeof(int)]; // Padding to 64 bytes
};
*/

U32 syscall_sysinfo(struct KThread* thread, U32 address) {
    writed(thread, address, getMilliesSinceStart()/1000); address+=4;
    writed(thread, address, 0); address+=4;
    writed(thread, address, 0); address+=4;
    writed(thread, address, 0); address+=4;
    writed(thread, address, getPageCount()); address+=4;
    writed(thread, address, getFreePageCount()); address+=4;
    writed(thread, address, 0); address+=4;
    writed(thread, address, 0); address+=4;
    writed(thread, address, 0); address+=4;
    writed(thread, address, 0); address+=4;
    writew(thread, address, getProcessCount()); address+=2;
    writed(thread, address, 0); address+=4;
    writed(thread, address, 0); address+=4;
    writed(thread, address, PAGE_SIZE);
    return 0;
}

U32 syscall_ioperm(struct KThread* thread, U32 from, U32 num, U32 turn_on) {
    return 0;
}

U32 syscall_iopl(struct KThread* thread, U32 level) {
    return 0;
}

const char* getFunctionName(const char* name, U32 moduleEip) {
#ifdef BOXEDWINE_VM
    return "";
#else
    struct KThread* thread;
    struct KProcess* process;
    const char* args[5];
    char tmp[16];
    struct FsOpenNodeFunc out = {0};
    struct FsNode* node;
    static char buffer[1024];
    struct KFileDescriptor* fd;
    int i;

    memset(buffer, 0, 1024);
    if (!name)
        return "Unknown";
    sprintf(tmp, "%X", moduleEip);
    args[0] = "/usr/bin/addr2line";
    args[1] = "-e";
    args[2] = name;
    args[3] = "-f";
    args[4] = tmp;
    thread = startProcess("/usr/bin", 5, args, 0, NULL, 0, 0, 0, 0);
    if (!thread)
        return "";
    makeBufferAccess(&out);
    out.data = buffer;
    out.dataLen = 1024;
    node = addVirtualFile("/dev/tty9", &out, K__S_IWRITE, (4<<8) | 9);
    process = thread->process;
    fd = openFile(process, "", "/dev/tty9", K_O_WRONLY); 
    if (fd) {
        syscall_dup2(thread, fd->handle, 1); // replace stdout without tty9    
        while (!process->terminated) {
            runThreadSlice(thread);
        }
    }
    removeProcess(thread, process);
    freeProcess(process);
    removeNodeFromCache(node);
    kfree(node, KALLOC_NODE);
    for (i=0;i<sizeof(buffer);i++) {
        if (buffer[i]==10 || buffer[i]==13) {
            buffer[i]=0;
            break;
        }
    }
    return buffer;
#endif
}
void walkStack(struct CPU* cpu, U32 eip, U32 ebp, U32 indent) {
    U32 prevEbp;
    U32 returnEip;
    U32 moduleEip = getModuleEip(cpu, cpu->segAddress[CS]+eip);
    const char* name = getModuleName(cpu, cpu->segAddress[CS]+eip);
    const char* functionName = getFunctionName(name, moduleEip);
    const char* n = name;
    
    if (n)
        n = strrchr(n, '/');
    if (n)
        name = n+1;

    klog("%*s %-20s %-40s %08x / %08x", indent, "", name?name:"Unknown", functionName, eip, moduleEip);
    
    if (isValidReadAddress(cpu->thread, ebp)) {
        prevEbp = readd(cpu->thread, ebp); 
        returnEip = readd(cpu->thread, ebp+4); 
        if (prevEbp==0)
            return;
        walkStack(cpu, returnEip, prevEbp, indent);
    }
}

void printStacks() {
    U32 index=0;
    struct KProcess* process=0;

    BOXEDWINE_LOCK(NULL, mutexProcess);
    while (getNextProcess(&index, &process)) {
        U32 threadIndex = 0;
        struct KThread* thread = 0;

        if (process) {
            klog("process %X %s%s", process->id, process->terminated?"TERMINATED ":"", process->commandLine);
            while (getNextObjectFromArray(&process->threads, &threadIndex, (void**)&thread)) {
                if (thread) {
                    struct CPU* cpu=&thread->cpu;

                    klog("  thread %X %s", thread->id, (IS_THREAD_WAITING(thread)?"WAITING":"RUNNING"));
                    if (IS_THREAD_WAITING(thread)) {
                        char buffer[1024];
                        syscallToString(&thread->cpu, buffer);
                        klog("    %s", buffer);                        
                    } else {
                        const char* name = getModuleName(cpu, cpu->segAddress[CS]+cpu->eip.u32);

                        klog("    0x%08d %s", getModuleEip(cpu, cpu->segAddress[CS]+cpu->eip.u32), name?name:"Unknown");
                    }
                    walkStack(cpu, cpu->eip.u32, EBP, 6);
                }
            }
        }
    }
    BOXEDWINE_LOCK(NULL, mutexProcess);
}