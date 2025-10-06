#include "winebox.h"

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)

#include "glMarshal.h"
#include "kthread.h"
#include "glcommon.h"
#include "../emulation/softmmu/soft_memory.h"

#ifndef BOXEDWINE_64BIT_MMU
U32 updateVertexPointer(struct CPU* cpu, struct OpenGLVetexPointer* p, U32 count) {
    U32 usingArrayBuffer = ARRAY_BUFFER();

    if (ARRAY_BUFFER()) {
        klog("updateVertexPointer might have failed");
        return 0;
    }
    if (p->ptr) {        
        U32 datasize = count * p->size * (p->stride?p->stride:getDataSize(p->type));    
        U32 pages = numberOfContiguousRamPages(cpu->memory, p->ptr >> PAGE_SHIFT);
        U32 available = PAGE_SIZE - (p->ptr & PAGE_MASK) + (pages << PAGE_SHIFT);

#ifndef UNALIGNED_MEMORY
        if (count == 0 || available > datasize) {
            unsigned char* previousMarshal = p->marshal;

            if (p->marshal_size) {
                free(p->marshal);
            }            
            p->marshal = getPhysicalAddress(cpu->thread, p->ptr);
            p->marshal_size = 0;
            
            if (p->marshal) {
                if (p->refreshEachCall)
                    return 1; 
                // the datasize is still < available so we don't need to marshal the pointer
                return 0;
            }
        }
#endif
        if (count == 0) {
            datasize = available; // :TODO: should this be capped at all?
        }
        if (p->marshal_size < datasize) {
            if (p->marshal_size) {
                free(p->marshal);
            }
            p->marshal = malloc(datasize);
            p->marshal_size = datasize;
        }
        memcopyToNative(cpu->thread, p->ptr, (S8*)p->marshal, datasize);
    } else {
        if (p->marshal_size) {
            free(p->marshal);
            p->marshal_size = 0;
        }
        p->marshal = (U8*)p->ptr;
    }
    return 1;
}

void updateVertexPointers(struct CPU* cpu, U32 count) {    
    if (cpu->thread->glVertextPointer.refreshEachCall) {        
        if (updateVertexPointer(cpu, &cpu->thread->glVertextPointer, count))
            GL_FUNC(glVertexPointer)(cpu->thread->glVertextPointer.size, cpu->thread->glVertextPointer.type, cpu->thread->glVertextPointer.stride, cpu->thread->glVertextPointer.marshal);
    }
    
    if (cpu->thread->glNormalPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glNormalPointer, count))
            GL_FUNC(glNormalPointer)(cpu->thread->glNormalPointer.type, cpu->thread->glNormalPointer.stride, cpu->thread->glNormalPointer.marshal);
    }
    
    if (cpu->thread->glFogPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glFogPointer, count)) {
            if (ext_glFogCoordPointer)
                ext_glFogCoordPointer(cpu->thread->glFogPointer.type, cpu->thread->glFogPointer.stride, cpu->thread->glFogPointer.marshal);
        }
    }

    if (cpu->thread->glFogPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glFogPointerEXT, count)) {
            if (ext_glFogCoordPointerEXT)
                ext_glFogCoordPointerEXT(cpu->thread->glFogPointerEXT.type, cpu->thread->glFogPointerEXT.stride, cpu->thread->glFogPointerEXT.marshal);
        }
    }

    if (cpu->thread->glColorPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glColorPointer, count))
            GL_FUNC(glColorPointer)(cpu->thread->glColorPointer.size, cpu->thread->glColorPointer.type, cpu->thread->glColorPointer.stride, cpu->thread->glColorPointer.marshal);
    }

    if (cpu->thread->glSecondaryColorPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointer, count)) {
            if (ext_glSecondaryColorPointer)
                ext_glSecondaryColorPointer(cpu->thread->glSecondaryColorPointer.size, cpu->thread->glSecondaryColorPointer.type, cpu->thread->glSecondaryColorPointer.stride, cpu->thread->glSecondaryColorPointer.marshal);
        }
    }

    if (cpu->thread->glSecondaryColorPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointerEXT, count)) {
            if (ext_glSecondaryColorPointerEXT)
                ext_glSecondaryColorPointerEXT(cpu->thread->glSecondaryColorPointerEXT.size, cpu->thread->glSecondaryColorPointerEXT.type, cpu->thread->glSecondaryColorPointerEXT.stride, cpu->thread->glSecondaryColorPointerEXT.marshal);
        }
    }
    
    if (cpu->thread->glIndexPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glIndexPointer, count))
            GL_FUNC(glIndexPointer)(cpu->thread->glIndexPointer.type, cpu->thread->glIndexPointer.stride, cpu->thread->glIndexPointer.marshal);
    }
    
    if (cpu->thread->glTexCoordPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glTexCoordPointer, count))
            GL_FUNC(glTexCoordPointer)(cpu->thread->glTexCoordPointer.size, cpu->thread->glTexCoordPointer.type, cpu->thread->glTexCoordPointer.stride, cpu->thread->glTexCoordPointer.marshal);
    }
    
    if (cpu->thread->glEdgeFlagPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointer, count))
            GL_FUNC(glEdgeFlagPointer)(cpu->thread->glEdgeFlagPointer.stride, cpu->thread->glEdgeFlagPointer.marshal);
    }

    if (cpu->thread->glEdgeFlagPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointerEXT, count)) {
            if (ext_glEdgeFlagPointerEXT)
                ext_glEdgeFlagPointerEXT(cpu->thread->glEdgeFlagPointerEXT.stride, cpu->thread->glEdgeFlagPointerEXT.count, cpu->thread->glEdgeFlagPointerEXT.marshal);
        }
    }
}

