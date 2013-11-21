#ifndef MODE_2D_H
#define MODE_2D_H

#include "glHelper.h"

#ifndef CMATH
#	define CMATH
#endif
#include "../Libraries/HQUtil/include/math/HQUtilMath.h"

class Mode2D
{
public:
	static void Begin();
	static void End();

	static void DrawRect(const HQRect & rect , GLuint texture);
private:
	struct StaticStuff{
		StaticStuff();
		
		HQMatrix4 _2DProjMatrix, _2DViewMatrix;

	};

	static StaticStuff staticStuff;
};

#endif