#include "glHelper.h"
#include "Model.h"
#include "../Libraries/Open Asset Import Library/include/aiPostProcess.h"
#include "../Libraries/Open Asset Import Library/include/aiScene.h"
#include <float.h>

Model::Model(const char *modelFileName) : m_pData(NULL) , m_numTriangles(0)
{
	this->m_pData = aiImportFile(modelFileName , aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);//| aiProcess_GenSmoothNormals  );
	if (m_pData != NULL)
	{
		m_numTriangles = m_pData->mMeshes[0]->mNumFaces;
		this->m_numIndices = m_pData->mMeshes[0]->mNumFaces * 3;//each face is a triangle , so number of indices = number of faces X 3
		//copy indices data
		this->m_indices = new unsigned short [this->m_numIndices];
		for (unsigned int i = 0 ; i < m_pData->mMeshes[0]->mNumFaces ; ++i)
		{
			m_indices[3 * i] = m_pData->mMeshes[0]->mFaces[i].mIndices[0];
			m_indices[3 * i + 1] = m_pData->mMeshes[0]->mFaces[i].mIndices[1];
			m_indices[3 * i + 2] = m_pData->mMeshes[0]->mFaces[i].mIndices[2];
		}
		
		//find bounding sphere
		m_BoundingBox.vMin.Set(FLT_MAX , FLT_MAX , FLT_MAX , 1.0f); 
		m_BoundingBox.vMax.Set(FLT_MIN , FLT_MIN , FLT_MIN , 1.0f ); 
		for (unsigned int i = 0 ; i < m_pData->mMeshes[0]->mNumVertices ; ++i)
		{
			if (m_pData->mMeshes[0]->mVertices[i].x < m_BoundingBox.vMin.x)
				m_BoundingBox.vMin.x = m_pData->mMeshes[0]->mVertices[i].x;
			if (m_pData->mMeshes[0]->mVertices[i].y < m_BoundingBox.vMin.y)
				m_BoundingBox.vMin.y = m_pData->mMeshes[0]->mVertices[i].y;
			if (m_pData->mMeshes[0]->mVertices[i].z < m_BoundingBox.vMin.z)
				m_BoundingBox.vMin.z = m_pData->mMeshes[0]->mVertices[i].z;

			if (m_pData->mMeshes[0]->mVertices[i].x > m_BoundingBox.vMax.x)
				m_BoundingBox.vMax.x = m_pData->mMeshes[0]->mVertices[i].x;
			if (m_pData->mMeshes[0]->mVertices[i].y > m_BoundingBox.vMax.y)
				m_BoundingBox.vMax.y = m_pData->mMeshes[0]->mVertices[i].y;
			if (m_pData->mMeshes[0]->mVertices[i].z > m_BoundingBox.vMax.z)
				m_BoundingBox.vMax.z = m_pData->mMeshes[0]->mVertices[i].z;
		}
		
		this->m_BoundingSphere.center = (m_BoundingBox.vMin + m_BoundingBox.vMax) / 2.0f;
		HQVector4 dist = m_BoundingBox.vMax - m_BoundingBox.vMin;
		this->m_BoundingSphere.radius = dist.Length() / 2.0f;
		
		//transalte sphere center to world origin
		for (unsigned int i = 0 ; i < m_pData->mMeshes[0]->mNumVertices ; ++i)
		{
			m_pData->mMeshes[0]->mVertices[i].x -= this->m_BoundingSphere.center.x;
			m_pData->mMeshes[0]->mVertices[i].y -= this->m_BoundingSphere.center.y;
			m_pData->mMeshes[0]->mVertices[i].z -= this->m_BoundingSphere.center.z;
		}
		m_BoundingBox.vMax -= this->m_BoundingSphere.center;
		m_BoundingBox.vMin -= this->m_BoundingSphere.center;

		this->m_BoundingSphere.center.Set(0,0,0,1);

		//get material
		unsigned int index = m_pData->mMeshes[0]->mMaterialIndex;//material index
		aiColor3D color;
		m_pData->mMaterials[index]->Get<aiColor3D>(AI_MATKEY_COLOR_DIFFUSE , color );
		memcpy(this->m_Material.diffuse , &color , 3 * sizeof(float));
		this->m_Material.diffuse[3] = 1.0f;

		m_pData->mMaterials[index]->Get<aiColor3D>(AI_MATKEY_COLOR_AMBIENT , color );
		memcpy(this->m_Material.ambient , &color , 3 * sizeof(float));
		this->m_Material.ambient[3] = 1.0f;

		m_pData->mMaterials[index]->Get<aiColor3D>(AI_MATKEY_COLOR_SPECULAR , color );
		memcpy(this->m_Material.specular , &color , 3 * sizeof(float));
		this->m_Material.specular[3] = 1.0f;

		m_pData->mMaterials[index]->Get<aiColor3D>(AI_MATKEY_COLOR_EMISSIVE , color );
		memcpy(this->m_Material.emissive , &color , 3 * sizeof(float));
		this->m_Material.emissive[3] = 1.0f;

		m_pData->mMaterials[index]->Get<float>(AI_MATKEY_SHININESS , this->m_Material.shininess);
		
		//load texture
		aiString textureName; 
		m_pData->mMaterials[index]->Get<aiString>(AI_MATKEY_TEXTURE_DIFFUSE(0) , textureName);
		if (textureName.length == 0)
			this->m_Material.textureID = 0;
		else
		{
			//find directory path
			const char *last = strrchr(modelFileName,'/') ;
			if(last == NULL)
				last = strrchr(modelFileName,'\\');
			int lastIndex = last - modelFileName;
			char *filePath = new char[lastIndex + 2 + textureName.length];
			strncpy(filePath,modelFileName,lastIndex + 1);
			strncpy(filePath + lastIndex + 1,textureName.data,textureName.length);
			filePath[lastIndex + 1 + textureName.length] = '\0';
			
			//load texture
			this->m_Material.textureID = LoadTexture(filePath , ORIGIN_TOP_LEFT , true);

			delete[] filePath;
		}
	}

#if USE_DISPLAY_LIST
	//create display list
	
	this->CreateDisplayList();
	this->ReleaseData();
#endif
}

