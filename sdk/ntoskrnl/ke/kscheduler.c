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
#include "kprocess.h"
#include "ksystem.h"
#include "devfb.h"
#include "kcircularlist.h"
#include "klist.h"
#include "log.h"
#include "kerror.h"
#include "kscheduler.h"
#include "kthread.h"

#include <stdio.h>

//#define LOG_SCHEDULER

#ifdef BOXEDWINE_VM
#include <SDL.h>

void unscheduleThread(struct KThread* currentThread, struct KThread* thread) {
    if (currentThread == thread)
        thread->cpu.done = TRUE;
    else
        killThread(thread);
}

U32 threadSleep(struct KThread* thread, U32 ms) {
    SDL_Delay(ms);
    return 0; // :TODO: what about signal handlers using this thread while its sleep
}

void startThread(struct KThread* thread) {
    platformStartThread(thread);
}

void wakeThread(struct KThread* currentThread, struct KThread* thread) {
    if (thread->waitingMutex) {
        BOXEDWINE_LOCK(currentThread, thread->waitingMutex);
        BOXEDWINE_SIGNAL(thread->waitingCondition);
        BOXEDWINE_UNLOCK(currentThread, thread->waitingMutex);
    }
}

void wakeThreads(struct KThread* currentThread, U32 wakeType) {
    if (wakeType == WAIT_PID) {
        BOXEDWINE_LOCK(currentThread, mutexProcess);
        BOXEDWINE_SIGNAL_ALL(condProcessPid);
        BOXEDWINE_UNLOCK(currentThread, mutexProcess);
    }
}

void addTimer(struct KTimer* timer) {
    kpanic("addTimer not implemented");
}

void removeTimer(struct KTimer* timer) {
    kpanic("removeTimer not implemented");
}

void BOXEDWINE_LOCK(struct KThread* thread, SDL_mutex* mutex) {
    if (thread) {
        if (thread->waitingMutex && thread->waitingMutex!=mutex) {
            kpanic("BOXEDWINE_LOCK: tried to lock a 2nd mutex, this is not allowed");
        }
        thread->waitingMutex = mutex;
        thread->waitingMutexReferenceCount++;
    }
    SDL_LockMutex(mutex);
}

void BOXEDWINE_UNLOCK(struct KThread* thread, SDL_mutex* mutex) {
    if (thread) {
        if (thread->waitingMutex != mutex) {
            kpanic("BOXEDWINE_UNLOCK called with mutex that was not used for lock");
        }
        thread->waitingMutexReferenceCount--;
        if (thread->waitingMutexReferenceCount==0) {
            thread->waitingMutex = NULL;
        }
    }
    SDL_UnlockMutex(mutex);
}

void BOXEDWINE_SIGNAL(SDL_cond* cond) {
    SDL_CondSignal(cond);
}

void BOXEDWINE_SIGNAL_ALL(SDL_cond* cond) {
    SDL_CondBroadcast(cond);
}

void BOXEDWINE_WAIT(struct KThread* thread, SDL_cond* cond, SDL_mutex* mutex) {
    if (thread->waitingCondition!=NULL) {
        kpanic("BOXEDWINE_WAIT: waitingCondition not NULL");
    }
    thread->waitingCondition = cond;
    while (1) {
        BOOL runSignal = thread->runSignals;
        if (!runSignal) {
            SDL_CondWait(cond, mutex);
            if (thread->cpu.done) {
                BOXEDWINE_UNLOCK(thread, mutex);
                longjmp(*thread->cpu.jmpBuf, 0);
            }
            runSignal = thread->runSignals;
        }
        if (runSignal) {
            thread->runSignals = 0;
            // Keep lock, this thread can re-enter this lock if necessary, this way the caller doesn't have to be way of this
            if (runSignals(thread, thread)) {
                BOXEDWINE_UNLOCK(thread, mutex);
                longjmp(*thread->syscallJmpBuf, 0);
            }
        } else {
            break;
        }
    }
    thread->waitingCondition = NULL;
}

