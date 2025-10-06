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
#include "cpu.h"
#include "log.h"
#include "decoder.h"
#include "platform.h"
#include "jit.h"
#include "pbl.h"
#include "ksignal.h"
#include "ksystem.h"
#include "memory.h"

#include <string.h>

Int99Callback* int99Callback;
U32 int99CallbackSize;

void onCreateCPU(struct CPU* cpu) {
    cpu->reg8[0] = &cpu->reg[0].u8;
    cpu->reg8[1] = &cpu->reg[1].u8;
    cpu->reg8[2] = &cpu->reg[2].u8;
    cpu->reg8[3] = &cpu->reg[3].u8;
    cpu->reg8[4] = &cpu->reg[0].h8;
    cpu->reg8[5] = &cpu->reg[1].h8;
    cpu->reg8[6] = &cpu->reg[2].h8;
    cpu->reg8[7] = &cpu->reg[3].h8;    
}

void initCPU(struct CPU* cpu, struct Memory* memory) {
    memset(cpu, 0, sizeof(struct CPU));
    onCreateCPU(cpu);
    cpu->memory = memory;
    cpu->lazyFlags = FLAGS_NONE;
    cpu->big = 1;
    cpu->df = 1;
    cpu->stackMask = 0xFFFFFFFF;
    cpu->stackNotMask = 0;    
    cpu->segValue[CS] = 0xF; // index 1, LDT, rpl=3
    cpu->segValue[SS] = 0x17; // index 2, LDT, rpl=3
    cpu->segValue[DS] = 0x17; // index 2, LDT, rpl=3
    cpu->segValue[ES] = 0x17; // index 2, LDT, rpl=3
    cpu->cpl = 3; // user mode
    cpu->cr0 = CR0_PROTECTION | CR0_FPUPRESENT | CR0_PAGING;
    cpu->flags|=IF;
    FPU_FINIT(&cpu->fpu);
}

struct Descriptor {
    union {
        struct {
            U32 limit_0_15 :16;
            U32 base_0_15 :16;
            U32 base_16_23 :8;
            U32 type :5;
            U32 dpl :2;
            U32 p :1;
            U32 limit_16_19 :4;
            U32 avl :1;
            U32 r :1;
            U32 big :1;
            U32 g :1;
            U32 base_24_31 :8; 
        };
        U64 fill;
    };
};

U32 CPU_CHECK_COND(struct CPU* cpu, U32 cond, const char* msg, int exc, int sel) {
    if (cond) {
        kwarn(msg);
        cpu_exception(cpu, exc, sel);
        return 1;
    }
    return 0;
}

U32 cpu_lar(struct CPU* cpu, U32 selector, U32 ar) {
    struct user_desc* ldt;

    fillFlags(cpu);
    if (selector == 0 || selector>=LDT_ENTRIES) {
        cpu->flags &=~ZF;
        return ar;
    }    
    ldt = getLDT(cpu->thread, selector >> 3);
    cpu->flags |= ZF;
    ar = 0;
    if (!ldt->seg_not_present)
        ar|=0x0100;
    ar|=0x0600; // ring 3 (dpl)
    ar|=0x0800; // not system
    // bits 5,6,7 = type, hopefully not used
    ar|=0x8000; // accessed;
    return ar;
}

U32 cpu_lsl(struct CPU* cpu, U32 selector, U32 limit) {
    struct user_desc* ldt;

    fillFlags(cpu);
    if (selector == 0 || selector>=LDT_ENTRIES) {
        cpu->flags &=~ZF;
        return limit;
    }    
    ldt = getLDT(cpu->thread, selector >> 3);
    if (!ldt) {
        cpu->flags &=~ZF;
        return limit;
    }
    cpu->flags |= ZF;
    return ldt->limit;
}

void cpu_ret(struct CPU* cpu, U32 big, U32 bytes, U32 eip) {
    if (cpu->flags & VM) {
        U32 new_ip;
        U32 new_cs;
        if (big) {
            new_ip = pop32(cpu);
            new_cs = pop32(cpu) & 0xffff;            
        } else {
            new_ip = pop16(cpu);
            new_cs = pop16(cpu);
        }
        ESP = (ESP & cpu->stackNotMask) | ((ESP + bytes ) & cpu->stackMask);
        cpu_setSegment(cpu, CS, new_cs);
        cpu->eip.u32 = new_ip;
        cpu->big = 0;
    } else {
        U32 offset,selector;
        U32 rpl; // requested privilege level
        struct user_desc* ldt;
        U32 index;

        if (big) 
            selector = peek32(cpu, 1);
        else 
            selector = peek16(cpu, 1);

        rpl=selector & 3;
        if(rpl < cpu->cpl) {
            // win setup
            cpu_exception(cpu, EXCEPTION_GP, selector & 0xfffc);
            return;
        }        
        index = selector >> 3;

        if (CPU_CHECK_COND(cpu, (selector & 0xfffc)==0, "RET:CS selector zero", EXCEPTION_GP,0))
            return;

        if (index>=LDT_ENTRIES) {
            CPU_CHECK_COND(cpu, 0, "RET:CS beyond limits", EXCEPTION_GP,selector & 0xfffc);
            return;
        }
        ldt = getLDT(cpu->thread, index);
        if (isLdtEmpty(ldt)) {
            cpu_exception(cpu, EXCEPTION_NP, selector & 0xfffc);
            return;
        }
        if (cpu->cpl==rpl) {
            // Return to same level             
            if (big) {
                offset = pop32(cpu);
                selector = pop32(cpu) & 0xffff;
            } else {
                offset = pop16(cpu);
                selector = pop16(cpu);
            }
            cpu->segAddress[CS] = ldt->base_addr;
            cpu->big = ldt->seg_32bit;
            cpu->segValue[CS] = selector;
#ifdef BOXEDWINE_VM
            cpu->mightSetReg[CS] = 1;
#endif
            cpu->eip.u32 = offset;
            ESP = (ESP & cpu->stackNotMask) | ((ESP + bytes ) & cpu->stackMask);
        } else {
            // Return to outer level
            U32 n_esp;
            U32 n_ss;
            U32 ssIndex;
            struct user_desc* ssLdt;

            if (big) {
                offset = pop32(cpu);
                selector = pop32(cpu) & 0xffff;
                ESP = (ESP & cpu->stackNotMask) | ((ESP + bytes ) & cpu->stackMask);
                n_esp = pop32(cpu);
                n_ss = pop32(cpu);
            } else {
                offset = pop16(cpu);
                selector = pop16(cpu);
                ESP = (ESP & cpu->stackNotMask) | ((ESP + bytes ) & cpu->stackMask);
                n_esp = pop16(cpu);
                n_ss = pop16(cpu);
            }
            ssIndex = n_ss >> 3;
            if (CPU_CHECK_COND(cpu, (n_ss & 0xfffc)==0, "RET to outer level with SS selector zero", EXCEPTION_GP,0))
                return;
            
            if (ssIndex>=LDT_ENTRIES) {
                CPU_CHECK_COND(cpu, 0, "RET:SS beyond limits", EXCEPTION_GP,selector & 0xfffc);
                return;
            }
            ssLdt = getLDT(cpu->thread, ssIndex);

            if (CPU_CHECK_COND(cpu, (n_ss & 3)!=rpl, "RET to outer segment with invalid SS privileges", EXCEPTION_GP,n_ss & 0xfffc))
                return;

            if (CPU_CHECK_COND(cpu, isLdtEmpty(ldt), "RET:Stack segment not present", EXCEPTION_SS,n_ss & 0xfffc))
                return;

            cpu->cpl = rpl; // don't think paging tables need to be messed with, this isn't 100% cpu emulator since we are assuming a user space program

                
            cpu->segAddress[CS] = ldt->base_addr;
            cpu->big = ldt->seg_32bit;
            cpu->segValue[CS] = (selector & 0xfffc) | cpu->cpl;
#ifdef BOXEDWINE_VM
            cpu->mightSetReg[CS] = 1;
#endif
            cpu->eip.u32 = offset;

            cpu->segAddress[SS] = ssLdt->base_addr;
            cpu->segValue[SS] = n_ss;

            if (ssLdt->seg_32bit) {
                cpu->stackMask = 0xFFFFFFFF;
                cpu->stackNotMask = 0;
                ESP=n_esp+bytes;
            } else {
                cpu->stackMask = 0x0000FFFF;
                cpu->stackNotMask = 0xFFFF0000;
                SP=n_esp+bytes;
            }
        }
    }
}

