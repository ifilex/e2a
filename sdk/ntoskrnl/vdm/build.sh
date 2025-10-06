#!/bin/bash

cc()
{
echo Compilando $1 ..
gcc -w -Wall -Wno-unused-variable -Wno-unused-function -I../include -I../include/SDL -c $1
}


cc glMarshal.c
cc glMarshalSize.c
cc glMarshalVertex.c
cc glcommon.c
cc glext.c
cc glfunctions_ext1.c
cc glfunctions_ext2.c
cc glfunctions_ext3.c
cc es/esdisplaylist.c
cc es/esopengl.c
cc es/glshim.c
cc mesa/mesagl.c
cc sdl/sdlgl.c

