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

#ifndef __KSCHEDULER_H__
#define __KSCHEDULER_H__

#include "kthread.h"

void unscheduleThread(struct KThread* currentThread, struct KThread* thread); // will not return if thread is current thread and BOXEDWINE_VM
U32 threadSleep(struct KThread* thread, U32 ms);
void wakeThread(struct KThread* currentThread, struct KThread* thread);
void startThread(struct KThread* thread);
void wakeThreads(struct KThread* currentThread, U32 wakeType);
void addTimer(struct KTimer* timer);
void removeTimer(struct KTimer* timer);
void pauseThread(struct KThread* thread);
void resumeThread(struct KThread* thread);

#ifdef BOXEDWINE_VM
void BOXEDWINE_LOCK(struct KThread* thread, SDL_mutex* mutex);
void BOXEDWINE_UNLOCK(struct KThread* thread, SDL_mutex* mutex);
void BOXEDWINE_SIGNAL(SDL_cond* cond);
void BOXEDWINE_SIGNAL_ALL(SDL_cond* cond);
void BOXEDWINE_WAIT(struct KThread* thread, SDL_cond* cond, SDL_mutex* mutex);
U32 BOXEDWINE_WAIT_TIMEOUT(struct KThread* thread, SDL_cond* cond, SDL_mutex* mutex, U32 ms);
#else

#define BOXEDWINE_LOCK(x,y)
#define BOXEDWINE_UNLOCK(thread, x)
#define BOXEDWINE_SIGNAL(cond)
#define BOXEDWINE_SIGNAL_ALL(cond)
#define BOXEDWINE_WAIT(thread, cond, lock)
#define BOXEDWINE_WAIT_TIMEOUT(thread, cond, lock, ms)

struct KTimer;

BOOL runSlice();
void runUntil(struct KThread* thread, U32 eip);
void runThreadSlice(struct KThread* thread);
void waitThread(struct KThread* thread);
void scheduleThread(struct KThread* thread);
#ifdef INCLUDE_CYCLES
U32 getMHz();
#endif
U32 getMIPS();
#endif

#endif