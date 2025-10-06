#ifndef __FSAPI_H__
#define __FSAPI_H__

#include "platform.h"
#include "memory.h"
#include "kthread.h"

#define K_O_RDONLY   0x0000
#define K_O_WRONLY   0x0001
#define K_O_RDWR     0x0002
#define K_O_ACCMODE  0x0003

#define K_O_CREAT	   0x0040
#define K_O_EXCL	   0x0080
#define K_O_TRUNC	   0x0200
#define K_O_APPEND     0x0400

// can change after open
#define K_O_NONBLOCK 0x0800
#define K_O_ASYNC 0x2000
#define K_O_CLOEXEC 0x80000

#define FD_CLOEXEC 1

// type of lock
#define K_F_RDLCK	   0
#define K_F_WRLCK	   1
#define K_F_UNLCK	   2

#define IOCTL_ARG1 EDX
#define IOCTL_ARG2 ESI
#define IOCTL_ARG3 EDI
#define IOCTL_ARG4 EBP

#define FS_BLOCK_SIZE 8192

struct FsNode;
struct FsOpenNode;
struct FsNodeFunc {
    BOOL (*isDirectory)(struct FsNode* node);
    BOOL (*exists)(struct FsNode* node);
    U32 (*rename)(struct FsNode* oldNode, struct FsNode* newNode);
    BOOL (*remove)(struct FsNode* node);
    U64 (*lastModified)(struct FsNode* node);
    U64 (*length)(struct FsNode* node);
    struct FsOpenNode* (*open)(struct KProcess* process, struct FsNode* node, U32 flags);
    BOOL (*canRead)(struct KProcess* process, struct FsNode* node);
    BOOL (*canWrite)(struct KProcess* process, struct FsNode* node);
    U32 (*getType)(struct FsNode* node, U32 checkForLink);
    U32 (*getMode)(struct KProcess* process, struct FsNode* node);
    U32 (*removeDir)(struct FsNode* node);
    U32 (*makeDir)(struct FsNode* node);    
    U32 (*setTimes)(struct FsNode* node, U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);
};

struct FsOpenNodeFunc {
    BOOL (*init)(struct KProcess* process, struct FsOpenNode* node);
    S64  (*length)(struct FsOpenNode* node);
    BOOL (*setLength)(struct FsOpenNode* node, S64 length);
    S64  (*getFilePointer)(struct FsOpenNode* node);
    S64  (*seek)(struct FsOpenNode* node, S64 pos);	
    U32  (*read)(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len);
    U32  (*write)(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len);
    void (*close)(struct FsOpenNode* node);
    U32  (*map)(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len, S32 prot, S32 flags, U64 off);
    BOOL (*canMap)(struct FsOpenNode* node);
    U32  (*ioctl)(struct KThread* thread, struct FsOpenNode* node, U32 request);	
    void (*setAsync)(struct FsOpenNode* node, struct KProcess* process, FD fd, BOOL isAsync);
    BOOL (*isAsync)(struct FsOpenNode* node, struct KProcess* process);
    void (*waitForEvents)(struct FsOpenNode* node, struct KThread* thread, U32 events);
    BOOL (*isWriteReady)(struct KThread* thread, struct FsOpenNode* node);
    BOOL (*isReadReady)(struct KThread* thread, struct FsOpenNode* node);
    void (*free)(struct FsOpenNode* node);
    struct FsNode* (*getDirectoryEntry)(struct FsOpenNode* node, U32 index);
    U32 (*getDirectoryEntryCount)(struct FsOpenNode* node);
    U32 (*readNative)(struct FsOpenNode* node, U8* buffer, U32 len);
    U32 (*writeNative)(struct FsOpenNode* node, U8* buffer, U32 len);
    void* data; // used by buffer
    U32 dataLen;
};

struct FsNode {
    const char* path; 
    const char* name;
    struct FsNodeFunc* func;    
    struct KObject* kobject; // used by unixsocket
    U32 id;
    U32 rdev;
    struct KFileLock* locks;

    const char* reserved1;
    BOOL  reserved2;
    struct FsOpenNode* reserved3;
    struct FsNode* reserved9;
    struct FsOpenNodeFunc* reserved10;
    U32 reserved11;
    BOOL reserved12;
    U64 reserved13;
    U64 reserved14;
    U64 reserved16;
    struct FsNode* reserved17;
    struct FsNode* reserved18;
    BOOL reserved19;
};

struct FsOpenNode {
    struct FsNode* node;
    struct FsOpenNodeFunc* func;
    U32 flags;
    void* data; // used by dsp,tty
    U32 idata; // used by devfb,meminfo,buffer

    U64 reserved4;
    U32 reserved5;
    struct FsOpenNode* reserved6;
    struct FsOpenNode* reserved7;
    struct FsNode* reserved8;
    U64 reserved15;
};

BOOL initFileSystem(const char* rootPath, const char* zipPath);
struct FsNode* getNodeFromLocalPath(const char* currentDirectory, const char* path, BOOL existing);
struct FsNode* addVirtualFile(const char* path, struct FsOpenNodeFunc* func, U32 mode, U32 rdev);
void removeNodeFromCache(struct FsNode* node);
BOOL isLink(struct FsNode* node);
BOOL readLink(struct FsNode*, char* buffer, int bufferSize, BOOL makeAbsolute);
U32 getHardLinkCount(struct FsNode* node);
struct FsNode* getParentNode(struct FsNode* node);    

U32 syscall_link(struct KThread* thread, U32 from, U32 to);
U32 syscall_symlinkat(struct KThread* thread, U32 oldpath, FD dirfd, U32 newpath);
U32 syscall_symlink(struct KThread* thread, U32 path1, U32 path2);
#endif
