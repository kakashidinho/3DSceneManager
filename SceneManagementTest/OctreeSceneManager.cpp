#include "OctreeSceneManager.h"
#include <iostream>
#include <math.h>
#include "glHelper.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MARK_TOTAL_OBJECTS 1

#define VISIBLE 0x1
const float _1overLog2 = 1.4426950408889634073599246810019f;//1.0f / logf(2);

/*----OctreeNode------------*/
struct OctreeNode
{
	HQAABB m_looseBoundingBox;//loose bounding box
#if MARK_TOTAL_OBJECTS
	OctreeNode *m_parent;
#endif
	OctreeNode *m_child[8];

	SceneObject *m_firstObject;
	unsigned int m_flags;
#if MARK_TOTAL_OBJECTS
	unsigned int m_totalObjects;//number of objects inside this node, including it's descendant's objects
#endif
};
struct OctreeLevel
{
	OctreeLevel(unsigned int level , const HQVector4 &worldBBoxSize) ;
	~OctreeLevel()
	{
		if (m_node != NULL)
		{
			free (m_node);
			m_node = NULL;
		}
	}
	
	inline OctreeNode * GetNode(unsigned int indexX , unsigned int indexY , unsigned int indexZ);
	inline const OctreeNode * GetNode(unsigned int indexX , unsigned int indexY , unsigned int indexZ) const ;
	inline OctreeNode * GetNode(const HQVector4 &index)  
	{
		return GetNode((unsigned int)index.x , (unsigned int)index.y , (unsigned int)index.z);
	}
	inline const OctreeNode * GetNode(const HQVector4 &index) const 
	{
		return GetNode((unsigned int)index.x , (unsigned int)index.y , (unsigned int)index.z);
	}

	OctreeNode * m_node;
	HQVector4 m_tightBoundingBoxSize;
	float m_maxSphereRadius;//max radius of the bounding sphere of the object that can be accommodated at this level

	unsigned int m_step1;//first dimension step for accessing element in <m_node>
	unsigned int m_step2;//second dimension step for accessing element in <m_node>
};

OctreeLevel::OctreeLevel(unsigned int level, const HQVector4 &worldBBoxSize)
: m_step1((0x1 << level) * (0x1 << level)) , 
m_step2(0x1 << level) ,
m_tightBoundingBoxSize(worldBBoxSize / ((float)(0x1 << level)) )
{
	unsigned int numNode = 0x1 << (3 * level);
	this->m_node = (OctreeNode*) malloc(numNode * sizeof(OctreeNode) );
	if (this->m_node == NULL)
		throw std::bad_alloc();
}

inline OctreeNode * OctreeLevel::GetNode(unsigned int indexX , unsigned int indexY , unsigned int indexZ)
{
	return m_node + m_step1 * indexX  + m_step2 * indexY + indexZ;
}
inline const OctreeNode * OctreeLevel::GetNode(unsigned int indexX , unsigned int indexY , unsigned int indexZ) const
{
	return m_node + m_step1 * indexX  + m_step2 * indexY + indexZ;
}


/*----OctreeSceneManager----*/
OctreeSceneManager::OctreeSceneManager(float minX , float maxX , 
									float minY , float maxY ,
									float minZ , float maxZ ,
									unsigned int numLevel)
: m_maxDepth (numLevel - 1) , m_numVisibleObjs(0)
{
	/*-------world bounding box-----------*/
	HQAABB box;
	box.vMin.Set(minX , minY , minZ , 1);
	box.vMax.Set(maxX , maxY , maxZ , 1);

	try{
		this->Init(box);
	}
	catch(std::bad_alloc e)
	{
		throw std::bad_alloc();
	}
}


OctreeSceneManager::OctreeSceneManager(const HQAABB &worldBoundingBox ,
									unsigned int numLevels)
