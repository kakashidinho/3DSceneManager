#ifndef _MODEL_H_
#define _MODEL_H_

#include <windows.h>
//openGL
#include "../Libraries/OpenGL/include/gl.h"
//3D math lib
#ifndef CMATH
#	define CMATH
#endif
#include "../Libraries/HQUtil/include/math/HQUtilMath.h"
//model loader lib
#include "../Libraries/Open Asset Import Library/include/assimp.h"
#pragma comment(lib , "assimp.lib")

struct Material
{
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	GLuint textureID;
};

#define USE_DISPLAY_LIST 0

/*---------------Model class - contains mesh's vertices & textures data-----------------*/
class Model
{
public:
	/*Limit : 
	-model file must contains only 1 mesh , 1 material and not more than 1 texture
	-number of vertices must be less than 2 ^ 16
	-texture coord is 2 dimensions
	*/
	Model (const char *modelFileName);
	~Model();
	
	const Material & GetMaterial() const {return m_Material;}
	const HQSphere & GetBoundingSphere() const {return m_BoundingSphere;}
	const HQAABB & GetBoundingBox() const {return m_BoundingBox;}
	unsigned int GetNumTriangles() const {return m_numTriangles;}

	void Render();
protected:

#if USE_DISPLAY_LIST
	void CreateDisplayList();
#endif
	void ReleaseData();


	const aiScene * m_pData;
	unsigned short *m_indices;
	unsigned int m_numIndices;
#if USE_DISPLAY_LIST
	GLuint m_displayList;
#endif
	
	Material m_Material;
	HQSphere m_BoundingSphere;
	HQAABB m_BoundingBox;
	unsigned int m_numTriangles;
};

#endif