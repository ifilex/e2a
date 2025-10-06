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
#ifndef BOXEDWINE_64BIT_MMU
#include "memory.h"
#include "log.h"
#include "soft_ram.h"
#include "kscheduler.h"
#include "kprocess.h"
#include "kalloc.h"
#include "soft_file_map.h"
#include "ksignal.h"
#include "ksystem.h"
#include "soft_memory.h"
#include "../cpu/decodedata.h"
#include "decoder.h"
#include "kobject.h"
#include "kobjectaccess.h"
#include "soft_ram.h"
#include "soft_native_page.h"

#include <string.h>
#include <setjmp.h>

//#undef LOG_OPS

extern jmp_buf runBlockJump;
extern U8* ram;

void log_pf(struct KThread* thread, U32 address) {
    U32 start = 0;
    U32 i;
    struct CPU* cpu = &thread->cpu;
    struct KProcess* process = thread->process;

    printf("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X\n", cpu->segAddress[CS] + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, getModuleName(cpu, cpu->segAddress[CS]+cpu->eip.u32), getModuleEip(cpu, cpu->segAddress[CS]+cpu->eip.u32));

    printf("Page Fault at %.8X\n", address);
    printf("Valid address ranges:\n");
    for (i=0;i<NUMBER_OF_PAGES;i++) {
        if (!start) {
            if (process->memory->mmu[i] != &invalidPage) {
                start = i;
            }
        } else {
            if (process->memory->mmu[i] == &invalidPage) {
                printf("    %.8X - %.8X\n", start*PAGE_SIZE, i*PAGE_SIZE);
                start = 0;
            }
        }
    }
    printf("Mapped Files:\n");
    for (i=0;i<MAX_MAPPED_FILE;i++) {
        if (process->mappedFiles[i].refCount)
            printf("    %.8X - %.8X %s\n", process->mappedFiles[i].address, process->mappedFiles[i].address+(int)process->mappedFiles[i].len, process->mappedFiles[i].file->openFile->node->path);
    }
    walkStack(cpu, cpu->eip.u32, EBP, 2);
    kpanic("pf");
}

