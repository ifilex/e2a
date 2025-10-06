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

#ifndef __KALLOC_H__
#define __KALLOC_H__

#include "platform.h"

#define KALLOC_BLOCK 1
#define KALLOC_OP 2
#define KALLOC_MEMORY 3
#define KALLOC_CODEPAGE 4
#define KALLOC_FREEPAGES 5
#define KALLOC_RAMREFCOUNT 6
#define KALLOC_CODEPAGE_ENTRY 7
#define KALLOC_NODE 8
#define KALLOC_DIRDATA 9
#define KALLOC_OPENNODE 10
#define KALLOC_TTYDATA 11
#define KALLOC_KEPOLL 12
#define KALLOC_KFILEDESCRIPTOR 13
#define KALLOC_KFILELOCK 14
#define KALLOC_MMAP_CACHE_RAMPAGE 15
#define KALLOC_MAPPEDFILECACHE 16
#define KALLOC_KOBJECT 17
#define KALLOC_KTHREAD 18
#define KALLOC_KPROCESS 19
#define KALLOC_SHM_PAGE 20
#define KALLOC_KSOCKET 21
#define KALLOC_KSOCKETMSG 22
#define KALLOC_SDLWNDTEXT 23
#define KALLOC_WND 24
#define KALLOC_KLISTNODE 25
#define KALLOC_KCNODE 26
#define KALLOC_KARRAYOBJECTS 27
#define KALLOC_SRCGENBUFFER 28
#define KALLOC_SRCGENBYTES 29
#define KALLOC_FRAMEBUFFER 30
#define KALLOC_RAM 31
#define KALLOC_OPENGL 32
#define KALLOC_IP_CACHE 33

#define KALLOC_COUNT 34

void* kalloc(U32 len, U32 type);
void kfree(void* p, U32 type);
void printMemUsage();

#endif