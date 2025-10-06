#!/bin/sh 

if [ -e explorer.exe.so ]; then
exit 0
fi

cc()
{
echo Compilando $1
gcc -w -c $1 -I. -I../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement \
  -Wno-packed-not-aligned -Wstrict-prototypes -Wunused-but-set-parameter \
  -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
}

cc appbar.c
cc desktop.c
cc explorer.c
cc startmenu.c
cc systray.c

sh link.sh