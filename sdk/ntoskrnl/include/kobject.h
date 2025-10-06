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

#ifndef __KOBJECT_H__
#define __KOBJECT_H__

#include "platform.h"
#include "memory.h"
#include "fsapi.h"

#define KTYPE_FILE 0
#define KTYPE_SOCK 1
#define KTYPE_EPOLL 2
#define KTYPE_SIGNAL 3

struct KObject {
    struct KObjectAccess* access;
    U32 refCount;
    U32 type;
    struct FsOpenNode* openFile;
    struct KSocket* socket;
    void* data;
    U32 idata;
    U32 idata2;
    struct KObject* next; // used for free list
};

struct KObject* allocKObject(struct KObjectAccess* access, U32 type, struct FsOpenNode* openNode, struct KSocket* socket);
void closeKObject(struct KObject* kobject);
void writeStat(struct FsNode* node, struct KThread* thread, U32 buf, BOOL is64, U64 st_dev, U64 st_ino, U32 st_mode, U64 st_rdev, U64 st_size, U32 st_blksize, U64 st_blocks, U64 mtime, U32 linkCount);

#endif