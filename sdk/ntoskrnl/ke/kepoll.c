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
#include "kepoll.h"
#include "platform.h"
#include "kscheduler.h"
#include "log.h"
#include "kobject.h"
#include "kerror.h"
#include "kalloc.h"
#include "kobjectaccess.h"
#include "kprocess.h"

#include <string.h>

struct KEpoll {
    U32 fd;
    U64 data;
    U32 events;
    struct KEpoll* next;
};

void freeEpoll(struct KEpoll* epoll);

void kepoll_onDelete(struct KObject* obj) {
    struct KEpoll* e = (struct KEpoll*)obj->data;

    freeEpoll(e);
    // :TODO:
    //wakeThreads(WAIT_FD);
}

void kepoll_setBlocking(struct KObject* obj, BOOL blocking) {
    if (blocking)
        kpanic("kepoll_setBlocking not implemented yet");
}

BOOL kepoll_isBlocking(struct KObject* obj) {
    return FALSE;
}

void kepoll_setAsync(struct KObject* obj, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kpanic("kepoll_setAsync not implemented yet");
}

BOOL kepoll_isAsync(struct KObject* obj, struct KProcess* process) {
    return FALSE;
}

struct KFileLock* kepoll_getLock(struct KObject* obj, struct KFileLock* lock) {
    kwarn("kepoll_getLock not implemented yet");
    return 0;
}

U32 kepoll_setLock(struct KObject* obj, struct KFileLock* lock, BOOL wait, struct KThread* thread) {
    kwarn("kepoll_setLock not implemented yet");
    return -1;
}

BOOL kepoll_isOpen(struct KObject* obj) {
    kpanic("kepoll_isOpen not implemented yet");
    return FALSE;
}

void kepoll_waitForEvents(struct KObject* obj, struct KThread* thread, U32 events) {
    kpanic("kepoll_waitForEvents not implemented yet");
}

BOOL kepoll_isReadReady(struct KThread* thread, struct KObject* obj) {
    kpanic("kepoll_isReadReady not implemented yet");
    return FALSE;
}

BOOL kepoll_isWriteReady(struct KThread* thread, struct KObject* obj) {
    kpanic("kepoll_isWriteReady not implemented yet");
    return FALSE;
}

U32 kepoll_write(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    kpanic("kepoll_write not implemented yet");
    return 0;
}

U32 kepoll_read(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    kpanic("kepoll_read not implemented yet");
    return 0;
}

U32 kepoll_stat(struct KThread* thread, struct KObject* obj, U32 address, BOOL is64) {
    kpanic("kepoll_stat not implemented yet");
    return 0;
}

