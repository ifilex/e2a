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
#include "softmmu/soft_page.h"
#include "op.h"
#include "memory.h"
#include "softmmu/soft_ram.h"

INLINE U8 host_readb(U32 address) {
#ifdef _DEBUG
    if (address>getPageCount()*PAGE_SIZE)
        kpanic("host_readb out of bounds");
#endif
    return ram[address];
}

INLINE void host_writeb(U32 address, U8 value) {
#ifdef _DEBUG
    if (address>getPageCount()*PAGE_SIZE)
        kpanic("host_writeb out of bounds");
#endif
    ram[address] = value;
}

INLINE U16 host_readw(U32 address) {
#ifdef _DEBUG
    if (address>getPageCount()*PAGE_SIZE)
        kpanic("host_readw out of bounds");
#endif
#ifdef UNALIGNED_MEMORY
        return ram[address] | (ram[address+1] << 8);
#else
        return *(U16*)(&ram[address]);
#endif
}

INLINE void host_writew(U32 address, U16 value) {
#ifdef _DEBUG
    if (address>getPageCount()*PAGE_SIZE)
        kpanic("host_writew out of bounds");
#endif
#ifdef UNALIGNED_MEMORY
        ram[address] = (U8)value;
        ram[address+1] = (U8)(value >> 8);
#else
        *(U16*)(&ram[address]) = value;
#endif
}

INLINE U32 host_readd(U32 address) {
#ifdef _DEBUG
    if (address>getPageCount()*PAGE_SIZE)
        kpanic("host_readd out of bounds");
#endif
#ifdef UNALIGNED_MEMORY
        return ram[address] | (ram[address+1] << 8) | (ram[address+2] << 16) | (ram[address+3] << 24);
#else
        return *(U32*)(&ram[address]);
#endif
}

INLINE void host_writed(U32 address, U32 value) {
#ifdef _DEBUG
    if (address>getPageCount()*PAGE_SIZE)
        kpanic("host_writed out of bounds");
#endif
#ifdef UNALIGNED_MEMORY
        ram[address++] = (U8)value;
        ram[address++] = (U8)(value >> 8);
        ram[address++] = (U8)(value >> 16);
        ram[address] = (U8)(value >> 24);
#else
        *(U32*)(&ram[address]) = value;
#endif
}

