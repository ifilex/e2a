#!/bin/sh 

echo Generando libSDLnet.a ..
ar cru ../libSDLnet.a *.o 
ranlib ../libSDLnet.a

