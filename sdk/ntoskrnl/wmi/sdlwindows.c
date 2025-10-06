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

#include <SDL.h>
#include "winebox.h"
#include "kalloc.h"
#include "pbl.h"
#include "platform.h"
#include "memory.h"
#include "kprocess.h"
#include "ksystem.h"
#include "sdlwindow.h"
#include "kscheduler.h"

#ifdef BOXEDWINE_VM
extern U32 sdlCustomEvent;
extern SDL_threadID sdlMainThreadId;

static struct SdlCallback* freeSdlCallbacks;
static SDL_mutex* freeSdlCallbacksMutex;

struct SdlCallback* allocSdlCallback(struct KThread* thread) {
    if (!freeSdlCallbacksMutex)
        freeSdlCallbacksMutex = SDL_CreateMutex();
    SDL_LockMutex(freeSdlCallbacksMutex);
    if (freeSdlCallbacks) {
        struct SdlCallback* result = freeSdlCallbacks;
        freeSdlCallbacks = freeSdlCallbacks->next;
        result->thread = thread;
        SDL_UnlockMutex(freeSdlCallbacksMutex);
        return result;
    } else {
        struct SdlCallback* result = malloc(sizeof(struct SdlCallback));
        memset(result, 0, sizeof(struct SdlCallback));
        result->sdlEvent.type = sdlCustomEvent;
        result->thread = thread;
        result->sdlEvent.user.data1 = result;
        result->mutex = SDL_CreateMutex();
        result->cond = SDL_CreateCond();
        SDL_UnlockMutex(freeSdlCallbacksMutex);
        return result;
    }    
}

void freeSdlCallback(struct SdlCallback* callback) {
    SDL_LockMutex(freeSdlCallbacksMutex);
    callback->next = freeSdlCallbacks;
    freeSdlCallbacks = callback;
    SDL_UnlockMutex(freeSdlCallbacksMutex);
}

#endif

int bits_per_pixel = 32;
int default_horz_res = 800;
int default_vert_res = 600;
int default_bits_per_pixel = 32;
int sdlScale = 100;
const char* sdlScaleQuality = "0";

static int firstWindowCreated;
U32 sdlFullScreen;

PblMap* cursors;
PblMap* hwndToWnd;
SDL_Color sdlPalette[256];
SDL_Color sdlSystemPalette[256] = {
    {0x00,0x00,0x00},
    {0x80,0x00,0x00},
    {0x00,0x80,0x00},
    {0x80,0x80,0x00},
    {0x00,0x00,0x80},
    {0x80,0x00,0x80},
    {0x00,0x80,0x80},
    {0xC0,0xC0,0xC0},
    {0xC0,0xDC,0xC0},
    {0xA6,0xCA,0xF0},
    {0x2A,0x3F,0xAA},
    {0x2A,0x3F,0xFF},
    {0x2A,0x5F,0x00},
    {0x2A,0x5F,0x55},
    {0x2A,0x5F,0xAA},
    {0x2A,0x5F,0xFF},
    {0x2A,0x7F,0x00},
    {0x2A,0x7F,0x55},
    {0x2A,0x7F,0xAA},
    {0x2A,0x7F,0xFF},
    {0x2A,0x9F,0x00},
    {0x2A,0x9F,0x55},
    {0x2A,0x9F,0xAA},
    {0x2A,0x9F,0xFF},
    {0x2A,0xBF,0x00},
    {0x2A,0xBF,0x55},
    {0x2A,0xBF,0xAA},
    {0x2A,0xBF,0xFF},
    {0x2A,0xDF,0x00},
    {0x2A,0xDF,0x55},
    {0x2A,0xDF,0xAA},
    {0x2A,0xDF,0xFF},
    {0x2A,0xFF,0x00},
    {0x2A,0xFF,0x55},
    {0x2A,0xFF,0xAA},
    {0x2A,0xFF,0xFF},
    {0x55,0x00,0x00},
    {0x55,0x00,0x55},
    {0x55,0x00,0xAA},
    {0x55,0x00,0xFF},
    {0x55,0x1F,0x00},
    {0x55,0x1F,0x55},
    {0x55,0x1F,0xAA},
    {0x55,0x1F,0xFF},
    {0x55,0x3F,0x00},
    {0x55,0x3F,0x55},
    {0x55,0x3F,0xAA},
    {0x55,0x3F,0xFF},
    {0x55,0x5F,0x00},
    {0x55,0x5F,0x55},
    {0x55,0x5F,0xAA},
    {0x55,0x5F,0xFF},
    {0x55,0x7F,0x00},
    {0x55,0x7F,0x55},
    {0x55,0x7F,0xAA},
    {0x55,0x7F,0xFF},
    {0x55,0x9F,0x00},
    {0x55,0x9F,0x55},
    {0x55,0x9F,0xAA},
    {0x55,0x9F,0xFF},
    {0x55,0xBF,0x00},
    {0x55,0xBF,0x55},
    {0x55,0xBF,0xAA},
    {0x55,0xBF,0xFF},
    {0x55,0xDF,0x00},
    {0x55,0xDF,0x55},
    {0x55,0xDF,0xAA},
    {0x55,0xDF,0xFF},
    {0x55,0xFF,0x00},
    {0x55,0xFF,0x55},
    {0x55,0xFF,0xAA},
    {0x55,0xFF,0xFF},
    {0x7F,0x00,0x00},
    {0x7F,0x00,0x55},
    {0x7F,0x00,0xAA},
    {0x7F,0x00,0xFF},
    {0x7F,0x1F,0x00},
    {0x7F,0x1F,0x55},
    {0x7F,0x1F,0xAA},
    {0x7F,0x1F,0xFF},
    {0x7F,0x3F,0x00},
    {0x7F,0x3F,0x55},
    {0x7F,0x3F,0xAA},
    {0x7F,0x3F,0xFF},
    {0x7F,0x5F,0x00},
    {0x7F,0x5F,0x55},
    {0x7F,0x5F,0xAA},
    {0x7F,0x5F,0xFF},
    {0x7F,0x7F,0x00},
    {0x7F,0x7F,0x55},
    {0x7F,0x7F,0xAA},
    {0x7F,0x7F,0xFF},
    {0x7F,0x9F,0x00},
    {0x7F,0x9F,0x55},
    {0x7F,0x9F,0xAA},
    {0x7F,0x9F,0xFF},
    {0x7F,0xBF,0x00},
    {0x7F,0xBF,0x55},
    {0x7F,0xBF,0xAA},
    {0x7F,0xBF,0xFF},
    {0x7F,0xDF,0x00},
    {0x7F,0xDF,0x55},
    {0x7F,0xDF,0xAA},
    {0x7F,0xDF,0xFF},
    {0x7F,0xFF,0x00},
    {0x7F,0xFF,0x55},
    {0x7F,0xFF,0xAA},
    {0x7F,0xFF,0xFF},
    {0xAA,0x00,0x00},
    {0xAA,0x00,0x55},
    {0xAA,0x00,0xAA},
    {0xAA,0x00,0xFF},
    {0xAA,0x1F,0x00},
    {0xAA,0x1F,0x55},
    {0xAA,0x1F,0xAA},
    {0xAA,0x1F,0xFF},
    {0xAA,0x3F,0x00},
    {0xAA,0x3F,0x55},
    {0xAA,0x3F,0xAA},
    {0xAA,0x3F,0xFF},
    {0xAA,0x5F,0x00},
    {0xAA,0x5F,0x55},
    {0xAA,0x5F,0xAA},
    {0xAA,0x5F,0xFF},
    {0xAA,0x7F,0x00},
    {0xAA,0x7F,0x55},
    {0xAA,0x7F,0xAA},
    {0xAA,0x7F,0xFF},
    {0xAA,0x9F,0x00},
    {0xAA,0x9F,0x55},
    {0xAA,0x9F,0xAA},
    {0xAA,0x9F,0xFF},
    {0xAA,0xBF,0x00},
    {0xAA,0xBF,0x55},
    {0xAA,0xBF,0xAA},
    {0xAA,0xBF,0xFF},
    {0xAA,0xDF,0x00},
    {0xAA,0xDF,0x55},
    {0xAA,0xDF,0xAA},
    {0xAA,0xDF,0xFF},
    {0xAA,0xFF,0x00},
    {0xAA,0xFF,0x55},
    {0xAA,0xFF,0xAA},
    {0xAA,0xFF,0xFF},
    {0xD4,0x00,0x00},
    {0xD4,0x00,0x55},
    {0xD4,0x00,0xAA},
    {0xD4,0x00,0xFF},
    {0xD4,0x1F,0x00},
    {0xD4,0x1F,0x55},
    {0xD4,0x1F,0xAA},
    {0xD4,0x1F,0xFF},
    {0xD4,0x3F,0x00},
    {0xD4,0x3F,0x55},
    {0xD4,0x3F,0xAA},
    {0xD4,0x3F,0xFF},
    {0xD4,0x5F,0x00},
    {0xD4,0x5F,0x55},
    {0xD4,0x5F,0xAA},
    {0xD4,0x5F,0xFF},
    {0xD4,0x7F,0x00},
    {0xD4,0x7F,0x55},
    {0xD4,0x7F,0xAA},
    {0xD4,0x7F,0xFF},
    {0xD4,0x9F,0x00},
    {0xD4,0x9F,0x55},
    {0xD4,0x9F,0xAA},
    {0xD4,0x9F,0xFF},
    {0xD4,0xBF,0x00},
    {0xD4,0xBF,0x55},
    {0xD4,0xBF,0xAA},
    {0xD4,0xBF,0xFF},
    {0xD4,0xDF,0x00},
    {0xD4,0xDF,0x55},
    {0xD4,0xDF,0xAA},
    {0xD4,0xDF,0xFF},
    {0xD4,0xFF,0x00},
    {0xD4,0xFF,0x55},
    {0xD4,0xFF,0xAA},
    {0xD4,0xFF,0xFF},
    {0xFF,0x00,0x55},
    {0xFF,0x00,0xAA},
    {0xFF,0x1F,0x00},
    {0xFF,0x1F,0x55},
    {0xFF,0x1F,0xAA},
    {0xFF,0x1F,0xFF},
    {0xFF,0x3F,0x00},
    {0xFF,0x3F,0x55},
    {0xFF,0x3F,0xAA},
    {0xFF,0x3F,0xFF},
    {0xFF,0x5F,0x00},
    {0xFF,0x5F,0x55},
    {0xFF,0x5F,0xAA},
    {0xFF,0x5F,0xFF},
    {0xFF,0x7F,0x00},
    {0xFF,0x7F,0x55},
    {0xFF,0x7F,0xAA},
    {0xFF,0x7F,0xFF},
    {0xFF,0x9F,0x00},
    {0xFF,0x9F,0x55},
    {0xFF,0x9F,0xAA},
    {0xFF,0x9F,0xFF},
    {0xFF,0xBF,0x00},
    {0xFF,0xBF,0x55},
    {0xFF,0xBF,0xAA},
    {0xFF,0xBF,0xFF},
    {0xFF,0xDF,0x00},
    {0xFF,0xDF,0x55},
    {0xFF,0xDF,0xAA},
    {0xFF,0xDF,0xFF},
    {0xFF,0xFF,0x55},
    {0xFF,0xFF,0xAA},
    {0xCC,0xCC,0xFF},
    {0xFF,0xCC,0xFF},
    {0x33,0xFF,0xFF},
    {0x66,0xFF,0xFF},
    {0x99,0xFF,0xFF},
    {0xCC,0xFF,0xFF},
    {0x00,0x7F,0x00},
    {0x00,0x7F,0x55},
    {0x00,0x7F,0xAA},
    {0x00,0x7F,0xFF},
    {0x00,0x9F,0x00},
    {0x00,0x9F,0x55},
    {0x00,0x9F,0xAA},
    {0x00,0x9F,0xFF},
    {0x00,0xBF,0x00},
    {0x00,0xBF,0x55},
    {0x00,0xBF,0xAA},
    {0x00,0xBF,0xFF},
    {0x00,0xDF,0x00},
    {0x00,0xDF,0x55},
    {0x00,0xDF,0xAA},
    {0x00,0xDF,0xFF},
    {0x00,0xFF,0x55},
    {0x00,0xFF,0xAA},
    {0x2A,0x00,0x00},
    {0x2A,0x00,0x55},
    {0x2A,0x00,0xAA},
    {0x2A,0x00,0xFF},
    {0x2A,0x1F,0x00},
    {0x2A,0x1F,0x55},
    {0x2A,0x1F,0xAA},
    {0x2A,0x1F,0xFF},
    {0x2A,0x3F,0x00},
    {0x2A,0x3F,0x55},
    {0xFF,0xFB,0xF0},
    {0xA0,0xA0,0xA4},
    {0x80,0x80,0x80},
    {0xFF,0x00,0x00},
    {0x00,0xFF,0x00},
    {0xFF,0xFF,0x00},
    {0x00,0x00,0xFF},
    {0xFF,0x00,0xFF},
    {0x00,0xFF,0xFF},
    {0xFF,0xFF,0xFF},
};

