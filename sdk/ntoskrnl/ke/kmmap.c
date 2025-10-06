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
#include "kmmap.h"
#include "log.h"
#include "kprocess.h"
#include "kerror.h"
#include "kobjectaccess.h"
#include "softmmu/soft_memory.h"
#include "hardmmu/hard_memory.h"
#include "ksystem.h"
#include "kalloc.h"
#include "kio.h"
#include "kfile.h"
#include "loader.h"

#include <string.h>

U32 madvise(struct KThread* thread, U32 addr, U32 len, U32 advice) {
    if (advice!=K_MADV_DONTNEED)
        kpanic("madvise %d is not implemented", advice);
    return 0;
}

U32 syscall_mlock(struct KThread* thread, U32 addr, U32 len) {
    return 0;
}

U32 syscall_mmap64(struct KThread* thread, U32 addr, U32 len, S32 prot, S32 flags, FD fildes, U64 off) {
    BOOL shared = (flags & K_MAP_SHARED)!=0;
    BOOL priv = (flags & K_MAP_PRIVATE)!=0;
    BOOL read = (prot & K_PROT_READ)!=0;
    BOOL write = (prot & K_PROT_WRITE)!=0;
    BOOL exec = (prot & K_PROT_EXEC)!=0;
    U32 pageStart = addr >> PAGE_SHIFT;
    U32 pageCount = (len+PAGE_SIZE-1)>>PAGE_SHIFT;
    struct KFileDescriptor* fd = 0;
    U32 i;

    if ((shared && priv) || (!shared && !priv)) {
        return -K_EINVAL;
    }

    if (!(flags & K_MAP_ANONYMOUS) && fildes>=0) {
        fd = getFileDescriptor(thread->process, fildes);
        if (fd == 0) {
            return -K_EBADF;
        }
        if (!fd->kobject->access->canMap(fd->kobject)) {
            return -K_EACCES;
        }
        if (len==0 || (off & 0xFFF)!=0) {
            return -K_EINVAL;
        }
        if ((!canReadFD(fd) && read) || (!priv && (!canWriteFD(fd) && write))) {
            return -K_EACCES;
        }
    }        
    if (flags & K_MAP_FIXED) {
        if (addr & (PAGE_SIZE-1)) {
#ifdef _DEBUG
            klog("tried to call mmap with invalid address: %X", addr);
            return -K_EINVAL;
#endif
        }
    } else {		
        if (pageStart + pageCount> ADDRESS_PROCESS_MMAP_START)
            return -K_ENOMEM;
        if (pageStart == 0)
            pageStart = ADDRESS_PROCESS_MMAP_START;
        if (!findFirstAvailablePage(thread->process->memory, pageStart, pageCount, &pageStart, addr!=0)) {
            // :TODO: what erro
            return -K_EINVAL;
        }
        if (addr!=0 && pageStart+pageCount> ADDRESS_PROCESS_MMAP_START)
            return -K_ENOMEM;
        addr = pageStart << PAGE_SHIFT;	
    }
    if (fd) {
        U32 result = fd->kobject->access->map(thread, fd->kobject, addr, len, prot, flags, off);
        if (result) {
            return result;
        }
    }

	// even if there are no permissions, it is important for MAP_ANONYMOUS|MAP_FIXED existing memory to be 0'd out
    // if (write || read || exec)
	{		
        U32 permissions = PAGE_MAPPED;

        if (write)
            permissions|=PAGE_WRITE;
        if (read)
            permissions|=PAGE_READ;
        if (exec)
            permissions|=PAGE_EXEC;
        if (shared)
            permissions|=PAGE_SHARED;
        if (fd) {	
            struct KProcess* process = thread->process;
            int index = -1;

            for (i=0;i<MAX_MAPPED_FILE;i++) {
                if (!process->mappedFiles[i].refCount) {
                    index = i;
                    break;
                }
            }
            if (index<0) {
                kwarn("MAX_MAPPED_FILE is not large enough");
            } else {
                process->mappedFiles[index].address = pageStart << PAGE_SHIFT;
                process->mappedFiles[index].len = pageCount << PAGE_SHIFT;
                process->mappedFiles[index].offset = off;     
#ifdef BOXEDWINE_VM
                process->mappedFiles[index].refCount = 1;
#else
                process->mappedFiles[index].refCount = 0;
#endif
                process->mappedFiles[index].file = fd->kobject;
                process->mappedFiles[index].file->refCount++;
            }
            allocPages(thread, pageStart, pageCount, permissions, fildes, off, index);
        } else {
            allocPages(thread, pageStart, pageCount, permissions, 0, 0, 0);
        }		
    }
    return addr;
}

