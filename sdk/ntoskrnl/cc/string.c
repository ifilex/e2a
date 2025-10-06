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


#include <string.h>
#include "log.h"
#include "winebox.h"

inline void safe_strcpy(char* dest, const char* src, int bufferSize) {
    int len = (int)strlen(src);
    if (len+1>bufferSize) {
        kpanic("safe_strcpy failed to copy %s, buffer is %d bytes", src, bufferSize);
    }
    strcpy(dest, src);
}

inline void safe_strcat(char* dest, const char* src, int bufferSize) {
    int len = (int)(strlen(src)+strlen(dest));
    if (len+1>bufferSize) {
        kpanic("safe_strcat failed to copy %s, buffer is %d bytes", src, bufferSize);
    }
    strcat(dest, src);
}