void seg_mapper(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;

    if (memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {
        U32 eip = thread->cpu.eip.u32;

        memory->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        memory->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        memory->process->sigActions[K_SIGSEGV].sigInfo[2] = 1; // SEGV_MAPERR
        memory->process->sigActions[K_SIGSEGV].sigInfo[3] = address;
        runSignal(thread, thread, K_SIGSEGV, EXCEPTION_PAGE_FAULT, 0);
#ifdef BOXEDWINE_HAS_SETJMP
        longjmp(runBlockJump, 1);		
#else
        runUntil(thread, eip);
#endif
    } else {
        log_pf(thread, address);
    }
}

void seg_access(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;

    if (memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {
        U32 eip = thread->cpu.eip.u32;

        memory->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        memory->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        memory->process->sigActions[K_SIGSEGV].sigInfo[2] = 2; // SEGV_ACCERR
        memory->process->sigActions[K_SIGSEGV].sigInfo[3] = address;
        runSignal(thread, thread, K_SIGSEGV, EXCEPTION_PERMISSION, 0);
        printf("seg fault %X\n", address);
#ifdef BOXEDWINE_HAS_SETJMP
        longjmp(runBlockJump, 1);		
#else 
        runUntil(thread, eip);
#endif
    } else {
        log_pf(thread, address);
    }
}

U8 invalid_readb(struct KThread* thread, U32 address) {
    seg_mapper(thread, address);
    return 0;
}

void invalid_writeb(struct KThread* thread, U32 address, U8 value) {
    seg_mapper(thread, address);
}

U16 invalid_readw(struct KThread* thread, U32 address) {
    seg_mapper(thread, address);
    return 0;
}

void invalid_writew(struct KThread* thread, U32 address, U16 value) {
    seg_mapper(thread, address);
}

U32 invalid_readd(struct KThread* thread, U32 address) {
    seg_mapper(thread, address);
    return 0;
}

void invalid_writed(struct KThread* thread, U32 address, U32 value) {
    seg_mapper(thread, address);
}


U8 nopermission_readb(struct KThread* thread, U32 address) {
    seg_access(thread, address);
    return 0;
}

void nopermission_writeb(struct KThread* thread, U32 address, U8 value) {
    seg_access(thread, address);
}

U16 nopermission_readw(struct KThread* thread, U32 address) {
    seg_access(thread, address);
    return 0;
}

void nopermission_writew(struct KThread* thread, U32 address, U16 value) {
    seg_access(thread, address);
}

U32 nopermission_readd(struct KThread* thread, U32 address) {
    seg_access(thread, address);
    return 0;
}

void nopermission_writed(struct KThread* thread, U32 address, U32 value) {
    seg_access(thread, address);
}

void pf_clear(struct Memory* memory, U32 page) {
}

static U8* invalid_physicalAddress(struct KThread* thread, U32 address) {
    return 0;
}

struct Page invalidPage = {invalid_readb, invalid_writeb, invalid_readw, invalid_writew, invalid_readd, invalid_writed, pf_clear, invalid_physicalAddress};

U8 readb(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
    int index = address >> 12;
#ifdef LOG_OPS
    U8 result;
    if (memory->read[index])
        result = host_readb(address-memory->read[index]);
    else
        result = memory->mmu[index]->readb(thread, address);
    if (memory->log && thread->cpu.log)
        fprintf(logFile, "readb %X @%X\n", result, address);
    return result;
#else
    if (memory->read[index])
        return host_readb(address-memory->read[index]);
    return memory->mmu[index]->readb(thread, address);
#endif
}

void writeb(struct KThread* thread, U32 address, U8 value) {
    struct Memory* memory = thread->process->memory;
    int index = address >> 12;
#ifdef LOG_OPS
    if (memory->log && thread->cpu.log)
        fprintf(logFile, "writeb %X @%X\n", value, address);
#endif
    if (memory->write[index]) {
        host_writeb(address-memory->write[index], value);
    } else {
        memory->mmu[index]->writeb(thread, address, value);
    }
}

U16 readw(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
#ifdef LOG_OPS
    U16 result;

    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
        if (memory->read[index])
            result = host_readw(address-memory->read[index]);
        else 
            result = memory->mmu[index]->readw(thread, address);
    } else {
        result = readb(thread, address) | (readb(thread, address+1) << 8);
    }
    if (memory->log && thread->cpu.log)
        fprintf(logFile, "readw %X @%X\n", result, address);
    return result;
#else
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
        if (memory->read[index])
            return host_readw(address-memory->read[index]);
        return memory->mmu[index]->readw(thread, address);
    }
    return readb(thread, address) | (readb(thread, address+1) << 8);
#endif
}

void writew(struct KThread* thread, U32 address, U16 value) {
    struct Memory* memory = thread->process->memory;
#ifdef LOG_OPS
    if (memory->log && thread->cpu.log)
        fprintf(logFile, "writew %X @%X\n", value, address);
#endif
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
        if (memory->write[index]) {
            host_writew(address-memory->write[index], value);
        } else {
            memory->mmu[index]->writew(thread, address, value);
        }
    } else {
        writeb(thread, address, (U8)value);
        writeb(thread, address+1, (U8)(value >> 8));
    }
}

U32 readd(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
#ifdef LOG_OPS
    U32 result;

    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
        if (memory->read[index])
            result = host_readd(address-memory->read[index]);
        else
            result = memory->mmu[index]->readd(thread, address);
    } else {
        result = readb(thread, address) | (readb(thread, address+1) << 8) | (readb(thread, address+2) << 16) | (readb(thread, address+3) << 24);
    }
    if (memory->log && thread->cpu.log)
        fprintf(logFile, "readd %X @%X\n", result, address);
    return result;