void cpu_call(struct CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip) {
    if (cpu->flags & VM) {
        U32 esp = ESP; //  // don't set ESP until we are done with memory Writes / push so that we are reentrant
        if (big) {
            esp = push32_r(cpu, esp, cpu->segValue[CS]);
            esp = push32_r(cpu, esp, oldEip);
            cpu->eip.u32 = offset;
        } else {
            esp = push16_r(cpu, esp, cpu->segValue[CS]);
            esp = push16_r(cpu, esp, oldEip & 0xFFFF);
            cpu->eip.u32 = offset & 0xffff;
        } 
        ESP = esp;
        cpu->big = 0;
        cpu->segAddress[CS] = selector << 4;;
        cpu->segValue[CS] = selector;
#ifdef BOXEDWINE_VM
        cpu->mightSetReg[CS] = 1;
#endif
    } else {
        U32 rpl=selector & 3;
        U32 index = selector >> 3;
        struct user_desc* ldt;
        U32 esp;

        if (CPU_CHECK_COND(cpu, (selector & 0xfffc)==0, "CALL:CS selector zero", EXCEPTION_GP,0))
            return;
            
        if (index>=LDT_ENTRIES) {
            CPU_CHECK_COND(cpu, 0, "CALL:CS beyond limits", EXCEPTION_GP,selector & 0xfffc);
            return;
        }
        ldt = getLDT(cpu->thread, index);

        if (isLdtEmpty(ldt)) {
            cpu_exception(cpu, EXCEPTION_NP,selector & 0xfffc);
            return;
        }
       
        esp = ESP;
        // commit point
        if (big) {
            esp = push32_r(cpu, esp, cpu->segValue[CS]);
            esp = push32_r(cpu, esp, oldEip);
            cpu->eip.u32=offset;
        } else {
            esp = push16_r(cpu, esp, cpu->segValue[CS]);
            esp = push16_r(cpu, esp, oldEip);
            cpu->eip.u32=offset & 0xffff;
        }
        ESP = esp; // don't set ESP until we are done with Memory Writes / CPU_Push so that we are reentrant
        cpu->big = ldt->seg_32bit;
        cpu->segAddress[CS] = ldt->base_addr;
        cpu->segValue[CS] = (selector & 0xfffc) | cpu->cpl;
#ifdef BOXEDWINE_VM
        cpu->mightSetReg[CS] = 1;
#endif
    }
}

void cpu_jmp(struct CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldeip) {
    if (cpu->flags & VM) {
        if (!big) {
            cpu->eip.u32 = offset & 0xffff;
        } else {
            cpu->eip.u32 = offset;
        }
        cpu->segAddress[CS] = selector << 4;;
        cpu->segValue[CS] = selector;
        cpu->big = 0;
#ifdef BOXEDWINE_VM
        cpu->mightSetReg[CS] = 1;
#endif
    } else {
        U32 rpl=selector & 3;
        U32 index = selector >> 3;
        struct user_desc* ldt;

        if (CPU_CHECK_COND(cpu, (selector & 0xfffc)==0, "JMP:CS selector zero", EXCEPTION_GP,0))
            return;
            
        if (index>=LDT_ENTRIES) {
            if (CPU_CHECK_COND(cpu, 0, "JMP:CS beyond limits", EXCEPTION_GP,selector & 0xfffc))
                return;
        }
        ldt = getLDT(cpu->thread, index);

        if (isLdtEmpty(ldt)) {
            cpu_exception(cpu, EXCEPTION_NP,selector & 0xfffc);
            return;
        }

        cpu->big = ldt->seg_32bit;
        cpu->segAddress[CS] = ldt->base_addr;
        cpu->segValue[CS] = (selector & 0xfffc) | cpu->cpl;
#ifdef BOXEDWINE_VM
        cpu->mightSetReg[CS] = 1;
#endif
        if (!big) {
            cpu->eip.u32 = offset & 0xffff;
        } else {
            cpu->eip.u32 = offset;
        }
    }
}

 void setFlags(struct CPU* cpu, U32 word, U32 mask) {
    cpu->flags=(cpu->flags & ~mask)|(word & mask)|2;
    cpu->df=1-((cpu->flags & DF) >> 9);
} 

#define GET_IOPL(cpu) ((cpu->flags & IOPL) >> 12)

