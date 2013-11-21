#include "Terrain.h"
#include <string.h>
#include <float.h>

Terrain::Terrain(const char *desc , float scale) : numTriangles(0)
{
	numVertX = numVertZ = 0;
	this->desc = NULL;
	this->SetDesc(desc);

	this->scale = scale;
}
Terrain::~Terrain()
{
	SafeDeleteArray(desc);
}

void Terrain::SetDesc(const char *_desc)
{
	SafeDeleteArray(this->desc);
	//description
	this->desc = new char[strlen(_desc) + 1];
	strcpy(this->desc , _desc);
}

bool Terrain::LoadHeightMap(const char *fileName , int loadMethod)
{
	switch (loadMethod)
	{
	case 1: return this->LoadHeightMapFromTxt(fileName);
	case 2: return this->LoadHeightMapFromBmp(fileName);//greyscale bitmap
	case 3: return this->LoadHeightMapFromFloatBmp(fileName);//rgba as float bitmap
	default:
		return false;
	}
}

bool Terrain::LoadHeightMapFromTxt(const char *fileName)
{
	FILE *f = fopen(fileName , "r");
	if (f == NULL)
		return false;
	fscanf(f , "%f %f %f %f %u %u",
	&this->startX , &this->startZ, 
	&this->cellSizeX , &this->cellSizeZ , 
	&this->numVertX , &this->numVertZ);
	
	/*-------scaling--------*/
	this->cellSizeX *= this->scale;
	this->cellSizeZ *= this->scale;
	this->startX *= this->scale;
	this->startZ *= this->scale;

	float height;
	fscanf(f , "%f" , &height);//unused value
	
	this->maxX = this->startX + this->cellSizeX * (this->numVertX - 1);
	this->maxZ = this->startZ + this->cellSizeZ * (this->numVertZ - 1);
	this->Init();
	
	this->maxHeight = -FLT_MAX;
	this->minHeight = FLT_MAX;
	/*---------read height data--------------*/
	for (UINT z = 0; z < this->numVertZ ; ++z)
		for (UINT x = 0; x < this->numVertX ; ++x)
		{
			fscanf(f , "%f" ,&height);
			height *= this->scale;
			if (maxHeight < height)
				this->maxHeight = height;
			if (minHeight > height)
				this->minHeight = height;
			this->InitData(z , x , height);
		}

	fclose (f);

	this->FinishLoadHeightMap();

#if 0
	{
		//dump the height data to bitmap
		std::string dumpFile = fileName;
		size_t extPos = dumpFile.find_last_of('.');
		dumpFile.replace(extPos + 1, std::string::npos, "bmp"); 
		this->dumpHeightToBmp(dumpFile.c_str());

		//write information about the height bitmap
		std::string dumpFileInfo = fileName;
		size_t extPos2 = dumpFileInfo.find_last_of('.');
		dumpFileInfo.replace(extPos2, std::string::npos, "Type3.txt"); 

		FILE * f2 = fopen(dumpFileInfo.c_str(), "wb");
		fprintf(f2, "%f %f %f %f %s", this->startX , this->startZ, 
			this->cellSizeX , this->cellSizeZ , dumpFile.c_str());
		fclose(f2);
	}

#endif

	return true;
}

bool Terrain::LoadHeightMapFromBmp(const char *fileName)
{
	FILE *f = fopen(fileName , "r");
	if (f == NULL)
		return false;
	char bmpFileName[256];
	float heightUnit;
	Bitmap bitmap;
	fscanf(f , "%f %f %f %f %f %s",
	&this->startX , &this->startZ, 
	&this->cellSizeX , &this->cellSizeZ , &heightUnit , bmpFileName);

	fclose (f);
	
	bitmap.Load(bmpFileName);
	bitmap.DeCompressDXT();
	this->numVertX = bitmap.GetWidth();
	this->numVertZ = bitmap.GetHeight();

	/*-------scaling--------*/
	this->cellSizeX *= this->scale;
	this->cellSizeZ *= this->scale;
	this->startX *= this->scale;
	this->startZ *= this->scale;

	float height;
	
	this->maxX = this->startX + this->cellSizeX * (this->numVertX - 1);
	this->maxZ = this->startZ + this->cellSizeZ * (this->numVertZ - 1);
	this->Init();
	
	this->maxHeight = -FLT_MAX;
	this->minHeight = FLT_MAX;
	unsigned char *pixels = bitmap.GetPixelData();
	unsigned int pixelStep = bitmap.GetBits() / 8;
	/*---------read height data--------------*/
	for (UINT z = 0; z < this->numVertZ ; ++z)
		for (UINT x = 0; x < this->numVertX ; ++x)
		{
			unsigned char pixel = *(pixels + pixelStep * (z * this->numVertX + x));
			height = (float)pixel * heightUnit;
			
			height *= this->scale;
			if (maxHeight < height)
				this->maxHeight = height;
			if (minHeight > height)
				this->minHeight = height;
			this->InitData(z , x , height);
		}


	this->FinishLoadHeightMap();
	return true;
}

