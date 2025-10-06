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

#ifndef __DECODER_H__
#define __DECODER_H__

#include "cpu.h"
#include "block.h"
#include "kprocess.h"
#include "block.h"

#include <stdio.h>

struct Block* decodeBlock(struct CPU* cpu, U32 eip);
void decodeBlockWithBlock(struct CPU* cpu, U32 eip, struct Block* block);
void freeOp(struct Op* op);
void freeBlock(struct Block* op);
void addBlockNode(struct BlockNode** node, struct Block* block);
struct Block* decodeSingleOp(struct CPU* cpu, U32 eip);

#define G(rm) ((rm >> 3) & 7)
#define E(rm) (rm & 7)
#ifdef LOG_OPS
#define DONE() if (logFile && cpu->log) {if (cpu->big) {fprintf(logFile, "%d %.8X %-40s EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X es=%X(%X) cs=%X(%X) ss=%X(%X) ds=%X(%X) fs=%X(%X) gs=%X(%X) %s at %.8X\n", cpu->thread->id, cpu->segAddress[CS]+cpu->eip.u32, op->str, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[ES], cpu->segAddress[ES], cpu->segValue[CS], cpu->segAddress[CS], cpu->segValue[SS], cpu->segAddress[SS], cpu->segValue[DS], cpu->segAddress[DS], cpu->segValue[FS], cpu->segValue[GS], cpu->segAddress[GS], cpu->segAddress[FS], getModuleName(cpu, cpu->segAddress[CS]+cpu->eip.u32), getModuleEip(cpu, cpu->segAddress[CS]+cpu->eip.u32));fflush(logFile);}else{ fprintf(logFile, "%d %.4X:%.4X %-40s EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X es=%X(%X) cs=%X(%X) ss=%X(%X) ds=%X(%X) fs=%X(%X) gs=%X(%X) %s at %.8X\n", cpu->thread->id, cpu->segValue[CS], (int)cpu->eip.u16, op->str, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[ES], cpu->segAddress[ES], cpu->segValue[CS], cpu->segAddress[CS], cpu->segValue[SS], cpu->segAddress[SS], cpu->segValue[DS], cpu->segAddress[DS], cpu->segValue[FS], cpu->segValue[GS], cpu->segAddress[GS], cpu->segAddress[FS], getModuleName(cpu, cpu->segAddress[CS]+cpu->eip.u32), getModuleEip(cpu, cpu->segAddress[CS]+cpu->eip.u32));fflush(logFile);}}
#define NEXT()  DONE() cpu->eip.u32+=op->eipCount; op->next->func(cpu, op->next)
#define LOG_OP2(name, s1, s2) sprintf(data->op->str, "%s %s,%s", name, s1, s2);
#define LOG_OP1(name, s1) sprintf(data->op->str, "%s %s", name, s1);
#define LOG_OP(name) strcpy(data->op->str, name);
#define LOG_OP_CS_EIP(name, cs, eip) logOpCsEip(data->op->str, name, cs, eip)
#else
#define DONE()
#define NEXT() cpu->eip.u32+=op->eipCount; op->next->func(cpu, op->next)
#define LOG_OP2(name, s1, s2)
#define LOG_OP1(name, s1)
#define LOG_OP(name)
#define LOG_E8(name, rm, data);
#define LOG_E16(name, rm, data);
#define LOG_E32(name, rm, data);
#define LOG_E8C(name, rm, data);
#define LOG_E16C(name, rm, data);
#define LOG_E32C(name, rm, data);
#define LOG_OP_CS_EIP(name, cs, eip)
#endif

#define FINISH_OP(data) data->op->eipCount=data->ip-data->start

#ifdef INCLUDE_CYCLES
#define CYCLES(x) cpu->blockCounter += x
#else
#define CYCLES(x)
#endif
#define eaa16(cpu, op) cpu->segAddress[op->base] + (U16)(cpu->reg[op->e1].u16 + (S16)cpu->reg[op->e2].u16 + op->eData)
#define eaa32(cpu, op) cpu->segAddress[op->base] + cpu->reg[op->e1].u32 + (cpu->reg[op->e2].u32 << + op->eSib) + op->eData

#endif