U32 kepoll_map(struct KThread* thread, struct KObject* obj, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL kepoll_canMap(struct KObject* obj) {
    return FALSE;
}

S64 kepoll_seek(struct KObject* obj, S64 pos) {
    return -K_ESPIPE; // :TODO: is this right?
}

S64 kepoll_getPos(struct KObject* obj) {
    return 0;
}

U32 kepoll_ioctl(struct KThread* thread, struct KObject* obj, U32 request) {
    return -K_ENOTTY;
}

BOOL kepoll_supportsLocks(struct KObject* obj) {
    return FALSE;
}

S64 kepoll_klength(struct KObject* obj) {
    return -1;
}

struct KObjectAccess kepollAccess = {kepoll_ioctl, kepoll_seek, kepoll_klength, kepoll_getPos, kepoll_onDelete, kepoll_setBlocking, kepoll_isBlocking, kepoll_setAsync, kepoll_isAsync, kepoll_getLock, kepoll_setLock, kepoll_supportsLocks, kepoll_isOpen, kepoll_isReadReady, kepoll_isWriteReady, kepoll_waitForEvents, kepoll_write, kaccess_default_writev, kepoll_read, kepoll_stat, kepoll_map, kepoll_canMap};

struct KEpoll* freeEpolls;

struct KEpoll* allocEpoll() {
    struct KEpoll* result;
    if (freeEpolls) {
        result = freeEpolls;
        freeEpolls = result->next;		
        memset(result, 0, sizeof(struct KEpoll));
    } else {
        result = (struct KEpoll*)kalloc(sizeof(struct KEpoll), KALLOC_KEPOLL);
    }	
    return result;
}

void freeEpoll(struct KEpoll* epoll) {
    epoll->next = freeEpolls;
    freeEpolls = epoll;
}

U32 syscall_epollcreate(struct KThread* thread, U32 size, U32 flags) {
    struct KObject* o = allocKObject(&kepollAccess, KTYPE_EPOLL, 0, 0);
    struct KFileDescriptor* result = allocFileDescriptor(thread->process, o, K_O_RDWR, flags, -1, 0);
    return result->handle;
}

#define K_EPOLL_CTL_ADD 1
#define K_EPOLL_CTL_DEL 2
#define K_EPOLL_CTL_MOD 3

U32 syscall_epollctl(struct KThread* thread, FD epfd, U32 op, FD fd, U32 address) {
    struct KFileDescriptor* epollFD = getFileDescriptor(thread->process, epfd);
    struct KFileDescriptor* targetFD = getFileDescriptor(thread->process, fd);
    struct KEpoll* existing;
    struct KEpoll* prev = 0;

    if (!targetFD || !epollFD) {
        return -K_EBADF;
    }
    if (fd==epfd || epollFD->kobject->type != KTYPE_EPOLL) {
        return -K_EINVAL;
    }
    existing = (struct KEpoll*)epollFD->kobject->data;
    while (existing) {
        if (existing->fd == fd)
            break;
        prev = existing;
        existing = existing->next;
    }
    switch (op) {
        case K_EPOLL_CTL_ADD:
            if (existing) {
                return -K_EEXIST;
            }
            existing = allocEpoll();
            existing->fd = fd;
            existing->events = readd(thread, address);
            existing->data = readq(thread, address + 4);
            existing->next = (struct KEpoll*)epollFD->kobject->data;
            epollFD->kobject->data = existing;
            break;
        case K_EPOLL_CTL_DEL:
            if (!existing)
                return -K_ENOENT;
            if (prev) {
                prev->next = prev->next->next;
            } else {
                epollFD->kobject->data = existing->next;
            }
            freeEpoll(existing);
            break;
        case K_EPOLL_CTL_MOD:
            if (!existing)
                return -K_ENOENT;
            existing->events = readd(thread, address);
            existing->data = readq(thread, address + 4);
            break;
        default:
            return -K_EINVAL;
    }
    return 0;
}

U32 syscall_epollwait(struct KThread* thread, FD epfd, U32 events, U32 maxevents, U32 timeout) {
    struct KFileDescriptor* epollFD = getFileDescriptor(thread->process, epfd);
    struct KEpoll* next;
    S32 result = 0;
    U32 i;

    if (!epollFD) {
        return -K_EBADF;
    }
    if (epollFD->kobject->type != KTYPE_EPOLL) {
        return -K_EINVAL;
    }	
    thread->pollCount=0;
    next = (struct KEpoll*)epollFD->kobject->data;
    while (next) {
        thread->pollData[thread->pollCount].events = next->events;
        thread->pollData[thread->pollCount].fd = next->fd;
        thread->pollData[thread->pollCount].data = next->data;
        thread->pollCount++;
        next = next->next;		
    }
    result = kpoll(thread, thread->pollData, thread->pollCount, timeout);
    if (result < 0)
        return result;
    result = 0;
    for (i=0;i<thread->pollCount;i++) {
        if (thread->pollData[i].revents!=0) {
            writed(thread, events + result * 12, thread->pollData[i].revents);        
            writeq(thread, events + result * 12 + 4, thread->pollData[i].data);
            result++;
        }        
    }
    return result;
}