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
#include "ksocket.h"
#include "kprocess.h"
#include "memory.h"
#include "log.h"
#include "kobjectaccess.h"
#include "kerror.h"
#include "fsapi.h"
#include "kstat.h"
#include "kalloc.h"
#include "kscheduler.h"
#include "ksystem.h"
#include "kio.h"
#include "ringbuf.h"
#include "fsapi.h"

#ifdef WIN32
#undef BOOL
#include <winsock.h>
static int winsock_intialized;
#define BOOL unsigned int
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
void closesocket(int socket) { close(socket); }
#endif

#include <string.h>
#include <stdio.h>

#ifdef BOXEDWINE_VM
extern SDL_mutex* pollMutex;
extern SDL_cond* pollCond;
#endif

U32 wakeAndResetWaitingOnReadThreads(struct KSocket* s);
U32 wakeAndResetWaitingOnWriteThreads(struct KSocket* s);

#define MAX_BUFFER_SIZE 4*1024

#define MAX_PENDING_CONNECTIONS 10

struct KSockAddress {
    U16 family;
    char data[256];
};

struct KSocketMsgObject {
    struct KObject* object;
    U32 accessFlags;
};

#define MAX_NUMBER_OF_KSOCKET_MSG_OBJECTS_PER_MSG 10

struct KSocketMsg {
    struct KSocketMsgObject objects[MAX_NUMBER_OF_KSOCKET_MSG_OBJECTS_PER_MSG];
    U8 data[4096];
    U32 objectCount;
    struct KSocketMsg* next;
};

static struct KSocketMsg* freeSocketMsgs;
#ifdef BOXEDWINE_VM
static SDL_mutex* freeSocketMsgsMutex;
#endif

struct KSocketMsg* allocSocketMsg() {
#ifdef BOXEDWINE_VM
    if (!freeSocketMsgsMutex) {
        freeSocketMsgsMutex = SDL_CreateMutex();
    }
#endif
    BOXEDWINE_LOCK(NULL, freeSocketMsgsMutex);
    if (freeSocketMsgs) {
        struct KSocketMsg* result = freeSocketMsgs;
        freeSocketMsgs = freeSocketMsgs->next;
        memset(result, 0, sizeof(struct KSocketMsg));
        BOXEDWINE_UNLOCK(NULL, freeSocketMsgsMutex);
        return result;
    }
    BOXEDWINE_UNLOCK(NULL, freeSocketMsgsMutex);
    return (struct KSocketMsg*)kalloc(sizeof(struct KSocketMsg), KALLOC_KSOCKETMSG);		
}

void freeSocketMsg(struct KSocketMsg* msg) {
    U32 i;
    for (i=0;i<msg->objectCount;i++) {
        closeKObject(msg->objects[i].object);
    }
    BOXEDWINE_LOCK(NULL, freeSocketMsgsMutex);
    msg->next = freeSocketMsgs;	
    freeSocketMsgs = msg;
    BOXEDWINE_UNLOCK(NULL, freeSocketMsgsMutex);
}

#define MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET 5

struct KSocket {
    U32 domain;
    U32 type;
    U32 protocol;
    U32 pid;
    U64 lastModifiedTime;
    BOOL blocking;
    BOOL listening;
    U32 nl_port;
    struct FsNode* node;
    struct KSocket* next;
    struct KSocket* prev;
    struct KSocket* connection;
    BOOL connected; // this will be 0 and connection will be set while connect is blocking
    struct KSockAddress destAddress;
    U32 recvLen;
    U32 sendLen;
    struct KSocket* connecting;
    struct KSocket* pendingConnections[MAX_PENDING_CONNECTIONS];
    U32 pendingConnectionsCount;
    BOOL inClosed;
    BOOL outClosed;
    struct ringbuf* recvBuffer;
    struct KSocketMsg* msgs;	
#ifdef BOXEDWINE_VM
    SDL_mutex* bufferMutex;
    SDL_cond* bufferCond;
    SDL_mutex* connectionMutex;
    SDL_cond* connectionCond;
#endif
    struct KThread* waitingOnConnectThread;
    struct KThread* waitingOnReadThread[MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET];
    struct KThread* waitingOnWriteThread[MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET];    
    S32 nativeSocket;
    U32 flags;
    int error;
};

#ifdef BOXEDWINE_VM
void waitOnSocketRead(struct KSocket* s, struct KThread* thread) {    
    // BOXEDWINE_LOCK(thread, s->bufferMutex);
    // should already be locked
    if (ringbuf_is_empty(s->recvBuffer) && !s->msgs) {
        U32 i;
        BOOL found = FALSE;

        for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
            if (!s->waitingOnReadThread[i]) {
                s->waitingOnReadThread[i] = thread;
                found = TRUE;
                break;
            }
        }
        if (!found) {
            kpanic("%d tried to wait on a socket read, but the socket read queue is full.", thread->id);
        }
        while (ringbuf_is_empty(s->recvBuffer) && !s->msgs && !s->inClosed) {
            BOXEDWINE_WAIT(thread, s->bufferCond, s->bufferMutex);
        }
        s->waitingOnReadThread[i] = NULL;
    }
    //BOXEDWINE_UNLOCK(thread, s->bufferMutex);
}

void waitOnSocketWrite(struct KSocket* s, struct KThread* thread) {
    BOXEDWINE_LOCK(thread, s->connection->bufferMutex);
    if (ringbuf_is_full(s->connection->recvBuffer)) {
        U32 i;
        BOOL found = FALSE;

        for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
            if (!s->waitingOnWriteThread[i]) {
                s->waitingOnWriteThread[i] = thread;
                found = TRUE;
                break;
            }
        }
        if (!found) {
            kpanic("%d tried to wait on a socket write, but the socket write queue is full.", thread->id);
        }
        while (ringbuf_is_full(s->connection->recvBuffer)) {
            BOXEDWINE_WAIT(thread, s->connection->bufferCond, s->connection->bufferMutex);
        }
        s->waitingOnWriteThread[i] = NULL;
    }
    BOXEDWINE_UNLOCK(thread, s->connection->bufferMutex);
}

void waitOnSocketConnect(struct KSocket* s, struct KThread* thread) {    
    BOXEDWINE_LOCK(thread, s->connectionMutex);
    if (!s->connected) {
        s->waitingOnConnectThread = thread;
        BOXEDWINE_WAIT(thread, s->connectionCond, s->connectionMutex);
    }
    BOXEDWINE_UNLOCK(thread, s->connectionMutex);
}

#else
struct KSocket* waitingNativeSockets;
fd_set waitingReadset;
fd_set waitingWriteset;
fd_set waitingErrorset;
int maxSocketId;

U32 getWaitingOnReadThreadCount(struct KSocket* s) {
    U32 i;
    U32 result = 0;

    for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
        if (s->waitingOnReadThread[i])
            result++;
    }
    return result;
}

U32 getWaitingOnWriteThreadCount(struct KSocket* s) {
    U32 i;
    U32 result = 0;

    for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
        if (s->waitingOnWriteThread[i])
            result++;
    }
    return result;
}

void updateWaitingList() {
    struct KSocket* s;

    FD_ZERO(&waitingReadset);
    FD_ZERO(&waitingWriteset);
    FD_ZERO(&waitingErrorset);

    s = waitingNativeSockets;
    maxSocketId = 0;
    while (s) {
        BOOL errorSet = 0;
#ifndef BOXEDWINE_MSVC
        if (s->nativeSocket>=FD_SETSIZE) {
            kpanic("updateWaitingList %s socket is too large to select on", s->nativeSocket);
            break;
        }
#endif
        if (getWaitingOnReadThreadCount(s)) {
            FD_SET(s->nativeSocket, &waitingReadset);
            FD_SET(s->nativeSocket, &waitingErrorset);
            errorSet = 1;
        }
        if (getWaitingOnWriteThreadCount(s)) {
            FD_SET(s->nativeSocket, &waitingWriteset);
            if (!errorSet)
                FD_SET(s->nativeSocket, &waitingErrorset);
        }
        if (s->nativeSocket>maxSocketId)
            maxSocketId = s->nativeSocket;
        s = s->next;
    }
}

void addWaitingNativeSocket(struct KSocket* s) {
    if (s->prev || s->next || s == waitingNativeSockets) {
        return;
    }
    s->next = waitingNativeSockets;
    if (waitingNativeSockets)
        waitingNativeSockets->prev = s;
    waitingNativeSockets = s;
}

void removeWaitingSocket(struct KSocket* s) {
    if (s == waitingNativeSockets) {
        waitingNativeSockets = s->next;
        if (waitingNativeSockets)
            waitingNativeSockets->prev = NULL;
    } else {
        if (s->prev)
            s->prev->next = s->next;
        if (s->next)
            s->next->prev = s->prev;
    }
    s->next = NULL;
    s->prev = NULL;
    wakeAndResetWaitingOnReadThreads(s);
    wakeAndResetWaitingOnWriteThreads(s);
}

