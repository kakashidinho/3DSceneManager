#include "ChunkQuadTreeTerrain.h"
#include <float.h>
#include <assert.h>
#include <stdlib.h>

#define SQUARE_LEN

#define NW_Node 0
#define NE_Node 1
#define SE_Node 2
#define SW_Node 3

#define RENDER 0x1
#define HIGHER_LOD 0x2
#define RENDER_SKIRT 0x4

#define SKIRT_HEIGHT this->GetMinHeight()

/*-------------QuadTreeNode------------------*/

struct QuadTreeNode 
{
	QuadTreeNode(unsigned int centerRow , 
				unsigned int centerCol, 
				unsigned int cellSize);
	virtual ~QuadTreeNode() {};
	
	void CalculateD(float C);

	HQAABB boundingBox;
	GLuint texture;//texture used for this node
	float centerX , centerZ;//absolute location of center point of this chunk
	unsigned int centerRow , centerCol;//location of center point of this chunk (calculated in row and column)
	unsigned int cellSize;//cellSize = 1 => leaf node , highest level of detail.parent has double <cellSize>
	float e;//world space error which is used for determining level of detail
	float d;//d = e * C(an constant of ChunkQuadTreeTerrain class).d needs to be recomputed when C has been changed
	float distToEye;//current distance to camera
	float morphConst;//precalculated value.<morphConst> = 1.0f / (parent->d - this->d)
	float morphFactor;
	unsigned int renderFlags;
#ifdef NODE_COLOR
	unsigned int wirecolor;
#endif
	QuadTreeNode * childs[4];
};

QuadTreeNode::QuadTreeNode(unsigned int centerRow , unsigned int centerCol, unsigned int cellSize)
{
	for (int i = 0; i < 4 ; ++i)
		this->childs[i] = NULL;

	this->centerRow = centerRow;
	this->centerCol = centerCol;
	this->cellSize = cellSize;

	this->morphConst = 0.0f;

	this->renderFlags = 0;
}

void QuadTreeNode::CalculateD(float C)
{
	this->d = this->e * C;
#ifdef SQUARE_LEN
	this->d *= this->d ;// squared value
#endif
}

/*-----------------------------------
ChunkQuadTreeTerrain
------------------------------------*/
ChunkQuadTreeTerrain::ChunkQuadTreeTerrain(const char *heightMap , 
		const char * const * textureFiles , unsigned int numTextureFiles , 
		float T ,float cameraFOV , unsigned int viewportHeight,
		float scale , int loadMethod)
		:Terrain("Terrain rendering using chunked quadtree" , scale)
{
	this->pos = NULL;
	this->texcoord = NULL;
	this->textures = NULL;
	this->displacement = NULL;
	this->needMorph = NULL;
	this->Cchanged = false;
	this->displaySkirt = true;
	this->geoMorpthEnabled = false;
	/*--------calculate constant C ------------*/
	this->fov = cameraFOV;
	this->t = fabs(T);
	this->viewportHeight = viewportHeight;
	this->CalculateC();
	
	/*----load textures------------------------*/
	this->maxLevel = numTextureFiles - 1;
	this->LoadTextures(textureFiles);

	/*------load heightmap--------------------*/
	this->LoadHeightMap(heightMap , loadMethod);

	/*------create displacement array----------*/
	this->displacement = new float[this->numVertX * this->numVertZ];
	memset(this->displacement , 0 , this->numVertX * this->numVertZ * sizeof(float));
	
	/*---------buildTree----------------------*/
	this->root = this->NewNode(NULL ,
							   this->numVertZ / 2 ,
							   this->numVertX / 2,
							   (this->numVertX - 1) / this->chunkSize );
	this->BuildTree(root , 0 ,1 );
}

ChunkQuadTreeTerrain::ChunkQuadTreeTerrain(const char *heightMap , 
		const char * const * textureFiles , unsigned int numTextureFiles , 
		float T ,float cameraFOV , unsigned int viewportHeight,
		const char *desc , float scale , int loadMethod)
		:Terrain(desc , scale)
{
	this->root = NULL;
	this->pos = NULL;
	this->texcoord = NULL;
	this->textures = NULL;
	this->displacement = NULL;
	this->Cchanged = false;
	this->displaySkirt = true;
	this->geoMorpthEnabled = false;
	/*--------calculate constant C ------------*/
	this->fov = cameraFOV;
	this->t = fabs(T);
	this->viewportHeight = viewportHeight;
	this->CalculateC();
	
	/*----load textures------------------------*/
	this->maxLevel = numTextureFiles - 1;
	this->LoadTextures(textureFiles);

	/*------load heightmap--------------------*/
	this->LoadHeightMap(heightMap , loadMethod);
}