U32 cpu_setSegment(struct CPU* cpu, U32 seg, U32 value) {
#ifdef BOXEDWINE_VM
    cpu->mightSetReg[seg] = 1;
#endif
    value &= 0xffff;
    if (cpu->flags & VM) {
        cpu->segAddress[seg] = value << 4;
        cpu->segValue[seg] = value;
    } else  if ((value & 0xfffc)==0) {
        cpu->segValue[seg] = value;
        cpu->segAddress[seg] = 0;	// ??
    } else {
        U32 index = value >> 3;
        struct user_desc* ldt = getLDT(cpu->thread, index);

        if (!ldt) {
            cpu_exception(cpu, EXCEPTION_GP,value & 0xfffc);
            return 0;
        }
        if (ldt->seg_not_present) {
            if (seg==SS)
                cpu_exception(cpu, EXCEPTION_SS,value & 0xfffc);
            else
                cpu_exception(cpu, EXCEPTION_NP,value & 0xfffc);
            return 0;
        }
        cpu->segValue[seg] = value;
        cpu->segAddress[seg] = ldt->base_addr;
#ifdef BOXEDWINE_VM
        cpu->negSegAddress[seg] = (U32)(-((S32)(cpu->segAddress[seg])));
#endif
        if (seg == SS) {
            if (ldt->seg_32bit) {
                cpu->stackMask = 0xffffffff;
                cpu->stackNotMask = 0;
            } else {
                cpu->stackMask = 0xffff;
                cpu->stackNotMask = 0xffff0000;
            }
        }
    }
    return 1;
}

void cpu_iret(struct CPU* cpu, U32 big, U32 oldeip) {
    if (cpu->flags & VM) {
        if ((cpu->flags & IOPL)!=IOPL) {
            cpu_exception(cpu, EXCEPTION_GP, 0);
            return;
        } else {
            if (big) {
                U32 new_eip = peek32(cpu, 0);
                U32 new_cs = peek32(cpu, 1);
                U32 new_flags = peek32(cpu, 2);

                ESP = (ESP & cpu->stackNotMask) | ((ESP + 12) & cpu->stackMask);

                cpu->eip.u32 = new_eip;
                cpu_setSegment(cpu, CS, new_cs & 0xFFFF);

                /* IOPL can not be modified in v86 mode by IRET */
                setFlags(cpu, new_flags, FMASK_NORMAL | NT);
            } else {
                U32 new_eip = peek16(cpu, 0);
                U32 new_cs = peek16(cpu, 1);
                U32 new_flags = peek16(cpu, 2);

                ESP = (ESP & cpu->stackNotMask) | ((ESP + 6) & cpu->stackMask);

                cpu->eip.u32 = new_eip;
                cpu_setSegment(cpu, CS, new_cs);
                /* IOPL can not be modified in v86 mode by IRET */
                setFlags(cpu, new_flags, FMASK_NORMAL | NT);
            }
            cpu->big = 0;
            cpu->lazyFlags = FLAGS_NONE;
            return;
        }
    }
    /* Check if this is task IRET */
    if (cpu->flags & NT) {
        kpanic("cpu tasks not implemented");
        return;
    } else {
        U32 n_cs_sel, n_flags;
        U32 n_eip;
        U32 n_cs_rpl;
        U32 csIndex;
        struct user_desc* ldt;

        if (big) {
            n_eip = peek32(cpu, 0);
            n_cs_sel = peek32(cpu, 1);
            n_flags = peek32(cpu, 2);

            if ((n_flags & VM) && (cpu->cpl==0)) {
                U32 n_ss,n_esp,n_es,n_ds,n_fs,n_gs;

                // commit point
                ESP = (ESP & cpu->stackNotMask) | ((ESP + 12) & cpu->stackMask);
                cpu->eip.u32 = n_eip & 0xffff;
                n_esp=pop32(cpu);
                n_ss=pop32(cpu) & 0xffff;
                n_es=pop32(cpu) & 0xffff;
                n_ds=pop32(cpu) & 0xffff;
                n_fs=pop32(cpu) & 0xffff;
                n_gs=pop32(cpu) & 0xffff;

                setFlags(cpu, n_flags, FMASK_NORMAL | VM);
                cpu->lazyFlags = FLAGS_NONE;
                cpu->cpl = 3;

                cpu_setSegment(cpu, SS, n_ss);
                cpu_setSegment(cpu, ES, n_es);
                cpu_setSegment(cpu, DS, n_ds);
                cpu_setSegment(cpu, FS, n_fs);
                cpu_setSegment(cpu, GS, n_gs);
                ESP = n_esp;
                cpu->big = 0;
                cpu_setSegment(cpu, CS, n_cs_sel);
                return;
            }
            if (n_flags & VM) kpanic("IRET from pmode to v86 with CPL!=0");
        } else {
            n_eip = peek16(cpu, 0);
            n_cs_sel = peek16(cpu, 1);
            n_flags = peek16(cpu, 2);

            n_flags |= (cpu->flags & 0xffff0000);
            if (n_flags & VM) kpanic("VM Flag in 16-bit iret");
        }
        if (CPU_CHECK_COND(cpu, (n_cs_sel & 0xfffc)==0, "IRET:CS selector zero", EXCEPTION_GP, 0))
            return;
        n_cs_rpl=n_cs_sel & 3;
        csIndex = n_cs_sel >> 3;

        if (CPU_CHECK_COND(cpu, csIndex>=LDT_ENTRIES, "IRET:CS selector beyond limits", EXCEPTION_GP,(n_cs_sel & 0xfffc))) {
            return;
        }
        if (CPU_CHECK_COND(cpu, n_cs_rpl<cpu->cpl, "IRET to lower privilege", EXCEPTION_GP,(n_cs_sel & 0xfffc))) {
            return;
        }
        ldt = getLDT(cpu->thread, csIndex);

        if (CPU_CHECK_COND(cpu, isLdtEmpty(ldt), "IRET with nonpresent code segment",EXCEPTION_NP,(n_cs_sel & 0xfffc)))
            return;

        /* Return to same level */
        if (n_cs_rpl==cpu->cpl) {
            U32 mask;

            // commit point
            ESP = (ESP & cpu->stackNotMask) | ((ESP + (big?12:6)) & cpu->stackMask);
            cpu->segAddress[CS] = ldt->base_addr;
            cpu->big = ldt->seg_32bit;
            cpu->segValue[CS] = n_cs_sel;
            cpu->eip.u32 = n_eip;     
#ifdef BOXEDWINE_VM
            cpu->mightSetReg[CS] = 1;
#endif
            mask = cpu->cpl !=0 ? (FMASK_NORMAL | NT) : FMASK_ALL;
            if (((cpu->flags & IOPL) >> 12) < cpu->cpl) mask &= ~IF;
            setFlags(cpu, n_flags,mask);
            cpu->lazyFlags = FLAGS_NONE;
        } else {
            /* Return to outer level */
            U32 n_ss;
            U32 n_esp;
            U32 ssIndex;
            struct user_desc* ssLdt;
            U32 mask;

            if (big) {
                n_esp = peek32(cpu, 3);
                n_ss = peek32(cpu, 4);
            } else {
                n_esp = peek16(cpu, 3);
                n_ss = peek16(cpu, 4);
            }
            if (CPU_CHECK_COND(cpu, (n_ss & 0xfffc)==0, "IRET:Outer level:SS selector zero", EXCEPTION_GP,0))
                return;
            if (CPU_CHECK_COND(cpu, (n_ss & 3)!=n_cs_rpl, "IRET:Outer level:SS rpl!=CS rpl", EXCEPTION_GP,n_ss & 0xfffc))
                return;

            ssIndex = n_ss >> 3;
            if (CPU_CHECK_COND(cpu, ssIndex>=LDT_ENTRIES, "IRET:Outer level:SS beyond limit", EXCEPTION_GP,n_ss & 0xfffc))
                return;
            //if (CPU_CHECK_COND(n_ss_desc_2.DPL()!=n_cs_rpl, "IRET:Outer level:SS dpl!=CS rpl", EXCEPTION_GP,n_ss & 0xfffc))
            //    return;

            ssLdt = getLDT(cpu->thread, ssIndex);

            if (CPU_CHECK_COND(cpu, isLdtEmpty(ssLdt), "IRET:Outer level:Stack segment not present", EXCEPTION_NP,n_ss & 0xfffc))
                return;

            // commit point
            cpu->segAddress[CS] = ldt->base_addr;
            cpu->big = ldt->seg_32bit;
            cpu->segValue[CS] = n_cs_sel;
#ifdef BOXEDWINE_VM
            cpu->mightSetReg[CS] = 1;
#endif
            mask = cpu->cpl !=0 ? (FMASK_NORMAL | NT) : FMASK_ALL;
            if (((cpu->flags & IOPL) >> 12) < cpu->cpl) mask &= ~IF;
            setFlags(cpu, n_flags, mask);
            cpu->lazyFlags = FLAGS_NONE;

            cpu->cpl = n_cs_rpl;
            cpu->eip.u32 = n_eip;

            cpu->segAddress[SS] = ssLdt->base_addr;
            cpu->segValue[SS] = n_ss;

            if (ssLdt->seg_32bit) {
                cpu->stackMask = 0xffffffff;
                cpu->stackNotMask = 0;
                ESP = n_esp;
            } else {
                cpu->stackMask = 0xffff;
                cpu->stackNotMask = 0xffff0000;
                SP = (U16)n_esp;
            }
        }
    }
} 

