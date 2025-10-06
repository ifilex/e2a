#!/bin/sh

cc()
{
echo Compilando $1 ..

gcc -c -w $1 -I. -I../../include -D__WINESRC__ -Wall -pipe -fno-strict-aliasing \
  -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
}

cc async.c
cc atom.c
cc change.c
cc class.c
cc clipboard.c
cc completion.c
cc console.c
cc debugger.c
cc device.c
cc directory.c
cc event.c
cc fd.c
cc file.c
cc handle.c
cc hook.c
cc mach.c
cc mailslot.c
cc main.c
cc mapping.c
cc mutex.c
cc named_pipe.c
cc object.c
cc process.c
cc procfs.c
cc ptrace.c
cc queue.c
cc region.c
cc registry.c
cc request.c
cc semaphore.c
cc serial.c
cc signal.c
cc snapshot.c
cc sock.c
cc symlink.c
cc thread.c
cc timer.c
cc token.c
cc trace.c
cc unicode.c
cc user.c
cc window.c
cc winstation.c

sh link.sh
