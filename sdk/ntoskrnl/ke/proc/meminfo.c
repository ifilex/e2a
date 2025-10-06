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
#include "platform.h"
#include "kerror.h"
#include "log.h"
#include "fsapi.h"

#include <stdio.h>
#include <string.h>

char meminfo[128];
U32 meminfoLen;

BOOL meminfo_init(struct KProcess* process, struct FsOpenNode* node) {
    sprintf(meminfo, "MemTotal: %d kB\nMemFree: %d kB\n", getPageCount()<<2, getFreePageCount()<<2);
    meminfoLen = (U32)strlen(meminfo)+1;
    node->idata = 0; // file pos
    return TRUE;
}

S64 meminfo_length(struct FsOpenNode* node) {
    return meminfoLen;
}

BOOL meminfo_setLength(struct FsOpenNode* node, S64 len) {
    return FALSE;
}

S64 meminfo_getFilePointer(struct FsOpenNode* node) {
    return node->idata;
}

S64 meminfo_seek(struct FsOpenNode* node, S64 pos) {
    if (pos>meminfoLen)
        pos = meminfoLen;
    return node->idata = (U32)pos;
}

U32 meminfo_read(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    U32 pos = node->idata;
    if (pos>=meminfoLen)
        return -1;
    if (pos+len>meminfoLen)
        len = meminfoLen-pos;
    memcopyFromNative(thread, address, meminfo+pos, len);
    node->idata+=len;
    return len;
}

U32 meminfo_write(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    return len;
}

void meminfo_close(struct FsOpenNode* node) {
    node->func->free(node);
}

U32 meminfo_ioctl(struct KThread* thread, struct FsOpenNode* node, U32 request) {
    return -K_ENODEV;
}

void meminfo_setAsync(struct FsOpenNode* node, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kwarn("meminfo_setAsync not implemented");
}

BOOL meminfo_isAsync(struct FsOpenNode* node, struct KProcess* process) {
    return 0;
}

void meminfo_waitForEvents(struct FsOpenNode* node, struct KThread* thread, U32 events) {
    kpanic("waiting on meminfo is not implemented");
}

BOOL meminfo_isWriteReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

BOOL meminfo_isReadReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 meminfo_map(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL meminfo_canMap(struct FsOpenNode* node) {
    return FALSE;
}

struct FsOpenNodeFunc meminfoAccess = {meminfo_init, meminfo_length, meminfo_setLength, meminfo_getFilePointer, meminfo_seek, meminfo_read, meminfo_write, meminfo_close, meminfo_map, meminfo_canMap, meminfo_ioctl, meminfo_setAsync, meminfo_isAsync, meminfo_waitForEvents, meminfo_isWriteReady, meminfo_isReadReady};