void cpu_enter32(struct CPU* cpu, U32 bytes, U32 level) {
    U32 sp_index=ESP & cpu->stackMask;
    U32 bp_index=EBP & cpu->stackMask;

    sp_index-=4;
    writed(cpu->thread, cpu->segAddress[SS] + sp_index, EBP);
    EBP = ESP - 4;
    if (level!=0) {
        U32 i;

        for (i=1;i<level;i++) {
            sp_index-=4;
            bp_index-=4;
            writed(cpu->thread, cpu->segAddress[SS] + sp_index, readd(cpu->thread, cpu->segAddress[SS] + bp_index));
        }
        sp_index-=4;
        writed(cpu->thread, cpu->segAddress[SS] + sp_index, EBP);
    }
    sp_index-=bytes;
    ESP = (ESP & cpu->stackNotMask) | (sp_index & cpu->stackMask);
}

void cpu_enter16(struct CPU* cpu, U32 bytes, U32 level) {
    U32 sp_index=ESP & cpu->stackMask;
    U32 bp_index=EBP & cpu->stackMask;

    sp_index-=2;
    writew(cpu->thread, cpu->segAddress[SS] + sp_index, BP);
    BP = SP - 2;
    if (level!=0) {
        U32 i;

        for (i=1;i<level;i++) {
            sp_index-=2;bp_index-=2;
            writew(cpu->thread, cpu->segAddress[SS] + sp_index, readw(cpu->thread, cpu->segAddress[SS] + bp_index));
        }
        sp_index-=2;
        writew(cpu->thread, cpu->segAddress[SS] + sp_index, BP);
    }

    sp_index-=bytes;
    ESP = (ESP & cpu->stackNotMask) | (sp_index & cpu->stackMask);
}

void fillFlagsNoCFOF(struct CPU* cpu) {
    if (cpu->lazyFlags!=FLAGS_NONE) {
        int newFlags = cpu->flags & ~(CF|AF|OF|SF|ZF|PF);
        
        if (cpu->lazyFlags->getAF(cpu)) newFlags |= AF;
        if (cpu->lazyFlags->getZF(cpu)) newFlags |= ZF;
        if (cpu->lazyFlags->getPF(cpu)) newFlags |= PF;
        if (cpu->lazyFlags->getSF(cpu)) newFlags |= SF;
        cpu->flags = newFlags;
        cpu->lazyFlags = FLAGS_NONE;		 
    }
}

void fillFlags(struct CPU* cpu) {
    if (cpu->lazyFlags!=FLAGS_NONE) {
        int newFlags = cpu->flags & ~(CF|AF|OF|SF|ZF|PF);
             
        if (cpu->lazyFlags->getAF(cpu)) newFlags |= AF;
        if (cpu->lazyFlags->getZF(cpu)) newFlags |= ZF;
        if (cpu->lazyFlags->getPF(cpu)) newFlags |= PF;
        if (cpu->lazyFlags->getSF(cpu)) newFlags |= SF;
        if (cpu->lazyFlags->getCF(cpu)) newFlags |= CF;
        if (cpu->lazyFlags->getOF(cpu)) newFlags |= OF;
        cpu->flags = newFlags;
        cpu->lazyFlags = FLAGS_NONE;	
    }
}

