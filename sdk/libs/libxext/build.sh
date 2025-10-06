#!/bin/sh 
cc()
{
echo Compilando $1 ..
gcc -DHAVE_CONFIG_H -I. -include config.h -DXTHREADS -DXUSE_MTSAFE_API -I../headers   -g -O2 -c $1 
}

if [ -e ../libXext.a ]; then
exit 0
else

cc DPMS.c
cc MITMisc.c
cc XAppgroup.c
cc XEVI.c
cc XLbx.c
cc XMultibuf.c
cc XSecurity.c
cc XShape.c
cc XShm.c
cc XSync.c
cc XTestExt1.c
cc Xcup.c
cc Xdbe.c
cc extutil.c
cc globals.c

echo Generando libXext.a ..
ar cru ../libXext.a  DPMS.o MITMisc.o XAppgroup.o XEVI.o XLbx.o XMultibuf.o XSecurity.o XShape.o XShm.o XSync.o XTestExt1.o Xcup.o Xdbe.o extutil.o globals.o
ranlib ../libXext.a
fi