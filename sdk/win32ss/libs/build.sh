#!/bin/sh

echo -- Compilando libwine_ports..
cd port
sh build.sh
cd ..

echo -- Compilando libwpp..
cd wpp
sh build.sh
cd ..

echo -- Compilando libwine..
cd wine
sh build.sh
cd ..