void fillFlagsNoCF(struct CPU* cpu) {
    if (cpu->lazyFlags!=FLAGS_NONE) {
        int newFlags = cpu->flags & ~(CF|AF|OF|SF|ZF|PF);
        
        if (cpu->lazyFlags->getAF(cpu)) newFlags |= AF;
        if (cpu->lazyFlags->getZF(cpu)) newFlags |= ZF;
        if (cpu->lazyFlags->getPF(cpu)) newFlags |= PF;
        if (cpu->lazyFlags->getSF(cpu)) newFlags |= SF;
        if (cpu->lazyFlags->getOF(cpu)) newFlags |= OF;
        cpu->flags = newFlags;
        cpu->lazyFlags = FLAGS_NONE;		 
    }
}

void fillFlagsNoZF(struct CPU* cpu) {
    if (cpu->lazyFlags!=FLAGS_NONE) {
        int newFlags = cpu->flags & ~(CF|AF|OF|SF|ZF|PF);
        
        if (cpu->lazyFlags->getAF(cpu)) newFlags |= AF;
        if (cpu->lazyFlags->getCF(cpu)) newFlags |= CF;
        if (cpu->lazyFlags->getPF(cpu)) newFlags |= PF;
        if (cpu->lazyFlags->getSF(cpu)) newFlags |= SF;
        if (cpu->lazyFlags->getOF(cpu)) newFlags |= OF;
        cpu->flags = newFlags;
        cpu->lazyFlags = FLAGS_NONE;		 
    }
}

void fillFlagsNoOF(struct CPU* cpu) {
    if (cpu->lazyFlags!=FLAGS_NONE) {
        int newFlags = cpu->flags & ~(CF|AF|OF|SF|ZF|PF);
        
        if (cpu->lazyFlags->getAF(cpu)) newFlags |= AF;
        if (cpu->lazyFlags->getZF(cpu)) newFlags |= ZF;
        if (cpu->lazyFlags->getPF(cpu)) newFlags |= PF;
        if (cpu->lazyFlags->getSF(cpu)) newFlags |= SF;
        if (cpu->lazyFlags->getCF(cpu)) newFlags |= CF;
        cpu->lazyFlags = FLAGS_NONE;		 
        cpu->flags = newFlags;
    }
}

U8 parity_lookup[256] = {
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF
  };


U32 getCF_none(struct CPU* cpu) {return cpu->flags & CF;}
U32 getOF_none(struct CPU* cpu) {return cpu->flags & OF;}
U32 getAF_none(struct CPU* cpu) {return cpu->flags & AF;}
U32 getZF_none(struct CPU* cpu) {return cpu->flags & ZF;}
U32 getSF_none(struct CPU* cpu) {return cpu->flags & SF;}
U32 getPF_none(struct CPU* cpu) {return cpu->flags & PF;}

struct LazyFlags flagsNone = {getCF_none, getOF_none, getAF_none, getZF_none, getSF_none, getPF_none};
struct LazyFlags* FLAGS_NONE = &flagsNone;

U32 getZF_8(struct CPU* cpu) {return cpu->result.u8==0;}
U32 getZF_16(struct CPU* cpu) {return cpu->result.u16==0;}
U32 getZF_32(struct CPU* cpu) {return cpu->result.u32==0;}
U32 getSF_8(struct CPU* cpu) {return cpu->result.u8 & 0x80;}
U32 getSF_16(struct CPU* cpu) {return cpu->result.u16 & 0x8000;}
U32 getSF_32(struct CPU* cpu) {return cpu->result.u32 & 0x80000000;}
U32 getPF_8(struct CPU* cpu) {return parity_lookup[cpu->result.u8];}

U32 getCF_add8(struct CPU* cpu) {return cpu->result.u8<cpu->dst.u8;}
U32 getOF_add8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;}
U32 getAF_add8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}

struct LazyFlags flagsAdd8 = {getCF_add8, getOF_add8, getAF_add8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_ADD8 = &flagsAdd8;

U32 getCF_add16(struct CPU* cpu) {return cpu->result.u16<cpu->dst.u16;}
U32 getOF_add16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;}
U32 getAF_add16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}

struct LazyFlags flagsAdd16 = {getCF_add16, getOF_add16, getAF_add16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_ADD16 = &flagsAdd16;

U32 getCF_add32(struct CPU* cpu) {return cpu->result.u32<cpu->dst.u32;}
U32 getOF_add32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;}
U32 getAF_add32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}

struct LazyFlags flagsAdd32 = {getCF_add32, getOF_add32, getAF_add32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_ADD32 = &flagsAdd32;

U32 get_0(struct CPU* cpu) {return 0;}

struct LazyFlags flags0_8 = {get_0, get_0, get_0, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_OR8 = &flags0_8;
struct LazyFlags* FLAGS_AND8 = &flags0_8;
struct LazyFlags* FLAGS_XOR8 = &flags0_8;
struct LazyFlags* FLAGS_TEST8 = &flags0_8;

struct LazyFlags flags0_16 = {get_0, get_0, get_0, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_OR16 = &flags0_16;
struct LazyFlags* FLAGS_AND16 = &flags0_16;
struct LazyFlags* FLAGS_XOR16 = &flags0_16;
struct LazyFlags* FLAGS_TEST16 = &flags0_16;

struct LazyFlags flags0_r32 = {get_0, get_0, get_0, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_OR32 = &flags0_r32;
struct LazyFlags* FLAGS_AND32 = &flags0_r32;
struct LazyFlags* FLAGS_XOR32 = &flags0_r32;
struct LazyFlags* FLAGS_TEST32 = &flags0_r32;

U32 getCF_adc8(struct CPU* cpu) {return (cpu->result.u8 < cpu->dst.u8) || (cpu->oldcf && (cpu->result.u8 == cpu->dst.u8));}
U32 getOF_adc8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;}
U32 getAF_adc8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}

struct LazyFlags flagsAdc8 = {getCF_adc8, getOF_adc8, getAF_adc8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_ADC8 = &flagsAdc8;

U32 getCF_adc16(struct CPU* cpu) {return (cpu->result.u16 < cpu->dst.u16) || (cpu->oldcf && (cpu->result.u16 == cpu->dst.u16));}
U32 getOF_adc16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;}
U32 getAF_adc16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}

struct LazyFlags flagsAdc16 = {getCF_adc16, getOF_adc16, getAF_adc16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_ADC16 = &flagsAdc16;

U32 getCF_adc32(struct CPU* cpu) {return (cpu->result.u32 < cpu->dst.u32) || (cpu->oldcf && (cpu->result.u32 == cpu->dst.u32));}
U32 getOF_adc32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;}
U32 getAF_adc32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}

