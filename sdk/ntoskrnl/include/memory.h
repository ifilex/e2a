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

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "platform.h"


#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFF
#define PAGE_SHIFT 12
#define NUMBER_OF_PAGES 0x100000
#define ROUND_UP_TO_PAGE(x) ((x + 0xFFF) & 0xFFFFF000)

struct Memory;
struct KProcess;
struct KThread;

U8 readb(struct KThread* thread, U32 address);
void writeb(struct KThread* thread, U32 address, U8 value);
U16 readw(struct KThread* thread, U32 address);
void writew(struct KThread* thread, U32 address, U16 value);
U32 readd(struct KThread* thread, U32 address);
void writed(struct KThread* thread, U32 address, U32 value);

extern INLINE U64 readq(struct KThread* thread, U32 address) {
    return readd(thread, address) | ((U64)readd(thread, address + 4) << 32);
}

extern INLINE void writeq(struct KThread* thread, U32 address, U64 value) {
    writed(thread, address, (U32)value);
    writed(thread, address + 4, (U32)(value >> 32));
}

void zeroMemory(struct KThread* thread, U32 address, int len);
void readMemory(struct KThread* thread, U8* data, U32 address, int len);
void writeMemory(struct KThread* thread, U32 address, U8* data, int len);

struct Memory* allocMemory(struct KProcess* process);
void initMemory(struct Memory* memory);
void cloneMemory(struct KThread* thread, struct KThread* from);
void freeMemory(struct Memory* memory);
void releaseMemory(struct KThread* thread, U32 page, U32 pageCount);
void resetMemory(struct Memory* memory);
BOOL isValidReadAddress(struct KThread* thread, U32 address);
void mapMappable(struct KThread* thread, U32 page, U32 pageCount, void* p, U32 permissions);
void unmapMappable(struct KThread* thread, U32 page, U32 pageCount);
void* allocMappable(struct Memory* memory, U32 pageCount);
void freeMappable(struct Memory* memory, void* address);
BOOL isPageInMemory(struct Memory* memory, U32 page);

U32 mapNativeMemory(struct Memory* memory, void* hostAddress, U32 size);

// values in the upper byte of data
#define PAGE_READ 0x01
#define PAGE_WRITE 0x02
#define PAGE_EXEC 0x04
#define PAGE_SHARED 0x08
#define PAGE_MAPPED 0x20
#define PAGE_IN_RAM 0x80
#define PAGE_PERMISSION_MASK 0x07

#define GET_PAGE_PERMISSIONS(flags) (flags & PAGE_PERMISSION_MASK)
#define IS_PAGE_READ(flags) (flags & PAGE_READ)
#define IS_PAGE_WRITE(flags) (flags & PAGE_WRITE)
#define IS_PAGE_EXEC(flags) (flags & PAGE_EXEC)
#define IS_PAGE_SHARED(flags) (flags & PAGE_SHARED)
#define IS_PAGE_IN_RAM(data) (data & PAGE_IN_RAM)

BOOL findFirstAvailablePage(struct Memory* memory, U32 startingPage, U32 pageCount, U32* result, BOOL canBeMapped);

void allocPages(struct KThread* thread, U32 page, U32 pageCount, U8 permissions, U32 fd, U64 offset, U32 cacheIndex);
void protectPage(struct KThread* thread, U32 page, U32 permissions);
void freePage(struct KThread* thread, U32 page);

U8* getPhysicalAddress(struct KThread* thread, U32 address);

char* getNativeString(struct KThread* thread, U32 address, char* buffer, U32 cbBuffer);
char* getNativeStringW(struct KThread* thread, U32 address, char* buffer, U32 cbBuffer);
void writeNativeString(struct KThread* thread, U32 address, const char* str);
U32 writeNativeString2(struct KThread* thread, U32 address, const char* str, U32 len);
void writeNativeStringW(struct KThread* thread, U32 address, const char* str);
U32 getNativeStringLen(struct KThread* thread, U32 address);

void memcopyFromNative(struct KThread* thread, U32 address, const char* p, U32 len);
void memcopyToNative(struct KThread* thread, U32 address, char* p, U32 len);

U32 getMemoryAllocated(struct Memory* memory);

void initRAM(U32 pages);
U32 getPageCount();
U32 getFreePageCount();

#include "softmmu/soft_memory.h"
#include "softmmu/soft_page.h"
#endif
