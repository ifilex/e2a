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

#include "SDL_eos.h"

int sdl_eos_AllocHWSurface (_THIS, SDL_Surface *surface)
{
	return -1;
}

int sdl_eos_CheckHWBlit (_THIS, SDL_Surface *src, SDL_Surface *dst)
{
	return 0;
}

int sdl_eos_FillHWRect (_THIS, SDL_Surface *dst, SDL_Rect *rect, Uint32 color)
{
	return 0;
}

int sdl_eos_SetHWColorKey (_THIS, SDL_Surface *surface, Uint32 key)
{
	return 0;
}

int sdl_eos_SetHWAlpha (_THIS, SDL_Surface *surface, Uint8 value)
{
	return 0;
}

int sdl_eos_LockHWSurface (_THIS, SDL_Surface *surface)
{
	return 0;
}

void sdl_eos_UnlockHWSurface (_THIS, SDL_Surface *surface)
{
}

int sdl_eos_FlipHWSurface (_THIS, SDL_Surface *surface)
{
	return 0;
}

void sdl_eos_FreeHWSurface (_THIS, SDL_Surface *surface)
{
}
