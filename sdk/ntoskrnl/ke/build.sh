#!/bin/bash

cc()
{
echo Compilando $1 ..
gcc -w -Wall -Wno-unused-variable -Wno-unused-function -I../include -I../include/SDL -c $1
}


cc kalloc.c
cc kepoll.c
cc kfile.c
cc kfiledescriptor.c
cc kfilelock.c
cc kio.c
cc kmmap.c
cc kobject.c
cc kpoll.c
cc kprocess.c
cc kscheduler.c
cc kshm.c
cc ksignal.c
cc ksocket.c
cc ksystem.c
cc kthread.c
cc syscall.c
cc loader/elf.c
cc loader/loader.c
cc devs/devdsp.c
cc devs/devfb.c
cc devs/devinput.c
cc devs/devmixer.c
cc devs/devnull.c
cc devs/devtty.c
cc devs/devurandom.c
cc devs/devzero.c
cc proc/bufferaccess.c
cc proc/meminfo.c

