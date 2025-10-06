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

#ifndef __CPU_H__
#define __CPU_H__

#include "platform.h"
#include "reg.h"
#include "memory.h"
#include "block.h"
#include "fpu.h"

struct CPU;

struct LazyFlags {
    U32 (*getCF)(struct CPU* cpu);
    U32 (*getOF)(struct CPU* cpu);
    U32 (*getAF)(struct CPU* cpu);
    U32 (*getZF)(struct CPU* cpu);
    U32 (*getSF)(struct CPU* cpu);
    U32 (*getPF)(struct CPU* cpu);
};

#define LDT_ENTRIES 8192

struct user_desc {
    U32  entry_number;
    U32 base_addr;
    U32  limit;
    union {
        struct {
            U32  seg_32bit:1;
            U32  contents:2;
            U32  read_exec_only:1;
            U32  limit_in_pages:1;
            U32  seg_not_present:1;
            U32  useable:1;
        };
        U32 flags;
    };
};

// the first variables, <= 127 offset, will generate smaller code with BOXEDWINE_VM
struct CPU {
#ifdef BOXEDWINE_VM
    /* 0 */ void*** opToAddressPages; // must be first
#endif
    /* 4 */ struct Reg		reg[9]; // index 8 is 0
    /* 40 */ U32 stackMask;
    /* 44 */ U32 stackNotMask;    
    /* 48 */ U32		segAddress[7];    
    /* 84 */ U32		segValue[7]; // index 6 is for 0, used in LEA instruction	
    /* 120 */ U32		flags;
    /* 124 */ struct Reg		eip;	

    U8*		reg8[8];
    struct Memory* memory;
    struct KThread* thread;
    struct Reg     src;
    struct Reg     dst;
    struct Reg     dst2;
    struct Reg     result;
    struct LazyFlags* lazyFlags;
    int	    df;
    U32     oldcf;
    U32		big;
    struct FPU     fpu;
    struct Block* nextBlock;
    struct Block* currentBlock;    
    U64		timeStampCounter;
#ifdef INCLUDE_CYCLES
    U32     blockCounter; // number of clocks since the start of the block	
#else
    BOOL    yield;
#endif
    U32     blockInstructionCount;
    BOOL log;
    U32 cpl;    
    U32 cr0;
#ifdef BOXEDWINE_VM
    U64 memOffset;
    U64 negMemOffset;
    U32 negSegAddress[7];
    void* enterHost;
    U32 cmd;
    U32 cmdArg;
    U32 cmdArg2;
    U32 cmdEipCount;
    U8 mightSetReg[6];
    BOOL done;
    jmp_buf* jmpBuf;
    BOOL inException;
#ifdef LOG_OPS
    FILE* logFile;
#endif
#ifdef _DEBUG
    U64 lastRSP;
    U64 rsp;
#endif
#endif
};

void threadDone(struct CPU* cpu);

#define CR0_PROTECTION          0x00000001
#define CR0_MONITORPROCESSOR    0x00000002
#define CR0_FPUEMULATION        0x00000004
#define CR0_TASKSWITCH          0x00000008
#define CR0_FPUPRESENT          0x00000010
#define CR0_NUMERICERROR        0x00000020
#define CR0_WRITEPROTECT        0x00010000
#define CR0_PAGING              0x80000000

#define EXCEPTION_DIVIDE 0
#define EXCEPTION_BOUND 5
#define EXCEPTION_PERMISSION 13
#define EXCEPTION_PAGE_FAULT 14

#define addFlag(f) cpu->flags |= (f)
#define removeFlag(f) cpu->flags &=~ (f)

#define setCF(cpu, b) if (b) cpu->flags|=CF; else cpu->flags&=~CF
#define setOF(cpu, b) if (b) cpu->flags|=OF; else cpu->flags&=~OF
#define setSF(cpu, b) if (b) cpu->flags|=SF; else cpu->flags&=~SF
#define setPF(cpu, b) if (b) cpu->flags|=PF; else cpu->flags&=~PF
#define setZF(cpu, b) if (b) cpu->flags|=ZF; else cpu->flags&=~ZF

