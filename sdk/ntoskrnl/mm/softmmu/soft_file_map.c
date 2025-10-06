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
#include "kfiledescriptor.h"
#include "kprocess.h"
#include "kobject.h"
#include "kobjectaccess.h"
#include "kfiledescriptor.h"
#include "ksystem.h"
#include "soft_memory.h"
#include "soft_ram.h"
#include "kalloc.h"

#include <string.h>

// :TODO: what about sync'ing the writes back to the file?
#ifndef BOXEDWINE_64BIT_MMU
static void ondemmandFile(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
    U32 page = address >> PAGE_SHIFT;
    U32 flags = memory->flags[page];
    struct MapedFiles* mapped = &memory->process->mappedFiles[memory->ramPage[page] & 0xFFF];
    U32 ramPageIndexInCache = memory->ramPage[page] >> 12;
    U32 offset = ramPageIndexInCache << PAGE_SHIFT;
    U32 ram = 0;
    BOOL read = IS_PAGE_READ(flags) | IS_PAGE_EXEC(flags);
    BOOL write = IS_PAGE_WRITE(flags);
    U32 len;
    U64 oldPos;
    BOOL inCache = 0;	

    address = address & (~PAGE_MASK);
    if (!write) {
        ram = mapped->systemCacheEntry->ramPages[ramPageIndexInCache];
        if (ram) {
            incrementRamRef(ram);
            inCache = 1;
        } else {
            ram = allocRamPage();
            mapped->systemCacheEntry->ramPages[ramPageIndexInCache] = ram;
            incrementRamRef(ram);
        }
        memory->mmu[page] = & ramPageRO; // :TODO: what if an app uses mprotect to change this?
        memory->read[page] = TO_TLB(ram,  address);		
    } else {
        ram = allocRamPage();
        if (read && write) {
            memory->mmu[page] = &ramPageWR;
            memory->read[page] = TO_TLB(ram,  address);
        } else if (write) {
            memory->mmu[page] = &ramPageWO;
        } else {
            memory->mmu[page] = &ramPageRO;		
            memory->read[page] = TO_TLB(ram,  address);
        }
    }
    memory->flags[page] = GET_PAGE_PERMISSIONS(flags) | PAGE_IN_RAM;
    memory->ramPage[page] = ram;
    // filling the cache needs this
    memory->write[page] = TO_TLB(ram,  address);

    if (!inCache) {
        oldPos = mapped->file->access->getPos(mapped->file);
        mapped->file->access->seek(mapped->file, offset);
        len = mapped->file->access->read(thread, mapped->file, address, PAGE_SIZE);
        mapped->file->access->seek(mapped->file, oldPos);
        if (len<PAGE_SIZE) {
            // don't call zeroMemory because it might be read only
            memset(getAddressOfRamPage(ram)+len, 0, PAGE_SIZE-len);
        }
    }
    if (!write)
        memory->write[page] = 0;
    closeMemoryMapped(mapped);
}

static U8 ondemandfile_readb(struct KThread* thread, U32 address) {	
    ondemmandFile(thread, address);	
    return readb(thread, address);
}

static void ondemandfile_writeb(struct KThread* thread, U32 address, U8 value) {
    ondemmandFile(thread, address);	
    writeb(thread, address, value);
}

static U16 ondemandfile_readw(struct KThread* thread, U32 address) {
    ondemmandFile(thread, address);	
    return readw(thread, address);
}

static void ondemandfile_writew(struct KThread* thread, U32 address, U16 value) {
    ondemmandFile(thread, address);	
    writew(thread, address, value);
}

static U32 ondemandfile_readd(struct KThread* thread, U32 address) {
    ondemmandFile(thread, address);	
    return readd(thread, address);
}

static void ondemandfile_writed(struct KThread* thread, U32 address, U32 value) {
    ondemmandFile(thread, address);	
    writed(thread, address, value);
}

static void ondemandfile_clear(struct Memory* memory, U32 page) {
    struct MapedFiles* mapped = &memory->process->mappedFiles[memory->ramPage[page] & 0xFFF];
    closeMemoryMapped(mapped);
}

static U8* ondemandfile_physicalAddress(struct KThread* thread, U32 address) {
    ondemmandFile(thread, address);
    return getPhysicalAddress(thread, address);
}

struct Page ramOnDemandFilePage = {ondemandfile_readb, ondemandfile_writeb, ondemandfile_readw, ondemandfile_writew, ondemandfile_readd, ondemandfile_writed, ondemandfile_clear, ondemandfile_physicalAddress};

void closeMemoryMapped(struct MapedFiles* mapped) {
    mapped->refCount--;
    if (mapped->refCount == 0) {
        closeKObject(mapped->file);
        mapped->file = 0;
        mapped->systemCacheEntry->refCount--;
        if (mapped->systemCacheEntry->refCount == 0) {			
            U32 i;
            for (i=0;i<mapped->systemCacheEntry->pageCount;i++) {
                if (mapped->systemCacheEntry->ramPages[i])
                    freeRamPage(mapped->systemCacheEntry->ramPages[i]);
            }
            kfree(mapped->systemCacheEntry->ramPages, KALLOC_MMAP_CACHE_RAMPAGE);
            removeMappedFileInCache(mapped->systemCacheEntry);
            kfree(mapped->systemCacheEntry, KALLOC_MAPPEDFILECACHE);
        }
    }
}

#endif