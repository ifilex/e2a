#!/bin/sh


cc()
{
echo Compilando $1 ..
gcc -g -O2 -I../../include/kernel/SDL -D_GNU_SOURCE=1 -DXTHREADS -D_REENTRANT -DHAVE_LINUX_VERSION_H -c $1  -fPIC -DPIC 
}

cc ./SDL.c 
cc ./SDL_error.c
cc ./SDL_fatal.c
cc ./audio/SDL_audio.c
cc ./audio/SDL_audiocvt.c
cc ./audio/SDL_audiodev.c 
cc ./audio/SDL_mixer.c 
cc ./audio/SDL_mixer_MMX.c 
cc ./audio/SDL_mixer_MMX_VC.c 
cc ./audio/SDL_mixer_m68k.c 
cc ./audio/SDL_wave.c 
cc ./cdrom/SDL_cdrom.c 
cc ./cpuinfo/SDL_cpuinfo.c 
cc ./events/SDL_active.c
cc ./events/SDL_events.c 
cc ./events/SDL_expose.c 
cc ./events/SDL_keyboard.c 
cc ./events/SDL_mouse.c
cc ./events/SDL_quit.c 
cc ./events/SDL_resize.c 
cc ./file/SDL_rwops.c 
cc ./joystick/SDL_joystick.c 
cc ./stdlib/SDL_getenv.c 
cc ./stdlib/SDL_iconv.c 
cc ./stdlib/SDL_malloc.c 
cc ./stdlib/SDL_qsort.c 
cc ./stdlib/SDL_stdlib.c
cc ./stdlib/SDL_string.c
cc ./thread/SDL_thread.c 
cc ./timer/SDL_timer.c
cc ./video/SDL_RLEaccel.c
cc ./video/SDL_blit.c 
cc ./video/SDL_blit_0.c
cc ./video/SDL_blit_1.c 
cc ./video/SDL_blit_A.c
cc ./video/SDL_blit_N.c 
cc ./video/SDL_bmp.c 
cc ./video/SDL_cursor.c 
cc ./video/SDL_gamma.c 
cc ./video/SDL_pixels.c 
cc ./video/SDL_stretch.c 
cc ./video/SDL_surface.c
cc ./video/SDL_video.c
cc ./video/SDL_yuv.c 
cc ./video/SDL_yuv_mmx.c 
cc ./video/SDL_yuv_sw.c 
cc ./video/dummy/SDL_nullevents.c 
cc ./video/dummy/SDL_nullmouse.c 
cc ./video/dummy/SDL_nullvideo.c 
cc ./audio/disk/SDL_diskaudio.c 
cc ./audio/dummy/SDL_dummyaudio.c 
cc ./loadso/dummy/SDL_sysloadso.c 
cc ./audio/dsp/SDL_dspaudio.c 
cc ./audio/dma/SDL_dmaaudio.c 
cc ./video/x11/SDL_x11dga.c 
cc ./video/x11/SDL_x11dyn.c 
cc ./video/x11/SDL_x11events.c 
cc ./video/x11/SDL_x11gamma.c 
cc ./video/x11/SDL_x11gl.c
cc ./video/x11/SDL_x11image.c 
cc ./video/x11/SDL_x11modes.c 
cc ./video/x11/SDL_x11mouse.c 
cc ./video/x11/SDL_x11video.c
cc ./video/x11/SDL_x11wm.c 
cc ./video/Xext/XME/xme.c 
cc ./video/Xext/Xinerama/Xinerama.c 
cc ./video/Xext/Xv/Xv.c 
cc ./video/Xext/Xxf86dga/XF86DGA.c 
cc ./video/Xext/Xxf86dga/XF86DGA2.c
cc ./video/Xext/Xxf86vm/XF86VMode.c
cc ./thread/pthread/SDL_systhread.c 
cc ./thread/pthread/SDL_syssem.c 
cc ./thread/pthread/SDL_sysmutex.c 
cc ./thread/pthread/SDL_syscond.c 
cc ./joystick/linux/SDL_sysjoystick.c 
cc ./cdrom/linux/SDL_syscdrom.c
cc ./timer/unix/SDL_systimer.c

sh link.sh