: m_maxDepth (numLevels - 1)
{
	try{
		this->Init(worldBoundingBox);
	}
	catch(std::bad_alloc e)
	{
		throw std::bad_alloc();
	}
}

OctreeSceneManager::~OctreeSceneManager()
{
	if (this->m_level != NULL)
	{
		for (unsigned int i = 0 ; i <= m_maxDepth ; ++i)
		{
			this->m_level[i].~OctreeLevel();
		}
		free(this->m_level);
		this->m_level = NULL;
	}
}

void OctreeSceneManager::Init(const HQAABB &worldBoundingBox)
{
	if (m_maxDepth + 1 > MAX_NUM_OCTREE_LEVELS)
		m_maxDepth = MAX_NUM_OCTREE_LEVELS - 1;
	this->m_worldBBoxMin = worldBoundingBox.vMin;
	this->m_level = (OctreeLevel*) malloc( (m_maxDepth + 1) * sizeof(OctreeLevel) );
	if (this->m_level == NULL)
		throw std::bad_alloc();
	
	//find shortest edge of bounding box
	HQVector4 diagonal = worldBoundingBox.vMax - worldBoundingBox.vMin;
	this->m_minWorldBBoxEdge = min (diagonal.x , diagonal.y);
	this->m_minWorldBBoxEdge = min (this->m_minWorldBBoxEdge , diagonal.z);

	for (unsigned int i = 0 ; i <= m_maxDepth ; ++i)
	{
		try{
			new (this->m_level + i) OctreeLevel(i , diagonal);
			//find max radius of the bounding sphere of the object that can be accommodated at this level
			this->m_level[i].m_maxSphereRadius = 0.5f * this->m_minWorldBBoxEdge / (0x1 << i) ; //1/2 * minEdge / (2 ^ level)
		}
		catch (std::bad_alloc e)
		{
			throw std::bad_alloc();
		}
	}
	
	HQVector4 info(0 , 0 , 0 , 0);
	OctreeNode * root = this->BuildTree(info , worldBoundingBox);
#if MARK_TOTAL_OBJECTS
	root->m_parent = NULL;
#endif
}

