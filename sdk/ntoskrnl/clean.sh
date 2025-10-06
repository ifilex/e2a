#!/bin/bash
cd cc
sh clean.sh
cd ..

cd fstub
sh clean.sh
cd ..

cd io
sh clean.sh
cd ..

cd ke
sh clean.sh
cd ..

cd lpc
sh clean.sh
cd ..

cd mm
sh clean.sh
cd ..

cd vdm
sh clean.sh
cd ..

cd wmi
sh clean.sh
cd ..

rm ntoskrnl 2>/dev/null

