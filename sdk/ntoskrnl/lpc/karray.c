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
#include "karray.h"
#include "kalloc.h"
#include <stdlib.h>
#include <string.h>

void initArray(struct KArray* karray, int startingIndex) {
    karray->count = 0;
    karray->maxCount = 10;
    karray->objects = (void**)kalloc(sizeof(void*)*karray->maxCount, KALLOC_KARRAYOBJECTS);
    karray->startingIndex = startingIndex;
}

void destroyArray(struct KArray* karray) {
    free(karray->objects);
}

U32 addObjecToArray(struct KArray* karray, void* object) {
    U32 i=0;
    void** pObjects;
    U32 newSize;
    U32 index;

    for (i=0;i<karray->maxCount;i++) {
        if (karray->objects[i]==0) {
            karray->objects[i] = object;
            karray->count++;
            return i+karray->startingIndex;
        }
    }
    index = karray->maxCount;
    newSize=karray->maxCount*2;
    pObjects = (void**)kalloc(sizeof(void*)*newSize, KALLOC_KARRAYOBJECTS);
    memcpy(pObjects, karray->objects, karray->maxCount*sizeof(void*));
    karray->count++;
    kfree(karray->objects, KALLOC_KARRAYOBJECTS);
    karray->objects = pObjects;
    karray->objects[karray->maxCount] = object;
    karray->maxCount = newSize;
    return index+karray->startingIndex;
}

void removeObjectFromArray(struct KArray* karray, U32 index) {
    karray->objects[index-karray->startingIndex] = 0;
    karray->count--;
}

BOOL getNextObjectFromArray(struct KArray* karray, U32* index, void** result) {
    if (*index<karray->maxCount) {
        *result = getObjectFromArray(karray, *index+karray->startingIndex);
        *index=*index+1;
        return TRUE;
    }
    return FALSE;
}

void* getObjectFromArray(struct KArray* karray, U32 index) {
    if (index>=karray->startingIndex && index<karray->startingIndex+karray->maxCount)
        return karray->objects[index-karray->startingIndex];
    return 0;
}