OctreeNode* OctreeSceneManager::BuildTree(const HQVector4& info , const HQAABB & tightBox)//<info> = {indexX , indexY , indexZ , level}
{
	OctreeNode *node = this->m_level[(unsigned int)info.w].GetNode(info);
	node->m_flags = 0;
	node->m_firstObject = NULL;
#if MARK_TOTAL_OBJECTS
	node->m_totalObjects = 0;
#endif

	/*---find loose bounding box of this node------*/
	HQVector4 offset = (tightBox.vMax - tightBox.vMin) * 0.5f;
	node->m_looseBoundingBox.vMin = tightBox.vMin - offset;
	node->m_looseBoundingBox.vMax = tightBox.vMax + offset;
	
	/*-------childs------------*/
	if ((unsigned int)info.w == this->m_maxDepth)//this is last level , so this node has no child
	{
		for (int i = 0 ; i < 8 ; ++i)
			node->m_child[i] = NULL;
		return node;
	}
	HQAABB childTightBox;
	HQVector4 childInfo;
	childInfo.w = info.w + 1.0f;//next level
	//up right front child
	childTightBox.vMax = tightBox.vMax;
	childTightBox.vMin = tightBox.vMin + offset;
	childInfo.x = 2.f * info.x + 1.f;
	childInfo.y = 2.f * info.y + 1.f;
	childInfo.z = 2.f * info.z + 1.f;
	node->m_child[0] = this->BuildTree(childInfo , childTightBox);
	//up right back child
	childTightBox.vMax.x = tightBox.vMax.x; 
	childTightBox.vMax.y = tightBox.vMax.y; 
	childTightBox.vMax.z = tightBox.vMax.z - offset.z;
	childTightBox.vMin.x = tightBox.vMin.x + offset.x; 
	childTightBox.vMin.y = tightBox.vMin.y + offset.y; 
	childTightBox.vMin.z = tightBox.vMin.z ;
	childInfo.x = 2.f * info.x + 1.f;
	childInfo.y = 2.f * info.y + 1.f;
	childInfo.z = 2.f * info.z;
	node->m_child[1] = this->BuildTree(childInfo , childTightBox);
	//up left front child
	childTightBox.vMax.x = tightBox.vMax.x - offset.x; 
	childTightBox.vMax.y = tightBox.vMax.y; 
	childTightBox.vMax.z = tightBox.vMax.z;
	childTightBox.vMin.x = tightBox.vMin.x; 
	childTightBox.vMin.y = tightBox.vMin.y + offset.y; 
	childTightBox.vMin.z = tightBox.vMin.z + offset.z;
	childInfo.x = 2.f * info.x ;
	childInfo.y = 2.f * info.y + 1.f;
	childInfo.z = 2.f * info.z + 1.f;
	node->m_child[2] = this->BuildTree(childInfo , childTightBox);
	//up left back child
	childTightBox.vMax.x = tightBox.vMax.x - offset.x; 
	childTightBox.vMax.y = tightBox.vMax.y; 
	childTightBox.vMax.z = tightBox.vMax.z - offset.z;
	childTightBox.vMin.x = tightBox.vMin.x; 
	childTightBox.vMin.y = tightBox.vMin.y + offset.y; 
	childTightBox.vMin.z = tightBox.vMin.z ;
	childInfo.x = 2.f * info.x ;
	childInfo.y = 2.f * info.y + 1.f;
	childInfo.z = 2.f * info.z ;
	node->m_child[3] = this->BuildTree(childInfo , childTightBox);
	//down right front child
	childTightBox.vMax.x = tightBox.vMax.x ; 
	childTightBox.vMax.y = tightBox.vMax.y - offset.y; 
	childTightBox.vMax.z = tightBox.vMax.z ;
	childTightBox.vMin.x = tightBox.vMin.x + offset.x; 
	childTightBox.vMin.y = tightBox.vMin.y ; 
	childTightBox.vMin.z = tightBox.vMin.z + offset.z;
	childInfo.x = 2.f * info.x + 1.f;
	childInfo.y = 2.f * info.y;
	childInfo.z = 2.f * info.z + 1.f;
	node->m_child[4] = this->BuildTree(childInfo , childTightBox);
	//down right back child
	childTightBox.vMax.x = tightBox.vMax.x; 
	childTightBox.vMax.y = tightBox.vMax.y - offset.y; 
	childTightBox.vMax.z = tightBox.vMax.z - offset.z;
	childTightBox.vMin.x = tightBox.vMin.x + offset.x; 
	childTightBox.vMin.y = tightBox.vMin.y; 
	childTightBox.vMin.z = tightBox.vMin.z ;
	childInfo.x = 2.f * info.x + 1.f;
	childInfo.y = 2.f * info.y;
	childInfo.z = 2.f * info.z;
	node->m_child[5] = this->BuildTree(childInfo , childTightBox);
	//down left front child
	childTightBox.vMax.x = tightBox.vMax.x - offset.x; 
	childTightBox.vMax.y = tightBox.vMax.y - offset.y; 
	childTightBox.vMax.z = tightBox.vMax.z;
	childTightBox.vMin.x = tightBox.vMin.x; 
	childTightBox.vMin.y = tightBox.vMin.y; 
	childTightBox.vMin.z = tightBox.vMin.z + offset.z;
	childInfo.x = 2.f * info.x;
	childInfo.y = 2.f * info.y;
	childInfo.z = 2.f * info.z + 1.f;
	node->m_child[6] = this->BuildTree(childInfo , childTightBox);
	//down left back child
	childTightBox.vMax.x = tightBox.vMax.x - offset.x; 
	childTightBox.vMax.y = tightBox.vMax.y - offset.y; 
	childTightBox.vMax.z = tightBox.vMax.z - offset.z;
	childTightBox.vMin.x = tightBox.vMin.x; 
	childTightBox.vMin.y = tightBox.vMin.y; 
	childTightBox.vMin.z = tightBox.vMin.z ;
	childInfo.x = 2.f * info.x;
	childInfo.y = 2.f * info.y;
	childInfo.z = 2.f * info.z;
	node->m_child[7] = this->BuildTree(childInfo , childTightBox);
#if MARK_TOTAL_OBJECTS
	for (int i = 0 ; i < 8 ; ++i)
		node->m_child[i]->m_parent = node;
#endif

	return node;
}