ChunkQuadTreeTerrain::~ChunkQuadTreeTerrain()
{
	SafeDeleteArray(pos);
	SafeDeleteArray(texcoord);
	SafeDeleteArray(displacement);
	SafeDeleteArray(needMorph);
	
	//delete textures
	if (textures != NULL)
	{
		for (unsigned int i = 0; i < this->maxLevel + 1 ; ++i)
		{
			glDeleteTextures( 0x1 << (2 * i) , textures[i]); //number of textures for each level of detail = 4 ^ level (first level is 0)
			delete [] textures[i];
		}
		delete [] textures;
	}
	//delete quad tree
	if (root != NULL)
		ChunkQuadTreeTerrain::DeleteTree(root);
}


void ChunkQuadTreeTerrain::LoadTextures(const char * const * textureFiles)
{
	unsigned int numTexturesPerLevel;
	unsigned int numTexturesPerRow;
	Bitmap bitmap;

	this->textures = new GLuint *[this->maxLevel + 1];
	for (unsigned int i = 0; i < this->maxLevel + 1 ; ++i)
	{
		numTexturesPerRow = 0x1 << i;
		numTexturesPerLevel = 0x1 << (2 * i);//number of textures for each level of detail = 4 ^ level (first level is 0) = <numTexturesPerRow> ^ 2 = 2 ^ (2 * level)
		textures[i] = new GLuint[numTexturesPerLevel];
		glGenTextures( numTexturesPerLevel , textures[i]); 

		if ((bitmap.Load(textureFiles[i])== IMG_OK))
		{
			if (bitmap.IsCompressed()){
				bitmap.DeCompressDXT(true);
			}
			else
				bitmap.FlipRGB();
			if (i == 0)
			{
				this->edgeTexcoordOffset.s = 0.5f / bitmap.GetWidth();
				this->edgeTexcoordOffset.t = 0.5f / bitmap.GetHeight();
			}
			/*----------check if image can be splitted into equal size pieces--------*/
			ChunkQuadTreeTerrain::CheckImageNeedScaling(bitmap , numTexturesPerRow);
			/*----------split image into (numTexturesPerRow ^ 2) textures------------*/
			ChunkQuadTreeTerrain::SplitImageIntoTextures(textures[i] , bitmap , numTexturesPerRow);
		}
	}

}
void ChunkQuadTreeTerrain::CheckImageNeedScaling(Bitmap &bitmap , unsigned int numPiecesPerRow)
{
	unsigned int scaledW , scaledH;
	bool needScaling = false;
	if (bitmap.GetWidth() % numPiecesPerRow != 0)//image can't be split into equal size pieces
	{
		scaledW = (bitmap.GetWidth() / numPiecesPerRow + 1)* numPiecesPerRow;
		needScaling = true;
	}
	if (bitmap.GetHeight() % numPiecesPerRow != 0)//image can't be split into equal size pieces
	{
		scaledH = (bitmap.GetHeight() / numPiecesPerRow + 1)* numPiecesPerRow;
		needScaling = true;
	}
	if (needScaling)//scale image so it can be splitted into equal pieces
		bitmap.Scalei(scaledW , scaledH);
}

void ChunkQuadTreeTerrain::SplitImageIntoTextures(GLuint * textures , const Bitmap &bitmap , unsigned int numTexturesPerRow)
{
	unsigned int textureWidth , textureHeight;
	unsigned int textureIndex = 0;
	unsigned int rowSize;//size of 1 row of texture's data
	unsigned int blockSize ;//total size of 1 texture's data
	unsigned int rowOfBlockSize ;
	const unsigned char *startPixel;
	/*-----calculate texture size-----------*/
	textureWidth = bitmap.GetWidth() / numTexturesPerRow;
	textureHeight = bitmap.GetHeight() / numTexturesPerRow;

	rowSize = bitmap.GetBits() /8 * textureWidth;
	blockSize = rowSize * textureHeight;
	rowOfBlockSize = blockSize * numTexturesPerRow;

	typedef void (*DefineTexture)(GLuint texture , const unsigned char *pData, 
				   unsigned int width , unsigned int height ,unsigned int unpackRowLength , bool wrapMode);

	DefineTexture pDefineTexture = bitmap.GetBits() == 32 ? &DefineTextureRGBA : &DefineTextureRGB;

	for (unsigned int r  = 0 ; r < numTexturesPerRow ; ++r)
	{
		for (unsigned int c  = 0 ; c < numTexturesPerRow ; ++c)
		{
			startPixel = bitmap.GetPixelData() + 
				r * rowOfBlockSize + c * rowSize;

			pDefineTexture(textures[textureIndex ++] , 
				startPixel , 
				textureWidth,
				textureHeight,
				bitmap.GetWidth(),
				false
				);
		}
	}
}

