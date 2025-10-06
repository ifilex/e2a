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

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
#include GLH
#include "glcommon.h"
#include "glMarshal.h"
#include "kalloc.h"
#include "kprocess.h"
#include "log.h"

#ifdef BOXEDWINE_64BIT_MMU
#include "../emulation/hardmmu/hard_memory.h"
#endif

float fARG(struct CPU* cpu, U32 arg) {
    struct int2Float i;
    i.i = arg;
    return i.f;
}

double dARG(struct CPU* cpu, int address) {
    struct long2Double i;
    i.l = readq(cpu->thread, address);
    return i.d;
}

// GLintptr is 32-bit in the emulator, on the host it depends on void* size
GLintptr* bufferip;
U32 bufferip_len;

GLintptr* marshalip(struct CPU* cpu, U32 address, U32 count) {
    U32 i;

    if (!address)
        return NULL;
    if (bufferip && bufferip_len<count) {
        kfree(bufferip, KALLOC_OPENGL);
        bufferip=0;
    }
    if (!bufferip) {
        bufferip = (GLintptr*)kalloc(sizeof(GLintptr)*count, KALLOC_OPENGL);
        bufferip_len = count;
    }
    for (i=0;i<count;i++) {
        bufferip[i] = (GLintptr)readd(cpu->thread, address);
        address+=4;
    }
    return bufferip;
}

GLintptr* buffer2ip;
U32 buffer2ip_len;

GLintptr* marshal2ip(struct CPU* cpu, U32 address, U32 count) {
    U32 i;

    if (!address)
        return NULL;
    if (buffer2ip && buffer2ip_len<count) {
        kfree(buffer2ip, KALLOC_OPENGL);
        buffer2ip=0;
    }
    if (!buffer2ip) {
        buffer2ip = (GLintptr*)kalloc(sizeof(GLintptr)*count, KALLOC_OPENGL);
        buffer2ip_len = count;
    }
    for (i=0;i<count;i++) {
        buffer2ip[i] = (GLintptr)readd(cpu->thread, address);
        address+=4;
    }
    return buffer2ip;
}

static char* extentions[] = {
#include "glfunctions_ext_def.h"
};

// const GLubyte *glGetStringi(GLenum name, GLuint index);
void glcommon_glGetStringi(struct CPU* cpu) { 
    if (!ext_glGetStringi)
        kpanic("ext_glGetStringi is NULL");
    {
    const GLubyte* result = GL_FUNC(ext_glGetStringi)(ARG1, ARG2);
    if (result) {
        EAX = mapNativeMemory(cpu->memory, (void*)result, (U32)strlen(result)+1);
    } else {
        EAX = 0;
    }
    GL_LOG ("glGetStringi GLenum name=%d GLuint index=%d", ARG1,ARG2);
    }
}

// GLAPI const GLubyte* APIENTRY glGetString( GLenum name ) {
void glcommon_glGetString(struct CPU* cpu) {
    U32 name = ARG1;
    U32 index = 0;
    const char* result = (const char*)GL_FUNC(glGetString)(name);
    
    if (name == GL_VENDOR) {
        index = STRING_GL_VENDOR;
        GL_LOG("glGetString GLenum name=STRING_GL_VENDOR ret=%s", result);
    } else if (name == GL_RENDERER) {
        index = STRING_GL_RENDERER;
        GL_LOG("glGetString GLenum name=GL_RENDERER ret=%s", result);
    } else if (name == GL_VERSION) {
        index = STRING_GL_VERSION;
        result = "1.2 BoxedWine";
        GL_LOG("glGetString GLenum name=STRING_GL_VERSION ret=%s", result);
    } else if (name == GL_SHADING_LANGUAGE_VERSION) {
        index = STRING_GL_SHADING_LANGUAGE_VERSION;
        GL_LOG("glGetString GLenum name=GL_SHADING_LANGUAGE_VERSION ret=%s", result);
    } else if (name == GL_EXTENSIONS) {
        static char ext[8192]={0};
        index = STRING_GL_EXTENSIONS;
        if (ext[0]==0) {
            int i;

            for (i=0;i<sizeof(extentions)/sizeof(char*);i++) {
                if (strstr(result, extentions[i])) {
                    if (ext[0]!=0)
                        strcat(ext, " ");
                    strcat(ext, extentions[i]);
                }
            }
        }
        result = "GL_EXT_texture3D";
        GL_LOG("glGetString GLenum name=GL_EXTENSIONS ret=%s", result);
    }
    if (!cpu->thread->process->strings[index])
        addString(cpu->thread, index, result);
    EAX = cpu->thread->process->strings[index];
}