void OctreeSceneManager::InsertObject (SceneObject *object)
{
	/*-------remove from old node----------*/
	if (object->m_treeNode != NULL) 
	{
		OctreeNode * node = object->m_treeNode;
		//remove from object chain
		if(node->m_firstObject == object)//this object is the first object in its containing node's object list
			node->m_firstObject = object->m_nextObject;
#if MARK_TOTAL_OBJECTS
		//decrease total number of objects
		do
		{
			node->m_totalObjects--;
			node = node->m_parent;//decrease predecessor' total objects
		}while (node != NULL);
#endif

	}
	//remove from object chain
	if (object->m_nextObject != NULL)
		object->m_nextObject->m_prevObject = object->m_prevObject;
	if (object->m_prevObject != NULL)
	{
		object->m_prevObject->m_nextObject = object->m_nextObject;
		object->m_prevObject = NULL;
	}
	
	/*----------find new node--------------*/
	const HQSphere &sphere = object->GetBoundingSphere();
	//find depth
	HQVector4 info;
	float f = logf(this->m_minWorldBBoxEdge / sphere.radius) * _1overLog2  - 1.f;//log2(n) = log(n) / log(2)
	info.w = floorf(f);
	unsigned int depth = (unsigned int) info.w;
	if (depth > this->m_maxDepth)
		depth = this->m_maxDepth ;
	OctreeLevel & treeLevel = this->m_level[depth];

	//find closest node
	HQVector4 dV = sphere.center - this->m_worldBBoxMin;
	info.x = floorf(dV.x / treeLevel.m_tightBoundingBoxSize.x);
	info.y = floorf(dV.y / treeLevel.m_tightBoundingBoxSize.y);
	info.z = floorf(dV.z / treeLevel.m_tightBoundingBoxSize.z);

	OctreeNode *node = treeLevel.GetNode(info);
	
	if (node->m_child[0] != NULL)
	{
		//check if object can fit inside any child
		for (int i = 0 ; i < 8 ; ++i)
		{
			if (node->m_child[i]->m_looseBoundingBox.ContainsSphere(sphere))
			{
				node = node->m_child[i];
				break;
			}
		}
	}

	/*-------insert object into new node--------*/
	object->m_treeNode = node;
	//insert into objects list
	object->m_nextObject = node->m_firstObject;
	if (node->m_firstObject != NULL)
		node->m_firstObject->m_prevObject = object;
	node->m_firstObject = object;
	
#if MARK_TOTAL_OBJECTS
	//increase total number of objects
	do
	{
		node->m_totalObjects++;
		node = node->m_parent;//increase predecessor's total objects
	}while (node != NULL);
#endif
}
void OctreeSceneManager::Render(const HQPlane viewFrustum[6])
{
	this->m_viewFrustum = viewFrustum;
	this->m_numVisibleObjs = 0;
	this->m_numTriangles = 0;
	if (
#if MARK_TOTAL_OBJECTS
		m_level[0].m_node->m_totalObjects > 0 &&
#endif
		m_level[0].m_node->m_looseBoundingBox.Cull(this->m_viewFrustum , 6 ) != HQ_CULLED)
	{
		m_level[0].m_node->m_flags = VISIBLE;
		this->UpdateAndRenderNode(m_level[0].m_node );
	}
	else
		m_level[0].m_node->m_flags = 0;
}

