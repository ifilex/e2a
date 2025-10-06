#!/bin/sh 
cc()
{
echo Compilando $1 ..
gcc -I. -Wall -w -Wparentheses -pipe -fsigned-char -DREENTRANT -g -O2  -DFT2_BUILD_LIBRARY -c $1
}

cc e_acos.c
cc e_acosh.c
cc e_asin.c
cc e_cosh.c
cc e_exp.c
cc e_fmod.c
cc e_log.c
cc e_log10.c
cc e_pow.c
cc e_rem_pio2.c
cc e_scalb.c
cc e_sinh.c
cc e_sqrt.c
cc k_cos.c
cc k_rem_pio2.c
cc k_sin.c
cc k_standard.c
cc k_tan.c
cc s_asinh.c
cc s_ceil.c
cc s_ceilf.c
cc s_copysign.c
cc s_cos.c
cc s_floor.c
cc s_ilogb.c
cc s_lib_version.c
cc s_log1p.c
cc s_logb.c
cc s_matherr.c
cc s_rint.c
cc s_scalbn.c
cc s_sin.c
cc strlcpy.c
cc w_asin.c
cc w_cosh.c
cc w_fmod.c
cc w_log.c
cc w_log10.c
cc w_pow.c
cc w_scalb.c
cc w_sinh.c

sh link.sh