U32 BOXEDWINE_WAIT_TIMEOUT(struct KThread* thread, SDL_cond* cond, SDL_mutex* mutex, U32 ms) {
    U32 startTime = getMilliesSinceStart();
    U32 result = SDL_MUTEX_TIMEDOUT;
    if (thread->waitingCondition!=NULL) {
        kpanic("BOXEDWINE_WAIT_TIMEOUT: waitingCondition not NULL");
    }
    thread->waitingCondition = cond;
    while (1) {
        U32 elapsed = getMilliesSinceStart()-startTime;
        BOOL runSignal = thread->runSignals;
        if (elapsed >= ms) {
            result = SDL_CondWaitTimeout(cond, mutex, 0);
            if (thread->cpu.done) {
                BOXEDWINE_UNLOCK(thread, mutex);
                longjmp(*thread->cpu.jmpBuf, 0);
            }
            break;
        }
        if (!runSignal) {
            result = SDL_CondWaitTimeout(cond, mutex, ms-elapsed);
            if (thread->cpu.done) {
                BOXEDWINE_UNLOCK(thread, mutex);
                longjmp(*thread->cpu.jmpBuf, 0);
            }
            runSignal = thread->runSignals;
        }        
        if (runSignal) {
            thread->runSignals = 0;
            // Keep lock, this thread can re-enter this lock if necessary, this way the caller doesn't have to be way of this
            if (runSignals(thread, thread)) {
                BOXEDWINE_UNLOCK(thread, mutex);
                longjmp(*thread->syscallJmpBuf, 0);
            }
            continue;
        }
        break;
    }    
    thread->waitingCondition = NULL;
    return result;
};

#else


struct KCircularList scheduledThreads;
struct KCNode* nextThread;
struct KList waitingThreads;
struct KList timers;

void addTimer(struct KTimer* timer) {
#ifdef LOG_SCHEDULER
    klog("add timer");
    if (timer->thread)
        klog("    %d", timer->thread->id);
#endif
    timer->node = addItemToList(&timers, timer);
    timer->active = 1;
}

void removeTimer(struct KTimer* timer) {
#ifdef LOG_SCHEDULER
    klog("remove timer");
    if (timer->thread)
        klog("    %d", timer->thread->id);
#endif
    removeItemFromList(&timers, timer->node);
    timer->node = 0;
    timer->active = 0;
}

void wakeThread(struct KThread* currentThread, struct KThread* thread) {
    U32 i;
    if (!thread->waitNode) {
        kpanic("wakeThread: tried to wake a thread that is not asleep");
    }
    removeItemFromList(&waitingThreads, thread->waitNode);
    thread->waitNode = 0;
    for (i=0; i<thread->clearOnWakeCount; i++) {
        *thread->clearOnWake[i] = 0;
        thread->clearOnWake[i] = 0;
    }
    thread->clearOnWakeCount = 0;
    scheduleThread(thread);
}

void wakeThreads(struct KThread* currentThread, U32 wakeType) {
    struct KListNode* node = waitingThreads.first;
#ifdef LOG_SCHEDULER
    klog("wakeThreads %d", wakeType);
#endif
    while (node) {
        struct KListNode* next = node->next;
        struct KThread* thread = (struct KThread*)node->data;

        if (thread->waitType == wakeType) {
            wakeThread(currentThread, thread);
        }
        node = next;
    }
}

void startThread(struct KThread* thread) {
    scheduleThread(thread);
}