void ChunkQuadTreeTerrain::BuildTree(QuadTreeNode *node  , unsigned int level , unsigned int numTexturesPerRow)
{
	unsigned int startRow, startCol; //starting row and column of this chunk
	unsigned int textureIndex;
	unsigned int offset = node->cellSize * (this->chunkSize >> 2);//offset to center of child node (calculate in row & col)
	unsigned int numCellsPerChunkEdge = node->cellSize * this->chunkSize;
	unsigned int childCellSize;

#ifdef NODE_COLOR
	switch(level)
	{
	case 1:
		node->wirecolor = 0xff0000ff;
		break;
	case 2:
		node->wirecolor = 0x00ff00ff;
		break;
	case 3:
		node->wirecolor = 0x0000ffff;
		break;
	case 4:
		node->wirecolor = 0xff00ffff;
		break;
	case 5:
		node->wirecolor = 0x00ffffff;
		break;
	case 6:
		node->wirecolor = 0xffff00ff;
		break;
	case 0:
	default:
		node->wirecolor = 0xffffffff;
	}
#endif
	node->centerX = this->startX + this->cellSizeX * node->centerCol;
	node->centerZ = this->startZ + this->cellSizeZ * node->centerRow;

	startRow = node->centerRow - node->cellSize * (this->chunkSize  >> 1);
	startCol = node->centerCol - node->cellSize * (this->chunkSize  >> 1);

	textureIndex = startRow / numCellsPerChunkEdge * numTexturesPerRow + startCol / numCellsPerChunkEdge;
	node->texture = this->textures[level][textureIndex];

	if (level == this->maxLevel)//leaf node
	{
		node->e = 0;
	}
	else
	{
		childCellSize = node->cellSize >> 1;//cellSize /= 2
		numTexturesPerRow *= 2;
		//north west child
		node->childs[NW_Node] = this->NewNode(node ,
											  node->centerRow - offset ,	
											  node->centerCol - offset ,
											  childCellSize
											 );
		this->BuildTree(node->childs[NW_Node] , level + 1 ,numTexturesPerRow);

		//north east child
		node->childs[NE_Node] = this->NewNode(
						node,
						node->centerRow - offset ,
						node->centerCol + offset ,
						childCellSize
			);
		this->BuildTree(node->childs[NE_Node] , level + 1 ,
						numTexturesPerRow);

		//south east child
		node->childs[SE_Node] = this->NewNode(
						node,
						node->centerRow + offset ,
						node->centerCol + offset ,
						childCellSize
			);
		this->BuildTree(node->childs[SE_Node] , level + 1 ,
						numTexturesPerRow);

		//south west child
		node->childs[SW_Node] = this->NewNode(
						node,
						node->centerRow + offset ,
						node->centerCol - offset ,
						childCellSize
			);
		this->BuildTree(node->childs[SW_Node] , level + 1 ,
						numTexturesPerRow);

		//calculate local error value
		this->CalculateLocalE(node);
		//error of this node = sum of this node's local error and maximum error of 4 childs
		node->e += max(
			max(node->childs[0]->e , node->childs[1]->e) ,
			max(node->childs[2]->e , node->childs[3]->e))
			 ;

	}
	//calculate bounding box
	this->CalculateBoundingBox(node);
	
	//calcualte d value
	node->CalculateD(this->C);

	//calculate child's morph const
	if (node->cellSize != 1)
	{
		for (int i = 0 ; i < 4 ; ++i)
			node->childs[i]->morphConst = 1.0f / (node->d - node->childs[i]->d);
	}
}


