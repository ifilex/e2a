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
#include "kobjectaccess.h"
#include "kalloc.h"
#include "ksystem.h"
#include <string.h>
#include "kscheduler.h"

static struct KObject* freeKObjects;

#ifdef BOXEDWINE_VM
SDL_mutex* kobjectMutex;
#endif

struct KObject* allocKObject(struct KObjectAccess* access, U32 type, struct FsOpenNode* openNode, struct KSocket* socket) {
    struct KObject* result;

#ifdef BOXEDWINE_VM
    if (!kobjectMutex)
        kobjectMutex = SDL_CreateMutex();
#endif
    BOXEDWINE_LOCK(NULL, kobjectMutex);
    if (freeKObjects) {
        result = freeKObjects;
        freeKObjects = freeKObjects->next;
        memset(result, 0, sizeof(struct KObject));
    } else {
        result = (struct KObject*)kalloc(sizeof(struct KObject), KALLOC_KOBJECT);
    }
    BOXEDWINE_UNLOCK(NULL, kobjectMutex);
    result->access = access;
    result->refCount = 1;
    result->openFile = openNode;
    result->socket = socket;
    result->type = type;
    return result;
}

void closeKObject(struct KObject* kobject) {
    BOXEDWINE_LOCK(NULL, kobjectMutex);
    kobject->refCount--;
    if (kobject->refCount==0) {
        kobject->access->onDelete(kobject);
        kobject->next = freeKObjects;
        freeKObjects = kobject;
    }
    BOXEDWINE_UNLOCK(NULL, kobjectMutex);
}

 void writeStat(struct FsNode* node, struct KThread* thread, U32 buf, BOOL is64, U64 st_dev, U64 st_ino, U32 st_mode, U64 st_rdev, U64 st_size, U32 st_blksize, U64 st_blocks, U64 mtime, U32 linkCount) {
     if (is64) {
        U32 t = (U32)(mtime/1000); // ms to sec
        U32 n = (U32)(mtime % 1000) * 1000000;

        writeq(thread, buf, st_dev);buf+=8;//st_dev               // 0
        buf+=4; // padding                                          // 8
        writed(thread, buf, (U32)st_ino); buf += 4;//__st_ino     // 12
        writed(thread, buf, st_mode); buf += 4;//st_mode          // 16
        writed(thread, buf, linkCount); buf += 4;//st_nlink       // 20
        if (node && !strcmp("/etc/sudoers", node->path)) {
            writed(thread, buf, 0); buf += 4;//st_uid             // 24
            writed(thread, buf, 0); buf += 4;//st_gid               // 28
        } else {
            writed(thread, buf, thread->process->userId); buf += 4;//st_uid           // 24
            writed(thread, buf, thread->process->groupId); buf += 4;//st_gid               // 28
        }        
        writeq(thread, buf, st_rdev); buf += 8;//st_rdev          // 32
        buf+=4;                                                     // 40
        writeq(thread, buf, st_size); buf += 8;//st_size          // 44
        writed(thread, buf, st_blksize); buf += 4;//st_blksize    // 52
        writeq(thread, buf, st_blocks); buf += 8; //st_blocks     // 56
        writed(thread, buf, t); buf += 4; // st_atime             // 64
        writed(thread, buf, n); buf += 4; // st_atime_nsec        // 68
        writed(thread, buf, t); buf += 4; // st_mtime             // 72
        writed(thread, buf, n); buf += 4; // st_mtime_nsec        // 76
        writed(thread, buf, t); buf += 4; // st_ctime             // 80
        writed(thread, buf, n); buf += 4; // st_ctime_nsec        // 84
        writeq(thread, buf, st_ino); // st_ino                    // 88
     } else {
        U32 t = (U32)(mtime/1000); // ms to sec
        writed(thread, buf, (U32)st_dev); buf += 4;//st_dev
        writed(thread, buf, (U32)st_ino); buf += 4;//st_ino
        writed(thread, buf, st_mode); buf += 4;//st_mode
        writed(thread, buf, linkCount); buf += 4;//st_nlink
        writed(thread, buf, thread->process->userId); buf += 4;//st_uid
        writed(thread, buf, thread->process->groupId); buf += 4;//st_gid
        writed(thread, buf, (U32)st_rdev); buf += 4;//st_rdev
        writed(thread, buf, (U32)st_size); buf += 4;//st_size
        writed(thread, buf, t); buf += 4;//st_atime
        writed(thread, buf, t); buf += 4;//st_mtime
        writed(thread, buf, t); buf += 4;//st_ctime
        writed(thread, buf, st_blksize); buf += 4;//st_blksize (not used on wine)
        writed(thread, buf, (U32)st_blocks);//st_blocks
     }
}
