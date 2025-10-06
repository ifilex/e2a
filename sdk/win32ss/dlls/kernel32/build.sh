#!/bin/sh

cc()
{
echo Compilando $1 ..
gcc -w -c $1 -I. -I../../../include  -D__WINESRC__ -D_KERNEL32_ -D_NORMALIZE_ -D_REENTRANT \
  -fPIC -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
}

cc actctx.c
cc atom.c
cc change.c
cc comm.c
cc computername.c
cc console.c
cc cpu.c 
cc debugger.c 
cc editline.c 
cc environ.c 
cc except.c 
cc fiber.c
cc file.c 
cc format_msg.c 
cc heap.c 
cc kernel_main.c 
cc lcformat.c 
cc locale.c 
cc lzexpand.c 
cc module.c
cc nameprep.c 
cc oldconfig.c 
cc path.c 
cc powermgnt.c 
cc process.c 
cc profile.c 
cc resource.c 
cc string.c 
cc sync.c 
cc tape.c 
cc term.c
cc thread.c 
cc time.c 
cc toolhelp.c 
cc version.c 
cc virtual.c 
cc volume.c 
cc wer.c 

sh link.sh
