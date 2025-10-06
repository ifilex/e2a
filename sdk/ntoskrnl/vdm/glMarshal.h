#ifndef __GL_MARSHAL_H__
#define __GL_MARSHAL_H__

#include GLH
#include "platform.h"
#include "cpu.h"

// The reason for multiple copies of the same marshal function, like marshalf, marshal2f, marshal3f, etc is
// that each one will use a different temp buffer.  For a single OpenGL function, each marshalled pointer
// will need a different temp buffer, so a function like
//
// void glMultiDrawArrays(GLenum mode,  GLint * first,  GLsizei * count,  GLsizei primcount);
//
// will use 2 temp buffer, in this case GLsizei is mapped to GLint and thus 2 versions of marshali will
// be used

#ifdef BOXEDWINE_64BIT_MMU
#define marshald(cpu, address, count) (GLdouble*)getPhysicalAddress(cpu->thread, address)
#define marshalf(cpu, address, count) (GLfloat*)getPhysicalAddress(cpu->thread, address)
#define marshali(cpu, address, count) (GLint*)getPhysicalAddress(cpu->thread, address)
#define marshalui(cpu, address, count) (GLuint*)getPhysicalAddress(cpu->thread, address)
#define marshals(cpu, address, count) (GLshort*)getPhysicalAddress(cpu->thread, address)
#define marshalus(cpu, address, count) (GLushort*)getPhysicalAddress(cpu->thread, address)
#define marshalb(cpu, address, count) (GLbyte*)getPhysicalAddress(cpu->thread, address)
#define marshalub(cpu, address, count) (GLubyte*)getPhysicalAddress(cpu->thread, address)
#define marshalbool(cpu, address, count) (GLboolean*)getPhysicalAddress(cpu->thread, address)
#define marshal2d(cpu, address, count) (GLdouble*)getPhysicalAddress(cpu->thread, address)
#define marshal2f(cpu, address, count) (GLfloat*)getPhysicalAddress(cpu->thread, address)
#define marshal2i(cpu, address, count) (GLint*)getPhysicalAddress(cpu->thread, address)
#define marshal3i(cpu, address, count) (GLint*)getPhysicalAddress(cpu->thread, address)
#define marshal4i(cpu, address, count) (GLint*)getPhysicalAddress(cpu->thread, address)
#define marshal5i(cpu, address, count) (GLint*)getPhysicalAddress(cpu->thread, address)
#define marshal3f(cpu, address, count) (GLfloat*)getPhysicalAddress(cpu->thread, address)
#define marshal3ui(cpu, address, count) (GLuint*)getPhysicalAddress(cpu->thread, address)
#define marshali64(cpu, address, count) (GLint64*)getPhysicalAddress(cpu->thread, address)
#define marshalui64(cpu, address, count) (GLuint64*)getPhysicalAddress(cpu->thread, address)

#define marshal2ui(cpu, address, count) (GLuint*)getPhysicalAddress(cpu->thread, address)
#define marshal3ui(cpu, address, count) (GLuint*)getPhysicalAddress(cpu->thread, address)
#define marshal4ui(cpu, address, count) (GLuint*)getPhysicalAddress(cpu->thread, address)
#define marshal2s(cpu, address, count) (GLshort*)getPhysicalAddress(cpu->thread, address)
#define marshal2us(cpu, address, count) (GLushort*)getPhysicalAddress(cpu->thread, address)
#define marshal2b(cpu, address, count) (GLbyte*)getPhysicalAddress(cpu->thread, address)
#define marshal2ub(cpu, address, count) (GLubyte*)getPhysicalAddress(cpu->thread, address)
#define marshal2bool(cpu, address, count) (GLboolean*)getPhysicalAddress(cpu->thread, address)
#define marshalBackd(cpu, address, buffer, count)
#define marshalBackf(cpu, address, buffer, count)
#define marshalBacki(cpu, address, buffer, count)
#define marshalBackui(cpu, address, buffer, count)
#define marshalBackus(cpu, address, buffer, count)
#define marshalBacks(cpu, address, buffer, count)
#define marshalBackb(cpu, address, buffer, count)
#define marshalBackub(cpu, address,  buffer, count)
#define marshalBackbool(cpu, address, buffer, count)
#define marshalBackui64(cpu, address, buffer, count)
#define marshalBacki64(cpu, address, buffer, count)

#define marshalsz(cpu, address) (GLchar*)getPhysicalAddress(cpu->thread, address)

#define marshalType(cpu, type, count, address) (GLvoid*)getPhysicalAddress(cpu->thread, address)
#define marshalBackType(cpu, type, count, buffer, address)

GLvoid* marshalPixels(struct CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,  U32 pixels);
#define marshalBackPixels(cpu, is3d, width, height, depth, format, type, address, pixels)

#define marshalPixel(cpu, format, type, pixel) (GLvoid*)getPhysicalAddress(cpu->thread, pixel)