#else
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
        if (memory->read[index])
            return host_readd(address-memory->read[index]);
        return memory->mmu[index]->readd(thread, address);
    } else {
        return readb(thread, address) | (readb(thread, address+1) << 8) | (readb(thread, address+2) << 16) | (readb(thread, address+3) << 24);
    }
#endif
}

void writed(struct KThread* thread, U32 address, U32 value) {
    struct Memory* memory = thread->process->memory;
#ifdef LOG_OPS
    if (memory->log && thread->cpu.log)
        fprintf(logFile, "writed %X @%X\n", value, address);
#endif
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
        if (memory->write[index]) {
            host_writed(address-memory->write[index], value);
        } else {
            memory->mmu[index]->writed(thread, address, value);
        }		
    } else {
        writeb(thread, address, value);
        writeb(thread, address+1, value >> 8);
        writeb(thread, address+2, value >> 16);
        writeb(thread, address+3, value >> 24);
    }
}

struct Memory* allocMemory(struct KProcess* process) {
    struct Memory* memory = (struct Memory*)kalloc(sizeof(struct Memory), KALLOC_MEMORY);    
    initMemory(memory);
    memory->process = process;
    return memory;
}

void initMemory(struct Memory* memory) {
    int i=0;

    memset(memory, 0, sizeof(struct Memory));
    for (i=0;i<0x100000;i++) {
        memory->mmu[i] = &invalidPage;
    }
}

void resetMemory(struct Memory* memory) {
    U32 i=0;
    for (i=0;i<0x100000;i++) {
        memory->mmu[i]->clear(memory, i);
        memory->mmu[i] = &invalidPage;
        memory->flags[i] = 0;
        memory->read[i] = 0;
        memory->write[i] = 0;
        memory->ramPage[i] = 0;
    }
}

void cloneMemory(struct KThread* thread, struct KThread* fromThread) {
    int i=0;
    struct KProcess* p = thread->process;
    struct Memory* from = fromThread->process->memory;
    struct Memory* memory = thread->process->memory;

    memcpy(memory, from, sizeof(struct Memory));
    memory->process = p;
    for (i=0;i<0x100000;i++) {
        struct Page* page = memory->mmu[i];
        if (page == &ramPageRO || page == &ramPageWR || page == &ramPageWO || page == &codePage) {
            if (!IS_PAGE_SHARED(memory->flags[i])) {
                memory->mmu[i] = &ramCopyOnWritePage;
                from->mmu[i] = &ramCopyOnWritePage;
                memory->write[i] = 0;
                from->write[i] = 0;
            }
            incrementRamRef(memory->ramPage[i]);
        } else if (page == &softNativePage) {
            kpanic("softNativePage doesn't support clone");
        } else if (page == &ramCopyOnWritePage) {
            incrementRamRef(memory->ramPage[i]);
        } else if (IS_PAGE_SHARED(memory->flags[i])) {
            if (page == &ramOnDemandPage) {
                writeb(fromThread, i << PAGE_SHIFT, 0); // this will map the address to a real page of ram
                memory->mmu[i] = from->mmu[i];
                memory->flags[i] = from->flags[i];
                memory->ramPage[i] = from->ramPage[i];
                memory->read[i] = from->read[i];
                memory->write[i] = from->write[i];
                i--;
            } else if (page == &ramOnDemandFilePage) { 
                readb(fromThread, i << PAGE_SHIFT); // this will map the address to a real page of ram
                memory->mmu[i] = from->mmu[i];
                memory->flags[i] = from->flags[i];
                memory->ramPage[i] = from->ramPage[i];
                memory->read[i] = from->read[i];
                memory->write[i] = from->write[i];
                i--;
            } else {
                kpanic("Unhandled shared memory clone");
            }
        }
    }
}