void OctreeSceneManager::UpdateAndRenderNode(OctreeNode *node)
{
	/*-------render objects inside this node--------*/
	SceneObject *object = node->m_firstObject;
	
	while (object != NULL)
	{
		this->m_numVisibleObjs ++;
		this->m_numTriangles += object->GetNumTriangles();
		object->Render();
		object = object->m_nextObject;
	}

	/*-----update childs-----------*/
	if (node->m_child[0] != NULL)
	{
		for (int i = 0 ; i < 8 ; ++i)
		{
			if (
#if MARK_TOTAL_OBJECTS
				node->m_child[i]->m_totalObjects > 0 && 
#endif
				node->m_looseBoundingBox.Cull(this->m_viewFrustum , 6 ) != HQ_CULLED)
			{
				node->m_child[i]->m_flags = VISIBLE;
				this->UpdateAndRenderNode (node->m_child[i]);
			}
			else
				node->m_child[i]->m_flags = 0;
		}
	}
}


void OctreeSceneManager::RenderAgain(bool onlyDrawBoundingBox)
{
	if (m_level[0].m_node->m_flags == VISIBLE)
	{
		if (onlyDrawBoundingBox)
		{
			glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_TEXTURE_2D);
			glColor3f(1 , 0 , 0);
			glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
			
			this->RenderNodeBoundingBox(m_level[0].m_node );

			glColor3f(1 , 1 , 1);
			glPopAttrib();
		}
		else
		{
			this->RenderNode(m_level[0].m_node );
		}
	}
}

void OctreeSceneManager::RenderNode(OctreeNode *node )
{
	SceneObject *object = node->m_firstObject;
	
	/*------render objects in this node----------*/
	while (object != NULL)
	{
		object->Render();
		object = object->m_nextObject;
	}
	/*------render childs--------*/
	if (node->m_child[0] != NULL)
	{
		for (int i = 0 ; i < 8 ; ++i)
		{
			if (node->m_child[i]->m_flags == VISIBLE)
				this->RenderNode (node->m_child[i] );
		}
	}
}

void OctreeSceneManager::RenderNodeBoundingBox(OctreeNode *node )
{
	SceneObject *object = node->m_firstObject;
	if (object != NULL)
	{
		//draw loose bounding box of this node
		this->DrawBoundingBox(node);
	}
	
	/*------render childs--------*/
	if (node->m_child[0] != NULL)
	{
		for (int i = 0 ; i < 8 ; ++i)
		{
			if (node->m_child[i]->m_flags == VISIBLE)
				this->RenderNodeBoundingBox (node->m_child[i] );
		}
	}
}

void OctreeSceneManager::DrawBoundingBox(OctreeNode *node)
{
	glBegin(GL_QUADS);
		
	//front
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMax.z);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMax.z);
	glVertex3fv(node->m_looseBoundingBox.vMax.v);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMax.z);
	
	//back
	glVertex3fv(node->m_looseBoundingBox.vMin.v);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMin.z);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMin.z);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMin.z);

	//left
	glVertex3fv(node->m_looseBoundingBox.vMin.v);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMax.z);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMax.z);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMin.z);

	//right
	glVertex3fv(node->m_looseBoundingBox.vMax.v);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMax.z);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMin.z);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMin.z);

	//top
	glVertex3fv(node->m_looseBoundingBox.vMax.v);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMin.z);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMin.z);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMax.y , node->m_looseBoundingBox.vMax.z);

	//bottom
	glVertex3fv(node->m_looseBoundingBox.vMin.v);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMin.z);
	glVertex3f(node->m_looseBoundingBox.vMax.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMax.z);
	glVertex3f(node->m_looseBoundingBox.vMin.x , node->m_looseBoundingBox.vMin.y , node->m_looseBoundingBox.vMax.z);
	glEnd();
}