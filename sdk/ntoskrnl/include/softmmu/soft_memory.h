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

#ifndef __SOFT_MEMORY_H__
#define __SOFT_MEMORY_H__

#ifndef BOXEDWINE_64BIT_MMU

#include "platform.h"
#include "memory.h"

struct Memory {
    U8 flags[NUMBER_OF_PAGES];
    struct KProcess* process;
    struct Page* mmu[NUMBER_OF_PAGES];
    U32 read[NUMBER_OF_PAGES];
    U32 write[NUMBER_OF_PAGES];
    U32 ramPage[NUMBER_OF_PAGES];
    void* nativeAddressStart;
#ifdef LOG_OPS
    U32 log;
#endif
};

#define TO_TLB(ramPage, address) (((address) & 0xFFFFF000)-((ramPage) << PAGE_SHIFT))

extern struct Page invalidPage;

U8 nopermission_readb(struct KThread* thread, U32 address);
void nopermission_writeb(struct KThread* thread, U32 address, U8 value);
U16 nopermission_readw(struct KThread* thread, U32 address);
void nopermission_writew(struct KThread* thread, U32 address, U16 value);
U32 nopermission_readd(struct KThread* thread, U32 address);
void nopermission_writed(struct KThread* thread, U32 address, U32 value);

U32 numberOfContiguousRamPages(struct Memory* memory, U32 page);

#endif
#endif