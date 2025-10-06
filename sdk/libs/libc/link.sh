#!/bin/sh 

echo Generando libC.a ..
ar cru ../libC.a *.o 
ranlib ../libC.a

