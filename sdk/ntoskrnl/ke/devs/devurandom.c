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

#include <stdlib.h>
#include <time.h>

BOOL urandom_initialized;

BOOL urandom_init(struct KProcess* process, struct FsOpenNode* node) {
    if (!urandom_initialized) {
        urandom_initialized = TRUE;
        srand((U32)time(0));
    }
    return TRUE;
}

S64 urandom_length(struct FsOpenNode* node) {
    return 0;
}

BOOL urandom_setLength(struct FsOpenNode* node, S64 len) {
    return FALSE;
}

S64 urandom_getFilePointer(struct FsOpenNode* node) {
    return 0;
}

S64 urandom_seek(struct FsOpenNode* node, S64 pos) {
    return 0;
}

U32 urandom_read(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    U32 result = len;

    while (len>=4) {
        writed(thread, address, rand());
        address+=4;
        len-=4;
    }
    while (len>0) {
        writeb(thread, address, rand());
        address++;
        len--;
    }
    return result;
}

U32 urandom_write(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    return 0;
}

void urandom_close(struct FsOpenNode* node) {
    node->func->free(node);
}

U32 urandom_ioctl(struct KThread* thread, struct FsOpenNode* node, U32 request) {
    return -K_ENODEV;
}

void urandom_setAsync(struct FsOpenNode* node, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kwarn("urandom_setAsync not implemented");
}

BOOL urandom_isAsync(struct FsOpenNode* node, struct KProcess* process) {
    return 0;
}

void urandom_waitForEvents(struct FsOpenNode* node, struct KThread* thread, U32 events) {
    kpanic("urandom_waitForEvents not implemented");
}

BOOL urandom_isWriteReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

BOOL urandom_isReadReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 urandom_map(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL urandom_canMap(struct FsOpenNode* node) {
    return FALSE;
}

struct FsOpenNodeFunc urandomAccess = {urandom_init, urandom_length, urandom_setLength, urandom_getFilePointer, urandom_seek, urandom_read, urandom_write, urandom_close, urandom_map, urandom_canMap, urandom_ioctl, urandom_setAsync, urandom_isAsync, urandom_waitForEvents, urandom_isWriteReady, urandom_isReadReady};
