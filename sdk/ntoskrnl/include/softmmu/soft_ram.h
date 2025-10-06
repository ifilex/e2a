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

#ifndef __RAM_H__
#define __RAM_H__

#ifndef BOXEDWINE_64BIT_MMU

#include "soft_page.h"
#include "op.h"
#include "memory.h"

void initRAM(U32 pages);
U32 getPageCount();
U32 getFreePageCount();

extern struct Page ramPageRO;
extern struct Page ramPageWO;
extern struct Page ramPageWR;
extern struct Page ramOnDemandPage;
extern struct Page ramCopyOnWritePage;
extern struct Page codePage;

U32 allocRamPage();
void freeRamPage(int page);
U8* getAddressOfRamPage(U32 page);
int getRamRefCount(int page);
void incrementRamRef(int page);
void addCode(struct Block* op, struct CPU* cpu, U32 ip, U32 len);
struct Block* getCode(int ramPage, int offset);

extern U8* ram;

extern INLINE U8 host_readb(U32 address);
extern INLINE void host_writeb(U32 address, U8 value);
extern INLINE U16 host_readw(U32 address);
extern INLINE void host_writew(U32 address, U16 value);
extern INLINE U32 host_readd(U32 address);
extern INLINE void host_writed(U32 address, U32 value);

#endif
#endif