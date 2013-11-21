#ifndef SKY_DOME_H
#define SKY_DOME_H

//openGL
#include "glHelper.h"
//3D math lib
#ifndef CMATH
#	define CMATH
#endif
#include "../Libraries/HQUtil/include/math/HQUtilMath.h"

/*--------this class is used for rendering the sky--------*/
class SkyDome
{
public:
	//note : only support 24/32 bit images
	SkyDome(float zFar , unsigned int numSlide , unsigned int numStack ,
			const char *posXImage,
			const char *negXImage,
			const char *posYImage,
			const char *negYImage,
			const char *posZImage,
			const char *negZImage);

	~SkyDome();

	void Render(const HQVector4 &eyePos);
private:
	void LoadTexture(const char * images[6]);
	void CreateSphere(unsigned int numSlice , unsigned int numStack);
	void CreateDisplayList();
	void ReleaseData();

	GLuint displayList;
	GLuint skyBoxTexture;
	float sphereRadius;
	HQFloat3 *vertices;//vertices in sphere mesh
	unsigned short *indices;
	unsigned int numIndices;
};

#endif