void scheduleThread(struct KThread* thread) {
    if (thread->timer.node) {
        removeTimer(&thread->timer);
    }	
    thread->scheduledNode = addItemToCircularList(&scheduledThreads, thread);
    if (scheduledThreads.count == 1) {
        nextThread = thread->scheduledNode;
    }
#ifdef LOG_SCHEDULER
    klog("schedule %d(%X)", thread->id, thread->scheduledNode);
    if (scheduledThreads.node)
    {
        struct KCNode* head=scheduledThreads.node;
        struct KCNode* node = head;		

        do {
            U32 id = ((struct KThread*)node->data)->id;
            klog("    %d(%X)",id, node);
            node = node->next;
        } while (node!=head);
    }
    {
        struct KListNode* node = timers.first;
        klog("timers");
        while (node) {
            struct KTimer* timer = (struct KTimer*)node->data;
            if (timer->thread) {
                klog("    thread %d", timer->thread->id);
            } else {
                klog("    process %d", timer->process->id);
            }		
            node = node->next;
        }
    }
#endif
}

void unscheduleThread(struct KThread* currentThread, struct KThread* thread) {	
    if (thread->waitNode) {
        wakeThread(currentThread, thread);
    }
    removeItemFromCircularList(&scheduledThreads, thread->scheduledNode);
    thread->scheduledNode = 0;
    threadDone(&thread->cpu);
    if (nextThread->data == thread) {
        nextThread = scheduledThreads.node;
    }
#ifdef LOG_SCHEDULER
    klog("unschedule %d(%X)", thread->id, thread->scheduledNode);	
    if (scheduledThreads.node)
    {
        struct KCNode* head=scheduledThreads.node;
        struct KCNode* node = head;
        do {
            klog("    %d(%X)", ((struct KThread*)node->data)->id, node);
            node = node->next;
        } while (node!=head);
    }
    {
        struct KListNode* node = timers.first;
        klog("timers");
        while (node) {
            struct KTimer* timer = (struct KTimer*)node->data;
            if (timer->thread) {
                klog("    thread %d", timer->thread->id);
            } else {
                klog("    process %d", timer->process->id);
            }	
            node = node->next;
        }
    }
#endif
}

void waitThread(struct KThread* thread) {
    unscheduleThread(thread, thread);
    thread->waitNode = addItemToList(&waitingThreads, thread);
}

U32 contextTime = 100000;
#ifdef BOXEDWINE_HAS_SETJMP
jmp_buf runBlockJump;
#endif
int count;
extern struct Block emptyBlock;

void runThreadSlice(struct KThread* thread) {
    struct CPU* cpu;

    cpu = &thread->cpu;
#ifdef INCLUDE_CYCLES
    cpu->blockCounter = 0;
#else
    cpu->yield = FALSE;
#endif
    cpu->blockInstructionCount = 0;

    if (!cpu->nextBlock || cpu->nextBlock == &emptyBlock) {
        cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    }
#ifdef BOXEDWINE_HAS_SETJMP
    if (setjmp(runBlockJump)==0) {
#endif
        do {
            runBlock(cpu, cpu->nextBlock);
            runBlock(cpu, cpu->nextBlock);
            runBlock(cpu, cpu->nextBlock);
            runBlock(cpu, cpu->nextBlock);
            runBlock(cpu, cpu->nextBlock);
            runBlock(cpu, cpu->nextBlock);
            runBlock(cpu, cpu->nextBlock);
            runBlock(cpu, cpu->nextBlock);
#ifdef INCLUDE_CYCLES
        } while (cpu->blockCounter < contextTime);	
#else
        } while (cpu->blockInstructionCount < contextTime && !cpu->yield);
#endif

#ifdef BOXEDWINE_HAS_SETJMP
    } else {
        cpu->nextBlock = 0;
    }
#endif
#ifdef INCLUDE_CYCLES
    cpu->timeStampCounter+=cpu->blockCounter & 0x7FFFFFFF;
#else
    cpu->timeStampCounter+=cpu->blockInstructionCount;
#endif
}

void runUntil(struct KThread* thread, U32 eip) {
    // :TODO: what about switching threads in case we send a message to wineserver?
    kpanic("memory access exceptions not support without setjmp yet");
}