#define updateVertexPointers(cpu, count)
#define marshalVetextPointer(cpu, size, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalNormalPointer(cpu, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalColorPointer(cpu, size, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalIndexPointer(cpu, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalTexCoordPointer(cpu, size, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalEdgeFlagPointer(cpu, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalFogPointer(cpu, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalSecondaryColorPointer(cpu, size, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalSecondaryColorPointerEXT(cpu, size, type, stride, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalEdgeFlagPointerEXT(cpu, stride, count, ptr) marshalp(cpu, 0, ptr, 0)
#define marshalFogPointerEXT(cpu, type, stride, ptr) marshalp(cpu, 0, ptr, 0)

#define getDataSize(x) 1
#define components_in_format(format) 0
#define marshalGetColorTableWidth(target) 0
#define marshalGetColorTableWidthEXT(target) 0
#define marshalGetColorTableWidthSGI(target) 0
#define marshalGetCompressedMultiImageSizeEXT(texunit, target, level) 0
#define marshalGetCompressedImageSizeARB(target, level) 0
#define marshalGetCompressedImageSize(target, level) 0
#define marshalGetCompressedTextureSizeEXT(texture, target, lod) 0
#define marshalGetConvolutionWidth(target) 0
#define marshalGetConvolutionHeight(target) 0
#define marshalHistogramWidth(target)  0
#else
GLboolean* marshalbool(struct CPU* cpu, U32 address, U32 count);
GLboolean* marshal2bool(struct CPU* cpu, U32 address, U32 count);
void marshalBackbool(struct CPU* cpu, U32 address, GLboolean* buffer, U32 count);

GLbyte* marshalb(struct CPU* cpu, U32 address, U32 count);
GLbyte* marshal2b(struct CPU* cpu, U32 address, U32 count);
void marshalBackb(struct CPU* cpu, U32 address, GLubyte* buffer, U32 count);

GLubyte* marshalub(struct CPU* cpu, U32 address, U32 count);
GLubyte* marshal2ub(struct CPU* cpu, U32 address, U32 count);
void marshalBackub(struct CPU* cpu, U32 address, GLubyte* buffer, U32 count);

GLshort* marshals(struct CPU* cpu, U32 address, U32 count);
GLshort* marshal2s(struct CPU* cpu, U32 address, U32 count);
void marshalBacks(struct CPU* cpu, U32 address, GLshort* buffer, U32 count);

GLushort* marshalus(struct CPU* cpu, U32 address, U32 count);
GLushort* marshal2us(struct CPU* cpu, U32 address, U32 count);
void marshalBackus(struct CPU* cpu, U32 address, GLushort* buffer, U32 count);

GLint* marshali(struct CPU* cpu, U32 address, U32 count);
GLint* marshal2i(struct CPU* cpu, U32 address, U32 count);
GLint* marshal3i(struct CPU* cpu, U32 address, U32 count);
GLint* marshal4i(struct CPU* cpu, U32 address, U32 count);
GLint* marshal5i(struct CPU* cpu, U32 address, U32 count);
void marshalBacki(struct CPU* cpu, U32 address, GLint* buffer, U32 count);

GLuint* marshalui(struct CPU* cpu, U32 address, U32 count);
GLuint* marshal2ui(struct CPU* cpu, U32 address, U32 count);
GLuint* marshal3ui(struct CPU* cpu, U32 address, U32 count);
GLuint* marshal4ui(struct CPU* cpu, U32 address, U32 count);
GLuint* marshal5ui(struct CPU* cpu, U32 address, U32 count);
void marshalBackui(struct CPU* cpu, U32 address, GLuint* buffer, U32 count);

GLfloat* marshalf(struct CPU* cpu, U32 address, U32 count);
GLfloat* marshal2f(struct CPU* cpu, U32 address, U32 count);
GLfloat* marshal3f(struct CPU* cpu, U32 address, U32 count);
GLfloat* marshal4f(struct CPU* cpu, U32 address, U32 count);
void marshalBackf(struct CPU* cpu, U32 address, GLfloat* buffer, U32 count);

GLdouble* marshald(struct CPU* cpu, U32 address, U32 count);
GLdouble* marshal2d(struct CPU* cpu, U32 address, U32 count);
void marshalBackd(struct CPU* cpu, U32 address, GLdouble* buffer, U32 count);

GLint64* marshali64(struct CPU* cpu, U32 address, U32 count);
void marshalBacki64(struct CPU* cpu, U32 address, GLint64* buffer, U32 count);

GLuint64* marshalui64(struct CPU* cpu, U32 address, U32 count);
void marshalBackui64(struct CPU* cpu, U32 address, GLuint64* buffer, U32 count);

const GLchar* marshalsz(struct CPU* cpu, U32 address);

