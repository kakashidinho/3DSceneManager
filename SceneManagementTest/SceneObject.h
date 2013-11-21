#ifndef OBJECT_H
#define OBJECT_H

#ifndef CMATH
#	define CMATH
#endif
#include "../Libraries/HQUtil/include/math/HQUtilMath.h"

struct OctreeNode;
class OctreeSceneManager;

/*-------abstract class for octree scene manager----------*/
class SceneObject
{
public:
	SceneObject(OctreeSceneManager *manager = NULL) 
		: m_manager (manager) , m_treeNode(NULL) ,
		m_nextObject(NULL) , m_prevObject(NULL)
	{}

	virtual ~SceneObject() {} 
	virtual void Update(float dt) = 0;
	virtual void UpdateWithoutSceneManager(float dt) = 0;//update this object's state without scene manager
	virtual void Render() = 0;

	virtual const HQSphere & GetBoundingSphere() const = 0;
	virtual unsigned int GetNumTriangles() const = 0;//get number of visible triangles
	
	OctreeSceneManager * GetSceneManager() {return m_manager;}
	const OctreeSceneManager * GetSceneManager() const {return m_manager;}


	/*-----these attributes are used in octree scene manager---------*/
	OctreeNode * m_treeNode;
	SceneObject *m_nextObject;
	SceneObject *m_prevObject;
private:
	OctreeSceneManager *m_manager;
};

#endif