U32 checkWaitingNativeSockets(int timeout) {
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout*1000;

    if (waitingNativeSockets) {
        int result;        

        updateWaitingList();       
        
        if (maxSocketId==0)
            return 0;
        result = select(maxSocketId + 1, &waitingReadset, &waitingWriteset, &waitingErrorset, (timeout>=0?&t:0));
        if (result) {
            struct KSocket* s = waitingNativeSockets;
            struct KSocket* parent = 0;

            while (s) {
                U32 found = 0;

                if (FD_ISSET(s->nativeSocket, &waitingReadset) && wakeAndResetWaitingOnReadThreads(s)) {
                    found = 1;
                }
                if (FD_ISSET(s->nativeSocket, &waitingWriteset) && wakeAndResetWaitingOnWriteThreads(s)) {
                    found = 1;
                }
                if (FD_ISSET(s->nativeSocket, &waitingErrorset)) {
                    wakeAndResetWaitingOnWriteThreads(s);
                    wakeAndResetWaitingOnReadThreads(s);
                    found = 1;
                }
                if (found)
                    removeWaitingSocket(s);
                s = s->next;
            }
        }
        return 1;
    }
    return 0;
}

void waitOnSocketRead(struct KSocket* s, struct KThread* thread) {
    U32 i;
    for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
        if (!s->waitingOnReadThread[i]) {
            s->waitingOnReadThread[i] = thread;
            addClearOnWake(thread, &(s->waitingOnReadThread[i]));
            return;
        }
    }
    kpanic("%d tried to wait on a socket read, but the socket read queue is full.", thread->id);
}

void waitOnSocketWrite(struct KSocket* s, struct KThread* thread) {
    U32 i;
    for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
        if (!s->waitingOnWriteThread[i]) {
            s->waitingOnWriteThread[i] = thread;
            addClearOnWake(thread, &(s->waitingOnWriteThread[i]));
            return;
        }
    }
    kpanic("%d tried to wait on a socket write, but the socket write queue is full.", thread->id);
}

void waitOnSocketConnect(struct KSocket* s, struct KThread* thread) {
    if (s->waitingOnConnectThread)
        kpanic("%d tried to wait on a socket connect, but %d is already waiting.", thread->id, s->waitingOnConnectThread->id);
    s->waitingOnConnectThread = thread;
    addClearOnWake(thread, &s->waitingOnConnectThread);
}
#endif

U32 wakeAndResetWaitingOnReadThreads(struct KSocket* s) {
    U32 i;
    U32 result = 0;

    for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
        if (s->waitingOnReadThread[i]) {
            result++;
            wakeThread(NULL, s->waitingOnReadThread[i]);
            s->waitingOnReadThread[i] = 0;
        }
    } 
    return result;
}

U32 wakeAndResetWaitingOnWriteThreads(struct KSocket* s) {
    U32 i;
    U32 result = 0;

    for (i=0;i<MAX_THREADS_THAT_CAN_WAIT_ON_SOCKET;i++) {
        if (s->waitingOnWriteThread[i]) {
            result++;
            wakeThread(NULL, s->waitingOnWriteThread[i]);
            s->waitingOnWriteThread[i] = 0;
        }
    }    
    return result;
}

S32 translateNativeSocketError(int error) {
    S32 result;
#ifdef WIN32
    if (error == WSAENOTCONN)
        result = -K_ENOTCONN;
    else if (error == WSAEWOULDBLOCK)
        result = -K_EWOULDBLOCK;
    else if (error == WSAETIMEDOUT)
        result = -K_ETIMEDOUT;
    else if (error == WSAECONNRESET)
        result = -K_ECONNRESET;
    else if (error == WSAEDESTADDRREQ)
        result = -K_EDESTADDRREQ;
    else if (error == WSAEHOSTUNREACH)
        result = -K_EHOSTUNREACH;
     else if (error == WSAECONNREFUSED)
        result = -K_ECONNREFUSED;
    else if (error == WSAEISCONN)
        result = -K_EISCONN;
    else if (error == WSAEMSGSIZE)
        result = 0;
    else
        result =-K_EIO;

#else 
    if (error == ENOTCONN)
        result = -K_ENOTCONN;
    else if (error == EWOULDBLOCK)
        result = -K_EWOULDBLOCK;
    else if (error == ETIMEDOUT)
        result = -K_ETIMEDOUT;
    else if (error == ECONNRESET)
        result = -K_ECONNRESET;
    else if (error == EDESTADDRREQ)
        result = -K_EDESTADDRREQ;
    else if (error == EHOSTUNREACH)
        result = -K_EHOSTUNREACH;
    else if (error == EISCONN)
        result = -K_EISCONN;
    else if (error == ECONNREFUSED)
        result = -K_ECONNREFUSED;
    else
        result = -K_EIO;
#endif
    return result;
}

S32 handleNativeSocketError(struct KThread* thread, struct KSocket* s, U32 write) {
#ifdef WIN32
    S32 result = translateNativeSocketError(WSAGetLastError());
#else
    S32 result = translateNativeSocketError(errno);
#endif
    #ifndef BOXEDWINE_VM
    if (result == -K_EWOULDBLOCK) {
        if (write) {
            waitOnSocketWrite(s, thread);
        } else {
            waitOnSocketRead(s, thread);
        }
        addWaitingNativeSocket(s);        
        result = -K_WAIT;
    }
#endif
    s->error = -result;
    return result;
}
BOOL unixsocket_isDirectory(struct FsNode* node) {
    return FALSE;
}

BOOL unixsocket_remove(struct FsNode* node) {
    return 0;
}

U64 unixsocket_lastModified(struct FsNode* node) {
    return 0;
}

U64 unixsocket_length(struct FsNode* node) {
    return 0;
}

U32 unixsocket_getMode(struct KProcess* process, struct FsNode* node) {
    return K__S_IREAD | K__S_IWRITE | K_S_IFSOCK;
}

BOOL unixsocket_canRead(struct KProcess* process, struct FsNode* node) {
    return TRUE;
}

BOOL unixsocket_canWrite(struct KProcess* process, struct FsNode* node) {
    return TRUE;
}

struct FsOpenNode* unixsocket_open(struct KProcess* process, struct FsNode* node, U32 flags) {
    kwarn("unixsocket_open was called, this shouldn't happen.  syscall_open should detect we have a kobject already");
    return 0;
}

U32 unixsocket_getType(struct FsNode* node, U32 checkForLink) {
    return 12; // DT_SOCK
}

BOOL unixsocket_exists(struct FsNode* node) {
    return node->kobject!=0;
}

U32 unixsocket_rename(struct FsNode* oldNode, struct FsNode* newNode) {
    return -K_EIO;
}

U32 unixsocket_removeDir(struct FsNode* node) {
    kpanic("unixsocket_removeDir not implemented");
    return 0;
}

U32 unixsocket_makeDir(struct FsNode* node) {
    kpanic("unixsocket_makeDir not implemented");
    return 0;
}

U32 unixsocket_setTimes(struct FsNode* node, U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    klog("unixsocket_setTimes not implemented");
    return 0;
}

struct FsNodeFunc unixSocketNodeType = {unixsocket_isDirectory, unixsocket_exists, unixsocket_rename, unixsocket_remove, unixsocket_lastModified, unixsocket_length, unixsocket_open, unixsocket_canRead, unixsocket_canWrite, unixsocket_getType, unixsocket_getMode, unixsocket_removeDir, unixsocket_makeDir, unixsocket_setTimes};

void freeSocket(struct KSocket* socket);

void unixsocket_onDelete(struct KObject* obj) {
    struct KSocket* s = obj->socket;
    U32 i=0;

    if (s->node) {
        if (s->node->kobject==obj) {
            s->node->kobject = 0;
        }		
    }
    if (s->connection) {
        s->connection->connection = 0;
        s->connection->inClosed = 1;
        s->connection->outClosed = 1;
        wakeAndResetWaitingOnReadThreads(s->connection);
        wakeAndResetWaitingOnWriteThreads(s->connection);
        if (s->connection->waitingOnConnectThread)
            wakeThread(NULL, s->connection->waitingOnConnectThread);

        BOXEDWINE_LOCK(NULL, pollMutex);
        BOXEDWINE_SIGNAL_ALL(pollCond);
        BOXEDWINE_UNLOCK(NULL, pollMutex);
    }    
    if (s->connecting) {		
        BOXEDWINE_LOCK(NULL, s->connecting->connectionMutex);
        for (i=0;i<MAX_PENDING_CONNECTIONS;i++) {
            if (s->connecting->pendingConnections[i]==s) {				
                s->connecting->pendingConnections[i] = 0;
                s->connecting->pendingConnectionsCount--;
            }
        }
        BOXEDWINE_UNLOCK(NULL, s->connecting->connectionMutex);
    }
    if (s->pendingConnectionsCount) {
        BOXEDWINE_LOCK(NULL, s->connectionMutex);
        for (i=0;i<MAX_PENDING_CONNECTIONS;i++) {
            if (s->pendingConnections[i]) {				
                s->pendingConnections[i]->connecting = 0;
                s->pendingConnectionsCount--;
            }
        }
        BOXEDWINE_UNLOCK(NULL, s->connectionMutex);
    }
    if (s->recvBuffer)
        ringbuf_free(&s->recvBuffer);
    while (s->msgs) {
        struct KSocketMsg *next = s->msgs->next;
        freeSocketMsg(s->msgs);
        s->msgs = next;
    }
    wakeAndResetWaitingOnReadThreads(s);
    wakeAndResetWaitingOnWriteThreads(s);
    freeSocket(s);	
    if (s->waitingOnConnectThread)
        wakeThread(NULL, s->waitingOnConnectThread);
#ifdef BOXEDWINE_VM
    if (pollMutex) {
        BOXEDWINE_LOCK(NULL, pollMutex);
        BOXEDWINE_SIGNAL_ALL(pollCond);
        BOXEDWINE_UNLOCK(NULL, pollMutex);
    }
#endif
}

