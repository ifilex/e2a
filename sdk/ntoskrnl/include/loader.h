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

#ifndef __LOADER_H__
#define __LOADER_H__

#include "memory.h"
#include "platform.h"
#include "fsapi.h"
#include "kprocess.h"

BOOL loadProgram(struct KProcess* process, struct KThread* thread, struct FsOpenNode* openNode, U32* eip);
BOOL inspectNode(struct KProcess* process, const char* currentDirectory, struct FsNode* node, const char** loader, const char** interpreter, const char** interpreterArgs, U32* interpreterArgsCount, struct FsOpenNode** result);
int getMemSizeOfElf(struct FsOpenNode* openNode);
U32 getPELoadAddress(struct FsOpenNode* FsopenNode, U32* section, U32* numberOfSections, U32* sizeOfSection);

#endif
