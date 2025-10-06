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
#include "kshm.h"
#include "kerror.h"
#include "kthread.h"
#include "kprocess.h"
#include "log.h"

#include <string.h>

struct SHM {
    void* address;
    U32 len;
    U32 refCount;
    U32 key;
    U32 cpid;
    U32 lpid;
    U64 ctime;
    U64 dtime;
    U64 atime;
    U32 nattch;
    U32 markedForDelete;
    U32 cuid;
    U32 cgid;
};

struct SHM shm[MAX_SHM];

void freeShm(struct KProcess* process, int index) {
    freeMappable(process->memory, shm[index].address);
    memset(&shm[index], 0, sizeof(struct SHM));
}

#define IPC_CREAT  00001000   /* create if key is nonexistent */
#define IPC_EXCL   00002000   /* fail if key exists */

U32 syscall_shmget(struct KThread* thread, U32 key, U32 size, U32 flags) {
    U32 i;
    S32 index = -1;

    if (key==0) { // IPC_PRIVATE
        for (i=0;i<MAX_SHM;i++) {
            if (!shm[i].address) {
                index = i;
                break;
            }
        }
    } else {
        for (i=0;i<MAX_SHM;i++) {
            if (shm[i].key == key) {
                if (flags & IPC_EXCL)
                    return -K_EEXIST;
                if (!(flags & IPC_CREAT))
                    return -K_ENOENT;
                return i+1;
            }
        }
        for (i=0;i<MAX_SHM;i++) {
            if (!shm[i].address) {
                index = i;
                break;
            }
        }
    }
    if (index<0)
        return -K_ENOSPC;
    shm[index].key = key;
    shm[index].cpid = thread->process->id;
    shm[index].cuid = thread->process->effectiveUserId;
    shm[index].cgid = thread->process->effectiveGroupId;
    shm[index].ctime = getSystemTimeAsMicroSeconds();
    shm[index].len = size;
    shm[index].address = allocMappable(thread->process->memory, (size + PAGE_SIZE - 1) /PAGE_SIZE);
    return index+1;
}

#define SHM_RDONLY      010000  /* read-only access */
#define SHM_RND         020000  /* round attach address to SHMLBA boundary */
#define SHM_REMAP       040000  /* take-over region on attach */
#define SHM_EXEC        0100000 /* execution access */

void incrementShmAttach(struct KProcess* process, int shmid) {
    shm[shmid].atime = getSystemTimeAsMicroSeconds();
    shm[shmid].lpid = process->id;
    shm[shmid].nattch++;
}

U32 syscall_shmat(struct KThread* thread, U32 shmid, U32 shmaddr, U32 shmflg, U32 rtnAddr) {	
    U32 result = 0;
    U32 i;
    U32 permissions;
    S32 attachSlot = -1;

    shmid--; // external is 1 based, internal is 0 based
    for (i=0;i<MAX_SHM_ATTACH;i++) {
        if (!thread->process->shms[shmid][i]) {
            attachSlot = i;
            break;
        }
    }
    if (attachSlot<0) {
        kpanic("tried to attach the same shm more than %d times to a process (pid=%d)", MAX_SHM_ATTACH, thread->process->id);
    }
    if (shmaddr && (shmflg & SHM_RND)) {
        shmaddr = shmaddr & ~PAGE_MASK;
    }
    if (shmid >= MAX_SHM || !shm[shmid].address || (shmaddr && (shmaddr & PAGE_MASK))) {
        return -K_EINVAL;
    }
    if (shmflg & SHM_REMAP) {
        kpanic("syscall_shmat SHM_REMAP not implemented");
    }
    if (!shmaddr) {
        shmaddr = ADDRESS_PROCESS_MMAP_START << PAGE_SHIFT;
    }
    if (!findFirstAvailablePage(thread->process->memory, shmaddr >> PAGE_SHIFT, (shm[shmid].len + PAGE_SIZE - 1) / PAGE_SIZE, &result, 0)) {
        return -K_EINVAL;
    }
    if (shmflg & SHM_RDONLY) {
        permissions = PAGE_READ;
    } else {
        permissions = PAGE_READ|PAGE_WRITE;
    }
    mapMappable(thread, result, (shm[shmid].len + PAGE_SIZE - 1) / PAGE_SIZE, shm[shmid].address, permissions);    
    incrementShmAttach(thread->process, shmid);
    thread->process->shms[shmid][attachSlot]=(result << PAGE_SHIFT);
    writed(thread, rtnAddr, thread->process->shms[shmid][attachSlot]);
    return 0;
}

