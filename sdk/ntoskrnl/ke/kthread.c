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
#include "kthread.h"
#include "kscheduler.h"
#include "kprocess.h"
#include "log.h"
#include "ksystem.h"
#include "kerror.h"
#include "ksignal.h"
#include "kalloc.h"
#include "kmmap.h"
#include "kscheduler.h"
#ifdef BOXEDWINE_VM
#include "../emulation/x64dynamic/x64.h"
#endif
#include <string.h>

struct KThread* freeThreads;

struct KThread* allocThread() {
    if (freeThreads) {
        struct KThread* result = freeThreads;
        freeThreads = freeThreads->nextFreeThread;
        memset(result, 0, sizeof(struct KThread));
        return result;
    }
    return (struct KThread*)kalloc(sizeof(struct KThread), KALLOC_KTHREAD);		
}

void freeThread(struct KThread* currentThread, struct KThread* thread) {
    processRemoveThread(currentThread, thread->process, thread);    	
    if (thread->waitingForSignalToEnd) {
        wakeThread(currentThread, thread->waitingForSignalToEnd);
        thread->waitingForSignalToEnd = 0;
    }    
    threadClearFutexes(thread);
    releaseMemory(thread, thread->stackPageStart, thread->stackPageCount);
    processOnExitThread(thread);
    thread->nextFreeThread = freeThreads;	
    freeThreads = thread;
    unscheduleThread(currentThread, thread); // will not return if thread is current thread and BOXEDWINE_VM
}

void setupStack(struct KThread* thread) {
    U32 page = 0;
    U32 pageCount = MAX_STACK_SIZE >> PAGE_SHIFT; // 1MB for max stack
    pageCount+=2; // guard pages
    if (!findFirstAvailablePage(thread->cpu.memory, ADDRESS_PROCESS_STACK_START, pageCount, &page, 0))
		if (!findFirstAvailablePage(thread->cpu.memory, 0xC0000, pageCount, &page, 0))
			if (!findFirstAvailablePage(thread->cpu.memory, 0x80000, pageCount, &page, 0))
				kpanic("Failed to allocate stack for thread");
    allocPages(thread, page+1, pageCount-2, PAGE_READ|PAGE_WRITE, 0, 0, 0);
    // 1 page above (catch stack underrun)
    allocPages(thread, page+pageCount-1, 1, 0, 0, 0, 0);
    // 1 page below (catch stack overrun)
    allocPages(thread, page, 1, 0, 0, 0, 0);
    thread->stackPageCount = pageCount;
    thread->stackPageStart = page;
    thread->cpu.reg[4].u32 = (thread->stackPageStart + thread->stackPageCount - 1) << PAGE_SHIFT; // one page away from the top
    thread->cpu.thread = thread;
}

void initThread(struct KThread* currentThread, struct KThread* thread, struct KProcess* process) {
    int i;

    memset(thread, 0, sizeof(struct KThread));	
    initCPU(&thread->cpu, process->memory);	
    thread->process = process;
    thread->id = processAddThread(currentThread, process, thread);
    thread->sigMask = 0;
    for (i=0;i<TLS_ENTRIES;i++) {
        thread->tls[i].seg_not_present = 1;
        thread->tls[i].read_exec_only = 1;
    }
    setupStack(thread);
}

void cloneThread(struct KThread* thread, struct KThread* from, struct KProcess* process) {
    memset(thread, 0, sizeof(struct KThread));
    memcpy(&thread->cpu, &from->cpu, sizeof(struct CPU));
#ifdef BOXEDWINE_VM
#ifdef _DEBUG
    thread->cpu.lastRSP = 0;
#endif
#endif
    onCreateCPU(&thread->cpu); // sets up the 8-bit high low regs
    thread->cpu.nextBlock = 0;
    thread->cpu.thread = thread;	
#ifdef INCLUDE_CYCLES
    thread->cpu.blockCounter = 0;
#else
    thread->cpu.yield = FALSE;
#endif
    thread->process = process;
    thread->sigMask = from->sigMask;
    thread->cpu.memory = process->memory;
    thread->stackPageStart = from->stackPageStart;
    thread->stackPageCount = from->stackPageCount;
    thread->waitingForSignalToEndMaskToRestore = thread->waitingForSignalToEndMaskToRestore;
    thread->id = processAddThread(from, process, thread);
}

