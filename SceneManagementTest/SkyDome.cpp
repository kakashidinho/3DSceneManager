#include "SkyDome.h"

SkyDome::SkyDome(float zFar , unsigned int numSlice , unsigned int numStack ,
			const char *posXImage,
			const char *negXImage,
			const char *posYImage,
			const char *negYImage,
			const char *posZImage,
			const char *negZImage)
			:sphereRadius (zFar) , vertices (NULL) , 
			indices(NULL), numIndices (0) ,
			skyBoxTexture (0) , displayList(0)
{
	if (!cubeMapSupported)//cube map is not supported
		return;//do nothing

	const char * images[] = {
			posXImage,
			negXImage,
			posYImage,
			negYImage,
			posZImage,
			negZImage
	};
	this->LoadTexture(images);
	this->CreateSphere(numSlice , numStack );
	this->CreateDisplayList();
	this->ReleaseData();
}

SkyDome::~SkyDome()
{
	if (this->skyBoxTexture)
		glDeleteTextures(1 , &this->skyBoxTexture);
	if (this->displayList)
		glDeleteLists(this->displayList , 1);
}


void SkyDome::Render(const HQVector4 &eyePos)
{
	if (!cubeMapSupported)//cube map is not supported
		return;//do nothing
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(eyePos.x , eyePos.y , eyePos.z);
	glScalef(this->sphereRadius , this->sphereRadius , this->sphereRadius);

	glCallList(this->displayList);

	glPopMatrix();
}

void SkyDome::LoadTexture(const char * images[6])
{
	/*-------load sky cubemap------*/
	glGenTextures(1, &this->skyBoxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP , this->skyBoxTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	if (texClampEdge)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
	}
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
	
	GLenum target[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	
	Bitmap bitmap;
	for (int i = 0 ; i < 6 ; ++i)
	{
		bitmap.Load(images[i]);
		if (bitmap.IsCompressed())
			bitmap.DeCompressDXT(true);
		else
			bitmap.FlipRGB();
		bitmap.SetPixelOrigin(ORIGIN_TOP_LEFT);
		
		if (bitmap.GetBits() == 24)
			glTexImage2D(target[i] , 0 , GL_RGB8 , bitmap.GetWidth() , bitmap.GetHeight() , 
				0 , GL_RGB , GL_UNSIGNED_BYTE , bitmap.GetPixelData());
		else
			glTexImage2D(target[i] , 0 , GL_RGBA , bitmap.GetWidth() , bitmap.GetHeight() , 
				0 , GL_RGBA , GL_UNSIGNED_BYTE , bitmap.GetPixelData());
	}
}

void SkyDome::CreateSphere(unsigned int numSlice , unsigned int numStack)
{
	unsigned int numVertices = numSlice * (numStack - 1) + 2;
	this->numIndices = numSlice * (numStack - 1) * 6;
	float dStackAngle = HQPiFamily::PI / numStack;
	float dSliceAngle = HQPiFamily::_2PI / numSlice;
	float stackAngle , sliceAngle;

	this->vertices = new HQFloat3[numVertices];
	this->indices = new unsigned short [numIndices];

	//top and bottom vertices
	this->vertices[0].Set(0 , 1 , 0);
	this->vertices[numVertices - 1].Set(0 , -1 , 0);
	
	//the rest vertices
	unsigned int index = 1;

	for (unsigned int i = 1 ; i < numStack ; ++i)
	{
		stackAngle = dStackAngle * i;
		float radius = sinf(stackAngle);
		float y = cosf(stackAngle);
		for (unsigned int j = 0 ; j < numSlice  ; ++j)
		{
			sliceAngle = dSliceAngle * j;
			vertices[index].x = radius * cosf(sliceAngle);
			vertices[index].y = y;
			vertices[index].z = radius * sinf(sliceAngle);
			index ++;
		}
	}

	//create indices
	index = 0;
	for (unsigned int i = 1 ; i <= numSlice ; ++i)
	{
		unsigned nextSlice = (i + 1) % numSlice;
		//top
		indices[index ++] = 0;
		indices[index ++] = i ;
		indices[index ++] = nextSlice;
		
		unsigned int vertexIndex , base;
		//middle
		for (unsigned int j = 0 ; j < numStack - 2 ; ++j)
		{
			base = numSlice * j;
			vertexIndex = base + i;
			indices[index ++] = vertexIndex ;
			indices[index ++] = vertexIndex + numSlice ;
			indices[index ++] = base + nextSlice + numSlice;

			indices[index ++] = vertexIndex ;
			indices[index ++] = base + nextSlice + numSlice;
			indices[index ++] = base + nextSlice;
		}
		//bottom
		base = numSlice * (numStack - 2);
		vertexIndex = base + i;
		indices[index ++] = vertexIndex;
		indices[index ++] = numVertices - 1;
		indices[index ++] = base + nextSlice;

	}
}

void SkyDome::CreateDisplayList()
{
	this->displayList = glGenLists(1);
	glNewList(this->displayList , GL_COMPILE);
	
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP , this->skyBoxTexture);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(3 , GL_FLOAT , sizeof(HQFloat3) , this->vertices);

	glVertexPointer(3 , GL_FLOAT , sizeof(HQFloat3) , this->vertices);

	//draw
	glDrawElements(GL_TRIANGLES , this->numIndices , GL_UNSIGNED_SHORT , this->indices);
	

	glDisable(GL_TEXTURE_CUBE_MAP);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glPopAttrib();

	glEndList();
}
void SkyDome::ReleaseData(){
	delete[] this->vertices;
	delete[] this->indices;
}