void unixsocket_setBlocking(struct KObject* obj, BOOL blocking) {
    struct KSocket* s = obj->socket;
    s->blocking = blocking;
}

BOOL unixsocket_isBlocking(struct KObject* obj) {
    struct KSocket* s = obj->socket;
    return s->blocking;
}

void unixsocket_setAsync(struct KObject* obj, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kpanic("unixsocket_setAsync not implemented yet");
}

BOOL unixsocket_isAsync(struct KObject* obj, struct KProcess* process) {
    return FALSE;
}

struct KFileLock* unixsocket_getLock(struct KObject* obj, struct KFileLock* lock) {
    kwarn("unixsocket_getLock not implemented yet");
    return 0;
}

U32 unixsocket_setLock(struct KObject* obj, struct KFileLock* lock, BOOL wait, struct KThread* thread) {
    kwarn("unixsocket_setLock not implemented yet");
    return -1;
}

BOOL unixsocket_isOpen(struct KObject* obj) {
    struct KSocket* s = obj->socket;
    return s->listening || s->connection;
}

BOOL unixsocket_isReadReady(struct KThread* thread, struct KObject* obj) {
    struct KSocket* s = obj->socket;
    return s->inClosed || ringbuf_bytes_used(s->recvBuffer) || s->pendingConnectionsCount || s->msgs;
}

BOOL unixsocket_isWriteReady(struct KThread* thread, struct KObject* obj) {
    struct KSocket* s = obj->socket;
    return s->connection!=0;
}

void unixsocket_waitForEvents(struct KObject* obj, struct KThread* thread, U32 events) {
    struct KSocket* s = obj->socket;
    if (events & K_POLLIN) {
        waitOnSocketRead(s, thread);
    }
    if (events & K_POLLOUT) {
        waitOnSocketWrite(s, thread);
    }
    if ((events & ~(K_POLLIN | K_POLLOUT)) || s->listening) {
        waitOnSocketConnect(s, thread);
    }
}

U32 unixsocket_internal_write(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    struct KSocket* s = obj->socket;
    U32 count=0;

    if (s->type == K_SOCK_DGRAM) {
        if (!strcmp(s->destAddress.data, "/dev/log")) {
            char tmp[MAX_FILEPATH_LEN];
            printf("%s\n", getNativeString(thread, buffer, tmp, sizeof(tmp)));
        }
        return len;
    }
    if (s->outClosed || !s->connection)
        return -K_EPIPE;

    if (ringbuf_is_full(s->connection->recvBuffer)) {
        if (!s->blocking) {
            return -K_EWOULDBLOCK;
        }
        waitOnSocketWrite(s, thread);
#ifndef BOXEDWINE_VM
        return -K_WAIT;                
#endif
    }   
    
    while (!ringbuf_is_full(s->connection->recvBuffer) && len) {
        S8 tmp[4096];
        U32 todo = len;

        if (todo>4096)
            todo = 4096;
        if (todo>ringbuf_capacity(s->connection->recvBuffer))
            todo = (U32)ringbuf_capacity(s->connection->recvBuffer);
        memcopyToNative(thread, buffer, tmp, todo);
        ringbuf_memcpy_into(s->connection->recvBuffer, tmp, todo);
        buffer+=todo;
        len-=todo;
        count+=todo;
    }     
    return count;
}

U32 unixsocket_writev(struct KThread* thread, struct KObject* obj, U32 iov, S32 iovcnt) {
    struct KSocket* s = obj->socket;
    U32 len=0;
    S32 i;

    BOXEDWINE_LOCK(thread, s->connection->bufferMutex);
    for (i=0;i<iovcnt;i++) {
        U32 buf = readd(thread, iov + i * 8);
        U32 toWrite = readd(thread, iov + i * 8 + 4);
        S32 result;

        result = unixsocket_internal_write(thread, obj, buf, toWrite);
        if (result<0) {
            if (i>0) {
                kwarn("writev partial fail: TODO file pointer should not change");
            }
            return result;
        }
        len+=result;
    }    
    if (s->connection) {
        wakeAndResetWaitingOnReadThreads(s->connection);
    }
    BOXEDWINE_UNLOCK(thread, s->connection->bufferMutex);

    BOXEDWINE_LOCK(thread, pollMutex);
    BOXEDWINE_SIGNAL_ALL(pollCond);
    BOXEDWINE_UNLOCK(thread, pollMutex);  
    return len;
}

U32 unixsocket_write(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    struct KSocket* s = obj->socket;
    U32 result;

    BOXEDWINE_LOCK(thread, s->connection->bufferMutex);
    result = unixsocket_internal_write(thread, obj, buffer, len);    
    if (s->connection) {
        wakeAndResetWaitingOnReadThreads(s->connection);
    }
    BOXEDWINE_UNLOCK(thread, s->connection->bufferMutex);

    BOXEDWINE_LOCK(thread, pollMutex);
    BOXEDWINE_SIGNAL_ALL(pollCond);
    BOXEDWINE_UNLOCK(thread, pollMutex);    
    return result;
}

U32 unixsocket_write_native_nowait(struct Memory* memory, struct KObject* obj, U8* value, int len) {
    struct KSocket* s = obj->socket;

    if (s->type == K_SOCK_DGRAM) {
        return 1;
    }
    if (s->outClosed || !s->connection)
        return -K_EPIPE;
    BOXEDWINE_LOCK(NULL, s->connection->bufferMutex);
    if (ringbuf_bytes_free(s->connection->recvBuffer)<(U32)len) {
        BOXEDWINE_UNLOCK(NULL, s->connection->bufferMutex);
        return -K_EWOULDBLOCK;
    }
    //printf("SOCKET write len=%d bufferSize=%d pos=%d\n", len, s->connection->recvBufferLen, s->connection->recvBufferWritePos);
    ringbuf_memcpy_into(s->connection->recvBuffer, value, len);
    if (s->connection) {
        wakeAndResetWaitingOnReadThreads(s->connection);
    }
    BOXEDWINE_UNLOCK(NULL, s->connection->bufferMutex);

    BOXEDWINE_LOCK(NULL, pollMutex);
    BOXEDWINE_SIGNAL_ALL(pollCond);
    BOXEDWINE_UNLOCK(NULL, pollMutex);

    return len;
}

U32 unixsocket_read(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    struct KSocket* s = obj->socket;
    U32 count = 0;
    if (!s->inClosed && !s->connection)
        return -K_EPIPE;
    BOXEDWINE_LOCK(thread, s->bufferMutex);
    if (ringbuf_is_empty(s->recvBuffer)) {
        if (s->inClosed) {
            BOXEDWINE_UNLOCK(thread, s->bufferMutex);
            return 0;
        }
        if (!s->blocking) {
            BOXEDWINE_UNLOCK(thread, s->bufferMutex);
            return -K_EWOULDBLOCK;
        }
        waitOnSocketRead(s, thread);
#ifndef BOXEDWINE_VM
        return -K_WAIT;
#endif
    }
#ifdef BOXEDWINE_64BIT_MMU
    count = len;
    if (count>ringbuf_bytes_used(s->recvBuffer))
        count=(U32)ringbuf_bytes_used(s->recvBuffer);
    ringbuf_memcpy_from(getPhysicalAddress(thread, buffer), s->recvBuffer, count);
#else
    while (len && !ringbuf_is_empty(s->recvBuffer)) {
        S8 tmp[4096];
        U32 todo = len;

        if (todo > 4096)
            todo = 4096;
        if (todo > ringbuf_bytes_used(s->recvBuffer))
            todo = (U32)ringbuf_bytes_used(s->recvBuffer);

        ringbuf_memcpy_from(tmp, s->recvBuffer, todo);
        
        memcopyFromNative(thread, buffer, tmp, todo);

        buffer += todo;
        count += todo;
        len -= todo;
    }
#endif        
    if (s->connection)
        wakeAndResetWaitingOnWriteThreads(s->connection);
    BOXEDWINE_UNLOCK(thread, s->bufferMutex);

    BOXEDWINE_LOCK(thread, pollMutex);
    BOXEDWINE_SIGNAL_ALL(pollCond);
    BOXEDWINE_UNLOCK(thread, pollMutex);

    return count;
}

U32 unixsocket_stat(struct KThread* thread, struct KObject* obj, U32 address, BOOL is64) {
    struct KSocket* s = obj->socket;	
    writeStat(NULL, thread, address, is64, 1, (s->node?s->node->id:0), K_S_IFSOCK|K__S_IWRITE|K__S_IREAD, (s->node?s->node->rdev:0), 0, 4096, 0, s->lastModifiedTime, 1);
    return 0;
}

