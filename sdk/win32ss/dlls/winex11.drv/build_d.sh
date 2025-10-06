#!/bin/sh

cc()
{
echo Compilando $1 ..
gcc -c -w $1 -I. -I../../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -pthread -D_GNU_SOURCE -g -O2 2>>debug.log
}

cc surface.c
cc wineboxed.c
