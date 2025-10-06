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

#ifndef __KSYSTEM_H__
#define __KSYSTEM_H__

#include "platform.h"
#include "kprocess.h"

extern U32 screenCx;
extern U32 screenCy;

#define UID 1
#define GID 1000

#define MAX_STACK_SIZE (4*1024*1024)
#define MAX_ADDRESS_SPACE 0xFFFF0000
#define MAX_NUMBER_OF_FILES 0xFFF
#define MAX_DATA_SIZE 1024*1024*1024

#define CALL_BACK_ADDRESS 0xFFFF0000
#define SIG_RETURN_ADDRESS CALL_BACK_ADDRESS

void initSystem();
void initCallbacks();
void initCallbacksInProcess(struct KProcess* process);

// returns pid
U32 addProcess(struct KThread* thread, struct KProcess* process);
struct KProcess* getProcessById(struct KThread* thread, U32 pid);
void removeProcess(struct KThread* thread, struct KProcess* process);
U32 getProcessCount();
U32 syscall_uname(struct KThread* thread, U32 address);
U32 syscall_ugetrlimit(struct KThread* thread, U32 resource, U32 rlim);
U32 syscall_getrusuage(struct KThread* thread, U32 who, U32 usuage);
U32 syscall_clock_gettime(struct KThread* thread, U32 clock_id, U32 tp);
BOOL getNextProcess(U32* index, struct KProcess** process);

U32 getMilliesSinceStart();
U32 syscall_gettimeofday(struct KThread* thread, U32 tv, U32 tz);
U32 syscall_mincore(struct KThread* thread, U32 address, U32 length, U32 vec);
U32 syscall_times(struct KThread* thread, U32 buf);
U32 syscall_sysinfo(struct KThread* thread, U32 address);
U32 syscall_ioperm(struct KThread* thread, U32 from, U32 num, U32 turn_on);
U32 syscall_iopl(struct KThread* thread, U32 level);
void printStacks();
void syscallToString(struct CPU* cpu, char* buffer);
void runThreadSlice(struct KThread* thread);
void walkStack(struct CPU* cpu, U32 eip, U32 ebp, U32 indent);

#ifndef BOXEDWINE_64BIT_MMU
struct MappedFileCache {
    U32* ramPages;
    U32 pageCount;
    U32 refCount;
    char name[MAX_FILEPATH_LEN];
};

struct MappedFileCache* getMappedFileInCache(const char* name);
void putMappedFileInCache(struct MappedFileCache* file);
void removeMappedFileInCache(struct MappedFileCache* file);
#endif
#endif