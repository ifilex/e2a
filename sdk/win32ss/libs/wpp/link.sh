#!/bin/sh

echo Generando libwpp.a ..
rm -f libwpp.a
ar rc libwpp.a preproc.o wpp.o ppy.tab.o ppl.yy.o
ranlib libwpp.a