bool Terrain::LoadHeightMapFromFloatBmp(const char *fileName)
{
	FILE *f = fopen(fileName , "r");
	if (f == NULL)
		return false;
	char bmpFileName[256];
	Bitmap bitmap;
	fscanf(f , "%f %f %f %f %s",
	&this->startX , &this->startZ, 
	&this->cellSizeX , &this->cellSizeZ , bmpFileName);

	fclose (f);
	
	bitmap.Load(bmpFileName);
	bitmap.DeCompressDXT();
	this->numVertX = bitmap.GetWidth();
	this->numVertZ = bitmap.GetHeight();

	/*-------scaling--------*/
	this->cellSizeX *= this->scale;
	this->cellSizeZ *= this->scale;
	this->startX *= this->scale;
	this->startZ *= this->scale;

	float height;
	
	this->maxX = this->startX + this->cellSizeX * (this->numVertX - 1);
	this->maxZ = this->startZ + this->cellSizeZ * (this->numVertZ - 1);
	this->Init();
	
	this->maxHeight = -FLT_MAX;
	this->minHeight = FLT_MAX;
	unsigned char *pixels = bitmap.GetPixelData();
	/*---------read height data--------------*/
	for (UINT z = 0; z < this->numVertZ ; ++z)
		for (UINT x = 0; x < this->numVertX ; ++x)
		{
			height = *((float*)(pixels + 4 * (z * this->numVertX + x)));
			
			height *= this->scale;
			if (maxHeight < height)
				this->maxHeight = height;
			if (minHeight > height)
				this->minHeight = height;
			this->InitData(z , x , height);
		}


	this->FinishLoadHeightMap();
	return true;
}


void Terrain::dumpHeightToBmp(const char *fileName)const
{
	//dump the height data to greyscale format
	FILE * f = fopen(fileName, "wb");
	if (f == NULL)
		return;
	//write header
	unsigned int width = this->GetNumVertX();
	unsigned int height = this->GetNumVertZ();
	int linePadding = (4 - width * 4 % 4) % 4;
	unsigned int imgSize = (4 * this->GetNumVertX() + linePadding) * this->GetNumVertZ();
	unsigned int fileSize = 54 + imgSize;
	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 32,0,
		0,0,0,0, // compression is none
		0,0,0,0, // image size
		0x13,0x0B,0,0, // horz resoluition in pixel / m
		0x13,0x0B,0,0 // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
	};
	unsigned char bmppad[3] = {0,0,0};

	memcpy(bmpfileheader + 2, &fileSize, 4);//file size
	memcpy(bmpinfoheader + 4, &width, 4);//image width
	memcpy(bmpinfoheader + 8, &height, 4);//image height
	memcpy(bmpinfoheader + 24, &imgSize, 4);//image size

	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	for(unsigned int i = 0; i < height; i++)
	{
		for(unsigned int j = 0; j < width; j++)
		{
			float height = this->GetHeight(i, j);

			fwrite(&height, 4, 1, f);
		}
		fwrite(bmppad, 1, linePadding, f);//line padding
	}

	fclose(f);
}

bool Terrain::GetRelativePosInfo(float x , float z , RelativePostionInfo &infoOut) const
{
	if (this->numVertX == 0 || this->numVertZ == 0)
		return false;
	float rowf , colf;
	float dx, dz;
	dx = x - this->startX;
	dz = z - this->startZ;

	if (dx < 0.0f || dz < 0.0f)
		return false;
	dx /= this->cellSizeX;
	dz /= this->cellSizeZ;
	dx = modf( dx , &colf);
	dz = modf( dz , &rowf);

	infoOut.row = (unsigned int) rowf;
	infoOut.col = (unsigned int) colf;

	infoOut.dx = dx;
	infoOut.dz = dz;

	return true;
}

float Terrain::GetHeight(const RelativePostionInfo & info) const
{
	float height[3];
	/*-------------------------------------
	
	A   (row , col)     ---------------------- B   (row , col + 1)
		|										|
		|										|
		|										|
		|										|
		|										|
	D   (row + 1 , col) ---------------------- C   (row + 1 , col + 1)

	--------------------------------------*/

	if (info.dx > info.dz)//use ABC triangle
	{
		height[0] = this->GetHeight(info.row , info.col); //A
		height[1] = this->GetHeight(info.row , info.col + 1); //B
		height[2] = this->GetHeight(info.row + 1 , info.col + 1); //C

		return height[0] + info.dx * (height[1] - height[0]) + info.dz * (height[2] - height[1]);
	}
	else //use ACD triangle
	{
		height[0] = this->GetHeight(info.row , info.col); //A
		height[1] = this->GetHeight(info.row + 1 , info.col + 1); //C
		height[2] = this->GetHeight(info.row + 1, info.col); //D

		return height[0] + info.dx * (height[1] - height[2]) + info.dz * (height[2] - height[0]);
	}
}