void exitThread(struct KThread* thread, U32 status) {
    if (thread->clear_child_tid) {
        writed(thread, thread->clear_child_tid, 0);
        syscall_futex(thread, thread->clear_child_tid, 1, 1, 0);
    }
    freeThread(thread, thread);
}

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define FUTEX_WAIT_PRIVATE 128
#define FUTEX_WAKE_PRIVATE 129

struct futex {
    struct KThread* thread;
    U8* address;
#ifdef BOXEDWINE_VM    
    SDL_cond* cond;    
    BOOL pendingWakeUp;
#else    
    U32 expireTimeInMillies;
    BOOL wake;
#endif
};

#define MAX_FUTEXES 128

struct futex system_futex[MAX_FUTEXES];
#ifdef BOXEDWINE_VM
SDL_mutex* mutexFutex;
#endif

struct futex* getFutex(struct KThread* thread, U8* address) {
    int i=0;

    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].address == address && system_futex[i].thread==thread)
            return &system_futex[i];
    }
    return 0;
}

struct futex* allocFutex(struct KThread* thread, U8* address, U32 millies) {
    int i=0;

    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].thread==0) {
            system_futex[i].thread = thread;
            system_futex[i].address = address;
#ifdef BOXEDWINE_VM
            if (!system_futex[i].cond) {
                system_futex[i].cond = SDL_CreateCond();
            }
#else
            system_futex[i].expireTimeInMillies = millies;
            system_futex[i].wake = FALSE;
#endif
            return &system_futex[i];
        }
    }
    kpanic("ran out of futexes");
    return 0;
}

void freeFutex(struct futex* f) {
    f->thread = 0;
    f->address = 0;
#ifdef BOXEDWINE_VM
    f->pendingWakeUp = 0;
#endif
}

void threadClearFutexes(struct KThread* thread) {
    U32 i;

    BOXEDWINE_LOCK(thread, mutexFutex);  
    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].thread == thread) {
            freeFutex(&system_futex[i]);
        }
    }
    BOXEDWINE_UNLOCK(thread, mutexFutex);  
}

U32 syscall_futex(struct KThread* thread, U32 addr, U32 op, U32 value, U32 pTime) {
    U8* ramAddress = getPhysicalAddress(thread, addr);

#ifdef BOXEDWINE_VM
    if (!mutexFutex)
        mutexFutex = SDL_CreateMutex();
    BOXEDWINE_LOCK(thread, mutexFutex);    
    if (op==FUTEX_WAIT || op==FUTEX_WAIT_PRIVATE) {
        struct futex* f;
        U32 result = 0;

        if (readd(thread, addr) != value) {
            BOXEDWINE_UNLOCK(thread, mutexFutex);
            return -K_EWOULDBLOCK;
        }
        f=getFutex(thread, ramAddress);
        
        if (!f) {
            f = allocFutex(thread, ramAddress, 0);            
        }
        if (pTime) {
            U32 seconds = readd(thread, pTime);
            U32 nano = readd(thread, pTime + 4);
            U32 millies = seconds * 1000 + nano / 1000000;
            if (BOXEDWINE_WAIT_TIMEOUT(thread, f->cond, mutexFutex, millies)!=0) {
                result = -K_ETIMEDOUT;
            }            
        } else {
            BOXEDWINE_WAIT(thread, f->cond, mutexFutex);            
        }
        freeFutex(f);
        BOXEDWINE_UNLOCK(thread, mutexFutex);   
        return result;
    } else if (op==FUTEX_WAKE_PRIVATE || op==FUTEX_WAKE) {
        int i;
        U32 count = 0;
        for (i=0;i<MAX_FUTEXES && count<value;i++) {
            if (system_futex[i].address==ramAddress && !system_futex[i].pendingWakeUp) {
                system_futex[i].pendingWakeUp = TRUE;
                wakeThread(thread, system_futex[i].thread);                
                count++;
            }
        }
        BOXEDWINE_UNLOCK(thread, mutexFutex);
        return count;
    } else {
        kwarn("syscall __NR_futex op %d not implemented", op);
        BOXEDWINE_UNLOCK(thread, mutexFutex);
        return -1;
    }
#else
    if (op==FUTEX_WAIT || op==FUTEX_WAIT_PRIVATE) {
        struct futex* f=getFutex(thread, ramAddress);
        U32 millies;

        if (f) {
            if (f->wake) {
                freeFutex(f);
                return 0;
            }
            if (f->expireTimeInMillies<=getMilliesSinceStart()) {
                freeFutex(f);
                return -K_ETIMEDOUT;
            }
            thread->timer.process = thread->process;
            thread->timer.thread = thread;
            thread->timer.millies = f->expireTimeInMillies;
            if (f->expireTimeInMillies<0xF0000000)
                thread->timer.millies+=thread->waitStartTime;
            addTimer(&thread->timer);
            return -K_WAIT;
        }
        if (pTime == 0) {
            millies = 0xFFFFFFFF;
        } else {
            U32 seconds = readd(thread, pTime);
            U32 nano = readd(thread, pTime + 4);
            millies = seconds * 1000 + nano / 1000000 + getMilliesSinceStart();
        }
        if (readd(thread, addr) != value) {
            return -K_EWOULDBLOCK;
        }
        f = allocFutex(thread, ramAddress, millies);
        thread->waitStartTime = getMilliesSinceStart();			
        thread->timer.process = thread->process;
        thread->timer.thread = thread;
        thread->timer.millies = f->expireTimeInMillies;
        if (f->expireTimeInMillies<0xF0000000)
            thread->timer.millies+=thread->waitStartTime;
        addTimer(&thread->timer);
        return -K_WAIT;
    } else if (op==FUTEX_WAKE_PRIVATE || op==FUTEX_WAKE) {
        int i;
        U32 count = 0;
        for (i=0;i<MAX_FUTEXES && count<value;i++) {
            if (system_futex[i].address==ramAddress && !system_futex[i].wake) {
                system_futex[i].wake = TRUE;
                wakeThread(thread, system_futex[i].thread);				
                count++;
            }
        }
        return count;
    } else {
        kwarn("syscall __NR_futex op %d not implemented", op);
        return -1;
    }
#endif
}

