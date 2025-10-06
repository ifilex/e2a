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

#if 0
	#define debugf(a) printf(a);
#else
	#define debugf(a)
#endif

void sdl_eos_FreeWMCursor (_THIS, WMcursor *cursor)
{
	Bfree(cursor);
	debugf("sdl_eos_FreeWMCursor\n");
}

WMcursor * sdl_eos_CreateWMCursor (_THIS, Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y)
{
	WMcursor *cursor = (WMcursor *) Bmalloc(sizeof(WMcursor));
	debugf("sdl_eos_CreateWMCursor\n");
	return cursor;
}

int sdl_eos_ShowWMCursor (_THIS, WMcursor *cursor)
{
	debugf("sdl_eos_ShowWMCursor\n");
	return 1;
}

void sdl_eos_WarpWMCursor (_THIS, Uint16 x, Uint16 y)
{
	debugf("sdl_eos_WarpWMCursor\n");
}

void sdl_eos_MoveWMCursor (_THIS, int x, int y)
{
	debugf("sdl_eos_MoveWMCursor\n");
}

void sdl_eos_CheckMouseMode (_THIS)
{
	debugf("sdl_eos_CheckMouseMode\n");
}
