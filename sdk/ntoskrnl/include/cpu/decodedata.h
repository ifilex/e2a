#ifndef __DECODE_DATA_H__
#define __DECODE_DATA_H__

#include "platform.h"

struct DecodeData {
    int ds;
    int ss;
    int rep;
    int rep_zero;
    int ea16;
    U32 ip;
    U32 start;
    U32 opCode;
    struct CPU* cpu;
    struct Memory* memory;
    U8* page;
    U32 pagePos;
    struct Op* op;	
    int count;
    char tmp[256];
    U32 singleOp;
};

U32 FETCH32(struct DecodeData* data);
U16 FETCH16(struct DecodeData* data);
U8 FETCH8(struct DecodeData* data);
void initDecodeData(struct DecodeData* data);

#endif