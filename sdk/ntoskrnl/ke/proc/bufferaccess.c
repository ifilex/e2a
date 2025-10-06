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

#include <string.h>

BOOL buffer_init(struct KProcess* process, struct FsOpenNode* node) {	
    node->idata = 0; // file pos
    return TRUE;
}

S64 buffer_length(struct FsOpenNode* node) {
    return strlen((char*)node->func->data);
}

BOOL buffer_setLength(struct FsOpenNode* node, S64 len) {
    return FALSE;
}

S64 buffer_getFilePointer(struct FsOpenNode* node) {
    return node->idata;
}

S64 buffer_seek(struct FsOpenNode* node, S64 pos) {
    if (pos>(S64)strlen((char*)node->func->data))
        pos = strlen((char*)node->func->data);
    return node->idata = (U32)pos;
}

U32 buffer_read(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    U32 pos = node->idata;
    if (pos+len>strlen((char*)node->func->data))
        len = (U32)strlen((char*)node->func->data)-pos;
    memcopyFromNative(thread, address, ((char*)node->func->data)+pos, len);
    return len;
}

U32 buffer_write(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    U32 pos = node->idata;
    if (pos+len>node->func->dataLen)
        len = node->func->dataLen-pos;
    memcopyToNative(thread, address, ((char*)node->func->data)+pos, len);
    return len;
}

void buffer_close(struct FsOpenNode* node) {
    node->func->free(node);
}

U32 buffer_ioctl(struct KThread* thread, struct FsOpenNode* node, U32 request) {
    return -K_ENODEV;
}

void buffer_setAsync(struct FsOpenNode* node, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kwarn("buffer_setAsync not implemented");
}

BOOL buffer_isAsync(struct FsOpenNode* node, struct KProcess* process) {
    return 0;
}

void buffer_waitForEvents(struct FsOpenNode* node, struct KThread* thread, U32 events) {
    kpanic("buffer_waitForEvents not implemented");
}

BOOL buffer_isWriteReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

BOOL buffer_isReadReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 buffer_map(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL buffer_canMap(struct FsOpenNode* node) {
    return FALSE;
}

struct FsOpenNodeFunc bufferAccess = {buffer_init, buffer_length, buffer_setLength, buffer_getFilePointer, buffer_seek, buffer_read, buffer_write, buffer_close, buffer_map, buffer_canMap, buffer_ioctl, buffer_setAsync, buffer_isAsync, buffer_waitForEvents, buffer_isWriteReady, buffer_isReadReady};

void makeBufferAccess(struct FsOpenNodeFunc* nodeAccess) {
    nodeAccess->init = buffer_init;
    nodeAccess->length = buffer_length;
    nodeAccess->setLength = buffer_setLength;
    nodeAccess->getFilePointer = buffer_getFilePointer;
    nodeAccess->seek = buffer_seek;
    nodeAccess->read = buffer_read;
    nodeAccess->write = buffer_write;
    nodeAccess->close = buffer_close;
    nodeAccess->canMap = buffer_canMap;
    nodeAccess->ioctl = buffer_ioctl;
    nodeAccess->isWriteReady = buffer_isWriteReady;
    nodeAccess->isReadReady = buffer_isReadReady;
}