static void displayChanged(struct KThread* thread);

void initSDL() {
    default_horz_res = screenCx;
    default_vert_res = screenCy;
    hwndToWnd = pblMapNewHashMap();
}

BOOL isBoxedWineDriverActive() {
    return pblMapSize(hwndToWnd)!=0;
}

struct Wnd* getWnd(U32 hwnd) {
    struct Wnd** result;

    result = pblMapGet(hwndToWnd, &hwnd, sizeof(U32), NULL);
    if (result)
        return *result;
    return NULL;
}

static struct Wnd* getWndFromPoint(int x, int y) {
    PblIterator* it = pblMapIteratorNew(hwndToWnd);
    while (pblIteratorHasNext(it)) {
        struct Wnd** ppWnd = pblMapEntryValue((PblMapEntry*)pblIteratorNext(it));
        struct Wnd* wnd;
        if (ppWnd) {
            wnd = *ppWnd;
            if (x>=wnd->windowRect.left && x<=wnd->windowRect.right && y>=wnd->windowRect.top && y<=wnd->windowRect.bottom && wnd->surface) {
                pblIteratorFree(it);
                return wnd;
            }
        }
    }
    pblIteratorFree(it);
    return NULL;
}

static struct Wnd* getFirstVisibleWnd() {
    PblIterator* it = pblMapIteratorNew(hwndToWnd);
    while (pblIteratorHasNext(it)) {
        struct Wnd** ppWnd = pblMapEntryValue((PblMapEntry*)pblIteratorNext(it));
        struct Wnd* wnd;
        if (ppWnd) {
            wnd = *ppWnd;
#ifdef SDL2
            if (wnd->sdlTexture || wnd->openGlContext) {
#else
            if (wnd->sdlSurface) {
#endif
                pblIteratorFree(it);
                return wnd;
            }
        }
    }
    pblIteratorFree(it);
    return NULL;
}

#ifdef SDL2
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_GLContext *sdlCurrentContext;
#ifdef BOXEDWINE_VM
SDL_GLContext *sdlInitContext;
#endif
int contextCount;

static void destroySDL2(struct KThread* thread) {
    PblIterator* it = pblMapIteratorNew(hwndToWnd);
    while (pblIteratorHasNext(it)) {
        struct Wnd** ppWnd = pblMapEntryValue((PblMapEntry*)pblIteratorNext(it));
        struct Wnd* wnd;
        if (ppWnd) {
            wnd = *ppWnd;
            if (wnd->sdlTexture) {
                SDL_DestroyTexture(wnd->sdlTexture);
                wnd->sdlTexture = NULL;
            }
        }
    }
    pblIteratorFree(it);

#ifdef SDL2
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = 0;
    }

    if (thread->glContext) {
        SDL_GL_DeleteContext(thread->glContext);
        thread->glContext = 0;
    }
#ifdef BOXEDWINE_VM
    if (sdlInitContext) {
        SDL_GL_DeleteContext(sdlInitContext);
        sdlInitContext = 0;
    }
#endif
#endif
    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = 0;
    }    
    contextCount = 0;
}
#else
SDL_Surface* surface;
#endif

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
void loadExtensions();
#endif

#ifdef BOXEDWINE_VM
static void mainThreadDeleteContext(struct SdlCallback* callback) {
    sdlDeleteContext(callback->thread, callback->iArg1);
}
#endif

void sdlDeleteContext(struct KThread* thread, U32 context) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        callback->func = mainThreadDeleteContext;
        callback->iArg1 = context;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);
        return;
    }
#endif
#ifdef SDL2
    struct KThread* contextThread = processGetThreadById(thread, thread->process, context);
    if (contextThread && contextThread->glContext) {
        SDL_GL_DeleteContext(contextThread->glContext);
        contextThread->glContext = NULL;
        contextCount--;
        if (contextCount==0) {
            displayChanged(thread); 
        }
    }
#endif
}