U32 syscall_mprotect(struct KThread* thread, U32 address, U32 len, U32 prot) {
    BOOL read = (prot & K_PROT_READ)!=0;
    BOOL write = (prot & K_PROT_WRITE)!=0;
    BOOL exec = (prot & K_PROT_EXEC)!=0;
    U32 pageStart = address >> PAGE_SHIFT;
    U32 pageCount = (len+PAGE_SIZE-1)>>PAGE_SHIFT;
    U32 permissions = 0;
    U32 i;

    if (write)
        permissions|=PAGE_WRITE;
    if (read)
        permissions|=PAGE_READ;
    if (exec)
        permissions|=PAGE_EXEC;

    for (i=pageStart;i<pageStart+pageCount;i++) {
        protectPage(thread, i, permissions);
    }
    return 0;
}

U32 syscall_unmap(struct KThread* thread, U32 address, U32 len) {
    U32 pageStart = address >> PAGE_SHIFT;
    U32 pageCount = (len+PAGE_SIZE-1)>>PAGE_SHIFT;
    U32 i;

    for (i=0;i<pageCount;i++) {
        freePage(thread, pageStart+i);
    }
    return 0;
}

U32 syscall_mremap(struct KThread* thread, U32 oldaddress, U32 oldsize, U32 newsize, U32 flags) {
    if (flags > 1) {
        kpanic("__NR_mremap not implemented: flags=%X", flags);
    }
    if (newsize<oldsize) {
        syscall_unmap(thread, oldaddress+newsize, oldsize-newsize);
        return oldaddress;
    } else {
        struct Memory* memory = thread->process->memory;
        U32 result;
        U32 prot=0;
        U32 flags = memory->flags[oldaddress >> PAGE_SHIFT];
        U32 f = K_MAP_FIXED;
        if (IS_PAGE_READ(flags)) {
            prot|=K_PROT_READ;
        }
        if (IS_PAGE_WRITE(flags)) {
            prot|=K_PROT_WRITE;
        }
        if (IS_PAGE_EXEC(flags)) {
            prot|=K_PROT_EXEC;
        }
        if (IS_PAGE_SHARED(flags)) {
            f|=K_MAP_SHARED;
        } else {
            f|=K_MAP_PRIVATE;
        }
        result = syscall_mmap64(thread, oldaddress+oldsize, newsize-oldsize, prot, f, -1, 0);
        if (result==oldaddress+oldsize) {
            return oldaddress;
        }
       
        if ((flags & 1)!=0) { // MREMAP_MAYMOVE
            kpanic("__NR_mremap not implemented");
            return -K_ENOMEM;
        } else {
            return -K_ENOMEM;
        }
    }
}

U32 syscall_msync(struct KThread* thread, U32 addr, U32 len, U32 flags) {
    struct MapedFiles* file = 0;
    U32 i;

    for (i=0;i<MAX_MAPPED_FILE;i++) {
        if (thread->process->mappedFiles[i].refCount && addr>=thread->process->mappedFiles[i].address && thread->process->mappedFiles[i].address+thread->process->mappedFiles[i].len<addr) {
            file = & thread->process->mappedFiles[i];
        }
    }
    if (!file)
        return -K_ENOMEM;
    klog("syscall_msync not implemented");
    return 0;
}