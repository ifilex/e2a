#!/bin/sh 

echo Generando libSDL.a ..
ar cru ../libSDL.a *.o 
ranlib ../libSDL.a

