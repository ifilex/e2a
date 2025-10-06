#!/bin/sh 

echo Generando explorer.exe ..
../../tools/wrc/wrc -o explorer.res --nostdinc -I. -I../../include -D__WINESRC__ --po-dir=../../po explorer.rc
../../tools/winegcc/winegcc -o explorer.exe.so -B../../tools/winebuild -fasynchronous-unwind-tables -mwindows \
  -municode appbar.o desktop.o explorer.o startmenu.o systray.o explorer.res -lcomctl32 -lshell32 \
  -loleaut32 -lole32 -lshlwapi -lrpcrt4 -luser32 -lgdi32 -ladvapi32 ../../libs/port/libwine_port.a \
  -Wb,-dcomctl32 -Wb,-dshell32 -Wb,-doleaut32 -Wb,-dole32 -Wb,-dshlwapi 
../../tools/winegcc/winegcc -o explorer.exe.fake -B../../tools/winebuild -fasynchronous-unwind-tables -mwindows \
  -municode appbar.o desktop.o explorer.o startmenu.o systray.o explorer.res -lcomctl32 -lshell32 \
  -loleaut32 -lole32 -lshlwapi -lrpcrt4 -luser32 -lgdi32 -ladvapi32 ../../libs/port/libwine_port.a \
  -Wb,-dcomctl32 -Wb,-dshell32 -Wb,-doleaut32 -Wb,-dole32 -Wb,-dshlwapi 
strip *.so