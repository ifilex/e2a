gcc -c -o preproc.o preproc.c -I. -I../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
gcc -c -o wpp.o wpp.c -I. -I../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
gcc -c -o ppy.tab.o ppy.tab.c -I. -I../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
gcc -c -o ppl.yy.o ppl.yy.c -I. -I../../include -D__WINESRC__ -D_REENTRANT -fPIC -Wall -pipe \
  -fno-strict-aliasing -Wdeclaration-after-statement -Wno-packed-not-aligned -Wstrict-prototypes \
  -Wunused-but-set-parameter -Wwrite-strings -Wpointer-arith -gdwarf-2 -fno-omit-frame-pointer \
  -g -O2
rm -f libwpp.a
ar rc libwpp.a preproc.o wpp.o ppy.tab.o ppl.yy.o
ranlib libwpp.a
