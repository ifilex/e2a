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

#ifndef __KSHM_H__
#define __KSHM_H__

#include "platform.h"

#define MAX_SHM 256
// number of time a particular shm can be attached per process
#define MAX_SHM_ATTACH 4

struct KThread;
struct KProcess;

U32 syscall_shmget(struct KThread* thread, U32 key, U32 size, U32 flags);
U32 syscall_shmat(struct KThread* thread, U32 shmid, U32 shmaddr, U32 shmflg, U32 rtnAddr);
U32 syscall_shmdt(struct KThread* thread, U32 shmaddr);
U32 syscall_shmctl(struct KThread* thread, U32 shmid, U32 cmd, U32 buf);
void decrementShmAttach(struct KProcess* process, U32 i);
void incrementShmAttach(struct KProcess* process, int shmid);
#endif
