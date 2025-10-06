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

#ifndef __PAGE_H__
#define __PAGE_H__

#ifndef BOXEDWINE_64BIT_MMU

#include "platform.h"

struct Memory;
struct KThread;

struct Page {
    U8 (*readb)(struct KThread* thread, U32 address);
    void (*writeb)(struct KThread* thread, U32 address, U8 value);
    U16 (*readw)(struct KThread* thread, U32 address);
    void (*writew)(struct KThread* thread, U32 address, U16 value);
    U32 (*readd)(struct KThread* thread, U32 address);
    void (*writed)(struct KThread* thread, U32 address, U32 value);
    void (*clear)(struct Memory* memory, U32 page);
    U8* (*physicalAddress)(struct KThread* thread, U32 address);
};

#endif

#endif
