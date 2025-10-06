#!/bin/sh 

echo Generando winex11.drv ..
LD_LIBRARY_PATH="../../libs/wine:$LD_LIBRARY_PATH" ../../../tools/wrc/wrc --nostdinc -I. -I. -I../../../include -I../../../include  -D__WINESRC__   -foversion.res version.rc
../../../tools/winegcc/winegcc  -B../../../tools/winebuild --sysroot=../../.. -shared ./winex11.drv.spec    *.o   version.res    -o winex11.drv.so   -lcomctl32 -lole32 -lshell32 -limm32 ../../dlls/uuid/libuuid.a -luser32 -lgdi32 -ladvapi32 ../../libs/port/libwine_port.a  -L/usr/lib -Wl,-rpath,/usr/lib -lm -ldl -lpthread -lutil -Wb,-dcomctl32 -Wb, -dole32 -Wb,-dshell32 -Wb,-dimm32
../../../tools/winegcc/winegcc  -B../../../tools/winebuild --sysroot=../../.. -shared ./winex11.drv.spec    *.o   version.res    -o winex11.drv.fake -lcomctl32 -lole32 -lshell32 -limm32 ../../dlls/uuid/libuuid.a -luser32 -lgdi32 -ladvapi32 ../../libs/port/libwine_port.a  -L/usr/lib -Wl,-rpath,/usr/lib -lm -ldl -lpthread -lutil -Wb,-dcomctl32 -Wb, -dole32 -Wb,-dshell32 -Wb,-dimm32                                         
strip winex11.drv.so
