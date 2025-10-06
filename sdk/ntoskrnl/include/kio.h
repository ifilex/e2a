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

#ifndef __KIO_H__
#define __KIO_H__

#include "platform.h"
#include "kthread.h"

U32 syscall_close(struct KThread* thread, FD handle);
U32 syscall_open(struct KThread* thread, const char* currentDirectory, U32 name, U32 flags);
U32 syscall_openat(struct KThread* thread, FD dirfd, U32 name, U32 flags);
U32 syscall_read(struct KThread* thread, FD handle, U32 buffer, U32 len);
U32 syscall_write(struct KThread* thread, FD handle, U32 buffer, U32 len);
U32 syscall_writev(struct KThread* thread, FD handle, U32 iov, S32 iovcnt);
S32 syscall_seek(struct KThread* thread, FD handle, S32 offset, U32 whence);
U32 syscall_fstat64(struct KThread* thread, FD handle, U32 buf);
U32 syscall_access(struct KThread* thread, U32 fileName, U32 flags);
U32 syscall_faccessat(struct KThread* thread, U32 dirfd, U32 pathname, U32 mode, U32 flags);
U32 syscall_ftruncate64(struct KThread* thread, FD fildes, U64 length);
U32 syscall_stat64(struct KThread* thread, U32 path, U32 buffer);
U32 syscall_ioctl(struct KThread* thread, FD fildes, U32 request);
U32 syscall_dup2(struct KThread* thread, FD fildes, FD fildes2);
U32 syscall_dup(struct KThread* thread, FD fildes);
U32 syscall_unlink(struct KThread* thread, U32 path);
U32 syscall_fchmod(struct KThread* thread, FD fd, U32 mod);
U32 syscall_rename(struct KThread* thread, U32 oldName, U32 newName);
U32 syscall_lstat64(struct KThread* thread, U32 path, U32 buffer);
S64 syscall_llseek(struct KThread* thread, FD fildes, S64 offset, U32 whence);
U32 syscall_getdents(struct KThread* thread, FD fildes, U32 dirp, U32 count, BOOL is64);
U32 syscall_readlink(struct KThread* thread, U32 path, U32 buffer, U32 bufSize);
U32 syscall_readlinkat(struct KThread* thread, FD dirfd, U32 pathname, U32 buf, U32 bufsiz);
U32 syscall_mkdir(struct KThread* thread, U32 path, U32 mode);
U32 syscall_mkdirat(struct KThread* thread, U32 dirfd, U32 path, U32 mode);
U32 syscall_fstatfs64(struct KThread* thread, FD fildes, U32 len, U32 address);
U32 syscall_statfs64(struct KThread* thread, U32 path, U32 len, U32 address);
U32 syscall_statfs(struct KThread* thread, U32 path,U32 address);
U32 syscall_pread64(struct KThread* thread, FD fildes, U32 address, U32 len, U64 offset);
U32 syscall_rmdir(struct KThread* thread, U32 path);
U32 syscall_pwrite64(struct KThread* thread, FD fildes, U32 address, U32 len, U64 offset);
U32 syscall_fstatat64(struct KThread* thread, FD dirfd, U32 address, U32 buf, U32 flag);
U32 syscall_unlinkat(struct KThread* thread, FD dirfd, U32 address, U32 flags);
U32 syscall_utimesat(struct KThread* thread, FD dirfd, U32 path, U32 times, U32 flags);
U32 syscall_utimes(struct KThread* thread, U32 path, U32 times);
#endif