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

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "op.h"
#include "platform.h"
#include "pbl.h"

struct Block{	
    struct Op* ops;
    U32 count;    
	U32 instructionCount;
    U32 eipCount;
    struct Block* block1;
    struct Block* block2;
    U32 startFunction;
#ifdef BOXEDWINE_64BIT_MMU
    U32 ip;
    U32 page[2];
#endif
#ifdef BOXEDWINE_VM
    void* dynamicCode;
    U32 dynamicCodeLen;
#endif
    U32 jit;
    struct BlockNode* referencedFrom;
};

#endif