float ChunkQuadTreeTerrain::GetHeightInParentNode(unsigned int r, unsigned int c , unsigned int cellSize) const
{
	float height1 , height2;
	unsigned int _2cellSize = cellSize << 1;//cellSize * 2
	unsigned int rd = r % _2cellSize;
	unsigned int cd = c % _2cellSize;

	if (rd != 0 && cd != 0)//get 2 heights in diagonal
	{
		if (((r - cellSize) / _2cellSize) % 2 == 0)//even row in parent chunk
		{
			height1 = this->GetHeight(r + cellSize , c - cellSize);
			height2 = this->GetHeight(r - cellSize , c + cellSize);
		}
		else
		{
			height1 = this->GetHeight(r - cellSize, c - cellSize);
			height2 = this->GetHeight(r + cellSize , c + cellSize);
		}
	}
	else if (rd == 0)//get 2 heights in same row
	{
		height1 = this->GetHeight(r , c - cellSize);
		height2 = this->GetHeight(r , c + cellSize);
	}
	else if (cd == 0)//get 2 heights in same column
	{
		height1 = this->GetHeight(r - cellSize , c);
		height2 = this->GetHeight(r + cellSize , c);
	}

	return (height1 + height2)/2.0f;//approximated height in parent chunk
}

QuadTreeNode * ChunkQuadTreeTerrain::NewNode(QuadTreeNode *parent,unsigned int centerRow , unsigned int centerCol, unsigned int cellSize) 
{
	QuadTreeNode *node = new QuadTreeNode(centerRow , centerCol , cellSize);
	
	if (parent != NULL)//this node must not be root
	{
		unsigned int offset = cellSize * (this->chunkSize >> 1);
		unsigned int startRow = node->centerRow - offset;
		unsigned int startCol = node->centerCol - offset;
		unsigned int _2cellSize = parent->cellSize;
		unsigned int endRow = node->centerRow + offset;
		unsigned int endCol = node->centerCol + offset;
		unsigned int r , c , rd , cd;
		unsigned int index;
		
		/*----------------calculate displacement--------------*/
		for (r = startRow ; r <= endRow ; r+= node->cellSize )
		{
			rd = r % _2cellSize;
			for (c = startCol  ; c <= endCol ; c+= node->cellSize )
			{
				cd = c % _2cellSize;
				if (rd != 0 || cd != 0)
				{
					index = this->GetIndex(r , c);
					//difference between approximated height in parent chunk and real height of this vertex
					this->displacement[index] = this->GetHeightInParentNode(r , c , node->cellSize) - this->GetHeight(r , c);
				}
			}//(for c)
		}//(for r)
	}//not root

	return node;
}

void ChunkQuadTreeTerrain::CalculateLocalE(QuadTreeNode *node)
{
	unsigned int offset = node->cellSize * (this->chunkSize  >> 1);
	unsigned int startRow = node->centerRow - offset;
	unsigned int startCol = node->centerCol - offset;
	unsigned int endRow = node->centerRow + offset;
	unsigned int endCol = node->centerCol + offset;
	unsigned int dr , dc ;
	unsigned int halfCellSize = node->cellSize >> 1;
	float dh;
	node->e = 0.0f;

	for (unsigned int r = startRow ; r <= endRow ; r += halfCellSize)
	{
		dr = r % node->cellSize; 
		for (unsigned int c = startCol ; c <= endCol ; c += halfCellSize)
		{
			dc = c % node->cellSize;
			//calculate removed vertex's error
			if (dr != 0 || dc != 0)
			{
				//difference between approximated height and real height
				dh = fabs(this->GetHeightInParentNode(r , c , halfCellSize) - this->GetHeight(r , c));
				
				//store maximum error
				if (node->e < dh)
					node->e = dh;

			}//for (c)
		}//for (r)
	}
}

void ChunkQuadTreeTerrain::CalculateBoundingBox(QuadTreeNode *node)
{
	unsigned int offset = node->cellSize * (this->chunkSize  >> 1);
	unsigned int startRow = node->centerRow - offset;
	unsigned int startCol = node->centerCol - offset;
	unsigned int endRow = node->centerRow + offset;
	unsigned int endCol = node->centerCol + offset;
	float height;

	node->boundingBox.vMin.x = this->startX + this->cellSizeX * startCol;
	node->boundingBox.vMin.z = this->startZ + this->cellSizeZ * startRow;
	node->boundingBox.vMin.y = FLT_MAX;

	node->boundingBox.vMax.x = this->startX + this->cellSizeX * endCol;
	node->boundingBox.vMax.z = this->startZ + this->cellSizeZ * endRow;
	node->boundingBox.vMax.y = -FLT_MAX;

	if (node->cellSize == 1)//leaf node
	{
		for (unsigned int r = startRow ; r <= endRow ; ++r)
		{
			for (unsigned int c = startCol ; c <= endCol ; ++c)
			{
				height = this->GetHeight(r , c);
				if (node->boundingBox.vMax.y < height)
					node->boundingBox.vMax.y = height;
				if (node->boundingBox.vMin.y > height)
					node->boundingBox.vMin.y = height;
			}
		}
	}
	else
	{
		for (int i = 0; i < 4 ; ++i)
		{
			if (node->boundingBox.vMin.y > node->childs[i]->boundingBox.vMin.y)
				node->boundingBox.vMin.y = node->childs[i]->boundingBox.vMin.y;

			if (node->boundingBox.vMax.y < node->childs[i]->boundingBox.vMax.y)
				node->boundingBox.vMax.y = node->childs[i]->boundingBox.vMax.y;
		}
	}
}

