/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __WND_H__
#define __WND_H__

#include "platform.h"
#include "memory.h"
#include "pixelformat.h"

struct wRECT {
    S32 left;
    S32 top;
    S32 right;
    S32 bottom;
};

struct Wnd {
    U32 surface;
    struct wRECT windowRect;
    struct wRECT clientRect;
    const char* text;
    PixelFormat* pixelFormat;
    U32 pixelFormatIndex;
    void* openGlContext;
    U32 activated;
    U32 processId;
    U32 hwnd;
#ifdef SDL2
    void* sdlTexture;
    int sdlTextureHeight;
    int sdlTextureWidth;
#else
    void* sdlSurface;
#endif
};

void writeRect(struct KThread* thread, U32 address, struct wRECT* rect);
void readRect(struct KThread* thread, U32 address, struct wRECT* rect);

struct Wnd* getWnd(U32 hwnd);
struct Wnd* wndCreate(struct KThread* thread, U32 processId, U32 hwnd, U32 windowRect, U32 clientRect);
void wndDestroy(U32 hwnd);
void setWndText(struct Wnd* wnd, const char* text);
void updateScreen();

#endif