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

int sdl_eos_GL_LoadLibrary (_THIS, const char *path)
{
	return 0;
}

void * sdl_eos_GL_GetProcAddress (_THIS, const char *proc)
{
	return NULL;
}

int sdl_eos_GL_GetAttribute (_THIS, SDL_GLattr attrib, int *value)
{
	return 0;
}

int sdl_eos_GL_MakeCurrent (_THIS)
{
	return 0;
}

void sdl_eos_GL_SwapBuffers (_THIS)
{
}
