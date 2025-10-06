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
#include "kpoll.h"
#include "kprocess.h"
#include "kobjectaccess.h"
#include "ksystem.h"
#include "kerror.h"
#include "kthread.h"
#include "kscheduler.h"
#include "log.h"

#ifdef BOXEDWINE_VM
SDL_mutex* pollMutex;
SDL_cond* pollCond;

S32 kpoll(struct KThread* thread, struct KPollData* data, U32 count, U32 timeout) {
    S32 result = 0;
    U32 i;
    U32 startTime = getMilliesSinceStart();
    U32 elapsed;

    if (!pollMutex) {
        pollMutex = SDL_CreateMutex();
    }
    if (!pollCond) {
        pollCond = SDL_CreateCond();
    }
    BOXEDWINE_LOCK(thread, pollMutex);
    while (1) {
        for (i=0;i<count;i++) {
            struct KFileDescriptor* fd = getFileDescriptor(thread->process, data[i].fd);
            data[i].revents = 0;
            if (fd) {
                if (!fd->kobject->access->isOpen(fd->kobject)) {
                    data[i].revents = K_POLLHUP;
                } else {
                    if ((data[i].events & K_POLLIN) != 0 && fd->kobject->access->isReadReady(thread, fd->kobject)) {
                        data[i].revents |= K_POLLIN;
                    } else if ((data[i].events & K_POLLOUT) != 0 && fd->kobject->access->isWriteReady(thread, fd->kobject)) {
                        data[i].revents |= K_POLLOUT;
                    }
                }
                if (data[i].revents!=0) {
                    result++;
                }
            }
        }
        elapsed = getMilliesSinceStart()-startTime;
        if (!result) {
            if (elapsed>timeout) {
                BOXEDWINE_UNLOCK(thread, pollMutex);
                return 0;
            }
        }
        if (result) {
            BOXEDWINE_UNLOCK(thread, pollMutex);
            return result;
        }
        
        BOXEDWINE_WAIT_TIMEOUT(thread, pollCond, pollMutex, timeout-elapsed);
    } 
}
#else
S32 kpoll(struct KThread* thread, struct KPollData* data, U32 count, U32 timeout) {
    S32 result = 0;
    U32 i;
    U32 interrupted = !thread->inSignal && thread->interrupted;
    struct KPollData* firstData=data;

    if (interrupted)
        thread->interrupted = 0;
    for (i=0;i<count;i++) {
        struct KFileDescriptor* fd = getFileDescriptor(thread->process, data->fd);
        data->revents = 0;
        if (fd) {
            if (!fd->kobject->access->isOpen(fd->kobject)) {
                data->revents = K_POLLHUP;
            } else {
                if ((data->events & K_POLLIN) != 0 && fd->kobject->access->isReadReady(thread, fd->kobject)) {
                    data->revents |= K_POLLIN;
                } else if ((data->events & K_POLLOUT) != 0 && fd->kobject->access->isWriteReady(thread, fd->kobject)) {
                    data->revents |= K_POLLOUT;
                }
            }
            if (data->revents!=0) {
                result++;
            }
        }
        data++;
    }
    if (result>0) {		
        thread->waitStartTime = 0;
        return result;
    }
    if (timeout==0) {
        thread->waitStartTime = 0;
        return 0;
    }	
    if (thread->waitStartTime) {
        if (getMilliesSinceStart() - thread->waitStartTime > timeout) {
            thread->waitStartTime = 0;
            return 0;
        }	
        if (interrupted) {			
            return -K_EINTR;
        }
        data = firstData;
        for (i=0;i<count;i++) {
            struct KFileDescriptor* fd = getFileDescriptor(thread->process, data->fd);
            fd->kobject->access->waitForEvents(fd->kobject, thread, data->events);
            data++;
        }
        if (!thread->timer.active) {
            thread->timer.process = thread->process;
            thread->timer.thread = thread;
            if (timeout<0xF0000000)
                timeout+=thread->waitStartTime;
            thread->timer.millies = timeout;
            addTimer(&thread->timer);
        }
    } else {		
        if (interrupted) {			
            return -K_EINTR;
        }
        thread->waitStartTime = getMilliesSinceStart();

        data = firstData;
        for (i=0;i<count;i++) {
            struct KFileDescriptor* fd = getFileDescriptor(thread->process, data->fd);
            if (fd) {
                fd->kobject->access->waitForEvents(fd->kobject, thread, data->events);
            }
            data++;
        }
        if (!thread->timer.active) {
            thread->timer.process = thread->process;
            thread->timer.thread = thread;
            if (timeout<0xF0000000)
                timeout+=thread->waitStartTime;
            thread->timer.millies = timeout;
            addTimer(&thread->timer);
        }
    }	
    return -K_WAIT;
}
#endif