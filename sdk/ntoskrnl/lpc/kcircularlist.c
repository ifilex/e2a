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
#include "kcircularlist.h"
#include "kalloc.h"

struct KCNode* freeCNodes;

struct KCNode* allocCNode() {
    struct KCNode* result;

    if (freeCNodes) {
        result = freeCNodes;
        freeCNodes = freeCNodes->next;
    } else {
        result = (struct KCNode*)kalloc(sizeof(struct KCNode), KALLOC_KCNODE);
    }
    return result;
}

void freeCNode(struct KCNode* node) {
    node->next = freeCNodes;
    freeCNodes = node;
}

struct KCNode* addItemToCircularList(struct KCircularList* list, void* data) {
    struct KCNode* result = allocCNode();
    result->data = data;
    if (list->count==0) {
        list->node = result;
        result->next = result;
        result->prev = result;
    } else {
        result->prev = list->node;
        result->next = list->node->next;
        list->node->next->prev = result;
        list->node->next = result;
    }
    list->count++;
    return result;
}

void removeItemFromCircularList(struct KCircularList* list, struct KCNode* node) {
    if (list->count == 1) {
        list->node = 0;
    } else {
        if (node == list->node)
            list->node = list->node->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;		
    }
    list->count--;
    freeCNode(node);
}