#!/bin/sh

echo Generando kernel32.dll.so ..
../../../tools/winegcc/winegcc -o kernel32.dll.so -B../../../tools/winebuild -fasynchronous-unwind-tables -shared kernel32.spec \
  -nodefaultlibs -Wb,-F,KERNEL32.dll -Wl,--image-base,0x7b400000 actctx.o atom.o change.o comm.o \
  computername.o console.o cpu.o debugger.o editline.o environ.o except.o fiber.o file.o \
  format_msg.o heap.o kernel_main.o lcformat.o locale.o lzexpand.o module.o nameprep.o oldconfig.o \
  path.o powermgnt.o process.o profile.o resource.o string.o sync.o tape.o term.o thread.o time.o \
  toolhelp.o version.o virtual.o volume.o wer.o locale_rc.res version.res winerror.res \
  ../../dlls/winecrt0/libwinecrt0.a -lntdll ../../libs/port/libwine_port.a 
../../../tools/winegcc/winegcc -o kernel32.dll.fake -B../../../tools/winebuild -fasynchronous-unwind-tables -shared kernel32.spec \
  -nodefaultlibs -Wb,-F,KERNEL32.dll -Wl,--image-base,0x7b400000 actctx.o atom.o change.o comm.o \
  computername.o console.o cpu.o debugger.o editline.o environ.o except.o fiber.o file.o \
  format_msg.o heap.o kernel_main.o lcformat.o locale.o lzexpand.o module.o nameprep.o oldconfig.o \
  path.o powermgnt.o process.o profile.o resource.o string.o sync.o tape.o term.o thread.o time.o \
  toolhelp.o version.o virtual.o volume.o wer.o locale_rc.res version.res winerror.res \
  ../../dlls/winecrt0/libwinecrt0.a -lntdll ../../libs/port/libwine_port.a 

strip *.so
