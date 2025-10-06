#!/bin/sh 


cc()
{
echo Compilando $1 ..
gcc -DHAVE_CONFIG_H -I. -I. -I.. -I../include/X11/fonts -I../include -I../include/X11/fonts -DFONT_ENCODINGS_DIRECTORY=\"/usr/local/lib/X11/fonts/encodings/encodings.dir\"		    -Wall -Wpointer-arith -Wstrict-prototypes 	-Wmissing-prototypes -Wmissing-declarations -Wnested-externs -fno-strict-aliasing -g -O2 -I../../headers -c $1
}


if [ -e ../libXfont.a ]; then
exit 0
else

echo "* Compilando fontfile .."
cd fontfile

cc bitsource.c 
cc bufio.c 
cc decompress.c 
cc defaults.c 
cc dirfile.c 
cc encparse.c 
cc ffcheck.c 
cc fileio.c 
cc filewr.c 
cc fontdir.c 
cc fontenc.c 
cc fontencc.c 
cc fontfile.c 
cc fontscale.c 
cc gunzip.c 
cc printerfont.c 
cc register.c 
cc renderers.c 
cd ..

echo "* Compilando bitmap .."
cd bitmap
cc bdfread.c 
cc bdfutils.c 
cc bitmap.c 
cc bitmapfunc.c 
cc bitmaputil.c 
cc bitscale.c 
cc fontink.c 
cc pcfread.c 
cc pcfwrite.c 
cc snfread.c 
cd ..

echo "* Compilando builtins .."
cd builtins
cc dir.c 
cc file.c 
cc fonts.c 
cc fpe.c 
cc render.c 
cd ..

echo "* Compilando util .."
cd util
cc atom.c 
cc fontaccel.c 
cc fontnames.c 
cc fontutil.c 
cc fontxlfd.c 
cc format.c 
cc miscutil.c 
cc patcache.c 
cc private.c 
cc utilbitmap.c 
cd ..

echo "* Compilando stubs .."
cd stubs
cc cauthgen.c 
cc csignal.c 
cc delfntcid.c 
cc errorf.c 
cc fatalerror.c 
cc findoldfnt.c 
cc getcres.c 
cc getdefptsize.c 
cc getnewfntcid.c 
cc gettime.c 
cc initfshdl.c 
cc regfpefunc.c 
cc rmfshdl.c 
cc servclient.c 
cc setfntauth.c 
cc stfntcfnt.c 
cc xpstubs.c 
cd ..

echo Generando libXfont.a ..
ar cru ../libXfont.a fontfile/*.o bitmap/*.o builtins/*.o util/*.o stubs/*.o
ranlib ../libXfont.a
fi
