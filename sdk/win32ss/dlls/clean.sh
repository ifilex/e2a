#!/bin/sh

cd uuid
sh clean.sh
cd ..


cd winecrt0
sh clean.sh
cd ..

cd kernel32
sh clean.sh
cd ..

cd ntdll
sh clean.sh
cd ..

cd winex11.drv
sh clean.sh
cd ..