U32 unixsocket_map(struct KThread* thread, struct KObject* obj, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL unixsocket_canMap(struct KObject* obj) {
    return FALSE;
}

S64 unixsocket_seek(struct KObject* obj, S64 pos) {
    return -K_ESPIPE;
}

S64 unixsocket_getPos(struct KObject* obj) {
    return 0;
}

U32 unixsocket_ioctl(struct KThread* thread, struct KObject* obj, U32 request) {
    return -K_ENOTTY;
}

BOOL unixsocket_supportsLocks(struct KObject* obj) {
    return FALSE;
}

S64 unixsocket_klength(struct KObject* obj) {
    return -1;
}

struct KObjectAccess unixsocketAccess = {unixsocket_ioctl, unixsocket_seek, unixsocket_klength, unixsocket_getPos, unixsocket_onDelete, unixsocket_setBlocking, unixsocket_isBlocking, unixsocket_setAsync, unixsocket_isAsync, unixsocket_getLock, unixsocket_setLock, unixsocket_supportsLocks, unixsocket_isOpen, unixsocket_isReadReady, unixsocket_isWriteReady, unixsocket_waitForEvents, unixsocket_write, unixsocket_writev, unixsocket_read, unixsocket_stat, unixsocket_map, unixsocket_canMap};

U32 nativesocket_ioctl(struct KThread* thread, struct KObject* obj, U32 request) {
    struct KSocket* s = obj->socket;

    if (request == 0x541b) {
        int value=0;
#ifdef WIN32
        U32 result = ioctlsocket(s->nativeSocket, FIONREAD, &value);
#else        
        U32 result = ioctl(s->nativeSocket, FIONREAD, &value);        
#endif
        if (result!=0)
            return handleNativeSocketError(thread, s, 1);
        return value;
    }
    return -K_ENOTTY;
}

S64 nativesocket_seek(struct KObject* obj, S64 pos) {
    return -K_ESPIPE;
}

S64 nativesocket_klength(struct KObject* obj) {
    return -1;
}

S64 nativesocket_getPos(struct KObject* obj) {
    return 0;
}

void nativesocket_onDelete(struct KObject* obj) {
    struct KSocket* s = obj->socket;
    closesocket(s->nativeSocket);
    s->nativeSocket = 0;
#ifndef BOXEDWINE_VM
    removeWaitingSocket(s);
#endif
}

void setNativeBlocking(int nativeSocket, BOOL blocking) {
#ifdef WIN32
    u_long mode = blocking?0:1;
    ioctlsocket(nativeSocket, FIONBIO, &mode);           
#else
    if (blocking)
        fcntl(nativeSocket, F_SETFL, fcntl(nativeSocket, F_GETFL, 0) & ~O_NONBLOCK);
    else
        fcntl(nativeSocket, F_SETFL, fcntl(nativeSocket, F_GETFL, 0) | O_NONBLOCK);
#endif
}

void nativesocket_setBlocking(struct KObject* obj, BOOL blocking) {
    struct KSocket* s = obj->socket;
    s->blocking = blocking;
    setNativeBlocking(s->nativeSocket, blocking);
}

BOOL nativesocket_isBlocking(struct KObject* obj) {
    struct KSocket* s = obj->socket;
    return s->blocking;
}

void nativesocket_setAsync(struct KObject* obj, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kpanic("nativesocket_setAsync not implemented yet");
}

BOOL nativesocket_isAsync(struct KObject* obj, struct KProcess* process) {
    return FALSE;
}

struct KFileLock* nativesocket_getLock(struct KObject* obj, struct KFileLock* lock) {
    kwarn("nativesocket_getLock not implemented yet");
    return 0;
}

U32 nativesocket_setLock(struct KObject* obj, struct KFileLock* lock, BOOL wait, struct KThread* thread) {
    kwarn("nativesocket_setLock not implemented yet");
    return -1;
}

BOOL nativesocket_supportsLocks(struct KObject* obj) {
    return FALSE;
}

BOOL nativesocket_isOpen(struct KObject* obj) {
    struct KSocket* s = obj->socket;
    return s->listening || s->connected;
}

BOOL nativesocket_isReadReady(struct KThread* thread, struct KObject* obj) {
    struct KSocket* s = obj->socket;
    fd_set          sready;
    struct timeval  nowait;

    FD_ZERO(&sready);
    FD_SET(s->nativeSocket, &sready);
    memset((char *)&nowait,0,sizeof(nowait));

    select(s->nativeSocket+1,&sready,NULL,NULL,&nowait);
    return FD_ISSET(s->nativeSocket,&sready);
}

BOOL nativesocket_isWriteReady(struct KThread* thread, struct KObject* obj) {
    struct KSocket* s = obj->socket;
    fd_set          sready;
    struct timeval  nowait;

    FD_ZERO(&sready);
    FD_SET(s->nativeSocket, &sready);
    memset((char *)&nowait,0,sizeof(nowait));

    select(s->nativeSocket+1,NULL,&sready,NULL,&nowait);
    return FD_ISSET(s->nativeSocket,&sready);
}

void nativesocket_waitForEvents(struct KObject* obj, struct KThread* thread, U32 events) {
    struct KSocket* s = obj->socket;
#ifndef BOXEDWINE_VM
    if (events & K_POLLIN) {
        waitOnSocketRead(s, thread);
    }
    if (events & K_POLLOUT) {
        waitOnSocketWrite(s, thread);
    }
    addWaitingNativeSocket(s);
#endif
}

U32 nativesocket_write(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    struct KSocket* s = obj->socket;
    U32 done = 0;
    S32 result = 0;
    char tmp[PAGE_SIZE];

    while (len>0) {
        U32 todo = len;
        if (todo>sizeof(tmp))
            todo = sizeof(tmp);
        memcopyToNative(thread, buffer, tmp, todo);
        result = send(s->nativeSocket, tmp, todo, s->flags);
        if (result>0) {
            done+=result;
            len-=result;
            buffer+=result;
        } else {
            break;
        }
    }
    if (result>=0 || done) {            
        s->error = 0;
        return done;
    }
    return handleNativeSocketError(thread, s, 1);
}

U32 nativesocket_writev(struct KThread* thread, struct KObject* obj, U32 iov, S32 iovcnt) {
    struct KSocket* s = obj->socket;
    U32 len=0;
    S32 i;

    for (i=0;i<iovcnt;i++) {
        U32 buf = readd(thread, iov + i * 8);
        U32 toWrite = readd(thread, iov + i * 8 + 4);
        S32 result;

        result = obj->access->write(thread, obj, buf, toWrite);
        if (result<0) {
            if (i>0) {
                kwarn("writev partial fail: TODO file pointer should not change");
            }
            return result;
        }
        len+=result;
    }
    return len;
}

U32 nativesocket_read(struct KThread* thread, struct KObject* obj, U32 buffer, U32 len) {
    struct KSocket* s = obj->socket;	
    S32 result = 0;
    U32 done = 0;
    char tmp[PAGE_SIZE];

    while (len>0) {      
        U32 todo = len;
        if (todo>sizeof(tmp))
            todo = sizeof(tmp);
        result = recv(s->nativeSocket, tmp, todo, s->flags);
        if (result>0) {
            memcopyFromNative(thread, buffer, tmp, result);
            done+=result;
            len-=result;
            buffer+=result;
        } else {
            break;
        }
    }
    if (result>=0 || done) {  
        s->error = 0;
        return done;
    }
    return handleNativeSocketError(thread, s, 0);
}

U32 nativesocket_stat(struct KThread* thread, struct KObject* obj, U32 address, BOOL is64) {
    writeStat(NULL, thread, address, is64, 1, 0, K_S_IFSOCK|K__S_IWRITE|K__S_IREAD, 0, 0, 4096, 0, 0, 1);
    return 0;
}

U32 nativesocket_map(struct KThread* thread, struct KObject* obj, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL nativesocket_canMap(struct KObject* obj) {
    return FALSE;
}

struct KObjectAccess nativesocketAccess = {nativesocket_ioctl, nativesocket_seek, nativesocket_klength, nativesocket_getPos, nativesocket_onDelete, nativesocket_setBlocking, nativesocket_isBlocking, nativesocket_setAsync, nativesocket_isAsync, nativesocket_getLock, nativesocket_setLock, nativesocket_supportsLocks, nativesocket_isOpen, nativesocket_isReadReady, nativesocket_isWriteReady, nativesocket_waitForEvents, nativesocket_write, kaccess_default_writev, nativesocket_read, nativesocket_stat, nativesocket_map, nativesocket_canMap};

//char tmpSocketName[32];

const char* socketAddressName(struct KThread* thread, U32 address, U32 len, char* result, U32 cbResult) {
    U16 family = readw(thread, address);
    if (family == K_AF_UNIX) {
        return getNativeString(thread, address + 2, result, cbResult);
    } else if (family == K_AF_NETLINK) {
        sprintf(result, "port %d", readd(thread, address + 4));
        return result;
    } else if (family == K_AF_INET) {
        sprintf(result, "AF_INET %d.%d.%d.%d:%d", readb(thread, address + 4), readb(thread, address + 5), readb(thread, address + 6), readb(thread, address + 7), readb(thread, address + 3) | (readb(thread, address + 2) << 8));
        return result;
    }
    return "Unknown address family";
}

static struct KSocket* freeSockets;
#ifdef BOXEDWINE_VM
static SDL_mutex* freeSocketsMutex;
#endif

struct KSocket* allocSocket() {
    struct KSocket* result;
#ifdef BOXEDWINE_VM
    if (!freeSocketsMutex) {
        freeSocketsMutex = SDL_CreateMutex();
    }
#endif
    BOXEDWINE_LOCK(NULL, freeSocketsMutex);
    if (freeSockets) {
        result = freeSockets;
        freeSockets = result->next;		
        memset(result, 0, sizeof(struct KSocket));
    } else {
        result = (struct KSocket*)kalloc(sizeof(struct KSocket), KALLOC_KSOCKET);
    }	
    BOXEDWINE_UNLOCK(NULL, freeSocketsMutex);
    result->recvLen = 1048576;
    result->sendLen = 1048576;
    result->blocking = 1;
#ifdef BOXEDWINE_VM
    if (!result->bufferMutex)
        result->bufferMutex = SDL_CreateMutex();
    if (!result->bufferCond)
        result->bufferCond = SDL_CreateCond();
#endif
    if (!result->recvBuffer)
        result->recvBuffer = ringbuf_new(1024 * 1024);
    return result;
}

void freeSocket(struct KSocket* socket) {
    BOXEDWINE_LOCK(NULL, freeSocketsMutex);
    socket->next = freeSockets;
    freeSockets = socket;
    BOXEDWINE_UNLOCK(NULL, freeSocketsMutex);
}

U32 ksocket2(struct KThread* thread, U32 domain, U32 type, U32 protocol, struct KSocket** returnSocket) {
    if (domain==K_AF_UNIX) {
        struct KSocket* s = allocSocket();
        struct KObject* kSocket = allocKObject(&unixsocketAccess, KTYPE_SOCK, 0, s);
        struct KFileDescriptor* result = allocFileDescriptor(thread->process, kSocket, K_O_RDWR, 0, -1, 0);
        closeKObject(kSocket);
        s->pid = thread->process->id;
        s->domain = domain;
        s->protocol = protocol;
        s->type = type;
        if (returnSocket)
            *returnSocket = s;
        return result->handle;
    } else if (domain==K_AF_NETLINK) {
        // just fake it so that libudev doesn't crash
        struct KSocket* s = allocSocket();
        struct KObject* kSocket = allocKObject(&unixsocketAccess, KTYPE_SOCK, 0, s);
        struct KFileDescriptor* result = allocFileDescriptor(thread->process, kSocket, K_O_RDWR, 0, -1, 0);
        closeKObject(kSocket);
        s->pid = thread->process->id;
        s->domain = domain;
        s->protocol = protocol;
        s->type = type;
        if (returnSocket)
            *returnSocket = s;
        return result->handle;
    } else if (domain == K_AF_INET) {
        U32 nativeType;
        S32 nativeSocket;
        U32 nativeProtocol;

#ifdef WIN32
        if (!winsock_intialized) {
            WSADATA wsaData;
            WSAStartup(0x0202, &wsaData);
            winsock_intialized=1;
        }
#endif
        if (type == K_SOCK_STREAM) {
            nativeType = SOCK_STREAM;
        } else if (type == K_SOCK_DGRAM) {
            nativeType = SOCK_DGRAM;
        } else if (type == K_SOCK_RAW) {
            nativeType = SOCK_RAW;
        } else if (type == K_SOCK_RDM) {
            nativeType = SOCK_RDM;
        } else if (type == K_SOCK_SEQPACKET) {
            nativeType = SOCK_SEQPACKET;
        } else {
            return -K_EPROTOTYPE;
        }
        if (protocol == 0) {
            nativeProtocol = IPPROTO_IP ;
        } else if (protocol == 1) {
            nativeProtocol = IPPROTO_ICMP;
        } else if (protocol == 2) {
            nativeProtocol = IPPROTO_IGMP;
        } else if (protocol == 6) {
            nativeProtocol = IPPROTO_TCP;
        } else if (protocol == 12) {
            nativeProtocol = IPPROTO_PUP;
        } else if (protocol == 17) {
            nativeProtocol = IPPROTO_UDP;
        } else if (protocol == 22) {
            nativeProtocol = IPPROTO_IDP;
        } else if (protocol == 255) {
            nativeProtocol = IPPROTO_RAW;
        } else {
            return -K_EPROTOTYPE;
        }
        nativeSocket = (S32)socket(AF_INET, nativeType, nativeProtocol);
        if (nativeSocket<0) {
            return -K_EPROTOTYPE;
        } else {
            struct KSocket* s = allocSocket();
            struct KObject* kSocket = allocKObject(&nativesocketAccess, KTYPE_SOCK, 0, s);
            struct KFileDescriptor* result = allocFileDescriptor(thread->process, kSocket, K_O_RDWR, 0, -1, 0);
            closeKObject(kSocket);
            s->pid = thread->process->id;
            s->domain = domain;
            s->protocol = protocol;
            s->type = type;
            s->nativeSocket = nativeSocket;
#ifndef BOXEDWINE_VM
            setNativeBlocking(nativeSocket, FALSE);
#endif
            if (returnSocket)
                *returnSocket = s;
            return result->handle;
        }
    }
    return -K_EAFNOSUPPORT;
}

U32 ksocket(struct KThread* thread, U32 domain, U32 type, U32 protocol) {
    return ksocket2(thread, domain, type, protocol, 0);
}

U32 kbind(struct KThread* thread, U32 socket, U32 address, U32 len) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    U32 family;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    family = readw(thread, address);
    if (family==K_AF_UNIX) {
        char tmp[MAX_FILEPATH_LEN];
        const char* name = socketAddressName(thread, address, len, tmp, sizeof(tmp));
        struct FsNode* node;

        if (!name || !name[0]) {
            return 0; // :TODO: why does XOrg need this
        }
        node = getNodeFromLocalPath(thread->process->currentDirectory, name, 0);
        if (node && node->func->exists(node)) {
            return -K_EADDRINUSE;
        }
        node->func = &unixSocketNodeType;
        node->rdev = 2;
        node->kobject = fd->kobject;
        s->node = node;
        return 0;
    } else if (family == K_AF_NETLINK) {
        U32 port = readd(thread, address + 4);
        if (port == 0) {
            port = thread->process->id;
        }
        s->nl_port = port;
        s->listening = 1;
        return 0;
    } else if (family == K_AF_INET) {
        char tmp[4096];
        if (len>sizeof(tmp)) {
            kpanic("kbind: buffer not large enough: len=%d", len);
        }
        memcopyToNative(thread, address, tmp, len);
        if (bind(s->nativeSocket, (struct sockaddr*)tmp, len)==0) {
            s->error = 0;
            return 0;
        }
        return handleNativeSocketError(thread, s, 0);
    }
    return -K_EAFNOSUPPORT;
}

U32 kconnect(struct KThread* thread, U32 socket, U32 address, U32 len) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (s->connected) {
        return -K_EISCONN;
    }		
    if (s->nativeSocket) {
        char buffer[1024];
        int result = 0;

#ifndef BOXEDWINE_VM
        if (s->connecting) {
            if (nativesocket_isWriteReady(thread, fd->kobject)) {
                s->error = 0;
                s->connecting = 0;
                s->connected = 1;
                removeWaitingSocket(s);
                return 0;
            } else {
                int error=0;
                socklen_t len = 4;
                if (getsockopt(s->nativeSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0) {
                    return -K_EIO;
                }
                if (error) {
                    U32 result = translateNativeSocketError(error);
                    if (result!=-K_EWOULDBLOCK) {
                        return result;
                    }
                }
            }
            addWaitingNativeSocket(s); 
            waitOnSocketWrite(s, thread);
            return -K_WAIT;
        }
#endif
        memcopyToNative(thread, address, buffer, len);
        if (connect(s->nativeSocket, (struct sockaddr*)buffer, len)==0) {
            int error;
            len = 4;
            getsockopt(s->nativeSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
            if (error==0) {
                s->connected = 1;
                return 0;
            }
            s->connecting = 0;
            return -K_ECONNREFUSED;
        }   
        result = handleNativeSocketError(thread, s, 1);
        if (result == -K_WAIT) {            
            s->connecting = s;
        }
        return result;
    }
    if (len-2>sizeof(s->destAddress.data)) {
        kpanic("Socket address is too big");
    }
    s->destAddress.family = readw(thread, address);
    memcopyToNative(thread, address + 2, s->destAddress.data, len - 2);
    if (s->type==K_SOCK_DGRAM) {
        s->connected = 1;		
        return 0;
    } else if (s->type==K_SOCK_STREAM) {
        if (s->destAddress.data[0]==0) {
            return -K_ENOENT;
        }
        if (s->domain==K_AF_UNIX) {
            struct FsNode* node = getNodeFromLocalPath(thread->process->currentDirectory, s->destAddress.data, MAX_FILEPATH_LEN);
            struct KSocket* destination = 0;

            if (!node || !node->kobject) {
                s->destAddress.family = 0;
                return -K_ECONNREFUSED;
            }
            destination = node->kobject->socket;
            if (s->connection) {
                s->connected = 1;
                return 0;
            }
            if (s->connecting) {
                if (s->blocking) {
                    waitOnSocketConnect(s, thread);
                    return -K_WAIT;
                } else {
                    return -K_EINPROGRESS;
                }
            } else {
                U32 i;
                BOOL found = FALSE;

#ifdef BOXEDWINE_VM
                if (!destination->connectionMutex)
                    destination->connectionMutex = SDL_CreateMutex();
                if (!destination->connectionCond)
                    destination->connectionCond = SDL_CreateCond();
#endif
                BOXEDWINE_LOCK(NULL, destination->connectionMutex);
                for (i=0;i<MAX_PENDING_CONNECTIONS;i++) {
                    if (!destination->pendingConnections[i]) {
                        destination->pendingConnections[i] = s;
                        destination->pendingConnectionsCount++;
                        s->connecting = destination;
                        if (destination->waitingOnConnectThread)
                            wakeThread(NULL, destination->waitingOnConnectThread);

                        BOXEDWINE_LOCK(NULL, pollMutex);
                        BOXEDWINE_SIGNAL_ALL(pollCond);
                        BOXEDWINE_UNLOCK(NULL, pollMutex);
                        found = TRUE;
                        break;
                    }
                }
                BOXEDWINE_UNLOCK(NULL, destination->connectionMutex);
                if (!found) {
                    kwarn("connect: destination socket pending connections queue is full");
                    return -K_ECONNREFUSED;
                }
                if (!s->blocking) {
                    return -K_EINPROGRESS;
                }
                // :TODO: what about a time out
                
#ifdef BOXEDWINE_VM
                if (!s->connectionMutex)
                    s->connectionMutex = SDL_CreateMutex();
                if (!s->connectionCond)
                    s->connectionCond = SDL_CreateCond();
                while (s->connecting) {
                    waitOnSocketConnect(s, thread);
                }
                if (!s->connection) {
                    return -K_ECONNREFUSED;
                }
                s->connected = 1;
#else
                waitOnSocketConnect(s, thread);
                return -K_WAIT;
#endif
            }
        } else {
            kpanic("connect not implemented for domain %d", s->domain);
        }
    } else {
        kpanic("connect not implemented for type %d", s->type);
    }
    return 0;
}

U32 klisten(struct KThread* thread, U32 socket, U32 backlog) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (s->nativeSocket) {
        S32 result = listen(s->nativeSocket, backlog);
        if (result>=0) {
            s->listening = 1;
            return result;
        }
        s->listening = 0;
        return handleNativeSocketError(thread, s, 0);
    }
    if (!s->node) {
        return -K_EDESTADDRREQ;
    }
    if (s->connection) {
        return -K_EINVAL;
    }
    s->listening = TRUE;
    return 0;
}

U32 kaccept(struct KThread* thread, U32 socket, U32 address, U32 len) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    struct KSocket* connection = 0;
    struct KSocket* resultSocket = 0;
    U32 i;
    S32 result;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (!s->listening) {
        return -K_EINVAL;
    }
    if (s->nativeSocket) {
        struct sockaddr addr;
        U32 addrlen = sizeof(struct sockaddr);
        S32 result = (S32)accept(s->nativeSocket, &addr, &addrlen);
        if (result>=0) {
            struct KSocket* newSocket = allocSocket();
            struct KObject* kSocket = allocKObject(&nativesocketAccess, KTYPE_SOCK, 0, s);
            struct KFileDescriptor* fd = allocFileDescriptor(thread->process, kSocket, K_O_RDWR, 0, -1, 0);
            closeKObject(kSocket);

            newSocket->pid = thread->process->id;
            newSocket->domain = s->domain;
            newSocket->protocol = s->protocol;
            newSocket->type = s->type;
            newSocket->nativeSocket = result;
#ifndef BOXEDWINE_VM
            setNativeBlocking(result, FALSE);
#endif
            if (address)
                memcopyFromNative(thread, address, (char*)&addr, addrlen);
            if (len) {
                writed(thread, len, addrlen);
            }
            return fd->handle;
        }
        return handleNativeSocketError(thread, s, 0);
    }
#ifdef BOXEDWINE_VM
    if (!s->connectionMutex)
        s->connectionMutex = SDL_CreateMutex();
    if (!s->connectionCond) {
        s->connectionCond = SDL_CreateCond();
    }
#endif
    BOXEDWINE_LOCK(NULL, s->connectionMutex);
    if (!s->pendingConnectionsCount) {
        BOXEDWINE_UNLOCK(NULL, s->connectionMutex);
        if (!s->blocking)
            return -K_EWOULDBLOCK;
        waitOnSocketConnect(s, thread);
        return -K_WAIT;
    }
    // :TODO: should make this FIFO
    for (i=0;i<MAX_PENDING_CONNECTIONS;i++) {
        if (s->pendingConnections[i]) {
            connection = s->pendingConnections[i];
            s->pendingConnections[i] = 0;
            s->pendingConnectionsCount--;
            break;
        }
    }
    BOXEDWINE_UNLOCK(NULL, s->connectionMutex);
    if (!connection) {
        kpanic("socket pending connection count was greater than 0 but could not find a connnection");
    }
    result = ksocket2(thread, connection->domain, connection->type, connection->protocol, &resultSocket);
    if (!resultSocket) {
        kpanic("ksocket2 failed to create a socket in accept");
    }

    BOXEDWINE_LOCK(thread, connection->connectionMutex);
    connection->connection = resultSocket;
    //connection->connected = 1; this will be handled when the connecting thread is unblocked
    resultSocket->connected = 1;
    resultSocket->connection = connection;
    connection->inClosed = FALSE;
    connection->outClosed = FALSE;
    resultSocket->inClosed = FALSE;
    resultSocket->outClosed = FALSE;
    connection->connecting = 0;
    BOXEDWINE_UNLOCK(thread, connection->connectionMutex);

    if (connection->waitingOnConnectThread)
        wakeThread(NULL, connection->waitingOnConnectThread);

    BOXEDWINE_LOCK(thread, pollMutex);
    BOXEDWINE_SIGNAL_ALL(pollCond);
    BOXEDWINE_UNLOCK(thread, pollMutex);

    return result;
}