void ChunkQuadTreeTerrain::RecomputeD(QuadTreeNode *node)
{
	node->CalculateD(this->C);
	
	if (node->cellSize == 1)//leaf node
		return;
	for (int i = 0 ; i < 4 ; ++i)
	{
		RecomputeD(node->childs[i]);
		node->childs[i]->morphConst = 1.0f / (node->d - node->childs[i]->d);//recompute child's morph const
	}
}

void ChunkQuadTreeTerrain::DeleteTree(QuadTreeNode *node)
{
	if (node->cellSize != 1)//not leaf node
	{
		for (int i = 0 ; i < 4 ; ++i)
		{
			DeleteTree(node->childs[i]);
			node->childs[i] = NULL;
		}
	}
	delete node;
}


void ChunkQuadTreeTerrain::SetviewportHeight(unsigned int height)
{
	this->viewportHeight = height;
	this->CalculateC();
	this->Cchanged = true;
}
void ChunkQuadTreeTerrain::SetCameraFOV(float fov)
{
	this->fov = fov;
	this->CalculateC();
	this->Cchanged = true;
}
void ChunkQuadTreeTerrain::SetT (float T)
{
	this->t = fabs(T);
	this->CalculateC();
	this->Cchanged = true;
}
void ChunkQuadTreeTerrain::CalculateC()
{
	//			viewportHeight
	//C =	-------------------------
	//		2 * t * | tan (fov / 2) |
	this->C = this->viewportHeight / ((2 * this->t) * fabs(tanf(this->fov / 2))) ;  
}
float ChunkQuadTreeTerrain::GetHeight(unsigned int row , unsigned int col) const
{
	if (row >= this->numVertZ || col >= this->numVertX)
		return INVALID_HEIGHT;
	return this->pos[Terrain::GetIndex(row, col)].y;
}


void ChunkQuadTreeTerrain::Init()
{
	//calculate chunk size
	this->chunkSize = (this->numVertX - 1) / (0x1 << this->maxLevel);
	//number of triangles in chunk
	this->chunkTriangles = (this->chunkSize * this->chunkSize) << 1;
	this->chunkSkirtTriangles = this->chunkSize * 8;
	
	this->pos = new VertexPos[this->numVertX * this->numVertZ];
	//create texcoord array and boolean matrix , all chunks use the same texcoord array and boolean matrix
	//this boolean matrix indicates whether a vertex in chunk needs to be morphed or not
	this->texcoord = new VertexTexCoord[(this->chunkSize + 1) * (this->chunkSize + 1)];
	this->needMorph = new bool[(this->chunkSize + 1) * (this->chunkSize + 1)];
	this->ds = 1.0f / this->chunkSize;

	for (unsigned int i = 0; i <= this->chunkSize ; ++i)
	{
		for (unsigned int j = 0; j <= this->chunkSize ; ++j)
		{
			unsigned int index = i * (this->chunkSize + 1) + j;
			if (i % 2 == 0 && j % 2 ==0)
				this->needMorph[index] = false;
			else
				this->needMorph[index] = true;
			if (texClampEdge)//texture clamp to edge address mode is supported
			{
				this->texcoord[index].s = ds * j;
				this->texcoord[index].t = ds * i;
			}
			else
			{
				if (j == this->chunkSize )//edge
					this->texcoord[index].s = 1.0f - this->edgeTexcoordOffset.s;
				else if (j == 0)//edge
					this->texcoord[index].s = this->edgeTexcoordOffset.s;
				else
					this->texcoord[index].s = ds * j;

				if (i == this->chunkSize )//edge
					this->texcoord[index].t = 1.0f - this->edgeTexcoordOffset.t;
				else if (i == 0)//edge
					this->texcoord[index].t = this->edgeTexcoordOffset.t;
				else
					this->texcoord[index].t = ds * i;
			}
		}
	}
}
void ChunkQuadTreeTerrain::InitData(unsigned int row, unsigned int col, float height)
{
	unsigned int index = Terrain::GetIndex(row , col);
	this->pos[index].x = this->startX + this->cellSizeX * col;
	this->pos[index].z = this->startZ + this->cellSizeZ * row;
	this->pos[index].y = height;
}
void ChunkQuadTreeTerrain::FinishLoadHeightMap()
{
}

