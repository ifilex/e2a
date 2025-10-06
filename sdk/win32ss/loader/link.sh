#!/bin/sh

echo Generando wine-installed ..
gcc -o wine-installed main.o \
  -Wl,--rpath,\$ORIGIN/`../../server/tools/makedep -R /usr/local/bin /usr/local/lib` -Wl,--enable-new-dtags \
  -Wl,--export-dynamic -Wl,--section-start,.interp=0x7c000400 -Wl,-z,max-page-size=0x1000 -lwine \
  -lpthread ../libs/port/libwine_port.a -L../libs/wine 
echo Generando wine ..
gcc -o wine main.o -Wl,--rpath,\$ORIGIN/../libs/wine -Wl,--export-dynamic \
  -Wl,--section-start,.interp=0x7c000400 -Wl,-z,max-page-size=0x1000 -lwine -lpthread \
  ../libs/port/libwine_port.a -L../libs/wine 
echo Generando wine-preloader ..
gcc -o wine-preloader preloader.o -static -nostartfiles -nodefaultlibs -Wl,-Ttext=0x7c400000 \
  ../libs/port/libwine_port.a 
strip wine
strip wine-installed
strip wine-preloader
