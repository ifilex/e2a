#!/bin/sh


cc()
{
echo Compilando $1 ..
gcc -c -w $1 -I. -I../../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
}

cconfig()
{
echo Compilando config.c ..
gcc -c -w config.c -I. -I../../../include -D__WINESRC__ -DWINE_UNICODE_API="" \
  -DBINDIR='"/usr/local/bin"' -DDLLDIR='"/usr/local/lib/wine"' -DLIB_TO_BINDIR=\"`../../../server/tools/makedep -R /usr/local/lib \
  /usr/local/bin`\" -DLIB_TO_DLLDIR=\"`../../../server/tools/makedep -R /usr/local/lib /usr/local/lib/wine`\" \
  -DBIN_TO_DLLDIR=\"`../../../server/tools/makedep -R /usr/local/bin /usr/local/lib/wine`\" \
  -DBIN_TO_DATADIR=\"`../../../server/tools/makedep -R /usr/local/bin /usr/local/share/wine`\" -D_REENTRANT -fPIC -Wall \
  -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
}

cc casemap.c
cc collation.c
cconfig
cc debug.c 
cc ldt.c
cc loader.c
cc mmap.c
cc port.c
cc sortkey.c
cc string.c
cc wctype.c
cc version.c

sh link.sh