#define FMASK_TEST (CF | PF | AF | ZF | SF | OF)
#define FMASK_NORMAL (FMASK_TEST | DF | TF | IF | ID | AC)
#define FMASK_ALL (FMASK_NORMAL | IOPL | NT)

 void setFlags(struct CPU* cpu, U32 word, U32 mask);

void fillFlagsNoCFOF(struct CPU* cpu);
void fillFlagsNoCF(struct CPU* cpu);
void fillFlagsNoZF(struct CPU* cpu);
void fillFlags(struct CPU* cpu);
void fillFlagsNoOF(struct CPU* cpu);
void cpu_ret(struct CPU* cpu, U32 big, U32 bytes, U32 eip);
void cpu_call(struct CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip);
void cpu_iret(struct CPU* cpu, U32 big, U32 oldeip);
void cpu_enter16(struct CPU* cpu, U32 bytes, U32 level);
void cpu_enter32(struct CPU* cpu, U32 bytes, U32 level);
U32 cpu_setSegment(struct CPU* cpu, U32 seg, U32 value);
U32 cpu_lar(struct CPU* cpu, U32 selector, U32 ar);
U32 cpu_lsl(struct CPU* cpu, U32 selector, U32 limit);
void cpu_jmp(struct CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldeip);
U32 cpu_lmsw(struct CPU* cpu, U32 word);

int CPU_ARPL(struct CPU* cpu,  U32 dest_sel, U32 src_sel);

extern U8 parity_lookup[];


#define CF		0x00000001
#define PF		0x00000004
#define AF		0x00000010
#define ZF		0x00000040
#define SF		0x00000080
#define DF		0x00000400
#define OF		0x00000800

#define TF		0x00000100
#define IF		0x00000200

#define IOPL	0x00003000
#define NT		0x00004000
#define VM		0x00020000
#define AC		0x00040000
#define ID		0x00200000

#define EXCEPTION_UD 6
#define EXCEPTION_NM 7
#define EXCEPTION_TS 10
#define EXCEPTION_NP 11
#define EXCEPTION_SS 12
#define EXCEPTION_GP 13
#define EXCEPTION_PF 14
#define EXCEPTION_MF 16

extern struct CPU c;

#define ES 0
#define CS 1
#define SS 2
#define DS 3
#define FS 4
#define GS 5
#define SEG_ZERO 6

#define AL cpu->reg[0].u8
#define AH cpu->reg[0].h8
#define CL cpu->reg[1].u8
#define CH cpu->reg[1].h8
#define DL cpu->reg[2].u8
#define DH cpu->reg[2].h8
#define BL cpu->reg[3].u8
#define BH cpu->reg[3].h8

#define AX cpu->reg[0].u16
#define CX cpu->reg[1].u16
#define DX cpu->reg[2].u16
#define BX cpu->reg[3].u16
#define SP cpu->reg[4].u16
#define BP cpu->reg[5].u16
#define SI cpu->reg[6].u16
#define DI cpu->reg[7].u16

#define EAX cpu->reg[0].u32
#define ECX cpu->reg[1].u32
#define EDX cpu->reg[2].u32
#define EBX cpu->reg[3].u32
#define ESP cpu->reg[4].u32
#define EBP cpu->reg[5].u32
#define ESI cpu->reg[6].u32
#define EDI cpu->reg[7].u32

