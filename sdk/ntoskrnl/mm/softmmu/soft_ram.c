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
#include "block.h"
#include "op.h"
#include "soft_ram.h"
#include "soft_memory.h"
#include "kalloc.h"
#include "kthread.h"
#include "kprocess.h"

#include <string.h>
#include <stdlib.h>

struct CodePageEntry {
    struct Block* block;
    U32 offset;
	U32 len;
    struct CodePageEntry* next;
	struct CodePageEntry* prev;
	struct CodePageEntry* linkedPrev;
	struct CodePageEntry* linkedNext;
	struct CodePage* page;
};

#define CODE_ENTRIES 128
#define CODE_ENTRIES_SHIFT 5

struct CodePage {
    struct CodePageEntry* entries[CODE_ENTRIES];
};

U8* ram;
static U8* ramRefCount;
static U32 pageCount;
static U32 freePageCount;
static U32* freePages;
static struct CodePage* codePages;

struct CodePageEntry* freeCodePageEntries;

struct CodePageEntry* allocCodePageEntry() {
    struct CodePageEntry* result;

    if (freeCodePageEntries) {
        result = freeCodePageEntries;
        freeCodePageEntries = result->next;
        memset(result, 0, sizeof(struct CodePageEntry));
    } else {
        U32 count = 1024*1023/sizeof(struct CodePageEntry);
        U32 i;

        result = (struct CodePageEntry*)kalloc(1024*1023, KALLOC_CODEPAGE_ENTRY);
        for (i=0;i<count;i++) {
            result->next = freeCodePageEntries;
            freeCodePageEntries = result;
            result++;
        }
        return allocCodePageEntry();
    }	
    return result;
}

void freeCodePageEntry(struct CodePageEntry* entry) {	
	U32 offset = entry->offset >> CODE_ENTRIES_SHIFT;
	struct CodePageEntry** entries = entry->page->entries;
   

	// remove any entries linked to this one from other pages
	if (entry->linkedPrev) {
		entry->linkedPrev->linkedNext = NULL;
		freeCodePageEntry(entry->linkedPrev);
		entry->linkedPrev = NULL;
	}
	if (entry->linkedNext) {
		entry->linkedNext->linkedPrev = NULL;
		freeCodePageEntry(entry->linkedNext);
		entry->linkedNext = NULL;
	}

	// remove this entry from this page's list
	if (entry->prev)
		entry->prev->next = entry->next;
	else
		entries[offset] = entry->next;
    if (entry->next)
        entry->next->prev = entry->prev;

	// add the entry to the free list
	entry->next = freeCodePageEntries;
	freeCodePageEntries = entry;
}

void addCode_linked(struct Block* block, struct CPU* cpu, U32 ip, U32 len, struct CodePageEntry* link) {
	U32 ramPage = cpu->memory->ramPage[ip >> 12];
	U32 offset = ip & 0xFFF;
	struct CodePageEntry** entry = &(codePages[ramPage].entries[offset >> CODE_ENTRIES_SHIFT]);
    if (!*entry) {
        *entry = allocCodePageEntry();
        (*entry)->next = 0;
    } else {
        struct CodePageEntry* add = allocCodePageEntry();
        add->next = *entry;
		(*entry)->prev = add;
        *entry = add;
    }
    (*entry)->offset = offset;
    (*entry)->block = block;
	(*entry)->page = &codePages[ramPage];
	if (offset+len>PAGE_SIZE)
		(*entry)->len = PAGE_SIZE-offset;
	else
		(*entry)->len = len;
	if (link) {
		(*entry)->linkedPrev = link;
		link->linkedNext = (*entry);
	}
	if (offset + block->eipCount > PAGE_SIZE) {
		U32 nextPage = (ip + 0xFFF) & 0xFFFFF000;
		addCode_linked(block, cpu, nextPage, len - (nextPage - ip), *entry);
	}
}

void addCode(struct Block* op, struct CPU* cpu, U32 ip, U32 len) {
	addCode_linked(op, cpu, ip, len, NULL);
}

struct Block* getCode(int ramPage, int offset) {
	struct CodePageEntry* entry = codePages[ramPage].entries[offset >> CODE_ENTRIES_SHIFT];
    while (entry) {
        if (entry->offset == offset && !entry->linkedPrev)
            return entry->block;
        entry = entry->next;
    }
    return 0;
}