// GLAPI void APIENTRY glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ) {
void glcommon_glGetTexImage(struct CPU* cpu) {
    GLenum target = ARG1;
    GLint level = ARG2;
    GLsizei width;
    GLsizei height;
    GLsizei depth = 1;
    GLenum format = ARG3;
    GLenum type = ARG4;

    GLvoid* pixels;
    GLboolean b = PIXEL_PACK_BUFFER();

    //GL_LOG("glGetTexImage GLenum target=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, GLvoid *pixels=%.08x", ARG1, ARG2, ARG3, ARG4, ARG5);
    if (b) {
        pixels = (GLvoid*)ARG5;
    } else {
        GL_FUNC(glGetTexLevelParameteriv)(target, level, GL_TEXTURE_WIDTH, &width);
        GL_FUNC(glGetTexLevelParameteriv)(target, level, GL_TEXTURE_HEIGHT, &height);
        pixels = marshalPixels(cpu, target == GL_TEXTURE_3D, width, height, 1, format, type, ARG5);
    }
    GL_FUNC(glGetTexImage)(target, level, format, type, pixels);
    if (!b)
        marshalBackPixels(cpu, target == GL_TEXTURE_3D, width, height, 1, format, type, ARG5, pixels);
}

U32 isMap2(GLenum target) {
    switch (target) {
    case GL_MAP2_INDEX:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_VERTEX_3: 
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_VERTEX_4:
    case GL_MAP2_COLOR_4:
    case GL_MAP2_TEXTURE_COORD_4:
        return 1;
    }
    return 2;
}

