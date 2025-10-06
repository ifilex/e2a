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
#include "khashmap.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>

// :TODO: make the number of buckets dynamic

void initHashmap(struct KHashmap* hashMap) {
    hashMap->buckets = (struct KHashmapEntry**)calloc(1, sizeof(struct KHashmapEntry*)*4096);
    hashMap->numberOfBuckets = 4096;
    hashMap->numberOfEntries = 0;
}

void destroyHashmap(struct KHashmap* hashMap) {
    U32 i;

    for (i=0;i<hashMap->numberOfBuckets;i++) {
        struct KHashmapEntry* entry = hashMap->buckets[i];
        while (entry) {
            struct KHashmapEntry* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    hashMap->buckets = 0;
    hashMap->numberOfBuckets = 0;
    hashMap->numberOfEntries = 0;
}

// probably a better one out there, I just used one on wikipedia
// http://en.wikipedia.org/wiki/Jenkins_hash_function
static U32 calculateHash(const char* key) {
    U32 hash, i;
    U32 len = (int)strlen(key);

    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

static U32 getIndexFromHash(struct KHashmap* hashMap, U32 hash) {
    return hash & (hashMap->numberOfBuckets - 1);
}

struct KHashmapEntry* getHashmapEntry(struct KHashmap* hashMap, const char* key) {
    U32 hash = calculateHash(key);
    U32 index = getIndexFromHash(hashMap, hash);
    struct KHashmapEntry* entry = hashMap->buckets[index];
    while (entry) {
        if (!strcmp(key, entry->key)) {
            return entry;
        }
        entry = entry->next;
    }
    return 0;
}

void putHashmapValue(struct KHashmap* hashMap, const char* key, void* value) {
    struct KHashmapEntry* entry = getHashmapEntry(hashMap, key);
    if (entry) {
        entry->value = value;
    } else {
        struct KHashmapEntry* head;
        U32 index;

        entry = (struct KHashmapEntry*)malloc(sizeof(struct KHashmapEntry));
        entry->key = key;
        entry->value = value;
        entry->hash = calculateHash(key);
        entry->next = 0;

        index = getIndexFromHash(hashMap, entry->hash);
        head = hashMap->buckets[index];
        hashMap->buckets[index] = entry;
        entry->next = head;
    }
}

void* getHashmapValue(struct KHashmap* hashMap, const char* key) {
    struct KHashmapEntry* entry = getHashmapEntry(hashMap, key);
    if (entry)
        return entry->value;
    return 0;
}

void removeHashmapKey(struct KHashmap* hashMap, const char* key) {
    U32 hash = calculateHash(key);
    U32 index = getIndexFromHash(hashMap, hash);
    struct KHashmapEntry* entry = hashMap->buckets[index];
    struct KHashmapEntry* prev = 0;

    while (entry) {
        if (!strcmp(key, entry->key)) {
            if (prev) {
                prev->next = entry->next;
            } else {
                hashMap->buckets[index] = entry->next;
            }
            free(entry);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}