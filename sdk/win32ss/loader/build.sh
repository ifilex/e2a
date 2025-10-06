#!/bin/sh

cc()
{
echo Compilando $1 ..
gcc -c $1 -I. -I../../include -D__WINESRC__ -Wall -pipe -fno-strict-aliasing \
  -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
}

cc main.c 
cc preloader.c

sh link.sh