U32 kgetsockname(struct KThread* thread, U32 socket, U32 address, U32 plen) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    U32 len = readd(thread, plen);
    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (s->domain == K_AF_NETLINK) {
        if (len>0 && len<12)
            kpanic("getsocketname: AF_NETLINK wrong address size");
        writew(thread, address, s->domain);
        writew(thread, address + 2, 0);
        writed(thread, address + 4, s->nl_port);
        writed(thread, address + 8, 0);
        writed(thread, plen, 12);
    } else if (s->domain == K_AF_UNIX) {
        writew(thread, address, s->destAddress.family);
        len-=2;
        if (len>sizeof(s->destAddress.data))
            len = sizeof(s->destAddress.data);
        memcopyFromNative(thread, address + 2, s->destAddress.data, len);
        writed(thread, plen, 2 + (U32)strlen(s->destAddress.data) + 1);
    }
    else if (s->nativeSocket && len<=256) {
        char buf[256];
        U32 result = getsockname(s->nativeSocket, (struct sockaddr*)buf, &len);
        if (result)
            result = handleNativeSocketError(thread, s, 0);
        else {
            memcopyFromNative(thread, address, buf, len);
            writed(thread, plen, len);
        }
        return result;
    } else {
        kwarn("getsockname not implemented");
        return -K_EACCES;
    }
    return 0;
}