// GLAPI void APIENTRY glGetMapdv( GLenum target, GLenum query, GLdouble *v ) {
void glcommon_glGetMapdv(struct CPU* cpu) {
    GLenum target = ARG1;
    GLenum query = ARG2;
    
    GL_LOG("glGetMapdv GLenum target=%d, GLenum query=%d, GLdouble *v=%.08x", ARG1, ARG2, ARG3);

    switch (query) {
    case GL_COEFF: {
        GLdouble* buffer;
        GLint order[2];
        int count;

        GL_FUNC(glGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        buffer = marshald(cpu, ARG3, count);
        GL_FUNC(glGetMapdv)(target, query, buffer);
        marshalBackd(cpu, ARG3, buffer, count);
        break;
    }
    case GL_ORDER: {
        GLdouble buffer[2];
        GL_FUNC(glGetMapdv)(target, query, buffer);
        marshalBackd(cpu, ARG3, buffer, isMap2(target)?2:1);
    }
    case GL_DOMAIN: {
        GLdouble buffer[4];
        GL_FUNC(glGetMapdv)(target, query, buffer);
        marshalBackd(cpu, ARG3, buffer, isMap2(target)?4:2);
        break;
    }
    default:
        kpanic("glGetMapdv unknown query: %d", query);
    }	
}

// GLAPI void APIENTRY glGetMapfv( GLenum target, GLenum query, GLfloat *v ) {
void glcommon_glGetMapfv(struct CPU* cpu) {
    GLenum target = ARG1;
    GLenum query = ARG2;
    
    GL_LOG("glGetMapfv GLenum target=%d, GLenum query=%d, GLfloat *v=%.08x", ARG1, ARG2, ARG3);
    switch (query) {
    case GL_COEFF: {
        GLfloat* buffer;
        GLint order[2];
        int count;

        GL_FUNC(glGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        buffer = marshalf(cpu, ARG3, count);
        GL_FUNC(glGetMapfv)(target, query, buffer);
        marshalBackf(cpu, ARG3, buffer, count);
        break;
    }
    case GL_ORDER: {
        GLfloat buffer[2];
        GL_FUNC(glGetMapfv)(target, query, buffer);
        marshalBackf(cpu, ARG3, buffer, isMap2(target)?2:1);
    }
    case GL_DOMAIN: {
        GLfloat buffer[4];
        GL_FUNC(glGetMapfv)(target, query, buffer);
        marshalBackf(cpu, ARG3, buffer, isMap2(target)?4:2);
        break;
    }
    default:
        kpanic("glGetMapfv unknown query: %d", query);
    }	
}

// GLAPI void APIENTRY glGetMapiv( GLenum target, GLenum query, GLint *v ) {
void glcommon_glGetMapiv(struct CPU* cpu) {
    GLenum target = ARG1;
    GLenum query = ARG2;
    
    GL_LOG("glGetMapiv GLenum target=%d, GLenum query=%d, GLint *v=%.08x", ARG1, ARG2, ARG3);
    switch (query) {
    case GL_COEFF: {
        GLint* buffer;
        GLint order[2];
        int count;

        GL_FUNC(glGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        buffer = marshali(cpu, ARG3, count);
        GL_FUNC(glGetMapiv)(target, query, buffer);
        marshalBacki(cpu, ARG3, buffer, count);
        break;
    }
    case GL_ORDER: {
        GLint buffer[2];
        GL_FUNC(glGetMapiv)(target, query, buffer);
        marshalBacki(cpu, ARG3, buffer, isMap2(target)?2:1);
    }
    case GL_DOMAIN: {
        GLint buffer[4];
        GL_FUNC(glGetMapiv)(target, query, buffer);
        marshalBacki(cpu, ARG3, buffer, isMap2(target)?4:2);
        break;
    }
    default:
        kpanic("glGetMapfv unknown query: %d", query);
    }	
}

// GLAPI void APIENTRY glGetPointerv( GLenum pname, GLvoid **params ) {
void glcommon_glGetPointerv(struct CPU* cpu) {
    GL_LOG("glGetPointerv GLenum pname=%d, GLvoid **params=%.08x", ARG1, ARG2);
#ifdef BOXEDWINE_64BIT_MMU
    {
        GLvoid* params;
        GL_FUNC(glGetPointerv)(ARG1, &params);
        writed(cpu->thread, ARG2, getHostAddress(cpu->thread, params));
    }
#else
    switch (ARG1) {
    case GL_COLOR_ARRAY_POINTER: writed(cpu->thread, readd(cpu->thread, ARG2), cpu->thread->glColorPointer.ptr); break;
    case GL_EDGE_FLAG_ARRAY_POINTER: writed(cpu->thread, readd(cpu->thread, ARG2), cpu->thread->glEdgeFlagPointer.ptr); break;
    case GL_INDEX_ARRAY_POINTER: writed(cpu->thread, readd(cpu->thread, ARG2), cpu->thread->glIndexPointer.ptr); break;
    case GL_NORMAL_ARRAY_POINTER: writed(cpu->thread, readd(cpu->thread, ARG2), cpu->thread->glNormalPointer.ptr); break;
    case GL_TEXTURE_COORD_ARRAY_POINTER: writed(cpu->thread, readd(cpu->thread, ARG2), cpu->thread->glTexCoordPointer.ptr); break;
    case GL_VERTEX_ARRAY_POINTER: writed(cpu->thread, readd(cpu->thread, ARG2), cpu->thread->glVertextPointer.ptr); break;
    default: writed(cpu->thread, readd(cpu->thread, ARG2), 0);
    }
#endif
}

// GLAPI void APIENTRY glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ) {
void glcommon_glInterleavedArrays(struct CPU* cpu) {
    kpanic("glInterleavedArrays no supported");
}

// GLAPI void APIENTRY glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) {
void glcommon_glReadPixels(struct CPU* cpu) {
    GLvoid* pixels;
    GLsizei width = ARG3;
    GLsizei height = ARG4;
    GLenum format = ARG5;
    GLenum type = ARG6;
    GLboolean b = PIXEL_PACK_BUFFER();

    GL_LOG("glReadPixels GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, GLvoid *pixels=%.08x", ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);

    if (b)
        pixels = (GLvoid*)ARG7;
    else
        pixels = marshalPixels(cpu, 0, width, height, 1, format, type, ARG7);
    GL_FUNC(glReadPixels)(ARG1, ARG2, width, height, format, type, pixels);
    if (!b)
        marshalBackPixels(cpu, 0, width, height, 1, format, type, ARG7, pixels);
}

void OPENGL_CALL_TYPE debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    klog("%s", message);
}

void glcommon_glSamplePass(struct CPU* cpu) {
    if (!ext_glSamplePass)
        kpanic("ext_glSamplePass is NULL");
    {
    GL_FUNC(ext_glSamplePass)(ARG1);
    GL_LOG ("SamplePass GLenum pass=%d", ARG1);
    }
}

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) void glcommon_gl##func(struct CPU* cpu) { PRE GL_FUNC(gl##func)ARGS; POST} 

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) void glcommon_gl##func(struct CPU* cpu);

#include "glfunctions.h"

Int99Callback gl_callback[GL_FUNC_COUNT];

void esgl_init();
void sdlgl_init();
void gl_init() {    
    int99Callback=gl_callback;
    int99CallbackSize=GL_FUNC_COUNT;

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) gl_callback[func] = glcommon_gl##func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) gl_callback[func] = glcommon_gl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) gl_callback[func] = glcommon_gl##func;

#include "glfunctions.h"
       
#ifdef BOXEDWINE_OPENGL_SDL
    sdlgl_init();
#endif
#ifdef BOXEDWINE_ES
    esgl_init();
#endif        
}

#else
#include "cpu.h"
void gl_init() {
    int99CallbackSize=0;
}
#endif
