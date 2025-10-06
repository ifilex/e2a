#!/bin/sh

cd libs
sh clean.sh
cd ..

cd dlls
sh clean.sh
cd ..

cd server
sh clean.sh
cd ..

cd loader
sh clean.sh
cd ..