U32 kgetpeername(struct KThread* thread, U32 socket, U32 address, U32 plen) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    U32 len = readd(thread, plen);
    struct sockaddr a;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (s->nativeSocket) {        
        S32 result = getpeername(s->nativeSocket, &a, &len);
        if (result==0) {
            memcopyFromNative(thread, address, (char*)&a, len);
            s->error = 0;
            return 0;
        }
        return handleNativeSocketError(thread, s, 0);
    }
    if (!s->connection)
        return -K_ENOTCONN;
    writew(thread, address, s->destAddress.family);
    len-=2;
    if (len>sizeof(s->destAddress.data))
        len = sizeof(s->destAddress.data);
    memcopyFromNative(thread, address + 2, s->destAddress.data, len);
    writed(thread, plen, 2 + (U32)strlen(s->destAddress.data) + 1);
    return 0;
}

U32 ksocketpair(struct KThread* thread, U32 af, U32 type, U32 protocol, U32 socks) {
    FD fd1;
    FD fd2;
    struct KFileDescriptor* f1;
    struct KFileDescriptor* f2;
    struct KSocket* s1;
    struct KSocket* s2;

    if (af!=K_AF_UNIX) {
        kpanic("socketpair with adress family %d not implemented", af);
    }
    if (type!=K_SOCK_DGRAM && type!=K_SOCK_STREAM) {
        kpanic("socketpair with type %d not implemented", type);
    }
    fd1 = ksocket(thread, af, type, protocol);
    fd2 = ksocket(thread, af, type, protocol);
    f1 = getFileDescriptor(thread->process, fd1);
    f2 = getFileDescriptor(thread->process, fd2);
    s1 = f1->kobject->socket;
    s2 = f2->kobject->socket;
    
    s1->connection = s2;
    s2->connection = s1;
    s1->inClosed = FALSE;
    s1->outClosed = FALSE;
    s2->inClosed = FALSE;
    s2->outClosed = FALSE;
    s1->connected = TRUE;
    s2->connected = TRUE;
    f1->accessFlags = K_O_RDWR;
    f2->accessFlags = K_O_RDWR;
    writed(thread, socks, fd1);
    writed(thread, socks + 4, fd2);
    return 0;
}