void freeMemory(struct Memory* memory) {
    int i;

    for (i=0;i<0x100000;i++) {   
        memory->mmu[i]->clear(memory, i);    
    }
    kfree(memory, KALLOC_MEMORY);
}

void zeroMemory(struct KThread* thread, U32 address, int len) {
    int i;
    for (i=0;i<len;i++) {
        writeb(thread, address, 0);
        address++;
    }
}

void readMemory(struct KThread* thread, U8* data, U32 address, int len) {
    int i;
    for (i=0;i<len;i++) {
        *data=readb(thread, address);
        address++;
        data++;
    }
}

void writeMemory(struct KThread* thread, U32 address, U8* data, int len) {
    int i;
    for (i=0;i<len;i++) {
        writeb(thread, address, *data);
        address++;
        data++;
    }
}

void allocPages(struct KThread* thread, U32 page, U32 pageCount, U8 permissions, U32 fildes, U64 offset, U32 cacheIndex) {
    U32 i;
    U32 address = page << PAGE_SHIFT;
    struct Page* pageType;
    struct Memory* memory = thread->process->memory;

    if (fildes) {
        struct KFileDescriptor* fd = getFileDescriptor(memory->process, fildes);

        int filePage = (int)(offset>>PAGE_SHIFT);
        struct MappedFileCache* cache;
        struct KProcess* process = memory->process;

        cache = getMappedFileInCache(fd->kobject->openFile->node->path);
        if (!cache) {
            cache = (struct MappedFileCache*)kalloc(sizeof(struct MappedFileCache), KALLOC_MAPPEDFILECACHE);
            safe_strcpy(cache->name, fd->kobject->openFile->node->path, MAX_FILEPATH_LEN);
            cache->pageCount = (U32)((fd->kobject->access->length(fd->kobject) + PAGE_SIZE-1) >> PAGE_SHIFT);
            cache->ramPages = (U32*)kalloc(sizeof(U32)*cache->pageCount, KALLOC_MMAP_CACHE_RAMPAGE);
            putMappedFileInCache(cache);				
        }
        cache->refCount++;

        pageType = &ramOnDemandFilePage;

        if (cacheIndex>0xFFF || filePage>0xFFFFF) {
            kpanic("mmap: couldn't page file mapping info to memory data: fildes=%d filePage=%d", fd, filePage);
        }
        if (offset & PAGE_MASK) {
            kpanic("mmap: wasn't expecting the offset to be in the middle of a page");
        }
        process->mappedFiles[cacheIndex].systemCacheEntry = cache;        

        for (i=0;i<pageCount;i++) {
            if (memory->mmu[page]!=&invalidPage) {
                memory->mmu[page]->clear(memory, page);
            }
            memory->mmu[page] = pageType;
            memory->flags[page] = permissions;
            memory->ramPage[page] = cacheIndex | (filePage++ << 12);
            memory->read[page] = 0;
            memory->write[page] = 0;

            memory->process->mappedFiles[cacheIndex].refCount++;
            page++;
        }
    } else {
        pageType = &ramOnDemandPage;

        for (i=0;i<pageCount;i++) {
            if (memory->mmu[page]!=&invalidPage) {
                memory->mmu[page]->clear(memory, page);
            }
            memory->mmu[page] = pageType;
            memory->flags[page] = permissions;
            memory->ramPage[page] = 0;
            memory->read[page] = 0;
            memory->write[page] = 0;
            page++;
        }
    }    
}