struct Block* getBlockAt(struct Memory* memory, U32 address, U32 freeEntry) {
    int i;
    U32 page = address >> PAGE_SHIFT;
    U32 ram = memory->ramPage[page];
    struct CodePageEntry** entries = codePages[ram].entries;
    U32 offset = address & PAGE_MASK;
    U32 hasCode = 0;

    for (i=0;i<CODE_ENTRIES;i++) {
        struct CodePageEntry* entry = entries[i];

        while (entry) {                            
            if (offset>=entry->offset && offset<entry->offset+entry->len) {
                struct Block* result = entry->block;

                if (freeEntry) {
					if (entry->linkedPrev) {
						klog("modified code on next page");
					}
                    freeCodePageEntry(entry);					
                }
                return result;
            }
            hasCode = 1;
            entry = entry->next;
        }
    }
    if (!hasCode) {
        U32 flags = memory->flags[page];
        BOOL read = IS_PAGE_READ(flags) | IS_PAGE_EXEC(flags);
        BOOL write = IS_PAGE_WRITE(flags);

        memory->flags[page] |= PAGE_IN_RAM;
        if (read && write) {
            memory->mmu[page] = &ramPageWR;
            memory->read[page] = TO_TLB(ram,  address);
            memory->write[page] = TO_TLB(ram,  address);
        } else if (write) {
            memory->mmu[page] = &ramPageWO;
            memory->write[page] = TO_TLB(ram,  address);
        } else {
            memory->mmu[page] = &ramPageRO;
            memory->read[page] = TO_TLB(ram,  address);
        }
    }
    return NULL;
}

U8* getAddressOfRamPage(U32 page) {
    return &ram[page << 12];
}

void initRAM(U32 pages) {
    U32 i;

    pageCount = pages;
    ram = (U8*)kalloc(PAGE_SIZE*pages, KALLOC_RAM);
    ramRefCount = (U8*)kalloc(pages, KALLOC_RAMREFCOUNT);
    freePages = (U32*)kalloc(pages*sizeof(U32), KALLOC_FREEPAGES);
    freePageCount = pages;
    for (i=0;i<pages;i++) {
        freePages[i] = i;
        ramRefCount[i] = 0;
    }
    codePages = (struct CodePage*)kalloc(pages*sizeof(struct CodePage), KALLOC_CODEPAGE);
    memset(codePages, 0, sizeof(struct CodePage)*pages);
}

U32 getPageCount() {
    return pageCount;
}

U32 getFreePageCount() {
    return freePageCount;
}

U32 allocRamPage() {
    int result;

    if (freePageCount==0) {
        kpanic("Ran out of RAM pages");
    }
    result = freePages[--freePageCount];
    if (ramRefCount[result]!=0) {
        kpanic("RAM logic error");
    }
    ramRefCount[result]++;
    memset(&ram[result<<PAGE_SHIFT], 0, PAGE_SIZE);
    memset(&codePages[result], 0, sizeof(struct CodePage));
    return result;
}

void freeRamPage(int page) {
    ramRefCount[page]--;
    if (ramRefCount[page]==0) {
        int i;
        struct CodePageEntry** entries = codePages[page].entries;

        for (i=0;i<CODE_ENTRIES;i++) {
            struct CodePageEntry* entry = entries[i];
            while (entry) {
                struct CodePageEntry* next = entry->next;
                                
                freeCodePageEntry(entry);
				freeBlock(entry->block);
                entry = next;
            }
        }
        freePages[freePageCount++]=page;
    } else if (ramRefCount[page]<0) {
        kpanic("RAM logic error: freePage");
    }
}

void incrementRamRef(int page) {
    if (ramRefCount[page]==0) {
        kpanic("RAM logic error: incrementRef");
    }
    ramRefCount[page]++;
}

int getRamRefCount(int page) {
    return ramRefCount[page];
}

static U8 ram_readb(struct KThread* thread, U32 address) {
    int index = address >> PAGE_SHIFT;
    return host_readb(address-thread->process->memory->read[index]);
}

