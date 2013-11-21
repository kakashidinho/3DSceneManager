#ifndef SHPERE_OBJ_H
#define SHPERE_OBJ_H

#include "NonAnimObject.h"

extern float ge_gravityAcce;//gravity acceleration

/*----------sphere object------------------*/
class SphereObject : public NonAnimObject
{
public:
	SphereObject(
		OctreeSceneManager *manager ,
		Model *model , 
		const HQVector4& position = HQVector4(0,0,0,1) , //position of the center of the object's bounding sphere
		const HQVector4& velocity = HQVector4(0,0,0,0), //time unit is second
		float scale = 1.0f
		);
	~SphereObject();
	
	void Update(float dt);
	void UpdateWithoutSceneManager(float dt);
private:
	
	void ChangeDirection(const HQVector4 &negNormal) ;//change direction after the sphere has collided with surface.<negNormal> is the negation of the surface normal vector.
	//Get position of the sphere when it collides with plane.
	//<negPlaneNormal> is negation of plane normal vector
	void GetCollisionPosWithPlane( 
		const HQVector4 &negPlaneNormal , 
		float distanceToPlane ,
		HQVector4 & posOut) ;
	//collision respond with one of the bounding planes of terrain.
	//<negPlaneNormal> is negation of plane normal vector
	//return the period between prev time and the time when collision occurs
	float CollisionRespondWithPlane(
		const HQVector4 &negPlaneNormal , 
		float distanceToPlane , 
		int index);

	HQVector4 velocity ;
};

#endif