#!/bin/sh

echo Generando libwine_port.a ..
rm -f libwine_port.a
ar rc libwine_port.a c_037.o c_10000.o c_10001.o c_10002.o c_10003.o c_10004.o c_10005.o c_10006.o \
  c_10007.o c_10008.o c_10010.o c_10017.o c_10021.o c_10029.o c_1006.o c_10079.o c_10081.o c_10082.o \
  c_1026.o c_1250.o c_1251.o c_1252.o c_1253.o c_1254.o c_1255.o c_1256.o c_1257.o c_1258.o c_1361.o \
  c_20127.o c_20866.o c_20932.o c_21866.o c_28591.o c_28592.o c_28593.o c_28594.o c_28595.o \
  c_28596.o c_28597.o c_28598.o c_28599.o c_28600.o c_28603.o c_28604.o c_28605.o c_28606.o c_424.o \
  c_437.o c_500.o c_737.o c_775.o c_850.o c_852.o c_855.o c_856.o c_857.o c_860.o c_861.o c_862.o \
  c_863.o c_864.o c_865.o c_866.o c_869.o c_874.o c_875.o c_878.o c_932.o c_936.o c_949.o c_950.o \
  compose.o cpsymbol.o cptable.o decompose.o digitmap.o ffs.o fold.o fstatvfs.o getopt.o getopt1.o \
  interlocked.o isfinite.o isinf.o isnan.o lstat.o mbtowc.o memcpy_unaligned.o memmove.o mkstemps.o \
  poll.o pread.o pwrite.o readlink.o rint.o spawn.o statvfs.o strcasecmp.o strerror.o strncasecmp.o \
  strnlen.o symlink.o usleep.o utf8.o wctomb.o
ranlib libwine_port.a
