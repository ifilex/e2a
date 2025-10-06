#!/bin/sh

echo Generando libwinecrt0.a ..

rm -f libwinecrt0.a
ar rc libwinecrt0.a delay_load.o dll_entry.o dll_main.o drv_entry.o exception.o exe16_entry.o \
  exe_entry.o exe_main.o exe_wentry.o exe_wmain.o init.o register.o stub.o
ranlib libwinecrt0.a
