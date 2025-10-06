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
#include "kmmap.h"

BOOL zero_init(struct KProcess* process, struct FsOpenNode* node) {
    return TRUE;
}

S64 zero_length(struct FsOpenNode* node) {
    return 0;
}

BOOL zero_setLength(struct FsOpenNode* node, S64 len) {
    return FALSE;
}

S64 zero_getFilePointer(struct FsOpenNode* node) {
    return 0;
}

S64 zero_seek(struct FsOpenNode* node, S64 pos) {
    return 0;
}

U32 zero_read(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    zeroMemory(thread, address, len);
    return len;
}

U32 zero_write(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    return len;
}

void zero_close(struct FsOpenNode* node) {
    node->func->free(node);
}

U32 zero_ioctl(struct KThread* thread, struct FsOpenNode* node, U32 request) {
    return -K_ENODEV;
}

void zero_setAsync(struct FsOpenNode* node, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kwarn("null_setAsync not implemented");
}

BOOL zero_isAsync(struct FsOpenNode* node, struct KProcess* process) {
    return 0;
}

void zero_waitForEvents(struct FsOpenNode* node, struct KThread* thread, U32 events) {
    kpanic("null_waitForEvents not implemented");
}

BOOL zero_isWriteReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

BOOL zero_isReadReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 zero_map(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return syscall_mmap64(thread, address, len, prot, flags, -1, off);
}

BOOL zero_canMap(struct FsOpenNode* node) {
    return TRUE;
}

struct FsOpenNodeFunc zeroAccess = {zero_init, zero_length, zero_setLength, zero_getFilePointer, zero_seek, zero_read, zero_write, zero_close, zero_map, zero_canMap, zero_ioctl, zero_setAsync, zero_isAsync, zero_waitForEvents, zero_isWriteReady, zero_isReadReady};
