#!/bin/sh

echo Generando wineserver ..
gcc -o wineserver-installed async.o atom.o change.o class.o clipboard.o completion.o console.o debugger.o device.o \
  directory.o event.o fd.o file.o handle.o hook.o mach.o mailslot.o main.o mapping.o mutex.o \
  named_pipe.o object.o process.o procfs.o ptrace.o queue.o region.o registry.o request.o \
  semaphore.o serial.o signal.o snapshot.o sock.o symlink.o thread.o timer.o token.o trace.o \
  unicode.o user.o window.o winstation.o \
  -Wl,--rpath,\$ORIGIN/`../../server/tools/makedep -R /usr/local/bin /usr/local/lib` -Wl,--enable-new-dtags \
  ../libs/port/libwine_port.a -lwine -lrt -L../libs/wine -pie
gcc -o wineserver async.o atom.o change.o class.o clipboard.o completion.o console.o debugger.o device.o \
  directory.o event.o fd.o file.o handle.o hook.o mach.o mailslot.o main.o mapping.o mutex.o \
  named_pipe.o object.o process.o procfs.o ptrace.o queue.o region.o registry.o request.o \
  semaphore.o serial.o signal.o snapshot.o sock.o symlink.o thread.o timer.o token.o trace.o \
  unicode.o user.o window.o winstation.o -Wl,--rpath,\$ORIGIN/../libs/wine \
  ../libs/port/libwine_port.a -lwine -lrt -L../libs/wine -pie
strip wineserver*