BOOL runSignals(struct KThread* currentThread, struct KThread* thread) {
    U64 todo = thread->process->pendingSignals & ~(thread->inSignal?thread->inSigMask:thread->sigMask);
    todo |= thread->pendingSignals & ~(thread->inSignal?thread->inSigMask:thread->sigMask);

    if (todo!=0) {
        U32 i;

        for (i=0;i<32;i++) {
            if ((todo & ((U64)1 << i))!=0) {
                runSignal(currentThread, thread, i+1, -1, 0);
                return 1;
            }
        }
    }	
    return 0;
}
/*
typedef union compat_sigval {
        S32    sival_int;
        U32    sival_ptr;
} compat_sigval_t;

typedef struct compat_siginfo {
        S32 si_signo;
        S32 si_errno;
        S32 si_code;

        union {
                S32 _pad[29];

                // kill() 
                struct {
                        U32 _pid;      // sender's pid
                        U32 _uid;      // sender's uid
                } _kill;

                // POSIX.1b timers 
                struct {
                        S32 _tid;    // timer id 
                        S32 _overrun;           // overrun count 
                        compat_sigval_t _sigval;        // same as below 
                        S32 _sys_private;       // not to be passed to user 
                        S32 _overrun_incr;      // amount to add to overrun 
                } _timer;

                // POSIX.1b signals 
                struct {
                        U32 _pid;      // sender's pid 
                        U32 _uid;      // sender's uid 
                        compat_sigval_t _sigval;
                } _rt;

                // SIGCHLD 
                struct {
                        U32 _pid;      // which child 
                        U32 _uid;      // sender's uid
                        S32 _status;   // exit code 
                        S32 _utime;
                        S32 _stime;
                } _sigchld;

                // SIGCHLD (x32 version) 
                struct {
                        U32 _pid;      // which child
                        U32 _uid;      // sender's uid
                        S32 _status;   // exit code 
                        S64 _utime;
                        S64 _stime;
                } _sigchld_x32;

                // SIGILL, SIGFPE, SIGSEGV, SIGBUS 
                struct {
                        U32 _addr;     // faulting insn/memory ref. 
                } _sigfault;

                // SIGPOLL 
                struct {
                        S32 _band;      // POLL_IN, POLL_OUT, POLL_MSG 
                        S32 _fd;
                } _sigpoll;

                struct {
                        U32 _call_addr; // calling insn 
                        S32 _syscall;   // triggering system call number 
                        U32 _arch;     // AUDIT_ARCH_* of syscall 
                } _sigsys;
        } _sifields;
} compat_siginfo_t;
*/
typedef struct fpregset
  {
    union
      {
        struct fpchip_state
          {
            int state[27];
            int status;
          } fpchip_state;

        struct fp_emul_space
          {
            char fp_emul[246];
            char fp_epad[2];
          } fp_emul_space;

        int f_fpregs[62];
      } fp_reg_set;

    long int f_wregs[33];
  } fpregset_t;