U32 ksend(struct KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;	
    U32 result;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    s->flags = 0;
    if (flags == 0x4000) {
        //  MSG_NOSIGNAL
    }
    if (flags & 1) {
        s->flags|=MSG_OOB;
    } 
    result = syscall_write(thread, socket, buffer, len);
    s->flags = 0;
    return result;
}

U32 krecv(struct KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;	
    U32 result;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    s->flags = 0;
    if (flags == 0x4000) {
        //  MSG_NOSIGNAL
    } 
    if (flags & 1) {
        s->flags|=MSG_OOB;
    } 
    if (flags & 2) {
        s->flags|=MSG_PEEK;
    } 
    result = syscall_read(thread, socket, buffer, len);
    s->flags = 0;
    return result;
}

U32 kshutdown(struct KThread* thread, U32 socket, U32 how) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (s->nativeSocket) {
        if (shutdown(s->nativeSocket, how) == 0)
            return 0;
        return -1;
    }
    if (s->type == K_SOCK_DGRAM) {
        kpanic("shutdown on SOCK_DGRAM not implemented");
    }
    if (!s->connection) {
        return -K_ENOTCONN;
    }
    if (how == K_SHUT_RD) {
        s->inClosed=1;
        s->connection->outClosed=1;
    } else if (how == K_SHUT_WR) {
        s->outClosed=1;
        s->connection->inClosed=1;
    } else if (how == K_SHUT_RDWR) {
        s->outClosed=1;
        s->inClosed=1;
        s->connection->outClosed=1;
        s->connection->inClosed=1;
    }
    // :TODO: wake up sleeping theads
    return 0;
}

U32 ksetsockopt(struct KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (level == K_SOL_SOCKET) {
        switch (name) {
            case K_SO_RCVBUF:
                if (len!=4)
                    kpanic("setsockopt SO_RCVBUF expecting len of 4");
                s->recvLen = readd(thread, value);
            case K_SO_SNDBUF:
                if (len != 4)
                    kpanic("setsockopt SO_SNDBUF expecting len of 4");
                s->sendLen = readd(thread, value);
            case K_SO_PASSCRED:
                break;
            case K_SO_ATTACH_FILTER:
                break;
            default:
                kwarn("setsockopt name %d not implemented", name);
        }
    } else {
        kwarn("setsockopt level %d not implemented", level);
    }
    return 0;
}

U32 kgetsockopt(struct KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len_address) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    int len;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    len = readd(thread, len_address);
    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len!=4)
                kpanic("getsockopt SO_RCVBUF expecting len of 4");
            writed(thread, value, s->recvLen);
        } else if (name == K_SO_SNDBUF) {
            if (len != 4)
                kpanic("getsockopt SO_SNDBUF expecting len of 4");
            writed(thread, value, s->sendLen);
        } else if (name == K_SO_ERROR) {
            if (len != 4)
                kpanic("getsockopt SO_ERROR expecting len of 4");
            writed(thread, value, s->error);
        } else if (name == K_SO_TYPE) { 
            if (len != 4)
                kpanic("getsockopt K_SO_TYPE expecting len of 4");
            writed(thread, value, s->type);
        } else if (name == K_SO_PEERCRED) {
            if (s->domain!=K_AF_UNIX) {
                return -K_EINVAL; // :TODO: is this right
            }
            if (!s->connection) {
                return -K_EINVAL; // :TODO: is this right
            }
            if (len != 12)
                kpanic("getsockopt SO_PEERCRED expecting len of 12");
            writed(thread, value, s->connection->pid);
            writed(thread, value + 4, thread->process->userId);
            writed(thread, value + 8, thread->process->groupId);
        } else {
            kwarn("getsockopt name %d not implemented", name);
            return -K_EINVAL;
        }
    } else {
        kwarn("getsockopt level %d not implemented", level);
        return -K_EINVAL;
    }
    return 0;
}

struct MsgHdr {
    U32 msg_name;
    U32 msg_namelen;
    U32 msg_iov;
    U32 msg_iovlen;
    U32 msg_control;
    U32 msg_controllen;
    U32 msg_flags;
};

struct CMsgHdr {
    U32 cmsg_len;
    U32 cmsg_level;
    U32 cmsg_type;
};

void readMsgHdr(struct KThread* thread, U32 address, struct MsgHdr* hdr) {
    hdr->msg_name = readd(thread, address);address+=4;
    hdr->msg_namelen = readd(thread, address); address += 4;
    hdr->msg_iov = readd(thread, address); address += 4;
    hdr->msg_iovlen = readd(thread, address); address += 4;
    hdr->msg_control = readd(thread, address); address += 4;
    hdr->msg_controllen = readd(thread, address); address += 4;
    hdr->msg_flags = readd(thread, address);
}

void readCMsgHdr(struct KThread* thread, U32 address, struct CMsgHdr* hdr) {
    hdr->cmsg_len = readd(thread, address); address += 4;
    hdr->cmsg_level = readd(thread, address); address += 4;
    hdr->cmsg_type = readd(thread, address);
}

void writeCMsgHdr(struct KThread* thread, U32 address, U32 len, U32 level, U32 type) {
    writed(thread, address, len); address += 4;
    writed(thread, address, level); address += 4;
    writed(thread, address, type);
}

#define K_SOL_SOCKET 1
#define K_SCM_RIGHTS 1

U32 ksendmmsg(struct KThread* thread, U32 socket, U32 address, U32 vlen, U32 flags) {
    U32 i;

    for (i=0;i<vlen;i++) {
        S32 result = (S32)ksendmsg(thread, socket, address+i*32, flags);
        if (result>=0) {
            writed(thread, address+i*32+28, result);
        } else {
            return i;
        }
    }
    return vlen;
}

U32 ksendmsg(struct KThread* thread, U32 socket, U32 address, U32 flags) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;	
    struct MsgHdr hdr;
    struct KSocketMsg* msg;
    struct KSocketMsg* next;
    U32 pos = 0;
    U32 i;	
    U32 result = 0;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    msg = allocSocketMsg();
    readMsgHdr(thread, address, &hdr);

    if (hdr.msg_control) {
        struct CMsgHdr cmsg;			

        readCMsgHdr(thread, hdr.msg_control, &cmsg);
        if (cmsg.cmsg_level != K_SOL_SOCKET) {
            kpanic("sendmsg control level %d not implemented", cmsg.cmsg_level);
        } else if (cmsg.cmsg_type != K_SCM_RIGHTS) {
            kpanic("sendmsg control type %d not implemented", cmsg.cmsg_level);
        } else if ((cmsg.cmsg_len & 3) != 0) {
            kpanic("sendmsg control len %d not implemented", cmsg.cmsg_len);
        }
        msg->objectCount = hdr.msg_controllen/16;
        if (msg->objectCount>MAX_NUMBER_OF_KSOCKET_MSG_OBJECTS_PER_MSG) {
            kpanic("Too many objects sent in sendmsg: %d", msg->objectCount);
        }
        for (i=0;i<msg->objectCount;i++) {
            struct KFileDescriptor* f = getFileDescriptor(thread->process, readd(thread, hdr.msg_control + 16 * i + 12));
            if (!f) {
                kpanic("tried to sendmsg a bad file descriptor");
            } else {				
                f->kobject->refCount++;
                msg->objects[i].object = f->kobject;
                msg->objects[i].accessFlags = f->accessFlags;
            }
        }				
    }
    for (i=0;i<hdr.msg_iovlen;i++) {
        U32 p = readd(thread, hdr.msg_iov + 8 * i);
        U32 len = readd(thread, hdr.msg_iov + 8 * i + 4);
        if (s->nativeSocket) {
            result+=nativesocket_write(thread, fd->kobject, p, len);
        } else {
            if (pos + len>4096) {
                kpanic("sendmsg payload was larger than 4096 bytes");
            }
            msg->data[pos++] = len;
            msg->data[pos++] = len >> 8;
            msg->data[pos++] = len >> 16;
            msg->data[pos++] = len >> 24;
            while (len) {
                msg->data[pos++] = readb(thread, p++);
                len--;
                result++;
            }
        }
    }
    if (s->connection) {
        BOXEDWINE_LOCK(thread, s->connection->bufferMutex);
        if (!s->nativeSocket) {
            msg->next = 0;
            if (s->connection) {
                if (!s->connection->msgs) {
                    s->connection->msgs = msg;
                }
                else {
                    next = s->connection->msgs;
                    while (next->next) {
                        next = next->next;
                    }
                    next->next = msg;
                }            
            }
        }
        BOXEDWINE_UNLOCK(thread, s->connection->bufferMutex);
        wakeAndResetWaitingOnReadThreads(s->connection);
    }
    return result;
}

