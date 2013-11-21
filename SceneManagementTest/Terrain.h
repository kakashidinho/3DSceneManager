#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <windows.h>
#include <stdio.h>

#include "Common.h"

//openGL lib
#include "glHelper.h"
#include "../Libraries/OpenGL/include/gl.h"
#pragma comment(lib, "../Libraries/OpenGL/lib/opengl32.lib")

//3D math lib
#ifndef CMATH
#	define CMATH
#endif
#include "../Libraries/HQUtil/include/math/HQUtilMath.h"
#if defined DEBUG || defined _DEBUG
#pragma comment(lib , "../Libraries/HQUtil/lib/HQUtilMathD.lib")
#else
#pragma comment(lib , "../Libraries/HQUtil/lib/HQUtilMath.lib")
#endif


#ifndef SafeDelete
#define SafeDelete(p) {if (p != NULL) {delete p ; p = NULL ; }}
#define SafeDeleteArray(p) {if (p != NULL) {delete[] p ; p = NULL ; }}
#endif

#define INVALID_HEIGHT -9999.f

struct Vertex
{
	float x, y ,z;//position
	float s ,t ;//texture coords
};

struct RelativePostionInfo
{
	unsigned int row, col;
	float dx , dz;
};

class Terrain
{
public:
	virtual ~Terrain() ;
	const char *GetDesc() {return desc;}//get description
	virtual bool GetRelativePosInfo(float x , float z , RelativePostionInfo &infoOut) const;//get info of the surface on which the point (x , z) is
	virtual float GetHeight(float x , float z) const ;//get height in terrain at point (x , z)
	virtual float GetHeight(const RelativePostionInfo &info) const;
	virtual bool GetSurfaceNormal(float x , float z , HQVector4& normalOut)const;//get normal vector of the surface on which the point (x , z) is
	virtual void GetSurfaceNormal(const RelativePostionInfo &info , HQVector4& normalOut) const;
	virtual float GetHeight(unsigned int row , unsigned int col) const = 0;//get height of vertex in terrain grid
	virtual void Update(const HQVector4 & eye , const HQPlane viewFrustum[6]) {};
	virtual void Render() = 0;
	float GetStartX() const {return startX;}
	float GetStartZ() const {return startZ;}
	float GetMaxX() const {return maxX;}
	float GetMaxZ() const {return maxZ;}
	float GetCellSizeX() const {return cellSizeX;}
	float GetCellSizeZ() const {return cellSizeZ;}
	float GetMaxHeight() const {return maxHeight;}
	float GetMinHeight() const {return minHeight;}
	unsigned int GetNumVertX() const {return numVertX;}
	unsigned int GetNumVertZ() const {return numVertZ;}
	unsigned int GetNumTriangles() const {return numTriangles;}//get number of visible triangles

protected:
	Terrain(const char *desc , float scale) ;
	void SetDesc(const char *desc);//set description string
	inline unsigned int GetIndex(unsigned int row , unsigned int col) const
	{
		return row * this->numVertX + col;
	}

	bool LoadHeightMap(const char *fileName  , int loadMethod);
	bool LoadHeightMapFromTxt(const char *fileName); 
	bool LoadHeightMapFromBmp(const char *fileName);
	/*-------these 3  implement dependent worker methods will be called in LoadHeightMap() method------------*/
	virtual void Init() = 0;//init stuffs when number of vertices in x and z direction are known and just before height is assigned to each point in terrain grid
	virtual void InitData(unsigned int row, unsigned int col, float height) = 0;//set height to point at location <row> , <col>.<row> is index in z direction , <col> is index in x direction of terrain grid.
	virtual void FinishLoadHeightMap() {}//this will be called just before LoadHeightMap() method returns
	/*-------------------------------------------------*/
	float startX , startZ;
	unsigned int numVertX , numVertZ;//number of vertices in x and z direction respectively
	float cellSizeX , cellSizeZ;//distance between two vertices in x and z direction respectively
	float maxX , maxZ;
	float maxHeight , minHeight;
	float scale;
	unsigned int numTriangles;
	
	char *desc ; //description
};

/*-----------------------------------------
this class renders terrain using brute force
approach.
Render using immediate mode
------------------------------------------*/
class SimpleTerrain: public Terrain
{
public:
	SimpleTerrain(const char *heightMap , const char *texture , float scale , int loadMethod);
	~SimpleTerrain();
	
	float GetHeight(unsigned int row , unsigned int col) const ;

	void Render();
protected:
	SimpleTerrain(const char *heightMap , const char *textureFile , const char *desc , float scale , int loadMethod);
	void Init();
	void InitData(unsigned int row, unsigned int col, float height);
	inline Vertex * GetVertex(unsigned int row, unsigned int col)
	{
		return vertices + (row * this->numVertX + col);
	}
	
	inline const Vertex * GetVertex(unsigned int row, unsigned int col) const
	{
		return vertices + (row * this->numVertX + col);
	}

	static inline void RenderVertex(const Vertex *vertex)
	{
		glTexCoord2fv(&vertex->s);
		glVertex3fv(&vertex->x);
	}

	float ds , dt;
	Vertex * vertices;
	GLuint texture;
};


/*-----------------------------------------
this class renders terrain using brute force
approach.
Render using immediate mode and triangle fan
primitive mode
------------------------------------------*/
class SimpleTerrainTriFan : public SimpleTerrain
{
public:
	SimpleTerrainTriFan(const char *heightMap , const char *textureFile , float scale , int loadMethod);
	~SimpleTerrainTriFan() {}
	void Render();
};
/*-----------------------------------------
this class renders terrain using brute force
approach.
Render using immediate mode and triangle strip
primitive mode
------------------------------------------*/
class SimpleTerrainTriStrip : public SimpleTerrain
{
public:
	SimpleTerrainTriStrip(const char *heightMap , const char *textureFile,float scale , int loadMethod);
	~SimpleTerrainTriStrip() {}
	void Render();
};

#endif