// Number of general registers. 
#define NGREG   19

enum
{
  REG_GS = 0,
#define REG_GS  REG_GS
  REG_FS,
#define REG_FS  REG_FS
  REG_ES,
#define REG_ES  REG_ES
  REG_DS,
#define REG_DS  REG_DS
  REG_EDI,
#define REG_EDI REG_EDI
  REG_ESI,
#define REG_ESI REG_ESI
  REG_EBP,
#define REG_EBP REG_EBP
  REG_ESP,
#define REG_ESP REG_ESP
  REG_EBX,
#define REG_EBX REG_EBX
  REG_EDX,
#define REG_EDX REG_EDX
  REG_ECX,
#define REG_ECX REG_ECX
  REG_EAX,
#define REG_EAX REG_EAX
  REG_TRAPNO,
#define REG_TRAPNO      REG_TRAPNO
  REG_ERR,
#define REG_ERR REG_ERR
  REG_EIP,
#define REG_EIP REG_EIP
  REG_CS,
#define REG_CS  REG_CS
  REG_EFL,
#define REG_EFL REG_EFL
  REG_UESP,
#define REG_UESP        REG_UESP
  REG_SS
#define REG_SS  REG_SS
};

// Container for all general registers. 
typedef S32 gregset_t[NGREG];

// Context to describe whole processor state. 
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
  } mcontext_tt;

typedef struct sigaltstack {
        void *ss_sp;
        int ss_flags;
        S32 ss_size;
} stack_tt;

# define K_SIGSET_NWORDS (1024 / 32)
typedef struct
{
unsigned long int __val[K_SIGSET_NWORDS];
} k__sigset_t;


// Userlevel context. 
struct ucontext_ia32 {
        unsigned int      uc_flags;        // 0
        unsigned int      uc_link;         // 4
        stack_tt           uc_stack;        // 8
        mcontext_tt uc_mcontext;			   // 20
        k__sigset_t   uc_sigmask;   /* mask last for extensibility */
};


  
#define INFO_SIZE 128
#define CONTEXT_SIZE 128

void writeToContext(struct KThread* thread, U32 stack, U32 context, BOOL altStack, U32 trapNo, U32 errorNo) {	
    struct CPU* cpu = &thread->cpu;

    if (altStack) {
        writed(thread, context+0x8, thread->alternateStack);
        writed(thread, context+0xC, K_SS_ONSTACK);
        writed(thread, context+0x10, thread->alternateStackSize);
    } else {
        writed(thread, context+0x8, thread->alternateStack);
        writed(thread, context+0xC, K_SS_DISABLE);
        writed(thread, context+0x10, 0);
    }
    writed(thread, context+0x14, cpu->segValue[GS]);
    writed(thread, context+0x18, cpu->segValue[FS]);
    writed(thread, context+0x1C, cpu->segValue[ES]);
    writed(thread, context+0x20, cpu->segValue[DS]);
    writed(thread, context+0x24, cpu->reg[7].u32); // EDI
    writed(thread, context+0x28, cpu->reg[6].u32); // ESI
    writed(thread, context+0x2C, cpu->reg[5].u32); // EBP
    writed(thread, context+0x30, stack); // ESP
    writed(thread, context+0x34, cpu->reg[3].u32); // EBX
    writed(thread, context+0x38, cpu->reg[2].u32); // EDX
    writed(thread, context+0x3C, cpu->reg[1].u32); // ECX
    writed(thread, context+0x40, cpu->reg[0].u32); // EAX
    writed(thread, context+0x44, trapNo); // REG_TRAPNO
    writed(thread, context+0x48, errorNo); // REG_ERR
    writed(thread, context+0x4C, cpu->eip.u32);
    writed(thread, context+0x50, cpu->segValue[CS]);
    writed(thread, context+0x54, cpu->flags);
    writed(thread, context+0x58, 0); // REG_UESP
    writed(thread, context+0x5C, cpu->segValue[SS]);	
    writed(thread, context+0x60, 0); // fpu save state
}

