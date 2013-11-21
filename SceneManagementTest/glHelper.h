#ifndef _GL_HELPER_H_
#define _GL_HELPER_H_

#include "glExtension.h"

//image loader lib
#include "../Libraries/ImagesLoader/include/ImgLoader.h"
#pragma comment(lib,"../Libraries/ImagesLoader/lib/ImagesLoader.lib")

/*------------------------------------------*/
extern bool texClampEdge;//is texture clamp to edge address mode supported by OpenGL?
extern bool cubeMapSupported;//is cube map supported by OpenGL?

void InitExtensions();
GLuint LoadTexture(const char *fileName , ImgOrigin imageOrigin = ORIGIN_TOP_LEFT , bool wrapMode = false);//only accept 24 & 32 bit or DXT5 compressed images
void DefineTextureRGB(GLuint texture , const unsigned char *pData, 
				   unsigned int width , unsigned int height ,unsigned int unpackRowLength = 0, bool wrapMode = false);

#endif