void decrementShmAttach(struct KProcess* process, U32 i) {
    shm[i].dtime = getSystemTimeAsMicroSeconds();
    shm[i].lpid = process->id;
    shm[i].nattch--;
    if (shm[i].nattch == 0 && shm[i].markedForDelete) {
        freeShm(process, i);
    }
}

U32 syscall_shmdt(struct KThread* thread, U32 shmaddr) {
    U32 i;
    U32 j;
    S32 shmid = -1;
    U32 page = shmaddr >> PAGE_SHIFT;

    for (i=0;i<MAX_SHM;i++) {
        for (j=0;j<MAX_SHM_ATTACH;j++) {
            if (thread->process->shms[i][j]==shmaddr) {
                thread->process->shms[i][j] = 0;
                shmid = i;
                break;
            }
        }
    }
    if (shmid<0) {
        return -K_EINVAL;
    }
    unmapMappable(thread, page, (shm[shmid].len + PAGE_SIZE - 1) / PAGE_SIZE);
    decrementShmAttach(thread->process, shmid);
    return 0;
}

#define IPC_RMID 0     /* remove resource */
#define IPC_SET  1     /* set ipc_perm options */
#define IPC_STAT 2     /* get ipc_perm options */
#define IPC_INFO 3     /* see ipcs */

/* super user shmctl commands */
#define SHM_LOCK        11
#define SHM_UNLOCK      12

/* ipcs ctl commands */
#define SHM_STAT        13
#define SHM_INFO        14

#define IPC_64  0x0100  /* New version (support 32-bit UIDs, bigger
                           message sizes, etc. */

U32 syscall_shmctl(struct KThread* thread, U32 shmid, U32 cmd, U32 buf) {	
    shmid--;
    if (shmid >= MAX_SHM || !shm[shmid].address)
        return -K_EINVAL;
    if (!buf)
        return -K_EFAULT;
    if (cmd == (IPC_64 | IPC_STAT)) {
        // ipc_perm
        writed(thread, buf, shm[shmid].key); buf+=4;
        writed(thread, buf, shm[shmid].cuid); buf += 4;
        writed(thread, buf, shm[shmid].cgid); buf += 4;
        writed(thread, buf, shm[shmid].cuid); buf += 4;
        writed(thread, buf, shm[shmid].cgid); buf += 4;
        writew(thread, buf, 0777); buf += 2;
        writew(thread, buf, 0); buf += 2;
        writew(thread, buf, shmid); buf += 2;
        writew(thread, buf, 0); buf += 2;
        writed(thread, buf, 0); buf += 4;
        writed(thread, buf, 0); buf += 4;
        writed(thread, buf, shm[shmid].len); buf += 4;
        writed(thread, buf, (U32)(shm[shmid].atime / 1000000)); buf += 4;
        writed(thread, buf, (U32)(shm[shmid].dtime / 1000000)); buf += 4;
        writed(thread, buf, (U32)(shm[shmid].ctime / 1000000)); buf += 4;
        writed(thread, buf, shm[shmid].cpid); buf += 4;
        writed(thread, buf, shm[shmid].lpid); buf += 4;
        writew(thread, buf, shm[shmid].nattch); buf += 2;
        writew(thread, buf, 0); buf += 2;
        writed(thread, buf, 0);
    }  else if (cmd == (IPC_64 | IPC_RMID)) {
        shm[shmid].markedForDelete = 1;
    } else {
        kpanic("Unknown syscall_shmctl cmd=%X", cmd);
    }
    return 0;
}
