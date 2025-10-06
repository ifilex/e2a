#!/bin/bash
echo 
echo Compilando ntoskrnl ..
echo 
echo - Compilando cc ..
cd cc
sh build.sh
cd ..
echo - Compilando fstub ..
cd fstub
sh build.sh
cd ..
echo - Compilando io ..
cd io
sh build.sh
cd ..
echo - Compilando ke ..
cd ke
sh build.sh
cd ..
echo - Compilando lpc ..
cd lpc
sh build.sh
cd ..
echo - Compilando mm ..
cd mm
sh build.sh
cd ..
echo - Compilando vdm ..
cd vdm
sh build.sh
cd ..
echo - Compilando wmi ..
cd wmi
sh build.sh
cd ..

sh link.sh
