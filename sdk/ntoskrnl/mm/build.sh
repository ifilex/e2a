#!/bin/bash

cc()
{
echo Compilando $1 ..
gcc -w -Wall -Wno-unused-variable -Wno-unused-function -I../include -I../include/SDL -c $1
}


cc cpu.c
cc cpu/decoder.c
cc cpu/instructions.c
cc cpu/jit.c
cc cpu/shift.c
cc cpu/srcgen.c
cc cpu/strings.c
cc hardmmu/hard_memory.c
cc softmmu/soft_file_map.c
cc softmmu/soft_memory.c
cc softmmu/soft_native_page.c
cc softmmu/soft_ram.c
cc x64dynamic/x64asm.c
cc x64dynamic/x64dynamic.c
cc x64dynamic/x64ops.c