float Terrain::GetHeight(float x , float z) const
{
	RelativePostionInfo info ;
	if (!this->GetRelativePosInfo(x , z , info))
		return INVALID_HEIGHT;

	return this->GetHeight(info);
}

void Terrain::GetSurfaceNormal(const RelativePostionInfo& info , HQVector4& normalOut)const
{
	float height[3];
	HQVector4 v[2];
	/*-------------------------------------
	
	A   (row , col)     ---------------------- B   (row , col + 1)
		|										|
		|										|
		|										|
		|										|
		|										|
	D   (row + 1 , col) ---------------------- C   (row + 1 , col + 1)

	--------------------------------------*/

	if (info.dx > info.dz)//use ABC triangle
	{
		height[0] = this->GetHeight(info.row , info.col); //A
		height[1] = this->GetHeight(info.row , info.col + 1); //B
		height[2] = this->GetHeight(info.row + 1 , info.col + 1); //C

		v[0].Set(-cellSizeX , height[0] - height[1] , 0);
		v[1].Set(0 , height[2] - height[1] , cellSizeZ);

		normalOut.Cross(v[0] , v[1]);
		normalOut.Normalize();
	}
	else //use ACD triangle
	{
		height[0] = this->GetHeight(info.row , info.col); //A
		height[1] = this->GetHeight(info.row + 1 , info.col + 1); //C
		height[2] = this->GetHeight(info.row + 1, info.col); //D

		v[0].Set(cellSizeX , height[1] - height[2] , 0);
		v[1].Set(0 , height[0] - height[2] , -cellSizeZ);

		normalOut.Cross(v[0] , v[1]);
		normalOut.Normalize();
	}

}

bool Terrain::GetSurfaceNormal(float x , float z , HQVector4& normalOut)const
{
	RelativePostionInfo info ;
	if (!this->GetRelativePosInfo(x , z , info))
		return false;
	
	this->GetSurfaceNormal(info , normalOut);
	return true;
}
/*-----------------------------------------------------
Simple Terrain
------------------------------------------------------*/
SimpleTerrain::SimpleTerrain(const char *heightMap , const char *texture  , float scale , int loadMethod)
:Terrain("Simple brute force rendering using immediate mode" , scale)
{
	this->vertices = NULL;
	this->LoadHeightMap(heightMap , loadMethod);
	this->texture = LoadTexture(texture , ORIGIN_BOTTOM_LEFT);
	this->numTriangles = 2 * (this->numVertX - 1) *  (this->numVertZ - 1);
}

SimpleTerrain::SimpleTerrain(const char *heightMap , const char *texture , const char *desc , float scale , int loadMethod)
:Terrain(desc , scale)
{
	this->vertices = NULL;
	this->LoadHeightMap(heightMap , loadMethod);
	this->texture = LoadTexture(texture , ORIGIN_BOTTOM_LEFT);
	this->numTriangles = 2 * (this->numVertX - 1) *  (this->numVertZ - 1);
}

SimpleTerrain::~SimpleTerrain()
{
	SafeDeleteArray(this->vertices);
	glDeleteTextures(1 , &texture);
}

void SimpleTerrain::Init()
{
	ds = 1.0f / (this->numVertX - 1);
	dt = 1.0f / (this->numVertZ - 1);
	this->vertices = new Vertex[this->numVertX * this->numVertZ];
}
void SimpleTerrain::InitData(unsigned int row, unsigned int col, float height)
{
	unsigned int index = row * this->numVertX + col;
	this->vertices[index].x = this->startX + col * this->cellSizeX;
	this->vertices[index].y = height;
	this->vertices[index].z = this->startZ + row * this->cellSizeZ;
	this->vertices[index].s = col * this->ds;
	this->vertices[index].t = row * this->dt;
}

float SimpleTerrain::GetHeight(unsigned int row , unsigned int col) const
{
	if (row >= this->numVertZ || col >= this->numVertX)
		return INVALID_HEIGHT;
	return this->GetVertex(row, col)->y;
}

