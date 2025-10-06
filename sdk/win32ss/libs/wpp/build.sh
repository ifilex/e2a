#!/bin/sh

cc()
{
echo Compilando $1 ..
gcc -c $1 -I. -I../../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
}

cc preproc.c
cc wpp.c
cc ppy.tab.c
cc ppl.yy.c

sh link.sh