Model::~Model()
{
#if USE_DISPLAY_LIST
	glDeleteLists(this->m_displayList , 1);
#else
	this->ReleaseData();
#endif
	glDeleteTextures(1 , &this->m_Material.textureID);
}

void Model::ReleaseData()
{
	if (m_pData)
	{
		aiReleaseImport(m_pData);
		m_pData = NULL;
	}
	if (m_indices)
	{
		delete[] m_indices;
		m_indices = NULL;
	}
}


#if USE_DISPLAY_LIST
void Model ::CreateDisplayList()
{
	this->m_displayList = glGenLists(1);
	glNewList(this->m_displayList , GL_COMPILE);
#else
void Model :: Render() {
#endif
	//if has texture
	if (m_pData->mMeshes[0]->HasTextureCoords(0) && this->m_Material.textureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D , this->m_Material.textureID);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2 , GL_FLOAT , sizeof(aiVector3D) , m_pData->mMeshes[0]->mTextureCoords[0]);
	}
	else
		glDisable(GL_TEXTURE_2D);
	
	//if has normals
	if (m_pData->mMeshes[0]->HasNormals())
	{
		glMaterialfv(GL_FRONT , GL_DIFFUSE , this->GetMaterial().diffuse);
		glMaterialfv(GL_FRONT , GL_SPECULAR , this->GetMaterial().specular);
		glMaterialfv(GL_FRONT , GL_AMBIENT , this->GetMaterial().diffuse);
		glMaterialfv(GL_FRONT , GL_EMISSION , this->GetMaterial().emissive);
		glMaterialf(GL_FRONT , GL_SHININESS , this->GetMaterial().shininess);

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT , sizeof(aiVector3D) ,m_pData->mMeshes[0]->mNormals);
	}

	glVertexPointer(3 , GL_FLOAT , sizeof(aiVector3D) , m_pData->mMeshes[0]->mVertices);

	//draw
	glDrawElements(GL_TRIANGLES , m_numIndices , GL_UNSIGNED_SHORT , m_indices);
	

	//if has texture
	if (m_pData->mMeshes[0]->HasTextureCoords(0) && this->m_Material.textureID != 0)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	else
		glEnable(GL_TEXTURE_2D);
	
	//if has normals
	if (m_pData->mMeshes[0]->HasNormals())
		glDisableClientState(GL_NORMAL_ARRAY);

#if USE_DISPLAY_LIST
	glEndList();
#endif
}


#if USE_DISPLAY_LIST
void Model::Render()
{
	glCallList(this->m_displayList);
}
#endif