#!/bin/sh

cc()
{
echo Compilando $1 ..
gcc -c $1 -I. -I../../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
}

cc delay_load.c
cc dll_entry.c
cc dll_main.c
cc drv_entry.c
cc exception.c
cc exe16_entry.c
cc exe_entry.c
cc exe_main.c
cc exe_wentry.c
cc exe_wmain.c
cc init.c
cc register.c
cc stub.c


sh link.sh
