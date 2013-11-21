#ifndef _CHUNK_QUADTREE_TERRAIN_H_
#define _CHUNK_QUADTREE_TERRAIN_H_
#include "Terrain.h"

//#define NODE_COLOR
#define USE_TEXTURE_FLOAT

struct VertexPos{
	float x , y , z;
};

struct VertexTexCoord
{
	float s , t;
};


struct QuadTreeNode ;


/*-------------------------------------------------
This class renders terrain using chunked quadtree
algorithm and rendering method is immediate mode
-----------------------------------------*/

class ChunkQuadTreeTerrain : public Terrain 
{
public:
	/*--------------------------------------------------------------------------------------
	<textureFiles> - array of texture file names , first element is for lowest level of detail.
	Next element is for higher level of detail.Only support 24 bit RGB image
	<numTextureFiles> - equals to number of levels of detail , each texture file is for each level
	<T> is screen space error threshold.If <T> is negative , absolute value of it is used
	Note : 
		-height map must have (2^n + 1) * (2^n + 1) size (n > 1) 
		-and <numTextureFiles> mustn't be larger than n
	--------------------------------------------------------------------------------------*/
	ChunkQuadTreeTerrain(const char *heightMap , 
		const char * const * textureFiles , unsigned int numTextureFiles , 
		float T ,float cameraFOV , unsigned int viewportHeight,
		float scale , int loadMethod);
	~ChunkQuadTreeTerrain();
	
	void SetviewportHeight(unsigned int height);//should be called when viewport height has been changed
	void SetCameraFOV(float fov);//should be called when horizontal field of view has been changed
	void SetT (float T);//change screen space error threshold.If <T> is negative , absolute value of it is used
	void EnableDrawSkirt(bool enable) {this->displaySkirt = enable;}
	virtual void EnableGeoMorph(bool enable) {this->geoMorpthEnabled = enable;}
	float GetHeight(unsigned int row , unsigned int col) const;
	unsigned int GetMaxLevel() const {return maxLevel;}
	
	void Update(const HQVector4 &eye , const HQPlane viewFrustum[6]);
	void Render();
	void RenderBoundingBox();
protected:
	//this constructor can only be called in child class's constructor
	ChunkQuadTreeTerrain(const char *heightMap , 
		const char * const * textureFiles , unsigned int numTextureFiles , 
		float T ,float cameraFOV , unsigned int viewportHeight,
		const char *desc , float scale , int loadMethod);


	void Init();
	void InitData(unsigned int row, unsigned int col, float height);
	void FinishLoadHeightMap();
	void CalculateC();
	void LoadTextures(const char * const * textureFiles );
	static void CheckImageNeedScaling(Bitmap &bitmap , unsigned int numPiecesPerRow);//check if image can be splitted into equal size pieces
	static void SplitImageIntoTextures(GLuint * textures , const Bitmap &bitmap , unsigned int numTexturesPerRow);//split image into (<numTexturesPerRow> ^ 2) textures
	
	virtual void BuildTree(QuadTreeNode *node  , unsigned int level , unsigned int numTexturesPerRow);
	virtual float GetHeightInParentNode(unsigned int row , unsigned int col , unsigned int cellSize) const; 
	virtual QuadTreeNode * NewNode(QuadTreeNode *parent,unsigned int centerRow , unsigned int centerCol, unsigned int cellSize);//derived class can used this method to create derived class of QuadTreeNode
	void RecomputeD(QuadTreeNode *node);//recompute d values of all nodes in the tree
	void CalculateLocalE(QuadTreeNode *node);//caculate local error value
	void CalculateBoundingBox(QuadTreeNode *node);
	

	void UpdateNode(const HQVector4 &eye , QuadTreeNode *node);
	virtual void OnUpdateChosenNode(const HQVector4 &eye , QuadTreeNode *node) {}//derived class can do additional works in this virtual method
	
	virtual void SortChilds(QuadTreeNode *node);

	virtual void BeginRender(){}
	virtual void EndRender() {}
	void RenderNode(QuadTreeNode *node , unsigned int offset );
	void RenderNodeBoundingBox(QuadTreeNode *node);
	virtual void Draw(QuadTreeNode *node , unsigned int offset);
	virtual void DrawWithMorph(QuadTreeNode *node , unsigned int offset);

	static void DeleteTree(QuadTreeNode *node);

	inline unsigned int GetChunkIndex(unsigned int row , unsigned int col)
	{
		return row * (this->chunkSize + 1) + col;
	}
	

	static inline void RenderVertex(const VertexPos *pos , const VertexTexCoord *texcoord)
	{
		glTexCoord2fv(&texcoord->s);
		glVertex3fv(&pos->x);
	}

	static inline void RenderVertexWithMorph(bool needMorph , 
											const VertexPos *pPos , 
											const VertexTexCoord *pTexcoord,
											const float *pDisplacement , 
											float morphFactor )
	{
		glTexCoord2fv(&pTexcoord->s);
		if (!needMorph)//no need to morph
			glVertex3fv(&pPos->x);
		else
			glVertex3f(pPos->x , pPos->y + morphFactor * (*pDisplacement) , pPos->z);
	}

	QuadTreeNode *root;

	const HQPlane * viewFrustum;//6 planes of the view frustum
	unsigned int viewportHeight;
	float fov , t;
	float C;//constant used for determining level of detail
	VertexPos * pos;
	VertexTexCoord *texcoord;
	VertexTexCoord edgeTexcoordOffset;
	float *displacement;//for geomorphing
	bool *needMorph;// for geomorphing
	unsigned int chunkSize ; //number of cell in edge of chunk
	unsigned int maxLevel;//maximum level of detail
	unsigned int chunkTriangles;//number of triangles in chunk
	unsigned int chunkSkirtTriangles;//number of triangles in chunk's skirt
	GLuint **textures; //different textures for each LOD and chunk
	float ds;
	bool Cchanged ;//boolean flag indicates whether constant C has been changed or not
	bool displaySkirt;
	bool geoMorpthEnabled;
};


#endif