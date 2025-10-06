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

#ifndef __KFILEDESCRIPTOR_H__
#define __KFILEDESCRIPTOR_H__

#include "kobject.h"
#include "platform.h"
#include "kthread.h"

#define K_F_DUPFD    0
#define K_F_GETFD    1
#define K_F_SETFD    2
#define K_F_GETFL    3
#define K_F_SETFL    4
#define K_F_GETLK	 5
#define K_F_SETLK	 6
#define K_F_SETLKW   7
#define K_F_SETOWN   8
#define K_F_SETSIG   10
#define K_F_GETSIG   11
#define K_F_GETLK64  12
#define K_F_SETLK64  13
#define K_F_SETLKW64 14
#define K_F_DUPFD_CLOEXEC 1030	

// type of lock
#define K_F_RDLCK	   0
#define K_F_WRLCK	   1
#define K_F_UNLCK	   2

struct KFileDescriptor {
    U32 accessFlags;
    U32 descriptorFlags;
    U32 handle;
    union {
        struct KObject* kobject;
        struct KFileDescriptor* next; // used for free list
    };
    U32 refCount;
    struct KProcess* process;
};

BOOL canReadFD(struct KFileDescriptor* fd);
BOOL canWriteFD(struct KFileDescriptor* fd);
void closeFD(struct KFileDescriptor* fd);
struct KFileDescriptor* allocFileDescriptor(struct KProcess* process, struct KObject* kobject, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle);

U32 syscall_fcntrl(struct KThread* thread, FD fildes, U32 cmd, U32 arg);
U32 syscall_select(struct KThread* thread, U32 nfds, U32 readfds, U32 writefds, U32 errorfds, U32 timeout);
U32 syscall_poll(struct KThread* thread, U32 pfds, U32 nfds, U32 timeout);

#endif