void ChunkQuadTreeTerrain::Update(const HQVector4 &eye , const HQPlane viewFrustum[6])
{
	if (this->Cchanged)
	{
		this->RecomputeD(root);
		this->Cchanged = false;
	}
	
	this->viewFrustum = viewFrustum;
	
	this->numTriangles = 0;//restart number of visible triangles
	
	if (this->root->boundingBox.Cull(this->viewFrustum , 6) == HQ_CULLED)
		this->root->renderFlags = 0;
	else
	{
		this->root->renderFlags = RENDER;
		if (this->root->cellSize != 1)//not leaf
			this->UpdateNode(eye , this->root);
	}
}
void ChunkQuadTreeTerrain::Render()
{
	if (this->root->renderFlags & RENDER)
	{
		this->BeginRender();
		this->RenderNode(this->root , this->numVertX  >> 1);
		this->EndRender();
	}
}


void ChunkQuadTreeTerrain::RenderBoundingBox()
{
	if (this->root->renderFlags & RENDER)
	{
		glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
		this->RenderNodeBoundingBox(this->root);
		glPopAttrib();
	}
}

void ChunkQuadTreeTerrain::UpdateNode(const HQVector4 &eye , QuadTreeNode *node)
{
	float centerY = this->GetHeight(node->centerRow , node->centerCol);
	HQVector4 distance(eye.x - node->centerX , eye.y - centerY , eye.z - node->centerZ);
#ifdef SQUARE_LEN
	node->distToEye = distance.LengthSqr();
#else
	node->distToEye = distance.Length();
#endif

#if defined _DEBUG || defined DEBUG
	assert(node->distToEye >= 0.0f);
#endif

	if (node->cellSize != 1 && node->distToEye < node->d)//need higher level of detail
	{
		node->renderFlags |= HIGHER_LOD;
		for (int i = 0 ; i < 4 ; ++i)
		{
			if (node->childs[i]->boundingBox.Cull(this->viewFrustum , 6) == HQ_CULLED)
				node->childs[i]->renderFlags = 0;
			else
			{
				node->childs[i]->renderFlags = RENDER;
				this->UpdateNode(eye , node->childs[i]);
			}
		}

		/*-------sort childs node according on distance to eye------
		--------nearest childs will be the first----------------------*/
		this->SortChilds(node);
	}
	else //this node is chosen for rendering
	{
		//calculate morph factor
		if (this->geoMorpthEnabled)
		{
			node->morphFactor = (node->distToEye - node->d) * node->morphConst;
			if (node->morphFactor > 1.0f)
				node->morphFactor = 1.0f;

		}
		
		//calculate number of triangles
		this->numTriangles += this->chunkTriangles;
		if (this->displaySkirt)
			this->numTriangles += this->chunkSkirtTriangles;

		this->OnUpdateChosenNode(eye ,node);//derived class can do additional work in this virtual method
	}
}

void ChunkQuadTreeTerrain::SortChilds(QuadTreeNode *node)
{
	int j;
	QuadTreeNode *child;
	//insertion sort , child with nearest distance to eye will be the first element
	for (int i = 1 ; i < 4 ; ++i)
	{
		child = node->childs[i];
		j = i - 1;
		while (j >=0 && node->childs[j]->distToEye > child->distToEye)
		{
			node->childs[j + 1] = node->childs[j];
			--j;
		}
		node->childs[j + 1] = child;
	}
}

void ChunkQuadTreeTerrain::RenderNode(QuadTreeNode *node , unsigned int offset)
{
	if (node->renderFlags & HIGHER_LOD)//render at higher level of detail
	{
		offset >>= 1;
		for (int i = 0 ; i < 4 ; ++i)
		{
			if (node->childs[i]->renderFlags & RENDER)
				this->RenderNode(node->childs[i] , offset);
		}
	}
	else
	{
		if (this->geoMorpthEnabled)
			this->DrawWithMorph(node , offset);
		else	
			this->Draw(node , offset);

	}
}

