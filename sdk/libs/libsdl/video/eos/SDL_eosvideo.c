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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_eos.h"

#define eosVID_DRIVER_NAME "eos"

VideoBootStrap EOS_bootstrap = {
	eosVID_DRIVER_NAME, "BeSDL video driver",
	sdl_eos_Available, sdl_eos_CreateDevice
};

int sdl_eos_Available(void)
{
/*	const char *envr = getenv("SDL_VIDEODRIVER");
	if ((envr) && (strcmp(envr, eosVID_DRIVER_NAME) == 0)) {
		return(1);
	}*/

	return(1);
}

void sdl_eos_DeleteDevice(SDL_VideoDevice *device)
{
	Bfree(device->hidden);
	Bfree(device);
}

SDL_VideoDevice *sdl_eos_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;
	device = (SDL_VideoDevice *) Bmalloc(sizeof(SDL_VideoDevice));
	if (device) {
		memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *) Bmalloc((sizeof *device->hidden));
	}
	if ((device == NULL) || (device->hidden == NULL)) {
		SDL_OutOfMemory();
		if (device) {
			Bfree(device);
		}
		return(0);
	}
	memset(device->hidden, 0, (sizeof *device->hidden));

	device->VideoInit = sdl_eos_VideoInit;
	device->ListModes = sdl_eos_ListModes;
	device->SetVideoMode = sdl_eos_SetVideoMode;
	device->ToggleFullScreen = sdl_eos_ToggleFullScreen;
	device->UpdateMouse = sdl_eos_UpdateMouse;
	device->CreateYUVOverlay = sdl_eos_CreateYUVOverlay;
	device->SetColors = sdl_eos_SetColors;
	device->UpdateRects = sdl_eos_UpdateRects;
	device->VideoQuit = sdl_eos_VideoQuit;

	device->AllocHWSurface = sdl_eos_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = sdl_eos_LockHWSurface;
	device->UnlockHWSurface = sdl_eos_UnlockHWSurface;
	device->FlipHWSurface = NULL;
	device->FreeHWSurface = sdl_eos_FreeHWSurface;

	device->SetGamma = NULL;
	device->GetGamma = NULL;
	device->SetGammaRamp = NULL;
	device->GetGammaRamp = NULL;

	device->GL_LoadLibrary = NULL;
	device->GL_GetProcAddress = NULL;
	device->GL_GetAttribute = NULL;
	device->GL_MakeCurrent = NULL;
	device->GL_SwapBuffers = NULL;
	
	device->SetCaption = sdl_eos_SetCaption;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;

	device->FreeWMCursor = sdl_eos_FreeWMCursor;
	device->CreateWMCursor = sdl_eos_CreateWMCursor;
	device->ShowWMCursor = sdl_eos_ShowWMCursor;
	device->WarpWMCursor = sdl_eos_WarpWMCursor;
	device->MoveWMCursor = sdl_eos_MoveWMCursor;
	device->CheckMouseMode = sdl_eos_CheckMouseMode;	
	
	device->InitOSKeymap = sdl_eos_InitOSKeymap;
	device->PumpEvents = sdl_eos_PumpEvents;

	device->free = sdl_eos_DeleteDevice;

	return device;
}

int sdl_eos_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	if (BClientInit(&this->hidden->window)) {
		fprintf(stderr, "No se pudo conectar al AppServer.\n");
		exit(1);
	}

	vformat->BitsPerPixel = this->hidden->window->surface->bitsperpixel;
	vformat->BytesPerPixel = this->hidden->window->surface->bytesperpixel;;

	this->info.wm_available = 1;

	return 0;
}

SDL_Rect **sdl_eos_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
   	 return (SDL_Rect **) -1;
}

SDL_Surface * sdl_eos_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags)
{
	if (this->hidden->buffer) {
		Bfree(this->hidden->buffer);
	}

        bpp = this->hidden->window->surface->bitsperpixel;
        if (bpp == 15) {
		bpp = 16;
	}
        
	this->hidden->buffer = Bmalloc(width * height * (bpp / 8));
        this->hidden->bpp = bpp;
	memset(this->hidden->buffer, 0, width * height * (bpp / 8));

	if (!SDL_ReallocFormat(current, bpp, 0, 0, 0, 0)) {
		Bfree(this->hidden->buffer);
		this->hidden->buffer = NULL;
		SDL_SetError("No se pudo alojar los pixeles en el modo de video");
		return NULL;
	}

	current->flags = flags & SDL_FULLSCREEN;
	this->hidden->w = current->w = width;
	this->hidden->h = current->h = height;
	current->pitch = current->w * (bpp / 8);
	current->pixels = this->hidden->buffer;

	BWindowNew(this->hidden->window, B_WINDOW_MAIN, NULL);
//	BWindowBig(this->hidden->window);
	BWindowSetCoor(this->hidden->window, 0, -3, 20, width, height);
	BWindowSetCoor(this->hidden->window, B_WINDOW_NOFORM, this->hidden->window->surface->buf->x, this->hidden->window->surface->buf->y, width, height);
	BWindowSetTitle(this->hidden->window, "BeSDL");
	BWindowShow(this->hidden->window);

	BClientAtexit(this->hidden->window, sdl_eos_atexit);
	BClientAtevent(this->hidden->window, sdl_eos_atevent);

	this->hidden->tid = BThreadCreate((void * (*) (void *)) &BClientMain, this->hidden->window);
	sdl_eos_running = 1;

	return current ;
}

int sdl_eos_ToggleFullScreen (_THIS, int on)
{
	return -1;
}

void sdl_eos_UpdateMouse (_THIS)
{
}

SDL_Overlay * sdl_eos_CreateYUVOverlay (_THIS, int width, int height, Uint32 format, SDL_Surface *display)
{
	return NULL;
}

int sdl_eos_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	return 1;
}

void sdl_eos_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
        char *buf;
        int n = numrects;
        SDL_Rect *r = rects;

        if (!sdl_eos_running) {
		return;
	}
	
	sdl_eos_running_ = 1;
	
        if (this->hidden->bpp != this->hidden->window->surface->bitsperpixel) {
		Bcopybuffer(this->hidden->buffer, this->hidden->bpp, &buf, this->hidden->window->surface->bitsperpixel, this->hidden->w, this->hidden->h);
	        while (n--) {
			Bputboxpart(this->hidden->window->surface, r->x, r->y, r->w, r->h, this->hidden->w, this->hidden->h, buf, r->x, r->y);
			r++;
		}
		Bfree(buf);
	} else {
		while (n--) {
			Bputboxpart(this->hidden->window->surface, r->x, r->y, r->w, r->h, this->hidden->w, this->hidden->h, this->hidden->buffer, r->x, r->y);
			r++;
		}
	}
	
	sdl_eos_running_ = 0;
}

void sdl_eos_VideoQuit(_THIS)
{
	while (sdl_eos_running_) {
		usleep(20000);
	}
	
	if (this->screen->pixels != NULL)
	{
		Bfree(this->screen->pixels);
		this->screen->pixels = NULL;
	}
	
	if (sdl_eos_running) {
		BClientQuit(this->hidden->window);
		BThreadJoin(this->hidden->tid, NULL);
	}
	sdl_eos_running = 0;
}
