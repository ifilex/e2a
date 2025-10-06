#!/bin/bash

cc()
{
echo Compilando $1 ..
gcc -w -Wall -Wno-unused-variable -Wno-unused-function -I../include -I../include/SDL -c $1
}


cc crc.c
cc karray.c
cc kcircularlist.c
cc khashmap.c
cc klist.c
cc kstring.c
cc log.c
cc ringbuf.c
cc pbl/pbl.c
cc pbl/pblCollection.c
cc pbl/pblHeap.c
cc pbl/pblIterator.c
cc pbl/pblList.c
cc pbl/pblMap.c
cc pbl/pblPriorityQueue.c
cc pbl/pblSet.c
cc pbl/pblhash.c
cc pbl/pblisam.c
cc pbl/pblkf.c
