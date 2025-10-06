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

#ifndef __KPROCESS_H__
#define __KPROCESS_H__

#include "platform.h"
#include "memory.h"
#include "kthread.h"
#include "karray.h"
#include "kfiledescriptor.h"
#include "fsapi.h"
#include "ktimer.h"
#include "kshm.h"
#include "pbl.h"

#ifdef BOXEDWINE_VM
#include <SDL.h>

extern SDL_mutex *mutexProcess;
extern SDL_cond *condProcessPid;
#endif

#define ADDRESS_PROCESS_MMAP_START		0xD0000
#define ADDRESS_PROCESS_NATIVE          0xE0000
#define ADDRESS_PROCESS_LOADER			0xF0000
#define ADDRESS_PROCESS_STACK_START		0xF4000
#define ADDRESS_PROCESS_FRAME_BUFFER	0xF8000
#define ADDRESS_PROCESS_FRAME_BUFFER_ADDRESS 0xF8000000

struct MapedFiles {
#ifndef BOXEDWINE_64BIT_MMU
    struct MappedFileCache* systemCacheEntry;
#endif
    struct KObject* file;
    U32 refCount;
    U32 address;
    U64 len;
    U64 offset;
};
#define MAX_MAPPED_FILE 1024
#define MAX_SIG_ACTIONS 64
#define MAX_PATHS 10
#define K_SIG_INFO_SIZE 10
#define MAX_COMMANDLINE_LEN 65536

struct KSigAction {
    U32 handlerAndSigAction;
    U64 mask;
    U32 flags;
    U32 restorer;
    U32 sigInfo[K_SIG_INFO_SIZE];
};

#define NUMBER_OF_STRINGS 5
#define STRING_GL_VENDOR 0
#define STRING_GL_RENDERER 1
#define STRING_GL_VERSION 2
#define STRING_GL_SHADING_LANGUAGE_VERSION 3
#define STRING_GL_EXTENSIONS 4

struct KProcess {
    U32 id;
    U32 parentId;
    U32 groupId;
    U32 userId;
    U32 effectiveUserId;
    U32 effectiveGroupId;
    U64 pendingSignals;
    U32 signaled;
    U32 exitCode;
    BOOL terminated;
    struct Memory* memory; 
    struct MapedFiles mappedFiles[MAX_MAPPED_FILE];
    char currentDirectory[MAX_FILEPATH_LEN];
    U32 brkEnd;
    struct KFileDescriptor** fds;
	U32 maxFds;
    struct KSigAction sigActions[MAX_SIG_ACTIONS];
    struct KTimer timer;
    char commandLine[MAX_COMMANDLINE_LEN];
    char exe[MAX_FILEPATH_LEN];
    char name[MAX_FILEPATH_LEN];
    struct FsOpenNodeFunc commandLineAccess;
    struct FsNode* commandLineNode;
    struct KArray threads;
    char path[MAX_PATHS][MAX_FILEPATH_LEN];
    U32 shms[MAX_SHM][MAX_SHM_ATTACH];
    struct KProcess* next;
    struct KThread* waitingThread;
    U32 strings[NUMBER_OF_STRINGS];
    U32 stringAddress;
    U32 stringAddressIndex;
    U32 loaderBaseAddress;
    U32 phdr;
    U32 phnum;
    U32 phentsize;
    U32 entry;
    U32 eventQueueFD;
    struct user_desc ldt[LDT_ENTRIES];
    U32 usedTLS[TLS_ENTRIES];
    struct KThread* wakeOnExitOrExec;
#ifdef BOXEDWINE_VM
    void** opToAddressPages[0x100000];
    U32* hostToEip[0x100000];
    SDL_mutex* threadsMutex;
    SDL_mutex* fdMutex;
#endif
};

void processOnExitThread(struct KThread* thread);
struct KThread* startProcess(const char* currentDirectory, U32 argc, const char** args, U32 envc, const char** env, int userId, int groupId, int effectiveUserId, int effectiveGroupId);
void freeProcess(struct KProcess* process);
struct KFileDescriptor* getFileDescriptor(struct KProcess* process, FD handle);
struct KFileDescriptor* openFile(struct KProcess* process, const char* currentDirectory, const char* localPath, U32 accessFlags);
U32 syscall_waitpid(struct KThread* thread, S32 pid, U32 status, U32 options);
BOOL isProcessStopped(struct KProcess* process);
BOOL isProcessTerminated(struct KProcess* process);
struct FsNode* getNode(struct KThread* thread, U32 fileName);
const char* getModuleName(struct CPU* cpu, U32 eip);
U32 getModuleEip(struct CPU* cpu, U32 eip);
U32 getNextFileDescriptorHandle(struct KProcess* process, int after);
//void signalProcess(struct KProcess* process, U32 signal);
void signalIO(struct KThread* thread, struct KProcess* process, U32 code, S32 band, FD fd);
void signalCHLD(struct KThread* thread, struct KProcess* process, U32 code, U32 childPid, U32 sendingUID, S32 exitCode);
void signalALRM(struct KThread* thread, struct KProcess* process);
void signalIllegalInstruction(struct KThread* thread, int code);
void closeMemoryMapped(struct MapedFiles* mapped);

#ifdef BOXEDWINE_64BIT_MMU
void allocNativeMemory(struct Memory* memory, U32 page, U32 pageCount, U32 flags);
void freeNativeMemory(struct KProcess* process, U32 page, U32 pageCount);
#endif

// returns tid
U32 processAddThread(struct KThread* currentThread, struct KProcess* process, struct KThread* thread);
void processRemoveThread(struct KThread* currentThread, struct KProcess* process, struct KThread* thread);
struct KThread* processGetThreadById(struct KThread* currentThread, struct KProcess* process, U32 tid);
U32 processGetThreadCount(struct KProcess* process);
struct user_desc* getLDT(struct KThread* thread, U32 index);
U32 isLdtEmpty(struct user_desc* desc);

U32 syscall_getcwd(struct KThread* thread, U32 buffer, U32 size);
U32 syscall_clone(struct KThread* thread, U32 flags, U32 child_stack, U32 ptid, U32 tls, U32 ctid);
U32 syscall_alarm(struct KThread* thread, U32 seconds);
U32 syscall_getpgid(struct KThread* thread, U32 pid);
U32 syscall_setpgid(struct KThread* thread, U32 pid, U32 gpid);
U32 syscall_execve(struct KThread* thread, U32 path, U32 argv, U32 envp);
U32 syscall_chdir(struct KThread* thread, U32 path);
U32 syscall_exitgroup(struct KThread* thread, U32 code);
U32 syscall_setitimer(struct KThread* thread, U32 which, U32 newValue, U32 oldValue);
U32 syscall_prlimit64(struct KThread* thread, U32 pid, U32 resource, U32 newlimit, U32 oldlimit);
U32 syscall_prlimit(struct KThread* thread, U32 pid, U32 resource, U32 newlimit, U32 oldlimit);
U32 syscall_fchdir(struct KThread* thread, FD fd);
U32 syscall_prctl(struct KThread* thread, U32 option);
U32 syscall_tgkill(struct KThread* thread, U32 threadGroupId, U32 threadId, U32 signal);
U32 syscall_kill(struct KThread* thread, U32 pid, U32 signal);

void runProcessTimer(struct KTimer* timer);
void addString(struct KThread* thread, U32 index, const char* str);

#endif