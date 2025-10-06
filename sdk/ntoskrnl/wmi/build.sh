#!/bin/bash

cc()
{
echo Compilando $1 ..
gcc -w -Wall -Wno-unused-variable -Wno-unused-function -I../include -I../include/SDL -c $1
}


cc main.c
cc sdlopengl.c
cc sdlwindows.c
cc winedrv.c
