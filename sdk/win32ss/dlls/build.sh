#!/bin/sh

echo -- Compilando libuuid ..
cd uuid
sh build.sh
cd ..


echo -- Compilando libwinecrt0 ..
cd winecrt0
sh build.sh
cd ..


echo -- Compilando kernel32.dll ..
cd kernel32
sh build.sh
cd ..

echo -- Compilando ntdll.dll ..
cd ntdll
sh build.sh
cd ..

echo -- Compilando winex11.drv ..
cd winex11.drv
sh build.sh
cd ..