void sdlUpdateContextForThread(struct KThread* thread) {
#ifdef SDL2
    if (thread->currentContext && thread->currentContext!=sdlCurrentContext) {
        SDL_GL_MakeCurrent(sdlWindow, thread->glContext);
        sdlCurrentContext = thread->glContext;        
    }
#endif
}

#ifdef BOXEDWINE_VM
static void mainMakeCurrent(struct SdlCallback* callback) {
    callback->result = sdlMakeCurrent(callback->thread, callback->iArg1);
}
#endif

U32 sdlMakeCurrent(struct KThread* thread, U32 arg) {
#ifdef SDL2
    if (arg == thread->id) {
        if (SDL_GL_MakeCurrent(sdlWindow, thread->glContext)==0) {
            thread->currentContext = thread->glContext;
            sdlCurrentContext = thread->glContext;
#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
            loadExtensions();
#endif
            return 1;
        }
    } else if (arg == 0) {
        SDL_GL_MakeCurrent(sdlWindow, 0);
        thread->currentContext = 0;
        sdlCurrentContext = NULL;
        return 1;
    } else {
        kpanic("Tried to make an OpenGL context current for a different thread?");
    }
    return 0;
#else
    return 1;
#endif
}

#ifdef BOXEDWINE_VM
static void mainShareLists(struct SdlCallback* callback) {
    callback->result = sdlShareLists(callback->thread, callback->iArg1, callback->iArg2);
}
#endif

U32 sdlShareLists(struct KThread* thread, U32 srcContext, U32 destContext) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        U32 result;

        callback->func = mainShareLists;
        callback->iArg1 = srcContext;
        callback->iArg2 = destContext;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        result = callback->result;
        freeSdlCallback(callback);        
        return result;
    }
#endif
    {
#ifdef SDL2
        struct KThread* srcThread = processGetThreadById(thread, thread->process, srcContext);
        struct KThread* destThread = processGetThreadById(thread, thread->process, destContext);
        if (srcThread && destThread && srcThread->glContext && destThread->glContext) {
            if (destThread->currentContext) {
                kpanic("Wasn't expecting the OpenGL context that will share a list to already be current");
            }
            if (srcThread->currentContext != sdlCurrentContext) {
                kpanic("Something went wrong with sdlShareLists");
            }
            //SDL_GL_DeleteContext(destThread->glContext);
            //SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1); 
            //destThread->glContext = SDL_GL_CreateContext(sdlWindow);
            //SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0); 
            return 1;
        }
#endif
        return 0;
    }
}

#ifdef BOXEDWINE_VM
static void mainCreateOpenglWindow(struct SdlCallback* callback) {
    callback->result = sdlCreateOpenglWindow(callback->thread, callback->pArg1, callback->iArg1, callback->iArg2, callback->iArg3, callback->iArg4);
}
#endif

U32 sdlCreateOpenglWindow(struct KThread* thread, struct Wnd* wnd, int major, int minor, int profile, int flags) {
#ifdef BOXEDWINE_VM
    if (SDL_ThreadID()!=sdlMainThreadId) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        U32 result;

        callback->func = mainCreateOpenglWindow;
        callback->pArg1 = wnd;
        callback->iArg1 = major;
        callback->iArg2 = minor;
        callback->iArg3 = profile;
        callback->iArg4 = flags;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        result = callback->result;
        freeSdlCallback(callback);        
        return result;
    }
#endif
#ifdef SDL2
    if (wnd->openGlContext) {
        if (thread->glContext) {
            kpanic("Not sure what to do, trying to create another OpenGL context on the same thread");
        }
        thread->glContext = SDL_GL_CreateContext(sdlWindow);
        contextCount++;
        return thread->id;
    }
    destroySDL2(thread);

    firstWindowCreated = 1;
    SDL_GL_ResetAttributes();
#endif
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, wnd->pixelFormat->cRedBits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, wnd->pixelFormat->cGreenBits);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, wnd->pixelFormat->cBlueBits);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, wnd->pixelFormat->cAlphaBits);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, wnd->pixelFormat->cDepthBits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, wnd->pixelFormat->cStencilBits);

    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, wnd->pixelFormat->cAccumRedBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, wnd->pixelFormat->cAccumGreenBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, wnd->pixelFormat->cAccumBlueBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, wnd->pixelFormat->cAccumAlphaBits);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, wnd->pixelFormat->dwFlags & 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, (wnd->pixelFormat->dwFlags & 0x40)?0:1);
#ifdef SDL2
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1); 
#endif
    firstWindowCreated = 1;
#ifdef SDL2
    sdlWindow = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, wnd->windowRect.right-wnd->windowRect.left, wnd->windowRect.bottom-wnd->windowRect.top, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
    if (!sdlWindow) {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        displayChanged(thread);
        return 0;
    }

    thread->glContext = SDL_GL_CreateContext(sdlWindow);
#if BOXEDWINE_VM
    sdlInitContext = SDL_GL_CreateContext(sdlWindow);
#endif
    if (!thread->glContext) {
        fprintf(stderr, "Couldn't create context: %s\n", SDL_GetError());
        displayChanged(thread);
        return 0;
    }
    contextCount++;
    if (!wnd->openGlContext)
        wnd->openGlContext = thread->glContext;       
    return thread->id;
#else
    surface = NULL;
    SDL_SetVideoMode(wnd->windowRect.right-wnd->windowRect.left, wnd->windowRect.bottom-wnd->windowRect.top, wnd->pixelFormat->cDepthBits, SDL_OPENGL);        
    return 0x200;
#endif
}

#ifdef BOXEDWINE_VM
static void mainScreenResized(struct SdlCallback* callback) {
    sdlScreenResized(callback->thread);
}
#endif

void sdlScreenResized(struct KThread* thread) {
#ifdef BOXEDWINE_VM
    if (SDL_ThreadID()!=sdlMainThreadId) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        callback->func = mainScreenResized;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return;
    }
#endif
#ifdef SDL2
    if (thread->glContext)
        SDL_SetWindowSize(sdlWindow, screenCx, screenCy);
    else
        displayChanged(thread);
#else
    displayChanged(thread);
#endif
}

static void displayChanged(struct KThread* thread) {
#ifndef SDL2
    U32 flags;
#endif
    firstWindowCreated = 1;
#ifdef SDL2
    destroySDL2(thread);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, sdlScaleQuality);

    sdlWindow = SDL_CreateWindow("WineBOX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenCx*sdlScale/100, screenCy*sdlScale/100, SDL_WINDOW_SHOWN|(sdlFullScreen?SDL_WINDOW_FULLSCREEN:0));
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);	
#else
    flags = SDL_HWSURFACE;
    if (surface && SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
    printf("Switching to %dx%d@%d\n", screenCx, screenCy, bits_per_pixel);
    surface = SDL_SetVideoMode(screenCx, screenCy, 32, flags);
#endif
}

#ifdef BOXEDWINE_VM
static void mainSwapBuffers(struct SdlCallback* callback) {
    sdlSwapBuffers(callback->thread);
}
#endif

void sdlSwapBuffers(struct KThread* thread) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        callback->func = mainSwapBuffers;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return;
    }
#endif
#ifdef SDL2
    SDL_GL_SwapWindow(sdlWindow);
#else
    SDL_GL_SwapBuffers();
#endif
}

#ifdef SDL2
S8 b[1024*1024*4];
#endif

#ifdef BOXEDWINE_VM
static void mainWndBlt(struct SdlCallback* callback) {
    wndBlt(callback->thread, callback->iArg1, callback->iArg2, callback->iArg3, callback->iArg4, callback->iArg5, callback->iArg6, callback->iArg7);
}
#endif

#ifdef BOXEDWINE_VM
static SDL_mutex* wndBltMutex;
#endif

void wndBlt(struct KThread* thread, U32 hwnd, U32 bits, S32 xOrg, S32 yOrg, U32 width, U32 height, U32 rect) {
#ifdef SDL2
    if (!sdlRenderer) {
        return;
    }
#endif
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        callback->func = mainWndBlt;
        callback->iArg1 = hwnd;
        callback->iArg2 = bits;
        callback->iArg3 = xOrg;
        callback->iArg4 = yOrg;
        callback->iArg5 = width;
        callback->iArg6 = height;
        callback->iArg7 = rect;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return;
    }
    if (!wndBltMutex) {
        wndBltMutex = SDL_CreateMutex();
    }
    BOXEDWINE_LOCK(thread, wndBltMutex);
