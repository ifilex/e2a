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

void sdl_eos_SetCaption (_THIS, const char *title, const char *icon)
{
	BWindowSetTitle(this->hidden->window, (char *) title);
}

void sdl_eos_SetIcon (_THIS, SDL_Surface *icon, Uint8 *mask)
{
}

int sdl_eos_IconifyWindow (_THIS)
{
	return 0;
}

SDL_GrabMode sdl_eos_GrabInput (_THIS, SDL_GrabMode mode)
{
	return SDL_GRAB_OFF;
}

int sdl_eos_GetWMInfo (_THIS, SDL_SysWMinfo *info)
{
	return 0;
}
