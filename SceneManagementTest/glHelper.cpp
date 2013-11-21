#include "glHelper.h"
#include <stdio.h>
#include <string.h>
bool texClampEdge = false;
bool cubeMapSupported = false;

/*-------prototype---------------*/
void DefineTexture(GLuint texture , const unsigned char *pData, unsigned int width , unsigned int height , unsigned int unpackRowLength , GLenum format, bool wrapMode);

/*-------definition---------*/

bool glCheckExtension(const char* ext , const GLubyte *extListStr)
{
	const unsigned char* loc = (const unsigned char*)strstr((const char*)extListStr , ext);
	if(loc == NULL)
		return false;
	if (loc != extListStr) {
		unsigned char pre = *(loc - 1);
		if (pre != ' ')
			return false;
	}		
	unsigned char post = *(loc + strlen((const char*)ext));
	if(post != ' ' && post != '\0')
		return false;
	return true;
}

bool glCheckExtension(const GLubyte* ext , const GLubyte *extListStr)
{
	return glCheckExtension((const char*)ext , extListStr);
}

void InitExtensions()
{
	const GLubyte* glStr = glGetString(GL_VERSION);
	const GLubyte* glExtListStr = glGetString(GL_EXTENSIONS);//list of extensions
	float versionf;
	int re = sscanf((const char*)glStr , "%f" , &versionf);
	if (re != 1)
		return ; 
	
	texClampEdge = versionf >= 1.2f;
	cubeMapSupported = versionf >= 1.3f || 
		glCheckExtension("GL_ARB_texture_cube_map" , glExtListStr) ||
		glCheckExtension("GL_EXT_texture_cube_map" , glExtListStr) ;
}


GLuint LoadTexture(const char *fileName , ImgOrigin imageOrigin, bool wrapMode)
{
	
	Bitmap bitmap;
	if (bitmap.Load(fileName) != IMG_OK)
		return 0;
	bitmap.SetPixelOrigin(imageOrigin);
	bitmap.DeCompressDXT();
	bitmap.FlipRGB();

	GLuint texture;
	glGenTextures(1,&texture);     
	
	DefineTexture(texture , bitmap.GetPixelData(),
		bitmap.GetWidth() , bitmap.GetHeight() , 0 ,
		(bitmap.GetBits() == 24)? GL_RGB : GL_RGBA , wrapMode);

	return texture;
}

 void DefineTextureRGB(GLuint texture , const unsigned char *pData, unsigned int width , unsigned int height , unsigned int unpackRowLength, bool wrapMode)
 { 
	 return DefineTexture(texture , pData , width , height , unpackRowLength , GL_RGB , wrapMode);
 }

 void DefineTexture(GLuint texture , const unsigned char *pData, unsigned int width , unsigned int height , unsigned int unpackRowLength , GLenum format , bool wrapMode)
 { 
	GLint addressMode;
	GLenum internalFormat = GL_RGB8;
	
	if (wrapMode)
		addressMode = GL_REPEAT;
	else
	{
		if (texClampEdge)
			addressMode = GL_CLAMP_TO_EDGE;
		else
			addressMode = GL_CLAMP;
	}
	switch(format)
	{
	case GL_RGB:
		internalFormat = GL_RGB8;
	case GL_RGBA:
		internalFormat = GL_RGBA;
	}

	glBindTexture(GL_TEXTURE_2D,texture); 
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,addressMode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,addressMode);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei( GL_UNPACK_ROW_LENGTH , unpackRowLength);

	glTexImage2D(GL_TEXTURE_2D,0,internalFormat, width , height ,0
		,format,GL_UNSIGNED_BYTE,pData);

	glPixelStorei( GL_UNPACK_ROW_LENGTH , 0);
 }