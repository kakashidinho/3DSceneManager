#ifndef STATIC_OBJ_H
#define STATIC_OBJ_H
#include "Model.h"
#include "SceneObject.h"
#include "Terrain.h"

/*--------non animated object-------------*/
class NonAnimObject : public SceneObject
{
public:
	//construct static object, note that this object will not be inserted into octree by this constructor
	NonAnimObject(
		OctreeSceneManager *manager ,
		Model *model , 
		const HQVector4& position = HQVector4(0,0,0,1) , //position of the center of the object's bounding sphere
		float scale = 1.0f);

	~NonAnimObject();

	const Model *GetModel() const {return model;}
	const HQSphere & GetBoundingSphere() const {return currentBoundSphere ;};
	unsigned int GetNumTriangles() const {return model->GetNumTriangles();}

	void Update(float dt) {};
	void UpdateWithoutSceneManager(float dt) {}
	void Render();
protected:
	HQSphere currentBoundSphere;
	HQMatrix4 transform;
	Model * model;
};

#endif