void runTimers() {
    struct KListNode* node = timers.first;

    if (node) {		
        U32 millies = getMilliesSinceStart();
        while (node) {
            struct KTimer* timer = (struct KTimer*)node->data;
            struct KListNode* next = node->next;
            if (timer->millies<=millies) {
                if (timer->thread) {
                    removeTimer(timer);
                    wakeThread(NULL, timer->thread);
                } else {
                    runProcessTimer(timer);                    
                }
                node = timers.first; // next might have been removed
            } else {
                node = next;
            }
        }
    }
}

#ifdef INCLUDE_CYCLES
U64 elapsedTimeMHz;
U64 elapsedCyclesMHz;
#endif
extern U64 sysCallTime;
U64 elapsedTimeMIPS;
U64 elapsedInstructionsMIPS;

void dspCheck();
void sdlUpdateContextForThread(struct KThread* thread);

#ifdef BOXEDWINE_VM
void platformRunThreadSlice(struct KThread* thread);
#endif

BOOL runSlice() {
    runTimers();
    dspCheck();
    //flipFB();
    if (nextThread) {		
        U64 startTime = getMicroCounter();
        U64 endTime;
        U64 diff;

        struct KThread* currentThread = (struct KThread*)nextThread->data;
        sdlUpdateContextForThread(currentThread);
        nextThread = nextThread->next;						
        sysCallTime = 0;

#ifdef BOXEDWINE_64BIT_MMU
        platformRunThreadSlice(currentThread);
#else
        runThreadSlice(currentThread);
#endif
        endTime = getMicroCounter();
        diff = endTime-startTime;
        
#ifdef INCLUDE_CYCLES
        elapsedTimeMHz+=diff-sysCallTime;
#endif
        elapsedTimeMIPS+=diff-sysCallTime;

#ifdef INCLUDE_CYCLES
        if (!(currentThread->cpu.blockCounter & 0x80000000)) {			
            if (diff>20000) {
                contextTime-=1000;
            } else if (diff<10000) {
                contextTime+=1000;
            }
        }
        elapsedCyclesMHz+=currentThread->cpu.blockCounter & 0x7FFFFFFF;
#else
        if (currentThread->cpu.blockInstructionCount && !currentThread->cpu.yield) {
            if (diff>20000) {
                contextTime-=2000;
            } else if (diff<10000) {
                contextTime+=2000;
            }
        }
#endif        
        elapsedInstructionsMIPS+=currentThread->cpu.blockInstructionCount;

        currentThread->userTime+=diff;
        currentThread->kernelTime+=sysCallTime;
        return TRUE;
    }
    return FALSE;
}

#ifdef INCLUDE_CYCLES
U32 getMHz() {
    U32 result = 0;

    if (elapsedTimeMHz) {
        result = (U32)(elapsedCyclesMHz/elapsedTimeMHz);
        elapsedTimeMHz = 0;
        elapsedCyclesMHz = 0;
    }

    return result;
}
#endif

U32 getMIPS() {
    U32 result = 0;
    if (elapsedTimeMIPS) {
        result = (U32)(elapsedInstructionsMIPS/elapsedTimeMIPS);
        elapsedTimeMIPS = 0;
        elapsedInstructionsMIPS = 0;
    }
    return result;
}

U32 threadSleep(struct KThread* thread, U32 ms) {
    if (thread->waitStartTime) {
        U32 diff = getMilliesSinceStart()-thread->waitStartTime;
        if (diff >= ms) {
            thread->waitStartTime = 0;
            return 0;
        } else {
            thread->timer.process = thread->process;
            thread->timer.thread = thread;
            addTimer(&thread->timer);
            return -K_WAIT;
        }
    } else {
        thread->waitStartTime = getMilliesSinceStart();
        thread->timer.millies = thread->waitStartTime+ms;
        thread->timer.process = thread->process;
        thread->timer.thread = thread;
        addTimer(&thread->timer);
        return -K_WAIT;
    }
}

#endif
