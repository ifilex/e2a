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

#include <stdio.h>
#include <stdarg.h>
#include <SDL.h>

FILE* logFile;

void kpanic(const char* msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
    vfprintf(stderr, msg, argptr);
    if (logFile) {
        vfprintf(logFile, msg, argptr);
    }
    va_end(argptr);
    fprintf(stderr, "\n");
    if (logFile) {
        fprintf(logFile, "\n");
        fflush(logFile);
        fclose(logFile);
    }
    SDL_Delay(5000);
    exit(1);
}

void kwarn(const char* msg, ...) {
#ifdef _DEBUG
    va_list argptr;
    va_start(argptr, msg);
    vfprintf(stderr, msg, argptr);
    if (logFile) {
        vfprintf(logFile, msg, argptr);
    }
    va_end(argptr);
    fprintf(stderr, "\n");
    if (logFile) {
        fprintf(logFile, "\n");
    }
#endif
}

#ifdef BOXEDWINE_VM
static SDL_mutex* logMutex;
#endif

void klog(const char* msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
#ifdef BOXEDWINE_VM
    if (!logMutex) {
        logMutex = SDL_CreateMutex();
    }
    SDL_LockMutex(logMutex);
#endif
    vfprintf(stdout, msg, argptr);
    if (logFile) {
        vfprintf(logFile, msg, argptr);
    }
    va_end(argptr);
    fprintf(stdout, "\n");
    if (logFile) {
        fprintf(logFile, "\n");
    }
#ifdef BOXEDWINE_VM
    SDL_UnlockMutex(logMutex);
#endif
}