void ChunkQuadTreeTerrain::RenderNodeBoundingBox(QuadTreeNode *node)
{
	if (node->renderFlags & HIGHER_LOD)//render at higher level of detail
	{
		for (int i = 0 ; i < 4 ; ++i)
		{
			if (node->childs[i]->renderFlags & RENDER)
			{
#if 0//debug purpose
				if (i == 2)
					glColor3f(1,0,0);
				else
					glColor3f(1,1,1);
#endif
				this->RenderNodeBoundingBox(node->childs[i]);
			}
		}
	}
	else
	{
		//render bounding box
		glBegin(GL_QUADS);
		
		//front
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMin.y , node->boundingBox.vMax.z);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMin.y , node->boundingBox.vMax.z);
		glVertex3fv(node->boundingBox.vMax.v);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMax.y , node->boundingBox.vMax.z);
		
		//back
		glVertex3fv(node->boundingBox.vMin.v);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMax.y , node->boundingBox.vMin.z);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMax.y , node->boundingBox.vMin.z);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMin.y , node->boundingBox.vMin.z);

		//left
		glVertex3fv(node->boundingBox.vMin.v);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMin.y , node->boundingBox.vMax.z);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMax.y , node->boundingBox.vMax.z);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMax.y , node->boundingBox.vMin.z);

		//right
		glVertex3fv(node->boundingBox.vMax.v);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMin.y , node->boundingBox.vMax.z);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMin.y , node->boundingBox.vMin.z);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMax.y , node->boundingBox.vMin.z);

		//top
		glVertex3fv(node->boundingBox.vMax.v);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMax.y , node->boundingBox.vMin.z);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMax.y , node->boundingBox.vMin.z);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMax.y , node->boundingBox.vMax.z);

		//bottom
		glVertex3fv(node->boundingBox.vMin.v);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMin.y , node->boundingBox.vMin.z);
		glVertex3f(node->boundingBox.vMax.x , node->boundingBox.vMin.y , node->boundingBox.vMax.z);
		glVertex3f(node->boundingBox.vMin.x , node->boundingBox.vMin.y , node->boundingBox.vMax.z);
		glEnd();
	}
}

void ChunkQuadTreeTerrain::Draw(QuadTreeNode *node, unsigned int offset)
{
	glBindTexture(GL_TEXTURE_2D , node->texture);
	unsigned int startRow = node->centerRow - offset;
	unsigned int startCol = node->centerCol - offset;
	unsigned int _2cellSize = 2 * node->cellSize;
	unsigned int endRow = node->centerRow + offset;
	unsigned int endRowMinus2 = endRow - _2cellSize;
	unsigned int endCol = node->centerCol + offset;
	VertexPos *pPos;
	VertexTexCoord *pTexcoord;
	int rt , ct , r , c ;
	unsigned int index;

	glBegin(GL_TRIANGLE_STRIP);
#ifdef NODE_COLOR
	glColor3f(((node->wirecolor & 0xff000000) >> 24) / 255.0f,
			  ((node->wirecolor & 0xff0000) >> 16) / 255.0f,
			  ((node->wirecolor & 0xff00) >> 8 ) / 255.f);
#endif
	
	for (r = (int)startRow , rt = 0 ;  r <= (int)endRowMinus2 ; r += _2cellSize , rt += 2)
	{
		for (c = (int)startCol , ct = 0; c <= (int)endCol ; c += node->cellSize , ++ct)
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt , ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);
			
			index = this->GetIndex(r + node->cellSize , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt + 1 , ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);
		}
		//last vertex need to be repeated 2 more times
		ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);//to form degenerate triangle

		for (c = (int)endCol , ct = (int)this->chunkSize; c >= (int)startCol ; c -= node->cellSize , --ct)
		{
			index = this->GetIndex(r + node->cellSize , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt + 1 , ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);

			index = this->GetIndex(r + _2cellSize , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt + 2 , ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);
		}
		//last vertex need to be repeated 2 more times
		ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);//to form degenerate triangle

	}
	
	if (this->displaySkirt)
	{
		for ( c = (int)startCol , ct = 0 ; c <= (int)endCol ; c += node->cellSize , ++ct )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);

			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
			glTexCoord2fv(&pTexcoord->s);
		}

		r -= node->cellSize;
		--rt;

		for ( c = (int)endCol , ct = this->chunkSize ; 
			r >= (int)startRow ; r -= node->cellSize , --rt )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);

			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
			glTexCoord2fv(&pTexcoord->s);
		}
		
		c -= node->cellSize;
		--ct;
		for ( r = (int)startRow , rt = 0; c >= (int)startCol ; c -= node->cellSize, --ct )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);

			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
			glTexCoord2fv(&pTexcoord->s);
		}
		
		r += node->cellSize;
		++rt;
		for ( c = (int)startCol , ct = 0; 
			r <= (int)endRow ; r += node->cellSize , ++rt )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			ChunkQuadTreeTerrain::RenderVertex(pPos , pTexcoord);

			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
			glTexCoord2fv(&pTexcoord->s);
		}
	}
	glEnd();
}


