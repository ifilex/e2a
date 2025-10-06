gcc -c -o casemap.o casemap.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o collation.o collation.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT \
  -fPIC -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o config.o config.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" \
  -DBINDIR='"/usr/local/bin"' -DDLLDIR='"/usr/local/lib/wine"' -DLIB_TO_BINDIR=\"`../../tools/makedep -R /usr/local/lib \
  /usr/local/bin`\" -DLIB_TO_DLLDIR=\"`../../tools/makedep -R /usr/local/lib /usr/local/lib/wine`\" \
  -DBIN_TO_DLLDIR=\"`../../tools/makedep -R /usr/local/bin /usr/local/lib/wine`\" \
  -DBIN_TO_DATADIR=\"`../../tools/makedep -R /usr/local/bin /usr/local/share/wine`\" -D_REENTRANT -fPIC -Wall \
  -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o debug.o debug.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o ldt.o ldt.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o loader.o loader.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o mmap.o mmap.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o port.o port.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o sortkey.o sortkey.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o string.o string.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -c -o wctype.o wctype.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
version=`(GIT_DIR=../../.git git describe HEAD 2>/dev/null || echo "wine-3.8") | sed -n -e '$s/\(.*\)/const char wine_build[] = "\1";/p'` && (echo $version | cmp -s - version.c) || echo $version >version.c || (rm -f version.c && exit 1)
gcc -c -o version.o version.c -I. -I../../include -D__WINESRC__ -DWINE_UNICODE_API="" -D_REENTRANT -fPIC \
  -Wall -pipe -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned \
  -Wstrict-prototypes -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 \
  -fno-omit-frame-pointer -g -O2
gcc -o libwine.so.1.0 casemap.o collation.o config.o debug.o ldt.o loader.o mmap.o port.o sortkey.o string.o \
  wctype.o version.o -shared -Wl,-soname,libwine.so.1 -Wl,--version-script=./wine.map \
  ../../libs/port/libwine_port.a -L/usr/lib -Wl,-rpath,/usr/lib -lm -ldl -lpthread -lutil
rm -f libwine.so.1 && ln -s libwine.so.1.0 libwine.so.1
rm -f libwine.so && ln -s libwine.so.1 libwine.so