struct LazyFlags flagsAdc32 = {getCF_adc32, getOF_adc32, getAF_adc32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_ADC32 = &flagsAdc32;

U32 getCF_sbb8(struct CPU* cpu) {return (cpu->dst.u8 < cpu->result.u8) || (cpu->oldcf && (cpu->src.u8==0xff));}
U32 getOF_sbb8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;}
U32 getAF_sbb8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}

struct LazyFlags flagsSbb8 = {getCF_sbb8, getOF_sbb8, getAF_sbb8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_SBB8 = &flagsSbb8;

U32 getCF_sbb16(struct CPU* cpu) {return (cpu->dst.u16 < cpu->result.u16) || (cpu->oldcf && (cpu->src.u16==0xffff));}
U32 getOF_sbb16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;}
U32 getAF_sbb16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}

struct LazyFlags flagsSbb16 = {getCF_sbb16, getOF_sbb16, getAF_sbb16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_SBB16 = &flagsSbb16;

U32 getCF_sbb32(struct CPU* cpu) {return (cpu->dst.u32 < cpu->result.u32) || (cpu->oldcf && (cpu->src.u32==0xffffffff));}
U32 getOF_sbb32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;}
U32 getAF_sbb32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}

struct LazyFlags flagsSbb32 = {getCF_sbb32, getOF_sbb32, getAF_sbb32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_SBB32 = &flagsSbb32;

U32 getCF_sub8(struct CPU* cpu) {return cpu->dst.u8<cpu->src.u8;}
U32 getOF_sub8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;}
U32 getAF_sub8(struct CPU* cpu) {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}

struct LazyFlags flagsSub8 = {getCF_sub8, getOF_sub8, getAF_sub8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_SUB8 = &flagsSub8;
struct LazyFlags* FLAGS_CMP8 = &flagsSub8;

U32 getCF_sub16(struct CPU* cpu) {return cpu->dst.u16<cpu->src.u16;}
U32 getOF_sub16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;}
U32 getAF_sub16(struct CPU* cpu) {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}

struct LazyFlags flagsSub16 = {getCF_sub16, getOF_sub16, getAF_sub16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_SUB16 = &flagsSub16;
struct LazyFlags* FLAGS_CMP16 = &flagsSub16;

U32 getCF_sub32(struct CPU* cpu) {return cpu->dst.u32<cpu->src.u32;}
U32 getOF_sub32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;}
U32 getAF_sub32(struct CPU* cpu) {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}