void SimpleTerrain::Render()
{
	if (this->vertices == NULL)
		return;
	
	glBindTexture(GL_TEXTURE_2D , this->texture);

	unsigned int maxR = this->numVertZ - 1;
	unsigned int maxC = this->numVertX - 1;
	Vertex * vertex;
	glBegin(GL_TRIANGLES);
	for (unsigned int r = 0;  r < maxR ; r += 1)
	{
		for (unsigned int c = 0; c < maxC ; c += 1)
		{
			//first triangle
			vertex = this->GetVertex(r , c );
			SimpleTerrain::RenderVertex(vertex);
			
			
			vertex = this->GetVertex(r + 1 , c);
			SimpleTerrain::RenderVertex(vertex);
			
			vertex = this->GetVertex(r + 1 , c + 1);
			SimpleTerrain::RenderVertex(vertex);
	
			//second triangle
			vertex = this->GetVertex(r , c );
			SimpleTerrain::RenderVertex(vertex);
			
			vertex = this->GetVertex(r + 1 , c + 1);
			SimpleTerrain::RenderVertex(vertex);

			vertex = this->GetVertex(r , c + 1);
			SimpleTerrain::RenderVertex(vertex);
		}

	}
	glEnd();
}

/*--------------------------------------
SimpleTerrainTriFan
---------------------------------------*/
SimpleTerrainTriFan::SimpleTerrainTriFan(const char *heightMap , const char *texture , float scale , int loadMethod)
		:SimpleTerrain(heightMap , texture , 
		"Simple brute force rendering using immediate mode and triangle fan",
		scale , loadMethod) 
{
}
void SimpleTerrainTriFan::Render()
{
	if (this->vertices == NULL)
		return;
	
	glBindTexture(GL_TEXTURE_2D , this->texture);

	unsigned int maxR = this->numVertZ - 2;
	unsigned int maxC = this->numVertX - 2;
	Vertex * vertex;
	for (unsigned int r = 0;  r < maxR ; r += 2)
	{
		for (unsigned int c = 0; c < maxC ; c += 2)
		{
			glBegin(GL_TRIANGLE_FAN);
			vertex = this->GetVertex(r + 1 , c + 1);
			SimpleTerrain::RenderVertex(vertex);

			vertex = this->GetVertex(r , c );
			SimpleTerrain::RenderVertex(vertex);
			
			
			vertex = this->GetVertex(r + 1 , c);
			SimpleTerrain::RenderVertex(vertex);
			
			vertex = this->GetVertex(r + 2 , c);
			SimpleTerrain::RenderVertex(vertex);
			
			vertex = this->GetVertex(r + 2 , c + 1);
			SimpleTerrain::RenderVertex(vertex);
			
			vertex = this->GetVertex(r + 2 , c + 2);
			SimpleTerrain::RenderVertex(vertex);

			vertex = this->GetVertex(r + 1 , c + 2);
			SimpleTerrain::RenderVertex(vertex);

			vertex = this->GetVertex(r , c + 2);
			SimpleTerrain::RenderVertex(vertex);

			vertex = this->GetVertex(r , c + 1);
			SimpleTerrain::RenderVertex(vertex);
			

			vertex = this->GetVertex(r , c );
			SimpleTerrain::RenderVertex(vertex);

			glEnd();
		}

	}
}

/*--------------------------------------
SimpleTerrainTriStrip
---------------------------------------*/
SimpleTerrainTriStrip::SimpleTerrainTriStrip(const char *heightMap , const char *texture ,float scale , int loadMethod)
		:SimpleTerrain(heightMap , texture , 
		"Simple brute force rendering using immediate mode and triangle strip" ,
		scale , loadMethod) 
{
}
void SimpleTerrainTriStrip::Render()
{
	if (this->vertices == NULL)
		return;

	glBindTexture(GL_TEXTURE_2D , this->texture);

	unsigned int maxR = this->numVertZ - 2;
	unsigned int maxC = this->numVertX - 1;
	Vertex * vertex;
	glBegin(GL_TRIANGLE_STRIP);
	for (unsigned int r = 0;  r < maxR ; r += 2)
	{
		for (unsigned int c = 0; c < this->numVertX ; ++c)
		{
			vertex = this->GetVertex(r , c);
			SimpleTerrain::RenderVertex(vertex);

			vertex = this->GetVertex(r + 1, c);
			SimpleTerrain::RenderVertex(vertex);
		}
		//last vertex need to be repeated 2 more times
		SimpleTerrain::RenderVertex(vertex);//to form degenerate triangle

		for (int c = (int)maxC; c >= 0 ; --c)
		{
			vertex = this->GetVertex(r + 1, c);
			SimpleTerrain::RenderVertex(vertex);

			vertex = this->GetVertex(r + 2, c);
			SimpleTerrain::RenderVertex(vertex);

			
		}
		if (r != maxR - 1 )//last vertex need to be repeated 2 more times
			SimpleTerrain::RenderVertex(vertex);//to form degenerate triangle

	}
	glEnd();
}