GLvoid* marshalVetextPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glVertextPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glVertextPointer.size = size;
        cpu->thread->glVertextPointer.type = type;
        cpu->thread->glVertextPointer.stride = stride;
        cpu->thread->glVertextPointer.ptr = ptr;
        cpu->thread->glVertextPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glVertextPointer, 0);
        return cpu->thread->glVertextPointer.marshal;
    }
}

GLvoid* marshalNormalPointer(struct CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glNormalPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glNormalPointer.size = 1;
        cpu->thread->glNormalPointer.type = type;
        cpu->thread->glNormalPointer.stride = stride;
        cpu->thread->glNormalPointer.ptr = ptr;
        cpu->thread->glNormalPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glNormalPointer, 0);
        return cpu->thread->glNormalPointer.marshal;
    }
}

GLvoid* marshalFogPointer(struct CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glFogPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glFogPointer.size = 1;
        cpu->thread->glFogPointer.type = type;
        cpu->thread->glFogPointer.stride = stride;
        cpu->thread->glFogPointer.ptr = ptr;
        cpu->thread->glFogPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glFogPointer, 0);
        return cpu->thread->glFogPointer.marshal;
    }
}

GLvoid* marshalFogPointerEXT(struct CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glFogPointerEXT.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glFogPointerEXT.size = 1;
        cpu->thread->glFogPointerEXT.type = type;
        cpu->thread->glFogPointerEXT.stride = stride;
        cpu->thread->glFogPointerEXT.ptr = ptr;
        cpu->thread->glFogPointerEXT.refreshEachCall = 0;
        updateVertexPointer(cpu, &cpu->thread->glFogPointerEXT, 0);
        return cpu->thread->glFogPointerEXT.marshal;
    }
}

GLvoid* marshalColorPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glColorPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glColorPointer.size = size;
        cpu->thread->glColorPointer.type = type;
        cpu->thread->glColorPointer.stride = stride;
        cpu->thread->glColorPointer.ptr = ptr;
        cpu->thread->glColorPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glColorPointer, 0);
        return cpu->thread->glColorPointer.marshal;
    }
}

GLvoid* marshalSecondaryColorPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glSecondaryColorPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {        
        cpu->thread->glSecondaryColorPointer.size = size;
        cpu->thread->glSecondaryColorPointer.type = type;
        cpu->thread->glSecondaryColorPointer.stride = stride;
        cpu->thread->glSecondaryColorPointer.ptr = ptr;
        cpu->thread->glSecondaryColorPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointer, 0);
        return cpu->thread->glSecondaryColorPointer.marshal;
    }
}

GLvoid* marshalSecondaryColorPointerEXT(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glSecondaryColorPointerEXT.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {        
        cpu->thread->glSecondaryColorPointerEXT.size = size;
        cpu->thread->glSecondaryColorPointerEXT.type = type;
        cpu->thread->glSecondaryColorPointerEXT.stride = stride;
        cpu->thread->glSecondaryColorPointerEXT.ptr = ptr;
        cpu->thread->glSecondaryColorPointerEXT.refreshEachCall = 0;
        updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointerEXT, 0);
        return cpu->thread->glSecondaryColorPointerEXT.marshal;
    }
}

GLvoid* marshalIndexPointer(struct CPU* cpu,  GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glIndexPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glIndexPointer.size = 1;
        cpu->thread->glIndexPointer.type = type;
        cpu->thread->glIndexPointer.stride = stride;
        cpu->thread->glIndexPointer.ptr = ptr;
        cpu->thread->glIndexPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glIndexPointer, 0);
        return cpu->thread->glIndexPointer.marshal;
    }
}

GLvoid* marshalTexCoordPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glTexCoordPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glTexCoordPointer.size = size;
        cpu->thread->glTexCoordPointer.type = type;
        cpu->thread->glTexCoordPointer.stride = stride;
        cpu->thread->glTexCoordPointer.ptr = ptr;
        cpu->thread->glTexCoordPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glTexCoordPointer, 0);
        return cpu->thread->glTexCoordPointer.marshal;
    }
}

GLvoid* marshalEdgeFlagPointer(struct CPU* cpu, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glEdgeFlagPointer.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glEdgeFlagPointer.size = 1;
        cpu->thread->glEdgeFlagPointer.type = GL_BYTE;
        cpu->thread->glEdgeFlagPointer.stride = stride;
        cpu->thread->glEdgeFlagPointer.ptr = ptr;
        cpu->thread->glEdgeFlagPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointer, 0);
        return cpu->thread->glEdgeFlagPointer.marshal;
    }
}

GLvoid* marshalEdgeFlagPointerEXT(struct CPU* cpu, GLsizei stride, GLsizei count, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glEdgeFlagPointerEXT.refreshEachCall = 0;
        return (GLvoid*)ptr;
    } else {
        cpu->thread->glEdgeFlagPointerEXT.size = 1;
        cpu->thread->glEdgeFlagPointerEXT.type = GL_BYTE;
        cpu->thread->glEdgeFlagPointerEXT.stride = stride;
        cpu->thread->glEdgeFlagPointerEXT.ptr = ptr;
        cpu->thread->glEdgeFlagPointerEXT.refreshEachCall = 0;
        cpu->thread->glEdgeFlagPointerEXT.count = count;
        updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointerEXT, 0);
        return cpu->thread->glEdgeFlagPointerEXT.marshal;
    }
}

#endif
#endif
