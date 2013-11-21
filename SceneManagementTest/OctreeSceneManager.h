#ifndef OCTREE_SCENE_MAN_H
#define OCTREE_SCENE_MAN_H

#include "SceneObject.h"

struct OctreeLevel;

#define MAX_NUM_OCTREE_LEVELS 6

/*--------OctreeSceneManager--------------*/
class OctreeSceneManager
{
public:
	OctreeSceneManager(float worldMinX , float worldMaxX , 
						float worldMinY , float worldMaxY ,
						float worldMinZ , float worldMaxZ ,
						unsigned int numLevels) ;
	OctreeSceneManager(const HQAABB &worldBoundingBox ,
						unsigned int numLevels);
	~OctreeSceneManager();
	
	unsigned int GetNumTriangles() const {return m_numTriangles;}//get number of rendered triangles
	unsigned int GetNumVisbleObjects() const {return m_numVisibleObjs;}//get number of visible objects after it's updated by Render() method
	unsigned int GetMaxDepth() const {return m_maxDepth;}

	void InsertObject (SceneObject *object);
	void Render(const HQPlane viewFrustum[6]);
	void RenderAgain(bool onlyDrawBoundingBox = false);
private:

	void Init(const HQAABB &worldBoundingBox);
	OctreeNode* BuildTree(const HQVector4& info , const HQAABB & tightBoundingBox);//<info> = {indexX , indexY , indexZ , level}
	void UpdateAndRenderNode(OctreeNode *node);
	void RenderNode(OctreeNode *node);
	void RenderNodeBoundingBox(OctreeNode *node);
	void DrawBoundingBox(OctreeNode *node);

	const HQPlane *m_viewFrustum;
	HQVector4 m_worldBBoxMin;//min vector of the world 's bounding box
	float m_minWorldBBoxEdge;//length of the shortest edge of the world 's bounding box
	OctreeLevel * m_level;
	unsigned int m_maxDepth;//last level
	unsigned int m_numVisibleObjs;//number of visible objects
	unsigned int m_numTriangles;//number of visible triangles;
};

#endif