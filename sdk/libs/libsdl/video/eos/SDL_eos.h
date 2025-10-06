/***************************************************************************
    begin                : Thu Jan 22 2004
    copyright            : (C) 2004 - 2006 by Alper Akcan
    email                : distchx@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef SDL_eos_h
#define SDL_eos_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <B.h>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"

#define _THIS	SDL_VideoDevice *this

int sdl_eos_running;
int sdl_eos_running_;

struct WMcursor {
	int unused;
};

struct SDL_PrivateVideoData {
    int w;
    int h;
    int bpp;
    void *buffer;
    BThreadType *tid;
    BWindowType *window;
};

void sdl_eos_FreeWMCursor (_THIS, WMcursor *cursor);
WMcursor * sdl_eos_CreateWMCursor (_THIS, Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y);
int sdl_eos_ShowWMCursor (_THIS, WMcursor *cursor);
void sdl_eos_WarpWMCursor (_THIS, Uint16 x, Uint16 y);
void sdl_eos_MoveWMCursor (_THIS, int x, int y);
void sdl_eos_CheckMouseMode (_THIS);
void sdl_eos_atexit (BWindowType *window);
void sdl_eos_atevent (BWindowType *window, BEventType *event);
void sdl_eos_InitOSKeymap (_THIS);
void sdl_eos_PumpEvents (_THIS);
SDL_keysym * sdl_eos_translatekey(BEventType *event, SDL_keysym *keysym);
int sdl_eos_SetGamma (_THIS, float red, float green, float blue);
int sdl_eos_GetGamma (_THIS, float *red, float *green, float *blue);
int sdl_eos_SetGammaRamp (_THIS, Uint16 *ramp);
int sdl_eos_GetGammaRamp (_THIS, Uint16 *ramp);
int sdl_eos_GL_LoadLibrary (_THIS, const char *path);
void* sdl_eos_GL_GetProcAddress (_THIS, const char *proc);
int sdl_eos_GL_GetAttribute (_THIS, SDL_GLattr attrib, int* value);
int sdl_eos_GL_MakeCurrent (_THIS);
void sdl_eos_GL_SwapBuffers (_THIS);
int sdl_eos_AllocHWSurface (_THIS, SDL_Surface *surface);
int sdl_eos_CheckHWBlit (_THIS, SDL_Surface *src, SDL_Surface *dst);
int sdl_eos_FillHWRect (_THIS, SDL_Surface *dst, SDL_Rect *rect, Uint32 color);
int sdl_eos_SetHWColorKey (_THIS, SDL_Surface *surface, Uint32 key);
int sdl_eos_SetHWAlpha (_THIS, SDL_Surface *surface, Uint8 value);
int sdl_eos_LockHWSurface (_THIS, SDL_Surface *surface);
void sdl_eos_UnlockHWSurface (_THIS, SDL_Surface *surface);
int sdl_eos_FlipHWSurface (_THIS, SDL_Surface *surface);
void sdl_eos_FreeHWSurface (_THIS, SDL_Surface *surface);
int sdl_eos_Available(void);
void sdl_eos_DeleteDevice(SDL_VideoDevice *device);
SDL_VideoDevice *sdl_eos_CreateDevice(int devindex);
int sdl_eos_VideoInit (_THIS, SDL_PixelFormat *vformat);
SDL_Rect ** sdl_eos_ListModes (_THIS, SDL_PixelFormat *format, Uint32 flags);
SDL_Surface * sdl_eos_SetVideoMode (_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
int sdl_eos_ToggleFullScreen (_THIS, int on);
void sdl_eos_UpdateMouse (_THIS);
SDL_Overlay * sdl_eos_CreateYUVOverlay (_THIS, int width, int height, Uint32 format, SDL_Surface *display);
int sdl_eos_SetColors (_THIS, int firstcolor, int ncolors, SDL_Color *colors);
void sdl_eos_UpdateRects (_THIS, int numrects, SDL_Rect *rects);
void sdl_eos_VideoQuit (_THIS);
void sdl_eos_SetCaption (_THIS, const char *title, const char *icon);
void sdl_eos_SetIcon (_THIS, SDL_Surface *icon, Uint8 *mask);
int sdl_eos_IconifyWindow (_THIS);
SDL_GrabMode sdl_eos_GrabInput (_THIS, SDL_GrabMode mode);
int sdl_eos_GetWMInfo (_THIS, SDL_SysWMinfo *info);

#endif