void protectPage(struct KThread* thread, U32 i, U32 permissions) {
    struct Memory* memory = thread->process->memory;
    struct Page* page = memory->mmu[i];
    U32 flags = memory->flags[i];

    flags&=~PAGE_PERMISSION_MASK;
    flags|=(permissions & PAGE_PERMISSION_MASK);
    memory->flags[i] = flags;

    if (page == &invalidPage) {
        memory->mmu[i] = &ramOnDemandPage;
    } else if (page==&ramPageRO || page==&ramPageWO || page==&ramPageWR) {
        if ((permissions & PAGE_READ) && (permissions & PAGE_WRITE)) {
            memory->mmu[i] = &ramPageWR;
            memory->read[i] = TO_TLB(memory->ramPage[i], i << PAGE_SHIFT);
            memory->write[i] = TO_TLB(memory->ramPage[i], i << PAGE_SHIFT);
        } else if (permissions & PAGE_WRITE) {
            memory->mmu[i] = &ramPageWO;
            memory->read[i] = 0;
            memory->write[i] = TO_TLB(memory->ramPage[i], i << PAGE_SHIFT);
        } else {
            // :TODO: is this right?
            memory->mmu[i] = &ramPageRO;
            memory->read[i] = TO_TLB(memory->ramPage[i], i << PAGE_SHIFT);
            memory->write[i] = 0;
        }
    } else if (page==&ramCopyOnWritePage || page == &ramOnDemandPage || page==&ramOnDemandFilePage || page==&codePage || page==&softNativePage) {
    } else {
        kpanic("syscall_mprotect unknown page type");
    }
}

void freePage(struct KThread* thread, U32 page) {
    struct Memory* memory = thread->process->memory;

    memory->mmu[page]->clear(memory, page);
    memory->mmu[page]=&invalidPage;
    memory->flags[page]=0;
    memory->read[page]=0;
    memory->write[page]=0;
}

BOOL findFirstAvailablePage(struct Memory* memory, U32 startingPage, U32 pageCount, U32* result, BOOL canBeReMapped) {
    U32 i;
    
    for (i=startingPage;i<NUMBER_OF_PAGES;i++) {
        if (memory->mmu[i]==&invalidPage || (canBeReMapped && (memory->flags[i] & PAGE_MAPPED))) {
            U32 j;
            BOOL success = TRUE;

            for (j=1;j<pageCount;j++) {
                if (memory->mmu[i+j]!=&invalidPage && (!canBeReMapped || !(memory->flags[i+j] & PAGE_MAPPED))) {
                    success = FALSE;
                    break;
                }
            }
            if (success) {
                *result = i;
                return TRUE;
            }
            i+=j; // no reason to check all the pages again
        }
    }
    return FALSE;
}

void reservePages(struct Memory* memory, U32 startingPage, U32 pageCount, U32 flags) {
    U32 i;
    
    for (i=startingPage;i<startingPage+pageCount;i++) {
        memory->flags[i]=flags;
    }
}

void releaseMemory(struct KThread* thread, U32 startingPage, U32 pageCount) {
    U32 i;
    struct Memory* memory = thread->process->memory;

    for (i=startingPage;i<startingPage+pageCount;i++) {
        memory->mmu[i]->clear(memory, i);
        memory->mmu[i] = &invalidPage;
        memory->flags[i]=0;
        memory->read[i]=0;
        memory->write[i]=0;
    }
}

U8* getPhysicalAddress(struct KThread* thread, U32 address) {
    int index = address >> 12;
    return thread->process->memory->mmu[index]->physicalAddress(thread, address);
}

void memcopyFromNative(struct KThread* thread, U32 address, const char* p, U32 len) {
#ifdef UNALIGNED_MEMORY
    U32 i;
    for (i=0;i<len;i++) {
        writeb(thread, address+i, p[i]);
    }
#else
    U32 i;
    struct Memory* memory = thread->process->memory;

    if (len>4) {
        U8* ram = getPhysicalAddress(thread, address);
    
        if (ram) {
            U32 todo = PAGE_SIZE-(address & (PAGE_SIZE-1));
            if (todo>len)
                todo=len;
            while (1) {
                memcpy(ram, p, todo);
                len-=todo;
                if (!len) {
                    return;
                }
                address+=todo;
                p+=todo;
                ram = getPhysicalAddress(thread, address);
                if (!ram) {
                    break;
                }
                todo = PAGE_SIZE;
                if (todo>len)
                    todo=len;
            }
        }
    }

    for (i=0;i<len;i++) {
        writeb(thread, address+i, p[i]);
    }
#endif
}