#endif
    {
        struct Wnd* wnd = getWnd(hwnd);
        struct wRECT r;
        U32 y;    
        int pitch = (width*((bits_per_pixel+7)/8)+3) & ~3;
        static int i;

        readRect(thread, rect, &r);

        if (!firstWindowCreated) {
            displayChanged(thread);
        }
    #ifndef SDL2
        if (!surface)
            return;
    #endif
        if (wnd)
    #ifdef SDL2
        {
            SDL_Texture *sdlTexture = NULL;
        
            if (wnd->sdlTexture) {
                sdlTexture = wnd->sdlTexture;
                if (sdlTexture && (wnd->sdlTextureHeight != height || wnd->sdlTextureWidth != width)) {
                    SDL_DestroyTexture(wnd->sdlTexture);
                    wnd->sdlTexture = NULL;
                    sdlTexture = NULL;
                }
            }
            if (!sdlTexture) {
                U32 format = SDL_PIXELFORMAT_ARGB8888;
                if (bits_per_pixel == 16) {
                    format = SDL_PIXELFORMAT_RGB565;
                } else if (bits_per_pixel == 15) {
                    format = SDL_PIXELFORMAT_RGB555;
                }
                sdlTexture = SDL_CreateTexture(sdlRenderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);
                wnd->sdlTexture = sdlTexture;
                wnd->sdlTextureHeight = height;
                wnd->sdlTextureWidth = width;
            }

            for (y = 0; y < height; y++) {
                memcopyToNative(thread, bits+(height-y-1)*pitch, b+y*pitch, pitch);
            } 
            if (bits_per_pixel!=32) {
                // SDL_ConvertPixels(width, height, )
            }
            SDL_UpdateTexture(sdlTexture, NULL, b, pitch);
        }
    #else		
        {     
            SDL_Surface* s = NULL;
            if (wnd->surface) {
                s = wnd->sdlSurface;
                if (s && (s->w!=width || s->h!=height || s->format->BitsPerPixel!=bits_per_pixel)) {
                    SDL_FreeSurface(s);
                    wnd->sdlSurface = NULL;
                    s = NULL;
                }
            }
            if (!s) {
                U32 rMask = 0x00FF0000;
                U32 gMask = 0x0000FF00;
                U32 bMask = 0x000000FF;

                if (bits_per_pixel==15) {
                    rMask = 0x7C00;
                    gMask = 0x03E0;
                    bMask = 0x001F;
                } else if (bits_per_pixel == 16) {
                    rMask = 0xF800;
                    gMask = 0x07E0;
                    bMask = 0x001F;
                }
                s = SDL_CreateRGBSurface(0, width, height, bits_per_pixel, rMask, gMask, bMask, 0);
                wnd->sdlSurface = s;
            }
            if (SDL_MUSTLOCK(s)) {
                SDL_LockSurface(s);
            }
            for (y = 0; y < height; y++) {
                memcopyToNative(thread, bits+(height-y-1)*pitch, (U8*)(s->pixels)+y*s->pitch, pitch);
            }   
            if (SDL_MUSTLOCK(s)) {
                SDL_UnlockSurface(s);
            }      
        }	
    #endif
    }
#ifdef BOXEDWINE_VM
    BOXEDWINE_UNLOCK(thread, wndBltMutex);
#endif
}

#ifdef BOXEDWINE_VM
static void mainDrawAllWindows(struct SdlCallback* callback) {
    sdlDrawAllWindows(callback->thread, callback->iArg1, callback->iArg2);
}
#endif

#ifdef BOXEDWINE_VM
static SDL_mutex* drawAllMutex;
#endif

U32 sdlUpdated;

void sdlDrawAllWindows(struct KThread* thread, U32 hWnd, int count) {    
    int i;
#ifdef SDL2
    if (!sdlRenderer)
        return;
#endif
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        callback->func = mainDrawAllWindows;
        callback->iArg1 = hWnd;
        callback->iArg2 = count;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return;
    }
    if (!drawAllMutex) {
        drawAllMutex = SDL_CreateMutex();
    }
    BOXEDWINE_LOCK(thread, drawAllMutex);
#endif
#ifdef SDL2
    SDL_SetRenderDrawColor(sdlRenderer, 58, 110, 165, 255 );
    SDL_RenderClear(sdlRenderer);    
    for (i=count-1;i>=0;i--) {
        struct Wnd* wnd = getWnd(readd(thread, hWnd+i*4));
        if (wnd && wnd->sdlTextureWidth && wnd->sdlTexture) {
            SDL_Rect dstrect;
            dstrect.x = wnd->windowRect.left*sdlScale/100;
            dstrect.y = wnd->windowRect.top*sdlScale/100;
            dstrect.w = wnd->sdlTextureWidth*sdlScale/100;
            dstrect.h = wnd->sdlTextureHeight*sdlScale/100;
            SDL_RenderCopy(sdlRenderer,  wnd->sdlTexture, NULL, &dstrect);	
        }        	
    } 
    SDL_RenderPresent(sdlRenderer);
    sdlUpdated=1;
#else
    if (surface) {
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 58, 110, 165));
        for (i=count-1;i>=0;i--) {
            struct Wnd* wnd = getWnd(readd(thread, hWnd+i*4));
            if (wnd && wnd->sdlSurface) {
                SDL_Rect dstrect;
                dstrect.x = wnd->windowRect.left;
                dstrect.y = wnd->windowRect.top;
                dstrect.w = ((SDL_Surface*)(wnd->sdlSurface))->w;
                dstrect.h = ((SDL_Surface*)(wnd->sdlSurface))->h;
                if (bits_per_pixel==8)
                    SDL_SetPalette(wnd->sdlSurface, SDL_LOGPAL, sdlPalette, 0, 256);
                SDL_BlitSurface(wnd->sdlSurface, NULL, surface, &dstrect);
            }        	
        }    
        SDL_UpdateRect(surface, 0, 0, 0, 0);
    }
#endif
    BOXEDWINE_UNLOCK(thread, drawAllMutex);
}

struct Wnd* wndCreate(struct KThread* thread, U32 processId, U32 hwnd, U32 windowRect, U32 clientRect) {
    struct Wnd* wnd = kalloc(sizeof(struct Wnd), KALLOC_WND);
    readRect(thread, windowRect, &wnd->windowRect);
    readRect(thread, clientRect, &wnd->clientRect);
    wnd->processId = processId;
    wnd->hwnd = hwnd;
    pblMapAdd(hwndToWnd, &hwnd, sizeof(U32), &wnd, sizeof(void*));
    return wnd;
}

void wndDestroy(U32 hwnd) {
    pblMapRemove(hwndToWnd, &hwnd, sizeof(U32), NULL);
}

void writeRect(struct KThread* thread, U32 address, struct wRECT* rect) {
    if (address) {
        writed(thread, address, rect->left);
        writed(thread, address+4, rect->top);
        writed(thread, address+8, rect->right);
        writed(thread, address+12, rect->bottom);
    }
}

void readRect(struct KThread* thread, U32 address, struct wRECT* rect) {
    if (address) {
        rect->left = readd(thread, address);
        rect->top = readd(thread, address+4);
        rect->right = readd(thread, address+8);
        rect->bottom = readd(thread, address+12);
    }
}

#ifdef BOXEDWINE_VM
static void mainSdlShowWnd(struct SdlCallback* callback) {
    sdlShowWnd(callback->thread, callback->pArg1, callback->iArg1);
}
#endif

void sdlShowWnd(struct KThread* thread, struct Wnd* wnd, U32 bShow) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        callback->func = mainSdlShowWnd;
        callback->pArg1 = wnd;
        callback->iArg1 = bShow;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return;
    }
#endif
#ifdef SDL2
    if (!bShow && wnd && wnd->sdlTexture) {
        SDL_DestroyTexture(wnd->sdlTexture);
        wnd->sdlTexture = NULL;
    }
#else
    if (!bShow && wnd && wnd->sdlSurface) {
        SDL_FreeSurface(wnd->sdlSurface);
        wnd->sdlSurface = NULL;
    }
#endif
}

void setWndText(struct Wnd* wnd, const char* text) {
    int len = (int)strlen(text);
    char* buf;

    if (wnd->text)
        kfree((void*)wnd->text, KALLOC_SDLWNDTEXT);
    buf = kalloc(len + 1, KALLOC_SDLWNDTEXT);
    strcpy(buf, text);
    wnd->text = buf;
}

void updateScreen() {
    // this mechanism probably won't work well if multiple threads are updating the screen, there could be flickering
}

