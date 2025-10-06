#!/bin/sh

echo Generando libwine.so ..
gcc -o libwine.so.1.0 casemap.o collation.o config.o debug.o ldt.o loader.o mmap.o port.o sortkey.o string.o \
  wctype.o version.o -shared -Wl,-soname,libwine.so.1 -Wl,--version-script=./wine.map \
  ../../libs/port/libwine_port.a -L/usr/lib -Wl,-rpath,/usr/lib -lm -ldl -lpthread -lutil
rm -f libwine.so.1 && ln -s libwine.so.1.0 libwine.so.1
rm -f libwine.so && ln -s libwine.so.1 libwine.so
strip *.so