void memcopyToNative(struct KThread* thread, U32 address, char* p, U32 len) {
#ifdef UNALIGNED_MEMORY
    U32 i;

    for (i=0;i<len;i++) {
        p[i] = readb(thread, address+i);
    }
#else
    U32 i;
    struct Memory* memory = thread->process->memory;

    if (len>4) {
        U8* ram = getPhysicalAddress(thread, address);
    
        if (ram) {
            U32 todo = PAGE_SIZE-(address & (PAGE_SIZE-1));
            if (todo>len)
                todo=len;
            while (1) {
                memcpy(p, ram, todo);
                len-=todo;
                if (!len) {
                    return;
                }
                address+=todo;
                p+=todo;
                ram = getPhysicalAddress(thread, address);
                if (!ram) {
                    break;
                }
                todo = PAGE_SIZE;
                if (todo>len)
                    todo=len;
            }
        }
    }
    
    for (i=0;i<len;i++) {
        p[i] = readb(thread, address+i);
    }
#endif
}

void writeNativeString(struct KThread* thread, U32 address, const char* str) {	
    while (*str) {
        writeb(thread, address, *str);
        str++;
        address++;
    }
    writeb(thread, address, 0);
}

U32 writeNativeString2(struct KThread* thread, U32 address, const char* str, U32 len) {	
    U32 count=0;

    while (*str && count<len-1) {
        writeb(thread, address, *str);
        str++;
        address++;
        count++;
    }
    writeb(thread, address, 0);
    return count;
}

void writeNativeStringW(struct KThread* thread, U32 address, const char* str) {	
    while (*str) {
        writew(thread, address, *str);
        str++;
        address+=2;
    }
    writew(thread, address, 0);
}

char* getNativeString(struct KThread* thread, U32 address, char* buffer, U32 cbBuffer) {
    char c;
    U32 i=0;

    if (!address) {
        buffer[0]=0;
        return buffer;
    }
    do {
        c = readb(thread, address++);
        buffer[i++] = c;
    } while(c && i<cbBuffer);
    buffer[cbBuffer-1]=0;
    return buffer;
}

U32 getNativeStringLen(struct KThread* thread, U32 address) {
    char c;
    U32 i=0;

    if (!address) {
        return 0;
    }
    do {
        c = readb(thread, address++);
        i++;
    } while(c);
    return i;
}

char* getNativeStringW(struct KThread* thread, U32 address, char* buffer, U32 cbBuffer) {
    char c;
    U32 i=0;

    if (!address) {
        buffer[0]=0;
        return buffer;
    }
    do {
        c = (char)readw(thread, address);
        address+=2;
        buffer[i++] = c;
    } while(c && i<cbBuffer);
    buffer[cbBuffer-1]=0;
    return buffer;
}

BOOL isValidReadAddress(struct KThread* thread, U32 address) {
    return thread->process->memory->read[address >> PAGE_SHIFT]!=0;
}

BOOL isPageInMemory(struct Memory* memory, U32 page) {
    return memory->mmu[page]!=&invalidPage;
}

static U32 callbackPage;
static U8* callbackAddress;
static int callbackPos;
static void* callbacks[512];

void addCallback(void (OPCALL *func)(struct CPU*, struct Op*)) {
    U64 funcAddress = (U64)func;
    U8* address = callbackAddress+callbackPos*(4+sizeof(void*));
    callbacks[callbackPos++] = address;
    
    *address=0xFE;
    address++;
    *address=0x38;
    address++;
    *address=(U8)funcAddress;
    address++;
    *address=(U8)(funcAddress >> 8);
    address++;
    *address=(U8)(funcAddress >> 16);
    address++;
    *address=(U8)(funcAddress >> 24);
    if (sizeof(func)==8) {
        address++;
        *address=(U8)(funcAddress >> 32);
        address++;
        *address=(U8)(funcAddress >> 40);
        address++;
        *address=(U8)(funcAddress >> 48);
        address++;
        *address=(U8)(funcAddress >> 56);
    }
}