static void ram_writeb(struct KThread* thread, U32 address, U8 value) {
    int index = address >> PAGE_SHIFT;
    host_writeb(address-thread->process->memory->write[index], value);
}

static U16 ram_readw(struct KThread* thread, U32 address) {
    int index = address >> PAGE_SHIFT;
    return host_readw(address-thread->process->memory->read[index]);
}

static void ram_writew(struct KThread* thread, U32 address, U16 value) {
    int index = address >> PAGE_SHIFT;
    host_writew(address-thread->process->memory->write[index], value);
}

static U32 ram_readd(struct KThread* thread, U32 address) {
    int index = address >> PAGE_SHIFT;
    return host_readd(address-thread->process->memory->read[index]);
}

static void ram_writed(struct KThread* thread, U32 address, U32 value) {
    int index = address >> PAGE_SHIFT;
    host_writed(address-thread->process->memory->write[index], value);
}

static void ram_clear(struct Memory* memory, U32 page) {
    freeRamPage(memory->ramPage[page]);
}

static void ondemmand(struct KThread* thread, U32 address);

static U8 ondemand_ram_readb(struct KThread* thread, U32 address) {
    ondemmand(thread, address);
    return readb(thread, address);
}

static void ondemand_ram_writeb(struct KThread* thread, U32 address, U8 value) {
    ondemmand(thread, address);
    writeb(thread, address, value);
}

U16 ondemand_ram_readw(struct KThread* thread, U32 address) {
    ondemmand(thread, address);
    return readw(thread, address);
}

static void ondemand_ram_writew(struct KThread* thread, U32 address, U16 value) {
    ondemmand(thread, address);
    writew(thread, address, value);
}

static U32 ondemand_ram_readd(struct KThread* thread, U32 address) {
    ondemmand(thread, address);
    return readd(thread, address);
}

static void ondemand_ram_writed(struct KThread* thread, U32 address, U32 value) {
    ondemmand(thread, address);
    writed(thread, address, value);
}

static void ondemand_ram_clear(struct Memory* memory, U32 page) {
}

static U8* physicalAddress(struct KThread* thread, U32 address) {
    int index = address >> PAGE_SHIFT;
    struct Memory* memory = thread->process->memory;
    if (memory->write[index])
        return &ram[address - memory->write[index]];
    return &ram[address - memory->read[index]];
}

static U8* ondemand_physicalAddress(struct KThread* thread, U32 address) {
    ondemmand(thread, address);
    return physicalAddress(thread, address);
}

static void copyOnWrite(struct KThread* thread, U32 address);

static void copyonwrite_ram_writeb(struct KThread* thread, U32 address, U8 value) {
    copyOnWrite(thread, address);
    ram_writeb(thread, address, value);
}

static void copyonwrite_ram_writew(struct KThread* thread, U32 address, U16 value) {
    copyOnWrite(thread, address);
    ram_writew(thread, address, value);
}

static void copyonwrite_ram_writed(struct KThread* thread, U32 address, U32 value) {
    copyOnWrite(thread, address);
    ram_writed(thread, address, value);
}

static U8* copyonwrite_physicalAddress(struct KThread* thread, U32 address) {
    copyOnWrite(thread, address);
    return physicalAddress(thread, address);
}

void removeBlockAt(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
    struct Block* block = getBlockAt(memory, address, 1);

    while (block) {
        if (block) {
            if (block==thread->cpu.currentBlock) {
                if (address < thread->cpu.segAddress[CS] + thread->cpu.eip.u32) {
                    delayFreeBlock(block);
                } else {
                    delayFreeBlockAndKillCurrentBlock(block);
                }
            } else {
                freeBlock(block);
            }
        }
        block = getBlockAt(memory, address, 1);
    }
}

static void code_writeb(struct KThread* thread, U32 address, U8 value) {    
    if (value!=readb(thread, address)) {
        struct Memory* memory = thread->process->memory;
        int index = address >> PAGE_SHIFT;
        U32 ram = memory->ramPage[index];

        removeBlockAt(thread, address);
        host_writeb(address-TO_TLB(ram,  address), value);
    }
}

