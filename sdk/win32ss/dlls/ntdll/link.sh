#!/bin/sh

echo Generando ntdll.so ..
../../../tools/winegcc/winegcc -o ntdll.dll.so -B../../../tools/winebuild -fasynchronous-unwind-tables -shared ntdll.spec \
  -nodefaultlibs actctx.o atom.o cdrom.o critsection.o debugbuffer.o \
  debugtools.o directory.o env.o error.o exception.o file.o handletable.o heap.o large_int.o \
  loader.o loadorder.o misc.o nt.o om.o path.o printf.o process.o reg.o relay.o resource.o rtl.o \
  rtlbitmap.o rtlstr.o sec.o serial.o server.o signal_arm.o signal_arm64.o signal_i386.o \
  signal_powerpc.o signal_x86_64.o string.o sync.o tape.o thread.o threadpool.o time.o version.o \
  virtual.o wcstring.o version.res ../../dlls/winecrt0/libwinecrt0.a ../../libs/port/libwine_port.a \
  -lrt -lpthread 
../../../tools/winegcc/winegcc -o ntdll.dll.fake -B../../../tools/winebuild -fasynchronous-unwind-tables -shared ntdll.spec \
  -nodefaultlibs actctx.o atom.o cdrom.o critsection.o debugbuffer.o \
  debugtools.o directory.o env.o error.o exception.o file.o handletable.o heap.o large_int.o \
  loader.o loadorder.o misc.o nt.o om.o path.o printf.o process.o reg.o relay.o resource.o rtl.o \
  rtlbitmap.o rtlstr.o sec.o serial.o server.o signal_arm.o signal_arm64.o signal_i386.o \
  signal_powerpc.o signal_x86_64.o string.o sync.o tape.o thread.o threadpool.o time.o version.o \
  virtual.o wcstring.o version.res ../../dlls/winecrt0/libwinecrt0.a ../../libs/port/libwine_port.a \
  -lrt -lpthread 
strip *.so
