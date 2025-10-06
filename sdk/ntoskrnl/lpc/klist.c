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
#include "klist.h"
#include "kalloc.h"

struct KListNode* freeListNodes;

struct KListNode* allocListNode() {
    struct KListNode* result;

    if (freeListNodes) {
        result = freeListNodes;
        freeListNodes = freeListNodes->next;
    } else {
        result = (struct KListNode*)kalloc(sizeof(struct KListNode), KALLOC_KLISTNODE);
    }
    return result;
}

void freeListNode(struct KListNode* node) {
    node->next = freeListNodes;
    freeListNodes = node;
}

struct KListNode* addItemToList(struct KList* list, void* data) {
    struct KListNode* result = allocListNode();

    result->next = 0;
    result->data = data;
    if (list->count == 0) {
        list->first = result;
        result->prev = 0;
    } else {
        list->last->next = result;
        result->prev = list->last;
    }
    list->last = result;
    list->count++;
    return result;
}

void removeItemFromList(struct KList* list, struct KListNode* node) {
    if (!node)
        return;
    if (node->prev)
        node->prev->next = node->next;	
    if (node->next)
        node ->next->prev = node->prev;	
    if (node == list->first)
        list->first = list->first->next;
    if (node == list->last)
        list->last = list->last->prev;
    freeListNode(node);
    list->count--;
}