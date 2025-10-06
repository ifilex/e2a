#!/bin/sh

echo Generando libuuid.a ..
rm -f libuuid.a
ar rc libuuid.a d2d.o uuid.o
ranlib libuuid.a
