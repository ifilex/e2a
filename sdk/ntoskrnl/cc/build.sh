#!/bin/bash

cc()
{
echo Compilando $1 ..
gcc -w -Wall -Wno-unused-variable -Wno-unused-function -I../include -c $1
}


cc string.c
cc mem.c
cc block.c
cc host.c
cc unzip.c
cc ioapi.c