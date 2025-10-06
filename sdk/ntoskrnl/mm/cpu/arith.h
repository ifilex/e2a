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

void OPCALL addr8r8(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL adde8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL adde8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL addr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL addr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL add8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL add8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL add8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL addr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL adde16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL adde16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL addr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL addr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL add16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL add16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL add16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL addr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL adde32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL adde32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL addr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL addr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL add32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL add32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL add32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL orr8r8(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL ore8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL ore8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL orr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL orr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL or8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL or8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL or8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL orr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL ore16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL ore16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL orr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL orr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL or16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL or16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL or16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL orr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL ore32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL ore32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL orr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL orr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL or32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL or32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL or32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL adcr8r8(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL adce8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL adce8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL adcr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL adcr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL adc8_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL adc8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL adc8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL adcr16r16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL adce16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL adce16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL adcr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL adcr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL adc16_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL adc16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL adc16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL adcr32r32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL adce32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL adce32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL adcr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL adcr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL adc32_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL adc32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL adc32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldcf;
    cpu->lazyFlags = FLAGS_ADC32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbr8r8(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL sbbe8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbe8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL sbbr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL sbb8_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL sbb8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL sbb8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbr16r16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL sbbe16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbe16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL sbbr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL sbb16_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL sbb16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL sbb16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbr32r32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL sbbe32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbe32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL sbbr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL sbbr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL sbb32_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL sbb32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL sbb32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf = getCF(cpu);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldcf;
    cpu->lazyFlags = FLAGS_SBB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL andr8r8(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL ande8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL ande8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL andr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL andr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL and8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL and8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL and8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL andr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL ande16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL ande16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL andr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL andr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL and16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL and16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL and16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL andr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL ande32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL ande32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL andr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL andr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL and32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL and32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL and32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL subr8r8(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL sube8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL sube8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL subr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL subr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL sub8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL sub8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL sub8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL subr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL sube16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL sube16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL subr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL subr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL sub16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL sub16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL sub16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL subr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL sube32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL sube32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL subr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL subr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL sub32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL sub32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL sub32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL xorr8r8(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL xore8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL xore8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL xorr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL xorr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(2);
    NEXT();
}
void OPCALL xor8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->r1] =  cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL xor8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL xor8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    writeb(cpu->thread, eaa,  cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL xorr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL xore16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL xore16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL xorr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL xorr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(2);
    NEXT();
}
void OPCALL xor16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->r1].u16 =  cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL xor16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL xor16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    writew(cpu->thread, eaa,  cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL xorr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL xore32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL xore32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL xorr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL xorr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(2);
    NEXT();
}
void OPCALL xor32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->r1].u32 =  cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL xor32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL xor32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    writed(cpu->thread, eaa,  cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL cmpr8r8(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(1);
    NEXT();
}
void OPCALL cmpe8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpe8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(2);
    NEXT();
}
void OPCALL cmp8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(1);
    NEXT();
}
void OPCALL cmp8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(2);
    NEXT();
}
void OPCALL cmp8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(1);
    NEXT();
}
void OPCALL cmpe16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpe16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(2);
    NEXT();
}
void OPCALL cmp16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(1);
    NEXT();
}
void OPCALL cmp16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(2);
    NEXT();
}
void OPCALL cmp16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(1);
    NEXT();
}
void OPCALL cmpe32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpe32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(2);
    NEXT();
}
void OPCALL cmpr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(2);
    NEXT();
}
void OPCALL cmp32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(1);
    NEXT();
}
void OPCALL cmp32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(2);
    NEXT();
}
void OPCALL cmp32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    CYCLES(2);
    NEXT();
}
void OPCALL testr8r8(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = *cpu->reg8[op->r2];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(1);
    NEXT();
}
void OPCALL teste8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(2);
    NEXT();
}
void OPCALL teste8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(2);
    NEXT();
}
void OPCALL testr8e8_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa16(cpu, op));
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(2);
    NEXT();
}
void OPCALL testr8e8_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = readb(cpu->thread, eaa32(cpu, op));
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(2);
    NEXT();
}
void OPCALL test8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(1);
    NEXT();
}
void OPCALL test8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(2);
    NEXT();
}
void OPCALL test8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->src.u8 = op->data1;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    CYCLES(2);
    NEXT();
}
void OPCALL testr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = cpu->reg[op->r2].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(1);
    NEXT();
}
void OPCALL teste16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(2);
    NEXT();
}
void OPCALL teste16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(2);
    NEXT();
}
void OPCALL testr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa16(cpu, op));
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(2);
    NEXT();
}
void OPCALL testr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = readw(cpu->thread, eaa32(cpu, op));
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(2);
    NEXT();
}
void OPCALL test16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(1);
    NEXT();
}
void OPCALL test16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(2);
    NEXT();
}
void OPCALL test16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->src.u16 = op->data1;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    CYCLES(2);
    NEXT();
}
void OPCALL testr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(1);
    NEXT();
}
void OPCALL teste32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(2);
    NEXT();
}
void OPCALL teste32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(2);
    NEXT();
}
void OPCALL testr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa16(cpu, op));
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(2);
    NEXT();
}
void OPCALL testr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = readd(cpu->thread, eaa32(cpu, op));
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(2);
    NEXT();
}
void OPCALL test32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(1);
    NEXT();
}
void OPCALL test32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(2);
    NEXT();
}
void OPCALL test32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->src.u32 = op->data1;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    CYCLES(2);
    NEXT();
}
