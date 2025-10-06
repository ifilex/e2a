#!/bin/sh 

cd libx
sh clean.sh
cd ..
cd libxau
sh clean.sh
cd ..
cd libxext
sh clean.sh
cd ..
cd libxfont
sh clean.sh
cd ..
cd libsdl
sh clean.sh
cd ..
cd libsdlnet
sh clean.sh
cd ..

rm *.a 2>/dev/null