void ChunkQuadTreeTerrain::DrawWithMorph(QuadTreeNode *node, unsigned int offset)
{
	glBindTexture(GL_TEXTURE_2D , node->texture);
	unsigned int startRow = node->centerRow - offset;
	unsigned int startCol = node->centerCol - offset;
	unsigned int _2cellSize = 2 * node->cellSize;
	unsigned int endRow = node->centerRow + offset;
	unsigned int endRowMinus2 = endRow - _2cellSize;
	unsigned int endCol = node->centerCol + offset;
	VertexPos *pPos;//position
	VertexTexCoord *pTexcoord;//texcoord
	float *pDis;//morph displacement
	int rt , ct , r , c ;
	unsigned int index;
	bool isMorphNeeded;

	glBegin(GL_TRIANGLE_STRIP);
#ifdef NODE_COLOR
	glColor3f(((node->wirecolor & 0xff000000) >> 24) / 255.0f,
			  ((node->wirecolor & 0xff0000) >> 16) / 255.0f,
			  ((node->wirecolor & 0xff00) >> 8 ) / 255.f);
#endif
	
	for (r = (int)startRow , rt = 0 ;  r <= (int)endRowMinus2 ; r += _2cellSize , rt += 2)
	{
		for (c = (int)startCol , ct = 0; c <= (int)endCol ; c += node->cellSize , ++ct)
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt , ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);
			
			index = this->GetIndex(r + node->cellSize , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt + 1 , ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);
		}
		//last vertex need to be repeated 2 more times to form degenerate triangles
		ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
													pPos , pTexcoord ,
													pDis , node->morphFactor);

		for (c = (int)endCol , ct = (int)this->chunkSize; c >= (int)startCol ; c -= node->cellSize , --ct)
		{
			index = this->GetIndex(r + node->cellSize , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt + 1 , ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);
		
			index = this->GetIndex(r + _2cellSize , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt + 2 , ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);
		}
		//last vertex need to be repeated 2 more times to form degenerate triangles
		ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
													pPos , pTexcoord ,
													pDis , node->morphFactor);
	}
	
	if (this->displaySkirt)
	{
		for ( c = (int)startCol , ct = 0 ; c <= (int)endCol ; c += node->cellSize , ++ct )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);

			glTexCoord2fv(&pTexcoord->s);
			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
		}

		r -= node->cellSize;
		--rt;

		for ( c = (int)endCol , ct = this->chunkSize ; 
			r >= (int)startRow ; r -= node->cellSize , --rt )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);

			glTexCoord2fv(&pTexcoord->s);
			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
		}
		
		c -= node->cellSize;
		--ct;
		for ( r = (int)startRow , rt = 0; c >= (int)startCol ; c -= node->cellSize, --ct )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);

			glTexCoord2fv(&pTexcoord->s);
			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
		}
		
		r += node->cellSize;
		++rt;
		for ( c = (int)startCol , ct = 0; 
			r <= (int)endRow ; r += node->cellSize , ++rt )
		{
			index = this->GetIndex(r , c);
			pPos = this->pos + index;
			pDis = this->displacement + index;
			index = this->GetChunkIndex(rt, ct);
			pTexcoord = this->texcoord + index;
			isMorphNeeded = this->needMorph[index];
			ChunkQuadTreeTerrain::RenderVertexWithMorph(isMorphNeeded ,
														pPos , pTexcoord ,
														pDis , node->morphFactor);

			glTexCoord2fv(&pTexcoord->s);
			glVertex3f(pPos->x , SKIRT_HEIGHT , pPos->z);
		}
	}
	glEnd();
}