struct LazyFlags flagsSub32 = {getCF_sub32, getOF_sub32, getAF_sub32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_SUB32 = &flagsSub32;
struct LazyFlags* FLAGS_CMP32 = &flagsSub32;

U32 getCF_inc8(struct CPU* cpu) {return cpu->oldcf;}
U32 getOF_inc8(struct CPU* cpu) {return cpu->result.u8 == 0x80;}
U32 getAF_inc8(struct CPU* cpu) {return (cpu->result.u8 & 0x0f) == 0;}

struct LazyFlags flagsInc8 = {getCF_inc8, getOF_inc8, getAF_inc8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_INC8 = &flagsInc8;

U32 getCF_inc16(struct CPU* cpu) {return cpu->oldcf;}
U32 getOF_inc16(struct CPU* cpu) {return cpu->result.u16 == 0x8000;}
U32 getAF_inc16(struct CPU* cpu) {return (cpu->result.u16 & 0x0f) == 0;}

struct LazyFlags flagsInc16 = {getCF_inc16, getOF_inc16, getAF_inc16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_INC16 = &flagsInc16;

U32 getCF_inc32(struct CPU* cpu) {return cpu->oldcf;}
U32 getOF_inc32(struct CPU* cpu) {return cpu->result.u32 == 0x80000000;}
U32 getAF_inc32(struct CPU* cpu) {return (cpu->result.u32 & 0x0f) == 0;}

struct LazyFlags flagsInc32 = {getCF_inc32, getOF_inc32, getAF_inc32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_INC32 = &flagsInc32;

U32 getCF_dec8(struct CPU* cpu) {return cpu->oldcf;}
U32 getOF_dec8(struct CPU* cpu) {return cpu->result.u8 == 0x7f;}
U32 getAF_dec8(struct CPU* cpu) {return (cpu->result.u8 & 0x0f) == 0x0f;}

struct LazyFlags flagsDec8 = {getCF_dec8, getOF_dec8, getAF_dec8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_DEC8 = &flagsDec8;

U32 getCF_dec16(struct CPU* cpu) {return cpu->oldcf;}
U32 getOF_dec16(struct CPU* cpu) {return cpu->result.u16 == 0x7fff;}
U32 getAF_dec16(struct CPU* cpu) {return (cpu->result.u16 & 0x0f) == 0x0f;}

struct LazyFlags flagsDec16 = {getCF_dec16, getOF_dec16, getAF_dec16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_DEC16 = &flagsDec16;

U32 getCF_dec32(struct CPU* cpu) {return cpu->oldcf;}
U32 getOF_dec32(struct CPU* cpu) {return cpu->result.u32 == 0x7fffffff;}
U32 getAF_dec32(struct CPU* cpu) {return (cpu->result.u32 & 0x0f) == 0x0f;}

struct LazyFlags flagsDec32 = {getCF_dec32, getOF_dec32, getAF_dec32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_DEC32 = &flagsDec32;

U32 getCF_neg8(struct CPU* cpu) {return cpu->dst.u8!=0;}
U32 getOF_neg8(struct CPU* cpu) {return cpu->dst.u8 == 0x80;}
U32 getAF_neg8(struct CPU* cpu) {return cpu->dst.u8 & 0x0f;}

struct LazyFlags flagsNeg8 = {getCF_neg8, getOF_neg8, getAF_neg8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_NEG8 = &flagsNeg8;

U32 getCF_neg16(struct CPU* cpu) {return cpu->dst.u16!=0;}
U32 getOF_neg16(struct CPU* cpu) {return cpu->dst.u16 == 0x8000;}
U32 getAF_neg16(struct CPU* cpu) {return cpu->dst.u16 & 0x0f;}

struct LazyFlags flagsNeg16 = {getCF_neg16, getOF_neg16, getAF_neg16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_NEG16 = &flagsNeg16;

U32 getCF_neg32(struct CPU* cpu) {return cpu->dst.u32!=0;}
U32 getOF_neg32(struct CPU* cpu) {return cpu->dst.u32 == 0x80000000;}
U32 getAF_neg32(struct CPU* cpu) {return cpu->dst.u32 & 0x0f;}

struct LazyFlags flagsNeg32 = {getCF_neg32, getOF_neg32, getAF_neg32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_NEG32 = &flagsNeg32;

U32 getCF_shl8(struct CPU* cpu) {if (cpu->src.u8>8) return 0; else return (cpu->dst.u8 >> (8-cpu->src.u8)) & 1;}
U32 getOF_shl8(struct CPU* cpu) {return (cpu->result.u8 ^ cpu->dst.u8) & 0x80;}
U32 getAF_shl8(struct CPU* cpu) {return cpu->src.u8 & 0x1f;}

struct LazyFlags flagsShl8 = {getCF_shl8, getOF_shl8, getAF_shl8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_SHL8 = &flagsShl8;

U32 getCF_shl16(struct CPU* cpu) {if (cpu->src.u8>16) return 0; else return (cpu->dst.u16 >> (16-cpu->src.u8)) & 1;}
U32 getOF_shl16(struct CPU* cpu) {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
U32 getAF_shl16(struct CPU* cpu) {return cpu->src.u16 & 0x1f;}

struct LazyFlags flagsShl16 = {getCF_shl16, getOF_shl16, getAF_shl16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_SHL16 = &flagsShl16;

U32 getCF_shl32(struct CPU* cpu) {return (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;}
U32 getOF_shl32(struct CPU* cpu) {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
U32 getAF_shl32(struct CPU* cpu) {return cpu->src.u32 & 0x1f;}

struct LazyFlags flagsShl32 = {getCF_shl32, getOF_shl32, getAF_shl32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_SHL32 = &flagsShl32;

U32 getCF_dshl16(struct CPU* cpu) {if (cpu->src.u8>16) return (cpu->dst2.u16 >> (32-cpu->src.u8)) & 1; else return (cpu->dst.u16 >> (16-cpu->src.u8)) & 1;}
U32 getOF_dshl16(struct CPU* cpu) {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
U32 getAF_dshl16(struct CPU* cpu) {return 0;}

struct LazyFlags flagsDshl16 = {getCF_dshl16, getOF_dshl16, getAF_dshl16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_DSHL16 = &flagsDshl16;

U32 getCF_dshl32(struct CPU* cpu) {return (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;}
U32 getOF_dshl32(struct CPU* cpu) {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
U32 getAF_dshl32(struct CPU* cpu) {return 0;}

struct LazyFlags flagsDshl32 = {getCF_dshl32, getOF_dshl32, getAF_dshl32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_DSHL32 = &flagsDshl32;

U32 getCF_dshr16(struct CPU* cpu) {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_dshr16(struct CPU* cpu) {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
U32 getAF_dshr16(struct CPU* cpu) {return 0;}

struct LazyFlags flagsDshr16 = {getCF_dshr16, getOF_dshr16, getAF_dshr16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_DSHR16 = &flagsDshr16;

U32 getCF_dshr32(struct CPU* cpu) {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_dshr32(struct CPU* cpu) {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
U32 getAF_dshr32(struct CPU* cpu) {return 0;}

struct LazyFlags flagsDshr32 = {getCF_dshr32, getOF_dshr32, getAF_dshr32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_DSHR32 = &flagsDshr32;

U32 getCF_shr8(struct CPU* cpu) {return (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_shr8(struct CPU* cpu) {if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u8 >= 0x80); else return 0;}
U32 getAF_shr8(struct CPU* cpu) {return cpu->src.u8 & 0x1f;}

struct LazyFlags flagsShr8 = {getCF_shr8, getOF_shr8, getAF_shr8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_SHR8 = &flagsShr8;

U32 getCF_shr16(struct CPU* cpu) {return (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_shr16(struct CPU* cpu) {if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u16 >= 0x8000); else return 0;}
U32 getAF_shr16(struct CPU* cpu) {return cpu->src.u16 & 0x1f;}

struct LazyFlags flagsShr16 = {getCF_shr16, getOF_shr16, getAF_shr16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_SHR16 = &flagsShr16;

U32 getCF_shr32(struct CPU* cpu) {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_shr32(struct CPU* cpu) {if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u32 >= 0x80000000); else return 0;}
U32 getAF_shr32(struct CPU* cpu) {return cpu->src.u32 & 0x1f;}

struct LazyFlags flagsShr32 = {getCF_shr32, getOF_shr32, getAF_shr32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_SHR32 = &flagsShr32;

U32 getCF_sar8(struct CPU* cpu) {return (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_sar8(struct CPU* cpu) {return 0;}
U32 getAF_sar8(struct CPU* cpu) {return cpu->src.u8 & 0x1f;}

struct LazyFlags flagsSar8 = {getCF_sar8, getOF_sar8, getAF_sar8, getZF_8, getSF_8, getPF_8};
struct LazyFlags* FLAGS_SAR8 = &flagsSar8;

U32 getCF_sar16(struct CPU* cpu) {return (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_sar16(struct CPU* cpu) {return 0;}
U32 getAF_sar16(struct CPU* cpu) {return cpu->src.u16 & 0x1f;}

struct LazyFlags flagsSar16 = {getCF_sar16, getOF_sar16, getAF_sar16, getZF_16, getSF_16, getPF_8};
struct LazyFlags* FLAGS_SAR16 = &flagsSar16;

U32 getCF_sar32(struct CPU* cpu) {return (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;}
U32 getOF_sar32(struct CPU* cpu) {return 0;}
U32 getAF_sar32(struct CPU* cpu) {return cpu->src.u32 & 0x1f;}

struct LazyFlags flagsSar32 = {getCF_sar32, getOF_sar32, getAF_sar32, getZF_32, getSF_32, getPF_8};
struct LazyFlags* FLAGS_SAR32 = &flagsSar32;

void push16(struct CPU* cpu, U16 value) {
    U32 new_esp=(ESP & cpu->stackNotMask) | ((ESP - 2) & cpu->stackMask);
    writew(cpu->thread, cpu->segAddress[SS] + (new_esp & cpu->stackMask) ,value);
    ESP = new_esp;
}

U32 push16_r(struct CPU* cpu, U32 esp, U16 value) {
    U32 new_esp=(esp & cpu->stackNotMask) | ((esp - 2) & cpu->stackMask);
    writew(cpu->thread, cpu->segAddress[SS] + (new_esp & cpu->stackMask) ,value);
    return new_esp;
}

void push32(struct CPU* cpu, U32 value) {
    U32 new_esp=(ESP & cpu->stackNotMask) | ((ESP - 4) & cpu->stackMask);
    writed(cpu->thread, cpu->segAddress[SS] + (new_esp & cpu->stackMask) ,value);
    ESP = new_esp;
}

U32 push32_r(struct CPU* cpu, U32 esp, U32 value) {
    U32 new_esp=(esp & cpu->stackNotMask) | ((esp - 4) & cpu->stackMask);
    writed(cpu->thread, cpu->segAddress[SS] + (new_esp & cpu->stackMask) ,value);
    return new_esp;
}

U16 pop16(struct CPU* cpu) {
    U16 val = readw(cpu->thread, cpu->segAddress[SS] + (ESP & cpu->stackMask));
    ESP = (ESP & cpu->stackNotMask) | ((ESP + 2 ) & cpu->stackMask);
    return val;
}

U32 pop32(struct CPU* cpu) {
    U32 val = readd(cpu->thread, cpu->segAddress[SS] + (ESP & cpu->stackMask));
    ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask);
    return val;
}

U16 peek16(struct CPU* cpu, U32 index) {
    return readw(cpu->thread, cpu->segAddress[SS]+ ((ESP+index*2) & cpu->stackMask));
}

U32 peek32(struct CPU* cpu, U32 index) {
    return readd(cpu->thread, cpu->segAddress[SS] + ((ESP+index*4) & cpu->stackMask));
}

void cpu_exception(struct CPU* cpu, int code, int error) {
    struct KProcess* process = cpu->thread->process;

    if (code==EXCEPTION_GP && (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = error;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = 0; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 13; // trap #, TRAP_x86_PROTFLT
        runSignal(cpu->thread, cpu->thread, K_SIGSEGV, 13, error);
    } else if (code==EXCEPTION_DIVIDE && error == 0 && (process->sigActions[K_SIGFPE].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGFPE].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGFPE].sigInfo[0] = K_SIGFPE;		
        process->sigActions[K_SIGFPE].sigInfo[1] = error;
        process->sigActions[K_SIGFPE].sigInfo[2] = 0;
        process->sigActions[K_SIGFPE].sigInfo[3] = cpu->eip.u32; // address
        process->sigActions[K_SIGFPE].sigInfo[4] = 0; // trap #, TRAP_x86_DIVIDE
        runSignal(cpu->thread, cpu->thread, K_SIGFPE, 0, error);
    } else if (code==EXCEPTION_DIVIDE && error == 1 && (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = cpu->eip.u32; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 4; // trap #, TRAP_x86_OFLOW
        runSignal(cpu->thread, cpu->thread, K_SIGSEGV, 4, error);
    } else if (code==EXCEPTION_DIVIDE && error == 1 && (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = cpu->eip.u32; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 4; // trap #, TRAP_x86_OFLOW
        runSignal(cpu->thread, cpu->thread, K_SIGSEGV, 4, error);
    } else if (code==EXCEPTION_NP &&  (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = cpu->eip.u32; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 11; // trap #, TRAP_x86_SEGNPFLT
        runSignal(cpu->thread, cpu->thread, K_SIGSEGV, 11, error);
    } else if (code==EXCEPTION_SS &&  (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = cpu->eip.u32; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 12; // trap #, TRAP_x86_SEGNPFLT
        runSignal(cpu->thread, cpu->thread, K_SIGSEGV, 12, error);
    } else {        
        walkStack(cpu, cpu->eip.u32, EBP, 2);
        kpanic("unhandled exception: code=%d error=%d", code, error);        
    }
}

void exception(struct CPU* cpu, int code) {
    cpu_exception(cpu, code, 0);
}

U32 cpu_lmsw(struct CPU* cpu, U32 word) {
    if (cpu->cpl>0) {
        cpu_exception(cpu, EXCEPTION_GP, 0);
        return 0;
    }
    word&=0xf;
    if (cpu->cr0 & 1) word|=1;
    word|=(cpu->cr0 & 0xfffffff0l);
    cpu->cr0 = word | CR0_FPUPRESENT;
    if(cpu->cr0 ^ word) {
        kpanic("CR0 changed, this isn't supported");
    }
    return 1;
}

void OPCALL emptyInstruction(struct CPU* cpu, struct Op* op);

struct Op emptyOperation = {emptyInstruction};
struct Block emptyBlock = {&emptyOperation, 10000}; // count is 10000, no reason to run JIT, etc

void threadDone(struct CPU* cpu) {
#ifdef INCLUDE_CYCLES
    cpu->blockCounter |= 0x80000000;
#else
    cpu->yield = TRUE;
#endif
    cpu->nextBlock = &emptyBlock;
}

void OPCALL emptyInstruction(struct CPU* cpu, struct Op* op) {
    cpu->nextBlock = &emptyBlock;
}

void runCPU(struct CPU* cpu) {	
    runBlock(cpu, getBlock(cpu, cpu->eip.u32));
}

int CPU_ARPL(struct CPU* cpu,  U32 dest_sel, U32 src_sel) {
    fillFlags(cpu);
    if ((dest_sel & 3) < (src_sel & 3)) {
        dest_sel=(dest_sel & 0xfffc) + (src_sel & 3);
//		dest_sel|=0xff3f0000;
        cpu->flags |= ZF;
    } else {
        cpu->flags &= ZF;
    }
    return dest_sel;
}


#ifndef __TEST
struct Block* getBlock1(struct CPU* cpu) {
    if (!cpu->currentBlock->block1) {
        cpu->currentBlock->block1 = getBlock(cpu, cpu->eip.u32);
        addBlockNode(&cpu->currentBlock->block1->referencedFrom, cpu->currentBlock);
    }
    return cpu->currentBlock->block1; 
}

struct Block* getBlock2(struct CPU* cpu) {
    if (!cpu->currentBlock->block2) {
        cpu->currentBlock->block2 = getBlock(cpu, cpu->eip.u32);
        addBlockNode(&cpu->currentBlock->block2->referencedFrom, cpu->currentBlock);
    }
    return cpu->currentBlock->block2;
}
#endif