static void code_writew(struct KThread* thread, U32 address, U16 value) {
    if (value!=readw(thread, address)) {
        struct Memory* memory = thread->process->memory;
        int index = address >> PAGE_SHIFT;
        U32 ram = memory->ramPage[index];

        removeBlockAt(thread, address);
        host_writew(address-TO_TLB(ram,  address), value);
    }
}

static void code_writed(struct KThread* thread, U32 address, U32 value) {
    if (value!=readd(thread, address)) {
        struct Memory* memory = thread->process->memory;
        int index = address >> PAGE_SHIFT;
        U32 ram = memory->ramPage[index];

        removeBlockAt(thread, address);
        host_writed(address-TO_TLB(ram,  address), value);
    }
}

static U8* code_physicalAddress(struct KThread* thread, U32 address) {
    return NULL;
}

struct Page ramPageRO = {ram_readb, ram_writeb, ram_readw, ram_writew, ram_readd, ram_writed, ram_clear, physicalAddress};
struct Page ramPageWO = {nopermission_readb, ram_writeb, nopermission_readw, ram_writew, nopermission_readd, ram_writed, ram_clear, physicalAddress};
struct Page ramPageWR = {ram_readb, ram_writeb, ram_readw, ram_writew, ram_readd, ram_writed, ram_clear, physicalAddress};
struct Page ramOnDemandPage = {ondemand_ram_readb, ondemand_ram_writeb, ondemand_ram_readw, ondemand_ram_writew, ondemand_ram_readd, ondemand_ram_writed, ondemand_ram_clear, ondemand_physicalAddress};
struct Page ramCopyOnWritePage = {ram_readb, copyonwrite_ram_writeb, ram_readw, copyonwrite_ram_writew, ram_readd, copyonwrite_ram_writed, ram_clear, copyonwrite_physicalAddress};
struct Page codePage = {ram_readb, code_writeb, ram_readw, code_writew, ram_readd, code_writed, ram_clear, code_physicalAddress};

static void ondemmand(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
    U32 ram = allocRamPage();
    U32 page = address >> PAGE_SHIFT;
    U32 flags = memory->flags[page];
    BOOL read = IS_PAGE_READ(flags) | IS_PAGE_EXEC(flags);
    BOOL write = IS_PAGE_WRITE(flags);

    if (read || write) {
        memory->ramPage[page] = ram;	
        memory->flags[page] |= PAGE_IN_RAM;
    }
    if (read && write) {
        memory->mmu[page] = &ramPageWR;
        memory->read[page] = TO_TLB(ram,  address);
        memory->write[page] = TO_TLB(ram,  address);
    } else if (write) {
        memory->mmu[page] = &ramPageWO;
        memory->write[page] = TO_TLB(ram,  address);
    } else if (read) {
        memory->mmu[page] = &ramPageRO;
        memory->read[page] = TO_TLB(ram,  address);
    } else {
        memory->mmu[page] = &invalidPage;
    }
}

static void copyOnWrite(struct KThread* thread, U32 address) {	
    struct Memory* memory = thread->process->memory;
    U32 page = address >> PAGE_SHIFT;
    U32 flags = memory->flags[page];
    BOOL read = IS_PAGE_READ(flags) | IS_PAGE_EXEC(flags);
    BOOL write = IS_PAGE_WRITE(flags);

    if (getRamRefCount(memory->ramPage[page])>1) {
        U32 ram = allocRamPage();
        U32 oldRamPage = memory->ramPage[page];
        memcpy(getAddressOfRamPage(ram), getAddressOfRamPage(oldRamPage), PAGE_SIZE);
        freeRamPage(oldRamPage);
        memory->flags[page] = GET_PAGE_PERMISSIONS(flags) | PAGE_IN_RAM;
        memory->ramPage[page] = ram;

        // read ram addresses changed, write changes are handled below
        if (read) {
            memory->read[page] = TO_TLB(ram,  address);
        }
    }
    
    memory->write[page] = TO_TLB(memory->ramPage[page],  address);

    if (read && write) {
        memory->mmu[page] = &ramPageWR;		
    } else if (write) {
        memory->mmu[page] = &ramPageWO;
    } else { 
        // for the squeeze filesystem running wine 1.0 this happens a few times, the addresses seem to be in loaded libraries.
        memory->mmu[page] = &ramPageWR;
    }
}
#endif