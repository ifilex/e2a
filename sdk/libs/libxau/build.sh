#!/bin/sh 
cc()
{
echo Compilando $1 ..
gcc -DHAVE_CONFIG_H -I. -I. -I. -include config.h -Wall -Wpointer-arith -Wstrict-prototypes 	-Wmissing-prototypes -Wmissing-declarations 	-Wnested-externs -fno-strict-aliasing -I../headers -g -c $1
}

if [ -e ../libXau.a ]; then
exit 0
else

cc  AuDispose.c
cc  AuDispose.c
cc  AuFileName.c
cc  AuGetAddr.c
cc  AuGetBest.c
cc  AuLock.c
cc  AuRead.c
cc  AuUnlock.c
cc  AuWrite.c

echo Generando libXau.a ..
ar cru ../libXau.a  AuDispose.o AuFileName.o AuGetAddr.o AuGetBest.o AuLock.o AuRead.o AuUnlock.o AuWrite.o
ranlib ../libXau.a
fi
