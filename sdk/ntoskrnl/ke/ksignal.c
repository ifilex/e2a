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
#include "ksignal.h"
#include "kerror.h"
#include "kprocess.h"
#include "log.h"
#include "kthread.h"
#include "kscheduler.h"
#include "kobjectaccess.h"

void writeSigAction(struct KThread* thread, struct KSigAction* signal, U32 address, U32 sigsetSize) {
    writed(thread, address, signal->handlerAndSigAction);
    writed(thread, address + 4, signal->flags);
    writed(thread, address + 8, signal->restorer);
    if (sigsetSize==4)
        writed(thread, address + 12, (U32)signal->mask);
    else if (sigsetSize==8)
        writeq(thread, address + 12, signal->mask);
    else
        klog("writeSigAction: can't handle sigsetSize=%d", sigsetSize);
}

void readSigAction(struct KThread* thread, struct KSigAction* signal, U32 address, U32 sigsetSize) {
    signal->handlerAndSigAction = readd(thread, address);
    signal->flags = readd(thread, address + 4);
    signal->restorer = readd(thread, address + 8);
    if (sigsetSize==4)
        signal->mask = readd(thread, address + 12);
    else if (sigsetSize==8)
        signal->mask = readq(thread, address + 12);
    else
        klog("readSigAction: can't handle sigsetSize=%d", sigsetSize);
}

U32 syscall_rt_sigaction(struct KThread* thread, U32 sig, U32 act, U32 oact, U32 sigsetSize) {
    if (sig == K_SIGKILL || sig == K_SIGSTOP || sig>MAX_SIG_ACTIONS) {
        return -K_EINVAL;
    }

    if (oact!=0) {
        writeSigAction(thread, &thread->process->sigActions[sig], oact, sigsetSize);
    }
    if (act!=0) {
        readSigAction(thread, &thread->process->sigActions[sig], act, sigsetSize);
    }
    return 0;
}

U32 syscall_rt_sigprocmask(struct KThread* thread, U32 how, U32 set, U32 oset, U32 sigsetSize) {
    if (oset!=0) {
        if (sigsetSize==4)
            writed(thread, oset, (U32)thread->sigMask);
        else if (sigsetSize==8)
            writeq(thread, oset, thread->sigMask);
        else
            klog("syscall_rt_sigprocmask: can't handle sigsetSize=%d", sigsetSize);
        //klog("syscall_sigprocmask oset=%X", thread->sigMask);
    }
    if (set!=0) {
        U64 mask;
        
        if (sigsetSize==4)
            mask = readd(thread, set);
        else if (sigsetSize==8)
            mask = readq(thread, set);
        else
            klog("syscall_rt_sigprocmask: can't handle sigsetSize=%d", sigsetSize);
        if (how == K_SIG_BLOCK) {
            thread->sigMask|=mask;
            //klog("syscall_sigprocmask block %X(%X)", set, thread->sigMask);
        } else if (how == K_SIG_UNBLOCK) {
            thread->sigMask&=~mask;
            //klog("syscall_sigprocmask unblock %X(%X)", set, thread->sigMask);
        } else if (how == K_SIG_SETMASK) {
            thread->sigMask = mask;
            //klog("syscall_sigprocmask set %X(%X)", set, thread->sigMask);
        } else {
            kpanic("sigprocmask how %d unsupported", how);
        }
    }
    return 0;
}

U32 syscall_signalstack(struct KThread* thread, U32 ss, U32 oss) {
    if (oss!=0) {
        writed(thread, oss, thread->alternateStack);
        writed(thread, oss + 4, (thread->alternateStack && thread->inSignal) ? K_SS_ONSTACK : K_SS_DISABLE);
        writed(thread, oss + 8, thread->alternateStackSize);
    }
    if (ss!=0) {
        U32 flags = readd(thread, ss + 4);
        if (flags & K_SS_DISABLE) {
            thread->alternateStack = 0;
            thread->alternateStackSize = 0;
        } else {
            thread->alternateStack = readd(thread, ss);
            thread->alternateStackSize = readd(thread, ss + 8);
        }
    }
    return 0;
}

U32 syscall_rt_sigsuspend(struct KThread* thread, U32 mask, U32 sigsetSize) {
    if (thread->waitingForSignalToEndMaskToRestore==SIGSUSPEND_RETURN) {
        thread->waitingForSignalToEndMaskToRestore = 0;
        return -K_EINTR;
    }
    thread->waitingForSignalToEndMaskToRestore = thread->sigMask | RESTORE_SIGNAL_MASK;
    if (sigsetSize==4)
        thread->sigMask = readd(thread, mask);
    else if (sigsetSize==8)
        thread->sigMask = readq(thread, mask);
    else
        klog("syscall_rt_sigsuspend: can't handle sigsetSize=%d", sigsetSize);
#ifdef BOXEDWINE_VM
    {
        SDL_cond* cond = SDL_CreateCond();
        SDL_mutex* mutex = SDL_CreateMutex();
        BOXEDWINE_LOCK(thread, mutex);        
        BOXEDWINE_WAIT(thread, cond, mutex);
        BOXEDWINE_UNLOCK(thread, mutex);        
        SDL_DestroyCond(cond);
        SDL_DestroyMutex(mutex);
        return 0;
    }
#else
    waitThread(thread);			
#endif
    return -K_CONTINUE;
}