#ifdef BOXEDWINE_VM
static void mainGetGammaRamp(struct SdlCallback* callback) {
    callback->result = sdlGetGammaRamp(callback->thread, callback->iArg1);
}
#endif

U32 sdlGetGammaRamp(struct KThread* thread, U32 ramp) {
    U16 r[256];
    U16 g[256];
    U16 b[256];

#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        U32 result;

        callback->func = mainGetGammaRamp;
        callback->iArg1 = ramp;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        result = callback->result;
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return result;
    }
#endif
#ifdef SDL2
    if (SDL_GetWindowGammaRamp(sdlWindow, r, g, b)==0) {
#else
    if (SDL_GetGammaRamp(r, g, b)==0) {
#endif
        int i;
        for (i=0;i<256;i++) {
            writew(thread, ramp+i*2, r[i]);
            writew(thread, ramp+i*2+512, g[i]);
            writew(thread, ramp+i*2+124, b[i]);
        }
        return 1;
    }
    return 0;
}

void sdlGetPalette(struct KThread* thread, U32 start, U32 count, U32 entries) {
    U32 i;

    for (i=0;i<count;i++) {
        writeb(thread, entries+4*i, sdlPalette[i+start].r);
        writeb(thread, entries+4*i+1, sdlPalette[i+start].g);
        writeb(thread, entries+4*i+2, sdlPalette[i+start].b);
        writeb(thread, entries+4*i+3, 0);
    }
}

#ifdef BOXEDWINE_VM
static void mainGetNearestColor(struct SdlCallback* callback) {
    callback->result = sdlGetNearestColor(callback->thread, callback->iArg1);
}
#endif

U32 sdlGetNearestColor(struct KThread* thread, U32 color) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        U32 result;

        callback->func = mainGetNearestColor;
        callback->iArg1 = color;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        result = callback->result;
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return result;
    }