void readFromContext(struct CPU* cpu, U32 context) {
    cpu_setSegment(cpu, GS, readd(cpu->thread, context+0x14));
    cpu_setSegment(cpu, FS, readd(cpu->thread, context+0x18));
    cpu_setSegment(cpu, ES, readd(cpu->thread, context+0x1C));
    cpu_setSegment(cpu, DS, readd(cpu->thread, context+0x20));

    cpu->reg[7].u32 = readd(cpu->thread, context+0x24); // EDI
    cpu->reg[6].u32 = readd(cpu->thread, context+0x28); // ESI
    cpu->reg[5].u32 = readd(cpu->thread, context+0x2C); // EBP
    cpu->reg[4].u32 = readd(cpu->thread, context+0x30); // ESP

    cpu->reg[3].u32 = readd(cpu->thread, context+0x34); // EBX
    cpu->reg[2].u32 = readd(cpu->thread, context+0x38); // EDX
    cpu->reg[1].u32 = readd(cpu->thread, context+0x3C); // ECX
    cpu->reg[0].u32 = readd(cpu->thread, context+0x40); // EAX
    
    cpu->eip.u32 = readd(cpu->thread, context+0x4C);
    cpu_setSegment(cpu, CS, readd(cpu->thread, context+0x50));
    cpu->flags = readd(cpu->thread, context+0x54);
    cpu_setSegment(cpu, SS, readd(cpu->thread, context+0x5C));
}

U32 syscall_sigreturn(struct KThread* thread) {
    memcopyToNative(thread, thread->cpu.reg[4].u32, (char*)&thread->cpu, sizeof(struct CPU));
    //klog("signal return (threadId=%d)", thread->id);
    return -K_CONTINUE;
}

