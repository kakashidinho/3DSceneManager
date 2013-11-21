#ifndef _GL_EXT_H_
#define _GL_EXT_H_
#include <windows.h>
#include "../Libraries/OpenGL/include/gl.h"


typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef char GLchar;

/*----------clamp to edge address mode------*/
#define GL_CLAMP_TO_EDGE 0x812F

#define GL_TEXTURE_WRAP_R 0x8072

/*---------cube map constants---------------*/
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A


#endif