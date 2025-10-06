#include "winebox.h"
#include "soft_page.h"
#include "soft_memory.h"
#include "kthread.h"
#include "kprocess.h"

#ifndef BOXEDWINE_64BIT_MMU

static U8 native_readb(struct KThread* thread, U32 address) {	
    struct Memory* memory = thread->process->memory;
    U8* result = (U8*)memory->nativeAddressStart+memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK);
    return *result;
}

static void native_writeb(struct KThread* thread, U32 address, U8 value) {
    struct Memory* memory = thread->process->memory;
    U8* result = (U8*)memory->nativeAddressStart+memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK);
    *result = value;
}

static U16 native_readw(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
#ifdef UNALIGNED_MEMORY
    U8* result = (U8*)(memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    return *result | ((*(result+1)) << 8);
#else
    U16* result = (U16*)((U8*)memory->nativeAddressStart+memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    return *result;
#endif
}

static void native_writew(struct KThread* thread, U32 address, U16 value) {
    struct Memory* memory = thread->process->memory;
#ifdef UNALIGNED_MEMORY
    U8* result = (U8*)(memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    *result = value;
    *(result+1) = value >> 8;
#else
    U16* result = (U16*)((U8*)memory->nativeAddressStart+memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    *result = value;
#endif
}

static U32 native_readd(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
#ifdef UNALIGNED_MEMORY
    U8* result = (U8*)(memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    return *result | ((*(result+1)) << 8) | ((*(result+2)) << 16) | ((*(result+3)) << 24);
#else
    U32* result = (U32*)((U8*)memory->nativeAddressStart+memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    return *result;
#endif
}

static void native_writed(struct KThread* thread, U32 address, U32 value) {
    struct Memory* memory = thread->process->memory;
#ifdef UNALIGNED_MEMORY
    U8* result = (U8*)(memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    *result = value;
    *(result+1) = value >> 8;
    *(result+2) = value >> 16;
    *(result+3) = value >> 24;
#else
    U32* result = (U32*)((U8*)memory->nativeAddressStart+memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK));
    *result = value;
#endif
}

static void native_clear(struct Memory* memory, U32 page) {    
}

static U8* native_physicalAddress(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
    return (U8*)memory->nativeAddressStart+memory->ramPage[address >> PAGE_SHIFT]+(address & PAGE_MASK);
}

struct Page softNativePage = {native_readb, native_writeb, native_readw, native_writew, native_readd, native_writed, native_clear, native_physicalAddress};

#endif