U32 krecvmsg(struct KThread* thread, U32 socket, U32 address, U32 flags) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    struct MsgHdr hdr;
    struct KSocketMsg* msg;
    U32 result = 0;
    U32 i;
    U32 pos=0;
    U32 msg_namelen;

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (s->nativeSocket) {
        char tmp[PAGE_SIZE];

        msg_namelen = readd(thread, address+4);
        readMsgHdr(thread, address, &hdr);        
        for (i = 0; i < hdr.msg_iovlen; i++) {
            U32 p = readd(thread, hdr.msg_iov + 8 * i);
            U32 len = readd(thread, hdr.msg_iov + 8 * i + 4);

            if (s->type == K_SOCK_DGRAM && s->domain==K_AF_INET && msg_namelen>=sizeof(struct sockaddr_in)) {
                struct sockaddr_in in;
                S32 r;
                U32 inLen = sizeof(struct sockaddr_in);

                if (len>sizeof(tmp))
                    len = sizeof(tmp);
                r = recvfrom(s->nativeSocket, tmp, len, 0, (struct sockaddr*)&in, &inLen);
                if (r>=0) {
                    memcopyFromNative(thread, p, tmp, r);
                    // :TODO: maybe copied fields to the expected location rather than assume the structures are the same
                    memcopyFromNative(thread, readd(thread, address), (const char*)&in, sizeof(in));
                    writed(thread, address + 4, inLen);
                    result+=r;
                }
                else if (result)
                    break;
                else
                    result = handleNativeSocketError(thread, s, 0);
            } else {
                result += nativesocket_read(thread, fd->kobject, p, len);
            }
        }
        if (s->type==K_SOCK_STREAM)
            writed(thread, address + 4, 0); // msg_namelen, set to 0 for connected sockets
        writed(thread, address + 20, 0); // msg_controllen
        return result;
    }
    if (s->domain==K_AF_NETLINK)
        return -K_EIO;
    BOXEDWINE_LOCK(thread, s->bufferMutex);
    if (!s->msgs) {
        if (ringbuf_bytes_used(s->recvBuffer)) {
            char tmp[PAGE_SIZE];

            msg_namelen = readd(thread, address+4);
            readMsgHdr(thread, address, &hdr);        
            for (i = 0; i < hdr.msg_iovlen; i++) {
                U32 p = readd(thread, hdr.msg_iov + 8 * i);
                U32 len = readd(thread, hdr.msg_iov + 8 * i + 4);
                
                while (len>0 && !ringbuf_is_empty(s->recvBuffer)) {
                    U32 todo = len;

                    if (todo>sizeof(tmp))
                        todo = sizeof(tmp);                    
                    if (todo>ringbuf_bytes_used(s->recvBuffer))
                        todo=(U32)ringbuf_bytes_used(s->recvBuffer);

                    ringbuf_memcpy_from(tmp, s->recvBuffer, todo);     
                    memcopyFromNative(thread, p, tmp, todo);
                    p+=todo;
                    len-=todo;
                    result+=todo;
                }
            }
            if (s->type==K_SOCK_STREAM)
                writed(thread, address + 4, 0); // msg_namelen, set to 0 for connected sockets
            writed(thread, address + 20, 0); // msg_controllen
            return result;
        }
        if (!s->blocking) {
            BOXEDWINE_UNLOCK(thread, s->bufferMutex);
            return -K_EWOULDBLOCK;
        }
        // :TODO: what about a time out
        waitOnSocketRead(s, thread);	
#ifndef BOXEDWINE_VM
        return -K_WAIT;
#endif
    }
    msg = s->msgs;
    readMsgHdr(thread, address, &hdr);
    if (hdr.msg_control) {
        for (i=0;i<hdr.msg_controllen && i<msg->objectCount;i++) {
            struct KFileDescriptor* fd = allocFileDescriptor(thread->process, msg->objects[i].object, msg->objects[i].accessFlags, 0, -1, 0);
            writeCMsgHdr(thread, hdr.msg_control + i * 16, 16, K_SOL_SOCKET, K_SCM_RIGHTS);
            writed(thread, hdr.msg_control + i * 16 + 12, fd->handle);
            closeKObject(msg->objects[i].object);
        }
        writed(thread, address + 20, i * 20);
    }
    for (i=0;i<hdr.msg_iovlen;i++) {
        U32 p = readd(thread, hdr.msg_iov + 8 * i);
        U32 len = readd(thread, hdr.msg_iov + 8 * i + 4);
        U32 dataLen = msg->data[pos] | msg->data[pos + 1] | msg->data[pos + 2] | msg->data[pos + 3];
        pos+=4;
        if (len<dataLen) {
            kpanic("unhandled socket msg logic");
        }
        memcopyFromNative(thread, p, (S8*)msg->data + pos, dataLen);
        result+=dataLen;
    }
    msg->objectCount=0; // we closed the KObjects, this will prevent freeSocketMsg from closing them again;	
    s->msgs = s->msgs->next;    
    BOXEDWINE_UNLOCK(thread, s->bufferMutex);
    freeSocketMsg(msg);    
    if (s->connection)
        wakeAndResetWaitingOnWriteThreads(s->connection);
    return result;
}

U32 syscall_pipe(struct KThread* thread, U32 address) {
    return ksocketpair(thread, K_AF_UNIX, K_SOCK_STREAM, 0, address);
}


U32 syscall_pipe2(struct KThread* thread, U32 address, U32 flags) {
    U32 result = syscall_pipe(thread, address);
    if (result==0) {
        if ((flags & K_O_CLOEXEC)!=0) {
            syscall_fcntrl(thread, readd(thread, address), K_F_SETFD, FD_CLOEXEC);
            syscall_fcntrl(thread, readd(thread, address + 4), K_F_SETFD, FD_CLOEXEC);
        }
        if ((flags & K_O_NONBLOCK)!=0) {
            syscall_fcntrl(thread, readd(thread, address), K_F_SETFL, K_O_NONBLOCK);
            syscall_fcntrl(thread, readd(thread, address + 4), K_F_SETFL, K_O_NONBLOCK);
        }
        if (flags & ~(K_O_CLOEXEC|K_O_NONBLOCK)) {
            kwarn("Unknow flags sent to pipe2: %X", flags);
        }
    }
    return result;
}


// ssize_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
U32 ksendto(struct KThread* thread, U32 socket, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    int len=sizeof(struct sockaddr);
    S32 result;
    int f = 0;
    struct sockaddr dest;
    char tmp[PAGE_SIZE];

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (!s->nativeSocket)
        return 0;
    if (flags & 1)
        f|=MSG_OOB;
    if (flags & (~1)) {
        kwarn("ksendto unsupported flags: %d", flags);
    }
    memcopyToNative(thread, dest_addr, (char*)&dest, len);
    if (length>sizeof(tmp)) {
        kpanic("ksendto buffer not large enough: length = %d", length);
    }
    memcopyToNative(thread, message, tmp, length);
    result = sendto(s->nativeSocket, tmp, length, f, &dest, len);
    if (result>=0) {
        s->error = 0;
        return result;
    }
    return handleNativeSocketError(thread, s, 1);
}

// ssize_t recvfrom(int socket, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
U32 krecvfrom(struct KThread* thread, U32 socket, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, socket);
    struct KSocket* s;
    U32 inLen=0;
    U32 outLen=0;
    S32 result;
    int f = 0;
    char fromBuffer[256];
    char tmp[PAGE_SIZE];
    

    if (address_len) {
        inLen = readd(thread, address_len);
        if (inLen>sizeof(fromBuffer)) {
            kpanic("krecvfrom fromBuffer tmp is too small");
        }
    }
    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_SOCK) {
        return -K_ENOTSOCK;
    }
    s = fd->kobject->socket;
    if (!s->nativeSocket)
        return 0;
    if (flags & 1)
        f|=MSG_OOB;
    if (flags & 2)
        f|=MSG_PEEK;
    if (flags & (~3)) {
        kwarn("krecvfrom unsupported flags: %d", flags);
    }
    memcopyToNative(thread, address, (char*)fromBuffer, inLen);
    outLen = inLen;
    // :TODO: what about tmp size
    result = recvfrom(s->nativeSocket, tmp, length, f, (struct sockaddr*)fromBuffer, &outLen);
    if (result>=0) {
        memcopyFromNative(thread, buffer, tmp, result);
        memcopyFromNative(thread, address, (char*)fromBuffer, inLen);
        writed(thread, address_len, outLen);
        s->error = 0;
        return result;
    }
    result = handleNativeSocketError(thread, s, 0);
    if (result == 0) { // WSAEMSGSIZE for example
        result = length;
        memcopyFromNative(thread, address, (char*)fromBuffer, inLen);
        writed(thread, address_len, outLen);
        s->error = 0;
        return result;
    } 
    return result;
}

U32 isNativeSocket(struct KThread* thread, int desc) {
    struct KFileDescriptor* fd = getFileDescriptor(thread->process, desc);

    if (fd && fd->kobject->type == KTYPE_SOCK) {
        struct KSocket* s = fd->kobject->socket;
        if (s->nativeSocket) {
            return 1;
        }
    }
    return 0;
}