void OPCALL onExitSignal(struct CPU* cpu, struct Op* op) {
    U32 context;	
    U64 tsc = cpu->timeStampCounter;
    U32 b = cpu->blockInstructionCount;

    pop32(cpu); // signal
    pop32(cpu); // address
    context = pop32(cpu);
#ifndef BOXEDWINE_VM
    cpu->thread->waitStartTime = pop32(cpu);
#endif
    cpu->thread->interrupted = pop32(cpu);

#ifdef LOG_OPS
    //klog("onExitSignal signal=%d info=%X context=%X stack=%X interrupted=%d", signal, address, context, cpu->reg[4].u32, cpu->thread->interrupted);
    //klog("    before context %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[4], cpu->segAddress[4], cpu->segAddress[4]?readd(cpu->memory, cpu->segAddress[4]+0x18):0);
#endif
    readFromContext(cpu, context);
#ifdef LOG_OPS
    klog("    after  context %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[4], cpu->segAddress[4], cpu->segAddress[4] ? readd(cpu->thread, cpu->segAddress[4] + 0x18) : 0);
#endif
    cpu->timeStampCounter = tsc;
    cpu->blockInstructionCount = b;
    cpu->thread->inSignal--;
    
    if (cpu->thread->waitingForSignalToEnd) {
#ifndef BOXEDWINE_VM
        if (cpu->thread->waitingForSignalToEnd->waitNode)
#endif
            wakeThread(cpu->thread, cpu->thread->waitingForSignalToEnd);
        cpu->thread->waitingForSignalToEnd = 0;		
    }
    if (cpu->thread->waitingForSignalToEndMaskToRestore & RESTORE_SIGNAL_MASK) {
        cpu->thread->sigMask = cpu->thread->waitingForSignalToEndMaskToRestore & RESTORE_SIGNAL_MASK;
        cpu->thread->waitingForSignalToEndMaskToRestore = SIGSUSPEND_RETURN;
    }

#ifndef BOXEDWINE_VM
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
#endif
    /*
    if (action->flags & K_SA_RESTORER) {
        push32(&thread->cpu, thread->cpu.eip.u32);
        thread->cpu.eip.u32 = action->restorer;
        while (thread->cpu.eip.u32!=savedState.eip.u32) {
            runCPU(&thread->cpu);
        }
    }
    */
}

// interrupted and waitStartTime are pushed because syscall's during the signal will clobber them
void runSignal(struct KThread* currentThread, struct KThread* thread, U32 signal, U32 trapNo, U32 errorNo) {
    struct KSigAction* action = &thread->process->sigActions[signal];
    if (action->handlerAndSigAction==K_SIG_DFL) {

    } else if (action->handlerAndSigAction != K_SIG_IGN) {
        U32 context;
        U32 address = 0;
        U32 stack = thread->cpu.reg[4].u32;
        U32 interrupted = 0;
        struct CPU* cpu = &thread->cpu;
        BOOL altStack = (action->flags & K_SA_ONSTACK) != 0;

        fillFlags(cpu);        

#ifdef LOG_OPS
        klog("runSignal %d", signal);
        klog("    before signal %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[4], cpu->segAddress[4], cpu->segAddress[4]?readd(thread, cpu->segAddress[4]+0x18):0);
#endif
        thread->inSigMask=action->mask | thread->sigMask;
        if (action->flags & K_SA_RESETHAND) {
            action->handlerAndSigAction=K_SIG_DFL;
        } else if (!(action->flags & K_SA_NODEFER)) {
            thread->inSigMask|= (U64)1 << (signal-1);
        }
#ifdef BOXEDWINE_VM
        if (thread->waitingMutex) {
            if (!(action->flags & K_SA_RESTART))
                interrupted = 1;
        }
#else
        if (thread->waitNode) {
            if (!(action->flags & K_SA_RESTART))
                interrupted = 1;
            wakeThread(thread, thread);
        }		
#endif           
		if (altStack)
			context = thread->alternateStack + thread->alternateStackSize - CONTEXT_SIZE;
		else
	        context = cpu->segAddress[SS] + (ESP & cpu->stackMask) - CONTEXT_SIZE;
        writeToContext(thread, stack, context, altStack, trapNo, errorNo);
        
        cpu->stackMask = 0xFFFFFFFF;
        cpu->stackNotMask = 0;
        cpu->segAddress[SS] = 0;
        thread->cpu.reg[4].u32 = context;

        thread->cpu.reg[4].u32 &= ~15;
        push32(&thread->cpu, 0); // padding
        if (action->flags & K_SA_SIGINFO) {
            U32 i;
            
            thread->cpu.reg[4].u32-=INFO_SIZE;
            address = thread->cpu.reg[4].u32;
            for (i=0;i<K_SIG_INFO_SIZE;i++) {
                writed(thread, address+i*4, thread->process->sigActions[signal].sigInfo[i]);
            }
                        
            push32(&thread->cpu, interrupted);
#ifndef BOXEDWINE_VM
            push32(&thread->cpu, thread->waitStartTime);
#endif
            push32(&thread->cpu, context);
            push32(&thread->cpu, address);			
            push32(&thread->cpu, signal);
            thread->cpu.reg[0].u32 = signal;
            thread->cpu.reg[1].u32 = address;
            thread->cpu.reg[2].u32 = context;	
        } else {
            thread->cpu.reg[0].u32 = signal;
            thread->cpu.reg[1].u32 = 0;
            thread->cpu.reg[2].u32 = 0;	
            push32(&thread->cpu, interrupted);
#ifndef BOXEDWINE_VM
            push32(&thread->cpu, thread->waitStartTime);
#endif
            push32(&thread->cpu, context);
            push32(&thread->cpu, 0);			
            push32(&thread->cpu, signal);
        }
#ifdef LOG_OPS
        klog("    context %X interrupted %d", context, interrupted);
#endif
#ifdef BOXEDWINE_VM
        x64_addSignalExit(cpu);
#endif
        push32(&thread->cpu, SIG_RETURN_ADDRESS);
        thread->cpu.eip.u32 = action->handlerAndSigAction;

        thread->inSignal++;				

        cpu_setSegment(cpu, CS, 0xf);        
        cpu_setSegment(cpu, SS, 0x17);
        cpu_setSegment(cpu, DS, 0x17);
        cpu_setSegment(cpu, ES, 0x17);
        cpu->big = 1;
    }    
    thread->process->pendingSignals &= ~(1 << (signal - 1));
    thread->pendingSignals &= ~(1 << (signal - 1));
    threadDone(&thread->cpu);
}
