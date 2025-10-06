#!/bin/bash
#gcc -Wall -Wno-unused-variable -Wno-unused-function -I../../include ../../source/sdl/*.c ../../platform/linux/*.c ../../source/emulation/*.c ../../source/emulation/cpu/*.c ../../source/emulation/softmmu/*.c ../../source/io/*.c ../../source/kernel/*.c ../../source/kernel/devs/*.c ../../source/kernel/proc/*.c ../../source/kernel/loader/*.c ../../source/util/*.c ../../source/util/pbl/*.c ../../source/opengl/sdl/*.c ../../source/opengl/*.c -o boxedwine -lm -lz -lminizip -DBOXEDWINE_ZLIB -DBOXEDWINE_HAS_SETJMP -DSDL2=1 "-DGLH=<SDL_opengl.h>" -DBOXEDWINE_OPENGL_SDL `sdl2-config --cflags --libs` -O2 -lGL

echo Generando ntoskrnl ..
gcc -w -Wall -Wno-unused-variable -Wno-unused-function cc/*.o fstub/*.o io/*.o ke/*.o lpc/*.o mm/*.o vdm/*.o wmi/*.o -o ntoskrnl ../libs/libSDL.a ../libs/libX11.a ../libs/libXext.a ../libs/libXau.a ../libs/libC.a -L/usr/lib -lm -lz -ldl -lpthread
strip ntoskrnl

