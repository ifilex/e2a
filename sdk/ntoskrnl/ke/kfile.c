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
#include "kobject.h"
#include "kprocess.h"
#include "kobjectaccess.h"
#include "log.h"
#include "fsapi.h"

void kfile_onDelete(struct KObject* obj) {
    obj->openFile->func->close(obj->openFile);
}

void kfile_setBlocking(struct KObject* obj, BOOL blocking) {
    if (blocking) {
        kwarn("kfile_setBlocking not implemented");
    }
}

BOOL kfile_isBlocking(struct KObject* obj) {
    return FALSE;
}

void kfile_setAsync(struct KObject* obj, struct KProcess* process, FD fd, BOOL isAsync) {
    obj->openFile->func->setAsync(obj->openFile, process, fd, isAsync);
}

BOOL kfile_isAsync(struct KObject* obj, struct KProcess* process) {
    return obj->openFile->func->isAsync(obj->openFile, process);
}

struct KFileLock* kfile_getLock(struct KObject* obj, struct KFileLock* lock) {
    struct FsOpenNode* openNode = obj->openFile;
    struct FsNode* node = openNode->node;
    struct KFileLock* next = node->locks;
    U64 l1 = lock->l_start;
    U64 l2 = l1+lock->l_len;

    if (lock->l_len == 0)
        l2 = 0xFFFFFFFF;
    while (next) {
        U64 s1 = next->l_start;
        U64 s2 = s1+next->l_len;
        
        if (next->l_len == 0)
            s2 = 0xFFFFFFFF;
        if ((s1>=l1 && s1<=l2) || (s2>=l1 && s2<=l2)) {
            return next;
        }
        next = next->next;
    }
    return 0;
}

U32 kfile_setLock(struct KObject* obj, struct KFileLock* lock, BOOL wait, struct KThread* thread) {
    struct FsOpenNode* openNode = obj->openFile;
    struct FsNode* node = openNode->node;
    
    // :TODO: unlock, auto remove lock if process exits
    if (lock->l_type == K_F_UNLCK) {
    } else {
        struct KFileLock* f = allocFileLock();
        f->l_len = lock->l_len;
        f->l_pid = lock->l_pid;
        f->l_start = lock->l_start;
        f->l_type = lock->l_type;
        f->l_whence = lock->l_whence;
        f->next = node->locks;
        node->locks = f;
    }
    return 0;
}

BOOL kfile_isOpen(struct KObject* obj) {
    return TRUE;
}

BOOL kfile_isReadReady(struct KThread* thread, struct KObject* obj) {
    return obj->openFile->func->isReadReady(thread, obj->openFile);
}

BOOL kfile_isWriteReady(struct KThread* thread, struct KObject* obj) {
    return obj->openFile->func->isWriteReady(thread, obj->openFile);
}

void kfile_waitForEvents(struct KObject* obj, struct KThread* thread, U32 events) {
    obj->openFile->func->waitForEvents(obj->openFile, thread, events);
}

U32  kfile_write(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    return obj->openFile->func->write(thread, obj->openFile, buffer, len);
}

U32  kfile_read(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    return obj->openFile->func->read(thread, obj->openFile, buffer, len);
}

U32 kfile_stat(struct KThread* thread, struct KObject* obj, U32 address, BOOL is64) {
    struct FsOpenNode* openNode = obj->openFile;
    struct FsNode* node = openNode->node;
    U64 len = node->func->length(node);

    writeStat(node, thread, address, is64, 1, node->id, node->func->getMode(thread->process, node), node->rdev, len, FS_BLOCK_SIZE, (len+FS_BLOCK_SIZE-1)/FS_BLOCK_SIZE, node->func->lastModified(node), getHardLinkCount(node));
    return 0;
}

U32 kfile_map(struct KThread* thread, struct KObject* obj, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return obj->openFile->func->map(thread, obj->openFile, address, len, prot, flags, off);
}

BOOL kfile_canMap(struct KObject* obj) {
    return obj->openFile->func->canMap(obj->openFile);
}

S64 kfile_seek(struct KObject* obj, S64 pos) {
    return obj->openFile->func->seek(obj->openFile, pos);
}

S64 kfile_getPos(struct KObject* obj) {
    return obj->openFile->func->getFilePointer(obj->openFile);
}

U32 kfile_ioctl(struct KThread* thread, struct KObject* obj, U32 request) {
    return obj->openFile->func->ioctl(thread, obj->openFile, request);
}

BOOL kfile_supportsLocks(struct KObject* obj) {
    return TRUE;
}

S64 kfile_length(struct KObject* obj) {
    return obj->openFile->func->length(obj->openFile);
}

struct KObjectAccess kfileAccess = {kfile_ioctl, kfile_seek, kfile_length, kfile_getPos, kfile_onDelete, kfile_setBlocking, kfile_isBlocking, kfile_setAsync, kfile_isAsync, kfile_getLock, kfile_setLock, kfile_supportsLocks, kfile_isOpen, kfile_isReadReady, kfile_isWriteReady, kfile_waitForEvents, kfile_write, kaccess_default_writev, kfile_read, kfile_stat, kfile_map, kfile_canMap};

struct KObject* allocKFile(struct FsOpenNode* node) {
    return allocKObject(&kfileAccess, KTYPE_FILE, node, 0);
}