// type can be GL_UNSIGNED_BYTE, GL_BYTE, GL_2_BYTES, GL_UNSIGNED_SHORT, GL_SHORT, GL_3_BYTES, 
// GL_4_BYTES, GL_FLOAT, GL_UNSIGNED_INT, GL_INT
//
// base on the type, the correct marshal function will be called
GLvoid* marshalType(struct CPU* cpu, U32 type, U32 count, U32 address);
void marshalBackType(struct CPU* cpu, U32 type, U32 count, GLvoid* buffer, U32 address);

// will call the correct marshal function based on the type the count passed to the marshal
// function will be set to the correct number to include all the data for a single pixel for the format
GLvoid* marshalPixel(struct CPU* cpu, GLenum format, GLenum type, U32 pixel);

// Used by glBitmap, glDrawPixels, glTexImage1D, glTexImage2D, glTexSubImage1D, glTexSubImage2D
//
// This will take into account packing, like GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_PIXELS, GL_UNPACK_SKIP_ROWS,
// GL_UNPACK_ALIGNMENT, GL_PACK_SKIP_IMAGES
GLvoid* marshalPixels(struct CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,  U32 pixels);
void marshalBackPixels(struct CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, U32 address, GLvoid* pixels);

void updateVertexPointers(struct CPU* cpu, U32 count);
GLvoid* marshalVetextPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalNormalPointer(struct CPU* cpu, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalColorPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalIndexPointer(struct CPU* cpu,  GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalTexCoordPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalEdgeFlagPointer(struct CPU* cpu, GLsizei stride, U32 ptr);
GLvoid* marshalEdgeFlagPointerEXT(struct CPU* cpu, GLsizei stride, GLsizei count, U32 ptr);
GLvoid* marshalSecondaryColorPointer(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalSecondaryColorPointerEXT(struct CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalFogPointer(struct CPU* cpu, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalFogPointerEXT(struct CPU* cpu, GLenum type, GLsizei stride, U32 ptr);

U32 getDataSize(GLenum type);

U32 marshalGetColorTableWidth(U32 target);
U32 marshalGetColorTableWidthEXT(U32 target);
U32 marshalGetColorTableWidthSGI(U32 target);
U32 marshalGetCompressedImageSize(GLenum target, GLint level);
U32 marshalGetCompressedImageSizeARB(GLenum target, GLint level);
U32 marshalGetCompressedMultiImageSizeEXT(GLenum texunit, GLenum target, GLint level);
U32 marshalGetCompressedTextureSizeEXT(GLuint texture, GLenum target, GLint lod);
U32 marshalGetConvolutionWidth(U32 target);
U32 marshalGetConvolutionHeight(U32 target);
GLint components_in_format(GLenum format );
GLsizei marshalHistogramWidth(GLenum target);
#endif

GLintptr* marshalip(struct CPU* cpu, U32 address, U32 count);
GLintptr* marshal2ip(struct CPU* cpu, U32 address, U32 count);

GLsizeiptr* marshalsip(struct CPU* cpu, U32 address, U32 count);

// GLhandleARB is a U32 on Win32, but on Mac it is a void*
GLhandleARB* marshalhandle(struct CPU* cpu, U32 address, U32 count);
void marshalBackhandle(struct CPU* cpu, U32 address, GLhandleARB* buffer, U32 count);

GLsync marshalSync(struct CPU* cpu, U32 sync);
U32 marshalBackSync(struct CPU* cpu, GLsync sync);

GLvoid* marshalp(struct CPU* cpu, U32 instance, U32 buffer, U32 len);
U32 marshalBackp(struct CPU* cpu, GLvoid* buffer, U32 size);

GLvoid** marshalpp(struct CPU* cpu, U32 buffer, U32 count, U32 sizes, S32 bytesPerCount);

void* marshalDrawArraysIndirectBindlessCommandNV(struct CPU* cpu, U32 address, U32 count, U32 stride, U32 vertexCount);

void* marshalunhandled(const char* func, const char* param, struct CPU* cpu, U32 address);

// These functions help calculate the count that will be passed to various marshal function
int getSize(GLenum pname);
GLsizei floatPerTransformList(GLenum transformType);
U32 marshalGetActiveAtomicCountersCount(U32 program, U32 bufferIndex);
U32 marshalGetCompatibleSubroutinesCount(U32 program, U32 shadertype, U32 index);
U32 marshalGetUniformBlockActiveUnformsCount(U32 program, U32 uniformBlockIndex);
int glcommon_glLightv_size(GLenum e);
int glcommon_glLightModelv_size(GLenum e);
int glcommon_glMaterialv_size(GLenum e);
U32 getMap1Count(GLenum target);
U32 getMap2Count(GLenum target);
GLint glcommon_glGetPixelMap_size(GLenum map);
GLint get_bytes_per_pixel(GLenum format, GLenum type);
GLint marshalGet(GLenum param);
GLboolean PIXEL_PACK_BUFFER();
GLboolean ARRAY_BUFFER();
GLboolean ELEMENT_ARRAY_BUFFER();
GLboolean PIXEL_UNPACK_BUFFER();
void OPENGL_CALL_TYPE debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

#endif