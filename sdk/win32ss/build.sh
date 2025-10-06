#!/bin/sh

echo 
echo Compilando subsystem win32ss
echo 
echo - Compilando librerias ..
cd libs
sh build.sh
cd ..

echo - Compilando dlls ..
cd dlls
sh build.sh
cd ..

echo - Compilando servidor ..
cd server
sh build.sh
cd ..

echo - Compilando loader ..
cd loader
sh build.sh
cd ..