void initCallbacksInProcess(struct KProcess* process) {
    U32 page = CALL_BACK_ADDRESS >> PAGE_SHIFT;

    process->memory->mmu[page] = &ramPageRO;
    process->memory->flags[page] = PAGE_READ|PAGE_EXEC|PAGE_IN_RAM;
    process->memory->ramPage[page] = callbackPage;
    process->memory->read[page] = TO_TLB(callbackPage,  CALL_BACK_ADDRESS);
    incrementRamRef(callbackPage);
}

void initCallbacks() {
    callbackPage = allocRamPage();
    callbackAddress = getAddressOfRamPage(callbackPage);
    addCallback(onExitSignal);
}

void initBlockCache() {
}

struct Block* getBlock(struct CPU* cpu, U32 eip) {
    struct Block* block;	
    U32 ip;

    if (cpu->big)
        ip = cpu->segAddress[CS] + eip;
    else
        ip = cpu->segAddress[CS] + (eip & 0xFFFF);

    U32 page = ip >> PAGE_SHIFT;
    U32 flags = cpu->memory->flags[page];
    if (IS_PAGE_IN_RAM(flags)) {
        block = getCode(cpu->memory->ramPage[page], ip & 0xFFF);
        if (!block) {
            block = decodeBlock(cpu, eip);
            addCode(block, cpu, ip, block->eipCount);
        }
    } else {		
        block = decodeBlock(cpu, eip);
        addCode(block, cpu, ip, block->eipCount);
    }
    cpu->memory->write[page]=0;
    cpu->memory->mmu[page] = &codePage;
    return block;
}

void fillFetchPage(struct DecodeData* data) {
    U32 address;

    if (data->cpu->big)
        address = data->ip + data->cpu->segAddress[CS];
    else
        address = (data->ip & 0xFFFF) + data->cpu->segAddress[CS];
    data->pagePos = address & 0xFFF;
    readb(data->cpu->thread, address);
    data->page = &ram[(address & 0xFFFFF000) - data->memory->read[address>>12]];
}

void initDecodeData(struct DecodeData* data) {
    fillFetchPage(data);
}

U8 FETCH8(struct DecodeData* data) {
#ifndef BOXEDWINE_64BIT_MMU
    if (data->pagePos>=PAGE_SIZE)
        fillFetchPage(data);
    data->ip++;
    return data->page[data->pagePos++];
#else
    U32 address;

    if (data->cpu->big)
        address = data->ip + data->cpu->segAddress[CS];
    else
        address = (data->ip & 0xFFFF) + data->cpu->segAddress[CS];
    data->ip++;
    return readb(data->memory, address);
#endif
}

U16 FETCH16(struct DecodeData* data) {
    U16 result;

#ifndef UNALIGNED_MEMORY
    if (data->pagePos>=PAGE_SIZE-1) {
#endif
        result = FETCH8(data);
        result |= FETCH8(data) << 8;
        return result;
#ifndef UNALIGNED_MEMORY
    }
    data->ip+=2;
    result = *(U16*)(&data->page[data->pagePos]);
    data->pagePos+=2;
    return result;
#endif
}

U32 FETCH32(struct DecodeData* data) {
    U32 result;
#ifndef UNALIGNED_MEMORY
    if (data->pagePos>=PAGE_SIZE-3) {
#endif
        result = FETCH8(data);
        result |= FETCH8(data) << 8;
        result |= FETCH8(data) << 16;
        result |= FETCH8(data) << 24;
        return result;
#ifndef UNALIGNED_MEMORY
    }
    data->ip+=4;
    result = *(U32*)(&data->page[data->pagePos]);
    data->pagePos+=4;
    return result;
#endif
}

