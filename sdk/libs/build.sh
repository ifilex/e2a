#!/bin/sh 

echo - Compilando libx ..
cd libx
sh build.sh
cd ..
echo - Compilando libxau ..
cd libxau
sh build.sh
cd ..
echo - Compilando libxext ..
cd libxext
sh build.sh
cd ..
echo - Compilando libxfont ..
cd libxfont
sh build.sh
cd ..
echo - Compilando libsdl ..
cd libsdl
sh build.sh
cd ..
echo - Compilando libsdlnet ..
cd libsdlnet
sh build.sh
cd ..

