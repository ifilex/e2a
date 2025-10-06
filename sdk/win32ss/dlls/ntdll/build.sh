#!/bin/sh

cc()
{
echo Compilando $1 ..
gcc -w -c $1 -I. -I../../../include -D__WINESRC__ -D_NTSYSTEM_ -D_REENTRANT -fPIC -Wall \
  -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
}

cc actctx.c
cc atom.c
cc cdrom.c
cc critsection.c 
cc debugbuffer.c 
cc debugtools.c 
cc directory.c 
cc env.c 
cc error.c 
cc exception.c 
cc file.c 
cc handletable.c 
cc heap.c 
cc large_int.c 
cc loader.c 
cc loadorder.c 
cc misc.c 
cc nt.c 
cc om.c 
cc path.c 
cc printf.c 
cc process.c 
cc reg.c 
cc relay.c 
cc resource.c 
cc rtl.c 
cc rtlbitmap.c 
cc rtlstr.c 
cc sec.c 
cc serial.c 
cc server.c
cc signal_arm.c 
cc signal_arm64.c 
cc signal_i386.c 
cc signal_powerpc.c 
cc signal_x86_64.c
cc string.c 
cc sync.c 
cc tape.c 
cc thread.c 
cc threadpool.c 
cc time.c 
cc version.c 
cc virtual.c 
cc wcstring.c 

sh link.sh