U32 getMemoryAllocated(struct Memory* memory) {
    return 0;
}

void* allocMappable(struct Memory* memory, U32 pageCount) {
    U32* p = (U32*)kalloc((pageCount+1)*sizeof(U32), 0);
    U32 i;

    p[0] = pageCount;
    for (i=1;i<=pageCount;i++) {
        p[i] = allocRamPage();
    }
    return p;
}

void freeMappable(struct Memory* memory, void* address) {
    U32* p = (U32*)address;
    U32 pageCount = p[0];
    U32 i;

    for (i=1;i<=pageCount;i++) {
        freeRamPage(p[i]);
    }
    kfree(address, 0);
}

void mapMappable(struct KThread* thread, U32 page, U32 pageCount, void* address, U32 permissions) {
    U32 i;
    U32* p = (U32*)address;
    struct Memory* memory = thread->process->memory;

    for (i=0;i<pageCount;i++) {
        if (permissions & PAGE_WRITE) {
            memory->mmu[i+page]=&ramPageWR;
        } else {
            memory->mmu[i+page]=&ramPageRO;
        }
        memory->flags[i+page] = permissions|PAGE_IN_RAM;
        memory->ramPage[i+page] = p[i+1];
        memory->read[i+page] = TO_TLB(p[i+1], (i+page) << PAGE_SHIFT);
        if (permissions & PAGE_WRITE) {
            memory->write[i+page] = memory->read[i+page];
        } else {
            memory->write[i+page] = 0;
        }
        incrementRamRef(p[i+1]);
    }
}

void unmapMappable(struct KThread* thread, U32 page, U32 pageCount) {
    U32 i;
    struct Memory* memory = thread->process->memory;

    for (i=0;i<pageCount;i++) {
        memory->mmu[i + page]->clear(memory, i + page);
        memory->flags[i + page] = 0;
        memory->read[i + page] = 0;
        memory->write[i + page] = 0;
    }
}

U32 mapNativeMemory(struct Memory* memory, void* hostAddress, U32 size) {
    U32 result = 0;

    if (memory->nativeAddressStart && hostAddress>=memory->nativeAddressStart && (U8*)hostAddress+size<(U8*)memory->nativeAddressStart+0x10000000) {
        return (ADDRESS_PROCESS_NATIVE<<PAGE_SHIFT) + ((U8*)hostAddress-(U8*)memory->nativeAddressStart);
    }
    if (!memory->nativeAddressStart) {
        U32 i;

        // just assume we are in the middle, hopefully OpenGL won't want more than 128MB before or after this initial address
        memory->nativeAddressStart = ((U8*)hostAddress - ((U32)hostAddress & 0xFFF)) - 0x08000000;
        for (i=0;i<0x10000;i++) {
            memory->mmu[i+ADDRESS_PROCESS_NATIVE] = &softNativePage;
            memory->ramPage[i+ADDRESS_PROCESS_NATIVE] = PAGE_SIZE*i;
        }
        return mapNativeMemory(memory, hostAddress, size);
    }
    // hopefully this won't happen much, because it will leak address space
    if (!findFirstAvailablePage(memory, 0xD0000000, (size+PAGE_MASK)>>PAGE_SHIFT, &result, FALSE)) {
        kpanic("mapNativeMemory failed to map address: size=%d", size);
    }
    return result;
}

U32 numberOfContiguousRamPages(struct Memory* memory, U32 page) {
    U32 result = 0;
    U32 i;
    U32 ramPage = memory->ramPage[page];

    for (i=page+1;i<NUMBER_OF_PAGES;i++) {
        if (memory->ramPage[i]!=ramPage+1)
            return result;
        result++;
        ramPage++;
    }
    return result;
}
#endif