#endif
#ifdef SDL2
    return SDL_MapRGB(SDL_GetWindowSurface(sdlWindow)->format, color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
#else
    if (surface)
        return SDL_MapRGB(surface->format, color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
    return color;
#endif
}

U32 sdlRealizePalette(struct KThread* thread, U32 start, U32 numberOfEntries, U32 entries) {
    U32 i;
    int result = 0;

    if (numberOfEntries>256)
        numberOfEntries=256;
    for (i=0;i<numberOfEntries;i++) {
        sdlPalette[i+start].r = readb(thread, entries+4*i);
        sdlPalette[i+start].g = readb(thread, entries+4*i+1);
        sdlPalette[i+start].b = readb(thread, entries+4*i+2);
#ifdef SDL2
        sdlPalette[i+start].a = 0;
#else
        sdlPalette[i+start].unused = 0;
#endif
    }    
    return result;
}

void sdlRealizeDefaultPalette() {
    int i;

    for (i=0;i<256;i++) {
        sdlPalette[i] = sdlSystemPalette[i];
    }
}

/*
typedef struct tagMOUSEINPUT
{
    LONG    dx;
    LONG    dy;
    DWORD   mouseData;
    DWORD   dwFlags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} MOUSEINPUT, *PMOUSEINPUT, *LPMOUSEINPUT;

typedef struct tagKEYBDINPUT
{
    WORD    wVk;
    WORD    wScan;
    DWORD   dwFlags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} KEYBDINPUT, *PKEYBDINPUT, *LPKEYBDINPUT;

typedef struct tagHARDWAREINPUT
{
    DWORD   uMsg;
    WORD    wParamL;
    WORD    wParamH;
} HARDWAREINPUT, *PHARDWAREINPUT, *LPHARDWAREINPUT;

#define INPUT_MOUSE     0
#define INPUT_KEYBOARD  1
#define INPUT_HARDWARE  2

typedef struct tagINPUT
{
    DWORD type;
    union
    {
        MOUSEINPUT      mi;
        KEYBDINPUT      ki;
        HARDWAREINPUT   hi;
    } DUMMYUNIONNAME;
} INPUT, *PINPUT, *LPINPUT;
*/

#define MOUSEEVENTF_MOVE            0x0001
#define MOUSEEVENTF_LEFTDOWN        0x0002
#define MOUSEEVENTF_LEFTUP          0x0004
#define MOUSEEVENTF_RIGHTDOWN       0x0008
#define MOUSEEVENTF_RIGHTUP         0x0010
#define MOUSEEVENTF_MIDDLEDOWN      0x0020
#define MOUSEEVENTF_MIDDLEUP        0x0040
#define MOUSEEVENTF_XDOWN           0x0080
#define MOUSEEVENTF_XUP             0x0100
#define MOUSEEVENTF_WHEEL           0x0800
#define MOUSEEVENTF_HWHEEL          0x1000
#define MOUSEEVENTF_MOVE_NOCOALESCE 0x2000
#define MOUSEEVENTF_VIRTUALDESK     0x4000
#define MOUSEEVENTF_ABSOLUTE        0x8000

U32 unixsocket_write_native_nowait(struct Memory* memory, struct KObject* obj, U8* value, int len);

void writeLittleEndian_4(U8* buffer, U32 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
}

void writeLittleEndian_2(U8* buffer, U16 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
}

int sdlMouseMouse(int x, int y) {
    struct Wnd* wnd;

    if (!hwndToWnd)
        return 0;
    wnd = getWndFromPoint(x, y);
    if (!wnd)
        wnd = getFirstVisibleWnd();
    if (wnd) {
        struct KProcess* process = getProcessById(NULL, wnd->processId);        
        if (process) {
            struct KFileDescriptor* fd = getFileDescriptor(process, process->eventQueueFD);
            if (fd) {
                struct Memory* memory = process->memory;
                U8 buffer[28];

                writeLittleEndian_4(buffer, 0); // INPUT_MOUSE
                writeLittleEndian_4(buffer+4, (x*100+sdlScale/2)/sdlScale); // dx
                writeLittleEndian_4(buffer+8, (y*100+sdlScale/2)/sdlScale); // dy
                writeLittleEndian_4(buffer+12, 0); // mouseData
                writeLittleEndian_4(buffer+16,  MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE); // dwFlags
                writeLittleEndian_4(buffer+20, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+24, 0); // dwExtraInfo

                unixsocket_write_native_nowait(memory, fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

int sdlMouseWheel(int amount, int x, int y) {
    struct Wnd* wnd;

    if (!hwndToWnd)
        return 0;
    wnd = getWndFromPoint(x, y);
    if (!wnd)
        wnd = getFirstVisibleWnd();
    if (wnd) {
        struct KProcess* process = getProcessById(NULL, wnd->processId);        
        if (process) {
            struct KFileDescriptor* fd = getFileDescriptor(process, process->eventQueueFD);
            if (fd) {
                struct Memory* memory = process->memory;
                U8 buffer[28];

                writeLittleEndian_4(buffer, 0); // INPUT_MOUSE
                writeLittleEndian_4(buffer+4, (x*100+sdlScale/2)/sdlScale); // dx
                writeLittleEndian_4(buffer+8, (y*100+sdlScale/2)/sdlScale); // dy
                writeLittleEndian_4(buffer+12, amount); // mouseData
                writeLittleEndian_4(buffer+16, MOUSEEVENTF_WHEEL | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE); // dwFlags
                writeLittleEndian_4(buffer+20, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+24, 0); // dwExtraInfo

                unixsocket_write_native_nowait(memory, fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

int sdlMouseButton(U32 down, U32 button, int x, int y) {
    struct Wnd* wnd;

    if (!hwndToWnd)
        return 0;
    wnd = getWndFromPoint(x, y);
    if (!wnd)
        wnd = getFirstVisibleWnd();
    if (wnd) {
        struct KProcess* process = getProcessById(NULL, wnd->processId);        
        if (process) {
            struct KFileDescriptor* fd = getFileDescriptor(process, process->eventQueueFD);
            if (fd) {
                struct Memory* memory = process->memory;
                U32 flags = MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE;
                U8 buffer[28];

                if (down) {
                    switch (button) {
                        case 0: flags |= MOUSEEVENTF_LEFTDOWN; break;
                        case 1: flags |= MOUSEEVENTF_RIGHTDOWN; break;
                        case 2: flags |= MOUSEEVENTF_MIDDLEDOWN; break;
                    }
                } else {
                    switch (button) {
                        case 0: flags |= MOUSEEVENTF_LEFTUP; break;
                        case 1: flags |= MOUSEEVENTF_RIGHTUP; break;
                        case 2: flags |= MOUSEEVENTF_MIDDLEUP; break;
                    }
                }
                writeLittleEndian_4(buffer, 0); // INPUT_MOUSE
                writeLittleEndian_4(buffer+4, (x*100+sdlScale/2)/sdlScale); // dx
                writeLittleEndian_4(buffer+8, (y*100+sdlScale/2)/sdlScale); // dy
                writeLittleEndian_4(buffer+12, 0); // mouseData
                writeLittleEndian_4(buffer+16, flags); // dwFlags
                writeLittleEndian_4(buffer+20, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+24, 0); // dwExtraInfo

                unixsocket_write_native_nowait(memory, fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

static char cursorName[1024];

const char* getCursorName(char* moduleName, char* resourceName, int resource) {
    safe_strcpy(cursorName, moduleName, 1024);
    safe_strcat(cursorName, ":", 1024);
    if (strlen(resourceName))
        safe_strcat(cursorName, resourceName, 1024);
    else {
        char tmp[10];
        SDL_itoa(resource, tmp, 16);
        safe_strcat(cursorName, tmp, 1024);
    }
    return cursorName;
}

#ifdef BOXEDWINE_VM
static void mainSetCursor(struct SdlCallback* callback) {
    callback->result = sdlSetCursor(callback->thread, callback->pArg1, callback->pArg2, callback->iArg1);
}
#endif

U32 sdlSetCursor(struct KThread* thread, char* moduleName, char* resourceName, int resource) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        U32 result;

        callback->func = mainSetCursor;
        callback->pArg1 = moduleName;
        callback->pArg2 = resourceName;
        callback->iArg1 = resource;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        result = callback->result;
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return result;
    }
#endif
    if (cursors) {
        const char* name = getCursorName(moduleName, resourceName, resource);
        SDL_Cursor** cursor = pblMapGet(cursors, (void*)name, strlen(name), 0);
        if (!cursor)
            return 0;
        SDL_SetCursor(*cursor);
        return 1;
    }
    return 0;
}

#ifdef BOXEDWINE_VM
static void mainCreateAndSetCursor(struct SdlCallback* callback) {
    sdlCreateAndSetCursor(callback->thread, callback->pArg1, callback->pArg2, callback->iArg1, callback->pArg3, callback->pArg4, callback->iArg2, callback->iArg3, callback->iArg4, callback->iArg5);
}

#endif

void sdlCreateAndSetCursor(struct KThread* thread, char* moduleName, char* resourceName, int resource, U8* and_bits, U8* xor_bits, int width, int height, int hotX, int hotY) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        callback->func = mainCreateAndSetCursor;
        callback->pArg1 = moduleName;
        callback->pArg2 = resourceName;
        callback->iArg1 = resource;
        callback->pArg3 = and_bits;
        callback->pArg4 = xor_bits;
        callback->iArg2 = width;
        callback->iArg3 = height;
        callback->iArg4 = hotX;
        callback->iArg5 = hotY;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        freeSdlCallback(callback);        
        return;
    }
#endif
    {
        SDL_Cursor* cursor;
        int byteCount = (width+31) / 31 * 4 * height;
        int dst,src,y, x;
        U8 data_bits[64*64/8];
        U8 mask_bits[64*64/8];
        U32 srcPitch = (width+31)/32*4;
        U32 dstPitch = (width+7)/8;

        // AND | XOR | Windows cursor pixel | SDL
        // --------------------------------------
        // 0  |  0  | black                 | transparent
        // 0  |  1  | white                 | White
        // 1  |  0  | transparent           | Inverted color if possible, black if not
        // 1  |  1  | invert                | Black

        // 0 0 -> 1 1
        // 0 1 -> 0 1
        // 1 0 -> 0 0
        // 1 1 -> 1 0
        for (y=0;y<height;y++) {
            dst = dstPitch*y;
            src = srcPitch*y;
            for (x=0;x<(width+7)/8;src++,dst++,x++) {
                int j;

                for (j=0;j<8;j++) {
                    U8 aBit = (and_bits[src] >> j) & 0x1;
                    U8 xBit = (xor_bits[src] >> j) & 0x1;

                    if (aBit && xBit) {
                        xBit = 0;
                    } else if (aBit && !xBit) {
                        aBit = 0;
                    } else if (!aBit && xBit) {
                
                    } else if (!aBit && !xBit) {
                        aBit = 1;
                        xBit = 1;
                    }
                    if (aBit)
                        data_bits[dst] |= (1 << j);
                    else
                        data_bits[dst] &= ~(1 << j);

                    if (xBit)
                        mask_bits[dst] |= (1 << j);
                    else
                        mask_bits[dst] &= (1 << j);
                }
            }
        }

        cursor = SDL_CreateCursor(data_bits, mask_bits, width, height, hotX, hotY);
        if (cursor) {
            const char* name = getCursorName(moduleName, resourceName, resource);
            if (!cursors) {
                cursors = pblMapNewHashMap();
            }
            pblMapPut(cursors, (void*)name, strlen(name), &cursor, sizeof(SDL_Cursor*), NULL);
            SDL_SetCursor(cursor);
        }
    }
}

#define KEYEVENTF_EXTENDEDKEY        0x0001
#define KEYEVENTF_KEYUP              0x0002
#define KEYEVENTF_UNICODE            0x0004
#define KEYEVENTF_SCANCODE           0x0008

#define VK_CANCEL              0x03
#define VK_BACK                0x08
#define VK_TAB                 0x09
#define VK_RETURN              0x0D
#define VK_SHIFT               0x10
#define VK_CONTROL             0x11
#define VK_MENU                0x12
#define VK_PAUSE               0x13
#define VK_CAPITAL             0x14

#define VK_ESCAPE              0x1B

#define VK_SPACE               0x20
#define VK_PRIOR               0x21
#define VK_NEXT                0x22
#define VK_END                 0x23
#define VK_HOME                0x24
#define VK_LEFT                0x25
#define VK_UP                  0x26
#define VK_RIGHT               0x27
#define VK_DOWN                0x28
#define VK_INSERT              0x2D
#define VK_DELETE              0x2E
#define VK_HELP                0x2F

#define VK_MULTIPLY            0x6A
#define VK_ADD                 0x6B
#define VK_DECIMAL             0x6E
#define VK_DIVIDE              0x6F

#define VK_F1                  0x70
#define VK_F2                  0x71
#define VK_F3                  0x72
#define VK_F4                  0x73
#define VK_F5                  0x74
#define VK_F6                  0x75
#define VK_F7                  0x76
#define VK_F8                  0x77
#define VK_F9                  0x78
#define VK_F10                 0x79
#define VK_F11                 0x7A
#define VK_F12                 0x7B
#define VK_F24                 0x87

#define VK_NUMLOCK             0x90
#define VK_SCROLL              0x91

#define VK_LSHIFT              0xA0
#define VK_RSHIFT              0xA1
#define VK_LCONTROL            0xA2
#define VK_RCONTROL            0xA3
#define VK_LMENU               0xA4
#define VK_RMENU               0xA5

#define VK_OEM_1               0xBA
#define VK_OEM_PLUS            0xBB
#define VK_OEM_COMMA           0xBC
#define VK_OEM_MINUS           0xBD
#define VK_OEM_PERIOD          0xBE
#define VK_OEM_2               0xBF
#define VK_OEM_3               0xC0
#define VK_OEM_4               0xDB
#define VK_OEM_5               0xDC
#define VK_OEM_6               0xDD
#define VK_OEM_7               0xDE

#ifdef SDL2
#define SDLK_NUMLOCK SDL_SCANCODE_NUMLOCKCLEAR
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#define SDLK_KP0 SDLK_KP_0
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP2 SDLK_KP_2
#define SDLK_KP3 SDLK_KP_3
#define SDLK_KP4 SDLK_KP_4
#define SDLK_KP5 SDLK_KP_5
#define SDLK_KP6 SDLK_KP_6
#define SDLK_KP7 SDLK_KP_7
#define SDLK_KP8 SDLK_KP_8
#define SDLK_KP9 SDLK_KP_9
#endif

int sdlKey(U32 key, U32 down) {
    struct Wnd* wnd;
    
    if (!hwndToWnd)
        return 0;
    wnd = getFirstVisibleWnd();
    if (wnd) {
        struct KProcess* process = getProcessById(NULL, wnd->processId);        
        if (process) {
            struct KFileDescriptor* fd = getFileDescriptor(process, process->eventQueueFD);
            if (fd) {
                U16 vKey = 0;
                U16 scan = 0;
                struct Memory* memory = process->memory;
                U8 buffer[28];

                U32 flags = 0;
                if (!down) 
                    flags|=KEYEVENTF_KEYUP;

                switch (key) {
                case SDLK_ESCAPE:
                    vKey = VK_ESCAPE;
                    scan = 0x01;
                    break;
                case SDLK_1:
                    vKey = '1';
                    scan = 0x02;
                    break;
                case SDLK_2:
                    vKey = '2';
                    scan = 0x03;
                    break;
                case SDLK_3:
                    vKey = '3';
                    scan = 0x04;
                    break;
                case SDLK_4:
                    vKey = '4';
                    scan = 0x05;
                    break;
                case SDLK_5:
                    vKey = '5';
                    scan = 0x06;
                    break;
                case SDLK_6:
                    vKey = '6';
                    scan = 0x07;
                    break;
                case SDLK_7:
                    vKey = '7';
                    scan = 0x08;
                    break;
                case SDLK_8:
                    vKey = '8';
                    scan = 0x09;
                    break;
                case SDLK_9:
                    vKey = '9';
                    scan = 0x0a;
                    break;
                case SDLK_0:
                    vKey = '0';
                    scan = 0x0b;
                    break;
                case SDLK_MINUS:
                    vKey = VK_OEM_MINUS;
                    scan = 0x0c;
                    break;
                case SDLK_EQUALS:
                    vKey = VK_OEM_PLUS;
                    scan = 0x0d;
                    break;
                case SDLK_BACKSPACE:
                    vKey = VK_BACK;
                    scan = 0x0e;
                    break;
                case SDLK_TAB:
                    vKey = VK_TAB;
                    scan = 0x0f;
                    break;
                case SDLK_q:
                    vKey = 'Q';
                    scan = 0x10;
                    break;
                case SDLK_w:
                    vKey = 'W';
                    scan = 0x11;
                    break;
                case SDLK_e:
                    vKey = 'E';
                    scan = 0x12;
                    break;
                case SDLK_r:
                    vKey = 'R';
                    scan = 0x13;
                    break;
                case SDLK_t:
                    vKey = 'T';
                    scan = 0x14;
                    break;
                case SDLK_y:
                    vKey = 'Y';
                    scan = 0x15;
                    break;
                case SDLK_u:
                    vKey = 'U';
                    scan = 0x16;
                    break;
                case SDLK_i:
                    vKey = 'I';
                    scan = 0x17;
                    break;
                case SDLK_o:
                    vKey = 'O';
                    scan = 0x18;
                    break;
                case SDLK_p:
                    vKey = 'P';
                    scan = 0x19;
                    break;
                case SDLK_LEFTBRACKET:
                    vKey = VK_OEM_4;
                    scan = 0x1a;
                    break;
                case SDLK_RIGHTBRACKET:
                    vKey = VK_OEM_6;
                    scan = 0x1b;
                    break;
                case SDLK_RETURN:
                    vKey = VK_RETURN;
                    scan = 0x1c;
                    break;
                case SDLK_LCTRL:
                    vKey = VK_LCONTROL;
                    scan = 0x1d;
                    break;
                case SDLK_RCTRL:
                    vKey = VK_RCONTROL;
                    scan = 0x11d;
                    break;
                case SDLK_a:
                    vKey = 'A';
                    scan = 0x1e;
                    break;
                case SDLK_s:
                    vKey = 'S';
                    scan = 0x1f;
                    break;
                case SDLK_d:
                    vKey = 'D';
                    scan = 0x20;
                    break;
                case SDLK_f:
                    vKey = 'F';
                    scan = 0x21;
                    break;
                case SDLK_g:
                    vKey = 'G';
                    scan = 0x22;
                    break;
                case SDLK_h:
                    vKey = 'H';
                    scan = 0x23;
                    break;
                case SDLK_j:
                    vKey = 'J';
                    scan = 0x24;
                    break;
                case SDLK_k:
                    vKey = 'K';
                    scan = 0x25;
                    break;
                case SDLK_l:
                    vKey = 'L';
                    scan = 0x26;
                    break;
                case SDLK_SEMICOLON:
                    vKey = VK_OEM_1;
                    scan = 0x27;
                    break;
                case SDLK_QUOTE:
                    vKey = VK_OEM_7;
                    scan = 0x28;
                    break;
                case SDLK_BACKQUOTE:
                    vKey = VK_OEM_3;
                    scan = 0x29;
                    break;
                case SDLK_LSHIFT:
                    vKey = VK_LSHIFT;
                    scan = 0x2a;
                    break;
                case SDLK_RSHIFT:
                    vKey = VK_RSHIFT;
                    scan = 0x36;
                    break;
                case SDLK_BACKSLASH:
                    vKey = VK_OEM_5;
                    scan = 0x2b;
                    break;
                case SDLK_z:
                    vKey = 'Z';
                    scan = 0x2c;
                    break;
                case SDLK_x:
                    vKey = 'X';
                    scan = 0x2d;
                    break;
                case SDLK_c:
                    vKey = 'C';
                    scan = 0x2e;
                    break;
                case SDLK_v:
                    vKey = 'V';
                    scan = 0x2f;
                    break;
                case SDLK_b:
                    vKey = 'B';
                    scan = 0x30;
                    break;
                case SDLK_n:
                    vKey = 'N';
                    scan = 0x31;
                    break;
                case SDLK_m:
                    vKey = 'M';
                    scan = 0x32;
                    break;
                case SDLK_COMMA:
                    vKey = VK_OEM_COMMA;
                    scan = 0x33;
                    break;
                case SDLK_PERIOD:
                    vKey = VK_OEM_PERIOD;
                    scan = 0x34;
                    break;
                case SDLK_SLASH:
                    vKey = VK_OEM_2;
                    scan = 0x35;
                    break;
                case SDLK_LALT:
                    vKey = VK_LMENU;
                    scan = 0x38;
                    break;
                case SDLK_RALT:
                    vKey = VK_RMENU;
                    scan = 0x138;
                    break;
                case SDLK_SPACE:
                    vKey = VK_SPACE;
                    scan = 0x39;
                    break;
                case SDLK_CAPSLOCK:
                    vKey = VK_CAPITAL;
                    scan = 0x3a;
                    break;
                case SDLK_F1:
                    vKey = VK_F1;
                    scan = 0x3b;
                    break;
                case SDLK_F2:
                    vKey = VK_F2;
                    scan = 0x3c;
                    break;
                case SDLK_F3:
                    vKey = VK_F3;
                    scan = 0x3d;
                    break;
                case SDLK_F4:
                    vKey = VK_F4;
                    scan = 0x3e;
                    break;
                case SDLK_F5:
                    vKey = VK_F5;
                    scan = 0x3f;
                    break;
                case SDLK_F6:
                    vKey = VK_F6;
                    scan = 0x40;
                    break;
                case SDLK_F7:
                    vKey = VK_F7;
                    scan = 0x41;
                    break;
                case SDLK_F8:
                    vKey = VK_F8;
                    scan = 0x42;
                    break;
                case SDLK_F9:
                    vKey = VK_F9;
                    scan = 0x43;
                    break;
                case SDLK_F10:
                    vKey = VK_F10;
                    scan = 0x44;
                    break;
                case SDLK_NUMLOCK:
                    vKey = VK_NUMLOCK;
                    break;
                case SDLK_SCROLLOCK:
                    vKey = VK_SCROLL;
                    break;
                case SDLK_F11:
                    vKey = VK_F11;
                    scan = 0x57;
                    break;
                case SDLK_F12:
                    vKey = VK_F12;
                    scan = 0x58;
                    break;
                case SDLK_HOME:
                    vKey = VK_HOME;
                    scan = 0x147;
                    break;
                case SDLK_UP:
                    vKey = VK_UP;
                    scan = 0x148;
                    break;
                case SDLK_PAGEUP:
                    vKey = VK_PRIOR;
                    scan = 0x149;
                    break;
                case SDLK_LEFT:
                    vKey = VK_LEFT;
                    scan = 0x14b;
                    break;
                case SDLK_RIGHT:
                    vKey = VK_RIGHT;
                    scan = 0x14d;
                    break;
                case SDLK_END:
                    vKey = VK_END;
                    scan = 0x14f;
                    break;
                case SDLK_DOWN:
                    vKey = VK_DOWN;
                    scan = 0x150;
                    break;
                case SDLK_PAGEDOWN:
                    vKey = VK_NEXT;
                    scan = 0x151;
                    break;
                case SDLK_INSERT:
                    vKey = VK_INSERT;
                    scan = 0x152;
                    break;
                case SDLK_DELETE:
                    vKey = VK_DELETE;
                    scan = 0x153;
                    break;
                case SDLK_PAUSE:
                    vKey = VK_PAUSE;
                    scan = 0x154; // :TODO: is this right?
                    break;
                case SDLK_KP0:
                    scan = 0x52;
                    break;
                case SDLK_KP1:
                    vKey = VK_END;
                    scan = 0x4F;
                    break;
                case SDLK_KP2:
                    vKey = VK_DOWN;
                    scan = 0x50;
                    break;
                case SDLK_KP3:
                    vKey = VK_NEXT;
                    scan = 0x51;
                    break;
                case SDLK_KP4:
                    vKey = VK_LEFT;
                    scan = 0x4B;
                    break;
                case SDLK_KP5:
                    scan = 0x4C;
                    break;
                case SDLK_KP6:
                    vKey = VK_RIGHT;
                    scan = 0x4D;
                    break;
                case SDLK_KP7:
                    vKey = VK_HOME;
                    scan = 0x47;
                    break;
                case SDLK_KP8:
                    vKey = VK_UP;
                    scan = 0x48;
                    break;
                case SDLK_KP9:
                    vKey = VK_PRIOR;
                    scan = 0x49;
                    break;
                case SDLK_KP_PERIOD:
                    vKey = VK_DECIMAL;
                    scan = 0x53;
                    break;
                case SDLK_KP_DIVIDE:
                    vKey = VK_DIVIDE;
                    scan = 0x135;
                    break;
                case SDLK_KP_MULTIPLY:
                    vKey = VK_MULTIPLY;
                    scan = 0x137;
                    break;
                case SDLK_KP_MINUS:
                    scan = 0x4A;
                    break;
                case SDLK_KP_PLUS:
                    vKey = VK_ADD;
                    scan = 0x4E;
                    break;
                case SDLK_KP_ENTER:
                    vKey = VK_RETURN;
                    scan = 0x11C;
                    break;

                default:
                    kwarn("Unhandled key: %d", key);
                    return 1;
                }
                if (scan & 0x100)               
                    flags |= KEYEVENTF_EXTENDEDKEY;

                writeLittleEndian_4(buffer, 1); // INPUT_KEYBOARD
                writeLittleEndian_2(buffer+4, vKey); // wVk
                writeLittleEndian_2(buffer+6, scan & 0xFF); // wScan
                writeLittleEndian_4(buffer+8, flags); // dwFlags
                writeLittleEndian_4(buffer+12, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+16, 0); // dwExtraInfo
                writeLittleEndian_4(buffer+20, 0); // pad
                writeLittleEndian_4(buffer+24, 0); // pad

                unixsocket_write_native_nowait(memory, fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

U32 sdlToUnicodeEx(struct KThread* thread, U32 virtKey, U32 scanCode, U32 lpKeyState, U32 bufW, U32 bufW_size, U32 flags, U32 hkl) {
    U32 ret = 0;
    U8 c = 0;
    U32 shift = readb(thread, lpKeyState+VK_SHIFT) & 0x80;

    if (!virtKey)
        goto done;

    /* UCKeyTranslate, below, terminates a dead-key sequence if passed a
       modifier key press.  We want it to effectively ignore modifier key
       presses.  I think that one isn't supposed to call it at all for modifier
       events (e.g. NSFlagsChanged or kEventRawKeyModifiersChanged), since they
       are different event types than key up/down events. */
    switch (virtKey)
    {
        case VK_SHIFT:
        case VK_CONTROL:
        case VK_MENU:
        case VK_CAPITAL:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_LMENU:
        case VK_RMENU:
            goto done;
    }

    /* There are a number of key combinations for which Windows does not
       produce characters, but Mac keyboard layouts may.  Eat them.  Do this
       here to avoid the expense of UCKeyTranslate() but also because these
       keys shouldn't terminate dead key sequences. */
    if ((VK_PRIOR <= virtKey && virtKey <= VK_HELP) || (VK_F1 <= virtKey && virtKey <= VK_F24))
        goto done;

    /* Shift + <non-digit keypad keys>. */
    if (shift && VK_MULTIPLY <= virtKey && virtKey <= VK_DIVIDE)
        goto done;

    if (readb(thread, lpKeyState+VK_CONTROL) & 0x80)
    {
        /* Control-Tab, with or without other modifiers. */
        if (virtKey == VK_TAB)
            goto done;

        /* Control-Shift-<key>, Control-Alt-<key>, and Control-Alt-Shift-<key>
           for these keys. */
        if (shift || (readb(thread, lpKeyState+VK_MENU)))
        {
            switch (virtKey)
            {
                case VK_CANCEL:
                case VK_BACK:
                case VK_ESCAPE:
                case VK_SPACE:
                case VK_RETURN:
                    goto done;
            }
        }
    }

    if (shift) {
        if (virtKey>='A' && virtKey<='Z') {
            c = virtKey;
        } else {
            switch (virtKey) {
            case '1': c = '!'; break;
            case '2': c = '@'; break;
            case '3': c = '#'; break;
            case '4': c = '$'; break;
            case '5': c = '%'; break;
            case '6': c = '^'; break;
            case '7': c = '&'; break;
            case '8': c = '*'; break;
            case '9': c = '('; break;
            case '0': c = ')'; break;
            case VK_OEM_MINUS: c = '_'; break;
            case VK_OEM_PLUS: c = '+'; break;
            case VK_TAB: c = '\t'; break;
            case VK_OEM_4: c = '{'; break;
            case VK_OEM_6: c = '}'; break;
            case VK_OEM_1: c = ':'; break;
            case VK_OEM_7: c = '\"'; break;
            case VK_OEM_3: c = '~'; break;
            case VK_OEM_5: c = '|'; break;
            case VK_OEM_COMMA: c = '<'; break;
            case VK_OEM_PERIOD: c = '>'; break;
            case VK_OEM_2: c = '?'; break;
            case VK_SPACE: c = ' '; break;
            case VK_RETURN: c = 13; break;
            case VK_BACK: c = 8; break;
            case VK_ADD: c = '+'; break;
            default:
                kwarn("Unhandled key: %d", virtKey);
                break;
            }
        }
    } else {
        if (virtKey>='0' && virtKey<='9') {
            c = virtKey;
        } else if (virtKey>='A' && virtKey<='Z') {
            c = virtKey-'A'+'a';
        } else {
            switch (virtKey) {
            case VK_OEM_MINUS: c = '-'; break;
            case VK_OEM_PLUS: c = '='; break;
            case VK_TAB: c = '\t'; break;
            case VK_OEM_4: c = '['; break;
            case VK_OEM_6: c = ']'; break;
            case VK_OEM_1: c = ';'; break;
            case VK_OEM_7: c = '\''; break;
            case VK_OEM_3: c = '`'; break;
            case VK_OEM_5: c = '\\'; break;
            case VK_OEM_COMMA: c = ','; break;
            case VK_OEM_PERIOD: c = '.'; break;
            case VK_OEM_2: c = '/'; break;
            case VK_SPACE: c = ' '; break;
            case VK_RETURN: c = 13; break;
            case VK_BACK: c = 8; break;
            case VK_ADD: c = '+'; break;
            default:
                kwarn("Unhandled key: %d", virtKey);
                break;
            }
        }
    }
    if (c) {
        writew(thread, bufW, c);
        ret=1;
    }
done:
    /* Null-terminate the buffer, if there's room.  MSDN clearly states that the
       caller must not assume this is done, but some programs (e.g. Audiosurf) do. */
    if (1 <= ret && ret < bufW_size)
        writew(thread, bufW+ret*2, 0);

    return ret;
}

#ifdef BOXEDWINE_VM
static void mainGetMouseState(struct SdlCallback* callback) {
    callback->result = sdlGetMouseState(callback->thread, callback->pArg1, callback->pArg2);
}
#endif

unsigned int sdlGetMouseState(struct KThread* thread,int* x, int* y) {
#ifdef BOXEDWINE_VM
    if (0) {
        struct SdlCallback* callback = allocSdlCallback(thread);
        U32 result;

        callback->func = mainGetMouseState;
        callback->pArg1 = x;
        callback->pArg2 = y;
        BOXEDWINE_LOCK(thread, callback->mutex);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_WAIT(thread, callback->cond, callback->mutex);
        BOXEDWINE_UNLOCK(thread, callback->mutex);
        result = callback->result;
        freeSdlCallback(callback);        
        return result;
    }
#endif
    return SDL_GetMouseState(x, y);
}