void ksignal_onDelete(struct KObject* obj) {
}

void ksignal_setBlocking(struct KObject* obj, BOOL blocking) {
    obj->idata2 = blocking;
}

BOOL ksignal_isBlocking(struct KObject* obj) {
    return obj->idata2?TRUE:FALSE;
}

void ksignal_setAsync(struct KObject* obj, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kpanic("ksignal_setAsync not implemented yet");
}

BOOL ksignal_isAsync(struct KObject* obj, struct KProcess* process) {
    return FALSE;
}

struct KFileLock* ksignal_getLock(struct KObject* obj, struct KFileLock* lock) {
    kwarn("ksignal_getLock not implemented yet");
    return 0;
}

U32 ksignal_setLock(struct KObject* obj, struct KFileLock* lock, BOOL wait, struct KThread* thread) {
    kwarn("ksignal_setLock not implemented yet");
    return -1;
}

BOOL ksignal_isOpen(struct KObject* obj) {
    return TRUE;
}

void ksignal_waitForEvents(struct KObject* obj, struct KThread* thread, U32 events) {
    if (events & K_POLLIN) {
		if (obj->data)
			kpanic("%d tried to wait on a signal read, but %d is already waiting.", thread->id, ((struct KThread*)obj->data)->id);
		obj->data = thread;
		addClearOnWake(thread, (struct KThread**)&obj->data);
    }
}

BOOL ksignal_isReadReady(struct KThread* thread, struct KObject* obj) {
    if ((thread->process->pendingSignals & obj->idata) || (thread->pendingSignals & obj->idata))
        return TRUE;
    return FALSE;
}

BOOL ksignal_isWriteReady(struct KThread* thread, struct KObject* obj) {
    kpanic("ksignal_isWriteReady not implemented yet");
    return FALSE;
}

U32 ksignal_write(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    kpanic("ksignal_write not implemented yet");
    return 0;
}

U32 ksignal_read(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    kpanic("ksignal_read not implemented yet");
    return 0;
}

U32 ksignal_stat(struct KThread* thread, struct KObject* obj, U32 address, BOOL is64) {
    kpanic("ksignal_stat not implemented yet");
    return 0;
}

U32 ksignal_map(struct KThread* thread, struct KObject* obj, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL ksignal_canMap(struct KObject* obj) {
    return FALSE;
}

S64 ksignal_seek(struct KObject* obj, S64 pos) {
    return -K_ESPIPE; // :TODO: is this right?
}

S64 ksignal_getPos(struct KObject* obj) {
    return 0;
}

U32 ksignal_ioctl(struct KThread* thread, struct KObject* obj, U32 request) {
    return -K_ENOTTY;
}

BOOL ksignal_supportsLocks(struct KObject* obj) {
    return FALSE;
}

S64 ksignal_klength(struct KObject* obj) {
    return -1;
}

struct KObjectAccess ksignalAccess = {ksignal_ioctl, ksignal_seek, ksignal_klength, ksignal_getPos, ksignal_onDelete, ksignal_setBlocking, ksignal_isBlocking, ksignal_setAsync, ksignal_isAsync, ksignal_getLock, ksignal_setLock, ksignal_supportsLocks, ksignal_isOpen, ksignal_isReadReady, ksignal_isWriteReady, ksignal_waitForEvents, ksignal_write, kaccess_default_writev, ksignal_read, ksignal_stat, ksignal_map, ksignal_canMap};

U32 syscall_signalfd4(struct KThread* thread, S32 fildes, U32 mask, U32 flags) {
    struct KFileDescriptor* fd;

    if (fildes>=0) {
        fd = getFileDescriptor(thread->process, fildes);
        if (!fd)
            return -K_EBADF;
        if (fd->kobject->type!=KTYPE_SIGNAL)
            return -K_EINVAL;
    } else {
        struct KObject* o = allocKObject(&ksignalAccess, KTYPE_SIGNAL, 0, 0);
        fd =  allocFileDescriptor(thread->process, o, K_O_RDONLY, 0, -1, 0);
    }    
    if (flags & K_O_CLOEXEC) {
        fd->descriptorFlags|=FD_CLOEXEC;
    }
    if (flags & K_O_NONBLOCK) {
        fd->accessFlags |= K_O_NONBLOCK;
    }
    fd->kobject->idata = readd(thread, mask);
    fd->kobject->idata2 = fd->accessFlags & K_O_NONBLOCK;
    return fd->handle;
}