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
#include "platform.h"
#include "reg.h"
#include "memory.h"
#include "block.h"
#include "fpu.h"
#include "cpu.h"


inline void runBlock(struct CPU* cpu, struct Block* block) {
    cpu->currentBlock = block;
    block->count++;	
	cpu->blockInstructionCount += block->instructionCount;
    block->ops->func(cpu, block->ops);	
}

INLINE U32 getCF(struct CPU* cpu) 
{
    return cpu->lazyFlags->getCF(cpu);
}

INLINE U32 getOF(struct CPU* cpu) 
{
    return cpu->lazyFlags->getOF(cpu);
}

INLINE U32 getAF(struct CPU* cpu) 
{
    return cpu->lazyFlags->getAF(cpu);
}

INLINE U32 getZF(struct CPU* cpu) 
{
    return cpu->lazyFlags->getZF(cpu);
}

INLINE U32 getSF(struct CPU* cpu) 
{
    return cpu->lazyFlags->getSF(cpu);
}

INLINE U32 getPF(struct CPU* cpu) 
{
    return cpu->lazyFlags->getPF(cpu);
}


