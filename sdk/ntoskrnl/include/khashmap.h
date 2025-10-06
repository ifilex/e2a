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

#ifndef __KHASHMAP_H__
#define __KHASHMAP_H__

#include "platform.h"

struct KHashmapEntry {
    const char* key;
    void* value;
    struct KHashmapEntry* next;
    U32 hash;
};

struct KHashmap {
    struct KHashmapEntry** buckets;
    U32 numberOfBuckets;
    U32 numberOfEntries;
};

void initHashmap(struct KHashmap* hashMap);
void destroyHashmap(struct KHashmap* hashMap);
void putHashmapValue(struct KHashmap* hashMap, const char* key, void* value);
void* getHashmapValue(struct KHashmap* hashMap, const char* key);
void removeHashmapKey(struct KHashmap* hashMap, const char* key);

#endif