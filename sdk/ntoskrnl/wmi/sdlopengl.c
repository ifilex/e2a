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

#include "winebox.h"
#include "platform.h"
#include "memory.h"
#include <SDL.h>
#include "wnd.h"

static int modesInitialized;

PixelFormat pfs[512];
U32 numberOfPfs;

void initDisplayModes() {
    if (!modesInitialized) {
        modesInitialized = 1;
        numberOfPfs = getPixelFormats(pfs, sizeof(pfs)/sizeof(PixelFormat));            
    }
}

U32 sdl_wglDescribePixelFormat(struct KThread* thread, U32 hdc, U32 fmt, U32 size, U32 descr)
{
    initDisplayModes();

    if (!descr) return numberOfPfs;
    if (size < 40) return 0;
    if (fmt>numberOfPfs) {
        return 0;
    }

    writew(thread, descr, pfs[fmt].nSize); descr+=2;
    writew(thread, descr, pfs[fmt].nVersion); descr+=2;
    writed(thread, descr, pfs[fmt].dwFlags); descr+=4;
    writeb(thread, descr, pfs[fmt].iPixelType); descr++;
    writeb(thread, descr, pfs[fmt].cColorBits); descr++;
    writeb(thread, descr, pfs[fmt].cRedBits); descr++;
    writeb(thread, descr, pfs[fmt].cRedShift); descr++;
    writeb(thread, descr, pfs[fmt].cGreenBits); descr++;
    writeb(thread, descr, pfs[fmt].cGreenShift); descr++;
    writeb(thread, descr, pfs[fmt].cBlueBits); descr++;
    writeb(thread, descr, pfs[fmt].cBlueShift); descr++;
    writeb(thread, descr, pfs[fmt].cAlphaBits); descr++;
    writeb(thread, descr, pfs[fmt].cAlphaShift); descr++;
    writeb(thread, descr, pfs[fmt].cAccumBits); descr++;
    writeb(thread, descr, pfs[fmt].cAccumRedBits); descr++;
    writeb(thread, descr, pfs[fmt].cAccumGreenBits); descr++;
    writeb(thread, descr, pfs[fmt].cAccumBlueBits); descr++;
    writeb(thread, descr, pfs[fmt].cAccumAlphaBits); descr++;
    writeb(thread, descr, pfs[fmt].cDepthBits); descr++;
    writeb(thread, descr, pfs[fmt].cStencilBits); descr++;
    writeb(thread, descr, pfs[fmt].cAuxBuffers); descr++;
    writeb(thread, descr, pfs[fmt].iLayerType); descr++;
    writeb(thread, descr, pfs[fmt].bReserved); descr++;
    writed(thread, descr, pfs[fmt].dwLayerMask); descr+=4;
    writed(thread, descr, pfs[fmt].dwVisibleMask); descr+=4;
    writed(thread, descr, pfs[fmt].dwDamageMask);

    return numberOfPfs;
}

void writePixelFormat(struct KThread* thread, PixelFormat* pf, U32 descr) {
    pf->nSize = readw(thread, descr); descr+=2;
    pf->nVersion = readw(thread, descr); descr+=2;
    pf->dwFlags = readd(thread, descr); descr+=4;
    pf->iPixelType = readb(thread, descr); descr++;
    pf->cColorBits = readb(thread, descr); descr++;
    pf->cRedBits = readb(thread, descr); descr++;
    pf->cRedShift = readb(thread, descr); descr++;
    pf->cGreenBits = readb(thread, descr); descr++;
    pf->cGreenShift = readb(thread, descr); descr++;
    pf->cBlueBits = readb(thread, descr); descr++;
    pf->cBlueShift = readb(thread, descr); descr++;
    pf->cAlphaBits = readb(thread, descr); descr++;
    pf->cAlphaShift = readb(thread, descr); descr++;
    pf->cAccumBits = readb(thread, descr); descr++;
    pf->cAccumRedBits = readb(thread, descr); descr++;
    pf->cAccumGreenBits = readb(thread, descr); descr++;
    pf->cAccumBlueBits = readb(thread, descr); descr++;
    pf->cAccumAlphaBits = readb(thread, descr); descr++;
    pf->cDepthBits = readb(thread, descr); descr++;
    pf->cStencilBits = readb(thread, descr); descr++;
    pf->cAuxBuffers = readb(thread, descr); descr++;
    pf->iLayerType = readb(thread, descr); descr++;
    pf->bReserved = readb(thread, descr); descr++;
    pf->dwLayerMask = readd(thread, descr); descr+=4;
    pf->dwVisibleMask = readd(thread, descr); descr+=4;
    pf->dwDamageMask = readd(thread, descr);
}