void push16(struct CPU* cpu, U16 value);
U32 push16_r(struct CPU* cpu, U32 esp, U16 value);
void push32(struct CPU* cpu, U32 value);
U32 push32_r(struct CPU* cpu, U32 esp, U32 value);
U16 pop16(struct CPU* cpu);
U32 pop32(struct CPU* cpu);
U16 peek16(struct CPU* cpu, U32 index);
U32 peek32(struct CPU* cpu, U32 index);
void exception(struct CPU* cpu, int code);
void cpu_exception(struct CPU* cpu, int code, int error);
void initCPU(struct CPU* cpu, struct Memory* memory);
void onCreateCPU(struct CPU* cpu);
extern INLINE void runBlock(struct CPU* cpu, struct Block* block);
struct Block* getBlock(struct CPU* cpu, U32 eip);
#ifdef __TEST
#define getBlock1(cpu) 0
#define getBlock2(cpu) 0
#else
struct Block* getBlock1(struct CPU* cpu);
struct Block* getBlock2(struct CPU* cpu);
#endif

extern struct LazyFlags* FLAGS_NONE;
extern struct LazyFlags* FLAGS_ADD8;
extern struct LazyFlags* FLAGS_ADD16;
extern struct LazyFlags* FLAGS_ADD32;
extern struct LazyFlags* FLAGS_OR8;
extern struct LazyFlags* FLAGS_OR16;
extern struct LazyFlags* FLAGS_OR32;
extern struct LazyFlags* FLAGS_ADC8;
extern struct LazyFlags* FLAGS_ADC16;
extern struct LazyFlags* FLAGS_ADC32;
extern struct LazyFlags* FLAGS_SBB8;
extern struct LazyFlags* FLAGS_SBB16;
extern struct LazyFlags* FLAGS_SBB32;
extern struct LazyFlags* FLAGS_AND8;
extern struct LazyFlags* FLAGS_AND16;
extern struct LazyFlags* FLAGS_AND32;
extern struct LazyFlags* FLAGS_SUB8;
extern struct LazyFlags* FLAGS_SUB16;
extern struct LazyFlags* FLAGS_SUB32;
extern struct LazyFlags* FLAGS_XOR8;
extern struct LazyFlags* FLAGS_XOR16;
extern struct LazyFlags* FLAGS_XOR32;
extern struct LazyFlags* FLAGS_INC8;
extern struct LazyFlags* FLAGS_INC16;
extern struct LazyFlags* FLAGS_INC32;
extern struct LazyFlags* FLAGS_DEC8;
extern struct LazyFlags* FLAGS_DEC16;
extern struct LazyFlags* FLAGS_DEC32;
extern struct LazyFlags* FLAGS_SHL8;
extern struct LazyFlags* FLAGS_SHL16;
extern struct LazyFlags* FLAGS_SHL32;
extern struct LazyFlags* FLAGS_SHR8;
extern struct LazyFlags* FLAGS_SHR16;
extern struct LazyFlags* FLAGS_SHR32;
extern struct LazyFlags* FLAGS_SAR8;
extern struct LazyFlags* FLAGS_SAR16;
extern struct LazyFlags* FLAGS_SAR32;
extern struct LazyFlags* FLAGS_CMP8;
extern struct LazyFlags* FLAGS_CMP16;
extern struct LazyFlags* FLAGS_CMP32;
extern struct LazyFlags* FLAGS_TEST8;
extern struct LazyFlags* FLAGS_TEST16;
extern struct LazyFlags* FLAGS_TEST32;
extern struct LazyFlags* FLAGS_DSHL16;
extern struct LazyFlags* FLAGS_DSHL32;
extern struct LazyFlags* FLAGS_DSHR16;
extern struct LazyFlags* FLAGS_DSHR32;
extern struct LazyFlags* FLAGS_NEG8;
extern struct LazyFlags* FLAGS_NEG16;
extern struct LazyFlags* FLAGS_NEG32;

extern INLINE U32 getCF(struct CPU* cpu);
extern INLINE U32 getOF(struct CPU* cpu);
extern INLINE U32 getAF(struct CPU* cpu);
extern INLINE U32 getZF(struct CPU* cpu);
extern INLINE U32 getSF(struct CPU* cpu);
extern INLINE U32 getPF(struct CPU* cpu);

typedef void (*Int99Callback)(struct CPU* cpu);

extern Int99Callback* int99Callback;
extern U32 int99CallbackSize;
void initBlockCache();
#endif