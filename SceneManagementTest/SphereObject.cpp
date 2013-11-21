#include "SphereObject.h"
#include "OctreeSceneManager.h"
#include "Common.h"
#include <math.h>

#if defined DEBUG || defined _DEBUG
#include "assert.h"
#endif

float ge_gravityAcce = 9.8f;

#define LIMIT_VELOCITY 0

#if LIMIT_VELOCITY
const float maxVelocity = 10 * ge_gravityAcce;
//const float maxVelocitySqr = maxVelocity * maxVelocity;
#endif

#ifndef sqr
#define sqr(a) (a*a)
#endif


SphereObject::SphereObject(OctreeSceneManager *manager , Model *_model, const HQVector4 &_position , const HQVector4 &_velocity , float _scale)
:  NonAnimObject(manager , _model , _position , _scale) ,  velocity(_velocity)
{
}

SphereObject::~SphereObject()
{
}

void SphereObject::ChangeDirection(const HQVector4 &negNormal)//negNormal must be normalized
{
	//find reflection vector
	HQVector4 v = (negNormal * this->velocity) * negNormal;
	v = this->velocity - v;
	v *= 2;
	this->velocity = v - this->velocity;//reflection vector
}


void SphereObject::GetCollisionPosWithPlane(
		const HQVector4 &negPlaneNormal , 
		float distanceToPlane ,
		HQVector4 &posOut)
{
	float cos;
	HQVector4 normalizedVelocity;
	HQVector4Normalize(&this->velocity , &normalizedVelocity);

	cos = normalizedVelocity * negPlaneNormal;
	float _1overCos = 1.0f / cos;
	float temp1 = this->currentBoundSphere.radius * _1overCos;
	float temp2 = distanceToPlane * _1overCos;

	temp2 -= temp1;
	//collision position of the sphere
	posOut = this->currentBoundSphere.center + temp2 * normalizedVelocity;

}

float SphereObject::CollisionRespondWithPlane(
	const HQVector4 &negPlaneNormal ,
	float distanceToPlane ,  
	int index)
{
	//find collision position of the sphere
	HQVector4 collisionPos;
	this->GetCollisionPosWithPlane(negPlaneNormal , distanceToPlane , collisionPos);
	//find period between prev time and the time when the collision occurs
	float dCTime;
	float s = collisionPos.v[index] - this->currentBoundSphere.center.v[index];
	if (index == 1)//y direction velocity has acceleration
	{
		//s = v0*t + 0.5 * a * t ^ 2;
		//t = (-v0 + sqrt(vo ^ 2 + 2 * a * s )) / a
		dCTime = (-this->velocity.y + sqrtf( sqr( this->velocity.y ) + 2 * ge_gravityAcce * s)) / ge_gravityAcce;
	}
	else//constant velocity , t = s / v
		dCTime = s / this->velocity.v[index];
				
	//calculate velocity when the collision occurs
	this->velocity.y -= dCTime * ge_gravityAcce;

#if LIMIT_VELOCITY
	if (fabs(this->velocity.y) > maxVelocity )
	{
		if (this->velocity.y < 0.0f)
			this->velocity.y = -maxVelocity;
		else
			this->velocity.y = maxVelocity;
	}
#endif
	//change direction
	this->ChangeDirection(negPlaneNormal);
	//change currentBoundSphere.center to collision currentBoundSphere.center
	this->currentBoundSphere.center = collisionPos;

	return dCTime;
}


void SphereObject::Update(float dt)
{
	this->UpdateWithoutSceneManager(dt);

	/*-----scene management ---------*/
	if (this->GetSceneManager() != NULL)
	{
		this->GetSceneManager()->InsertObject(this);
	}
}


void SphereObject::UpdateWithoutSceneManager(float dt)
{
	bool collision = false;
	do {
		HQVector4 dS;//displacement in dt period
		dS.x = velocity.x * dt;
		//collides with left plane?
		if (this->velocity.x < 0)
		{
			float distance = this->currentBoundSphere.center.x - ge_terrain->GetStartX();
			
			if (- (dS.x) + this-> currentBoundSphere.radius > distance)//collision occurs
			{
				collision = true;
				//collision respond
				float dCTime = this->CollisionRespondWithPlane(HQVector4::negX ,distance,  0);
				dt -= dCTime;//remain period
				continue;
			}
			else collision =false;
		}
		//collides with right plane?
		else if (this->velocity.x > 0)
		{
			float distance = ge_terrain->GetMaxX()  - this->currentBoundSphere.center.x ;
			if (dS.x + this-> currentBoundSphere.radius > distance)//collision occurs
			{
				collision = true;
				//collision respond
				float dCTime = this->CollisionRespondWithPlane(HQVector4::posX ,distance, 0);
				dt -= dCTime;//remain period
				continue;
			}
			else collision =false;
		}
		dS.z = velocity.z * dt;
		//collides with back plane?
		if (this->velocity.z < 0)
		{
			float distance = this->currentBoundSphere.center.z - ge_terrain->GetStartZ();
			if (- dS.z + this-> currentBoundSphere.radius > distance)//collision occurs
			{
				collision = true;
				//collision respond
				float dCTime = this->CollisionRespondWithPlane(HQVector4::negZ ,distance, 2);
				dt -= dCTime;//remain period
				continue;
			}
			else collision =false;
		}
		//collides with front plane?
		else if (this->velocity.z > 0)
		{
			float distance = ge_terrain->GetMaxZ()  - this->currentBoundSphere.center.z ;
			if (dS.z + this-> currentBoundSphere.radius > distance)//collision occurs
			{
				collision = true;
				//collision respond
				float dCTime = this->CollisionRespondWithPlane(HQVector4::posZ ,distance, 2);
				dt -= dCTime;//remain period
				continue;
			}
			else collision =false;
		}
		
		float afterVelocityY = this->velocity.y - dt * ge_gravityAcce;//y direction velocity after dt period
		dS.y = (this->velocity.y + afterVelocityY) * 0.5f * dt;
		//collides upper plane of the world bounding box?
		if (this->velocity.y > 0)
		{
			float distance = ge_worldMaxHeight - this->currentBoundSphere.center.y;
			if (dS.y + this->currentBoundSphere.radius > distance)
			{
				collision = true;
				//collision respond
				float dCTime = this->CollisionRespondWithPlane(HQVector4::posY ,distance, 1);
				dt -= dCTime;//remain period
				continue;
			}
			else collision =false;
		}

		this->velocity.y = afterVelocityY;
#if	LIMIT_VELOCITY
		float length = this->velocity.Length();
		if (length > maxVelocity + 0.0001f)
			this->velocity *= maxVelocity / length;
#endif
		this->currentBoundSphere.center += dS;
		
		dt = 0.0f;

#if defined _DEBUG || defined DEBUG
		assert(
			this->currentBoundSphere.center.x <= ge_terrain->GetMaxX() && 
			this->currentBoundSphere.center.x >= ge_terrain->GetStartX() &&
			this->currentBoundSphere.center.z <= ge_terrain->GetMaxZ() && 
			this->currentBoundSphere.center.z >= ge_terrain->GetStartZ()
			);
#endif

		/*--------simple collision detection with terrain---------*/
		HQVector4 normal;
		//collides with terrain?
		if (dS.y < 0.0f)//only check when sphere is falling
		{	
			RelativePostionInfo info;
			ge_terrain->GetRelativePosInfo(this->currentBoundSphere.center.x , this->currentBoundSphere.center.z , info);
			float height = ge_terrain->GetHeight(info);

			if (height +this-> currentBoundSphere.radius > this->currentBoundSphere.center.y)//collision
			{
				ge_terrain->GetSurfaceNormal(info , normal);//get normal vector of collided surface
				this->ChangeDirection( -normal);
				this->currentBoundSphere.center.y = height + this-> currentBoundSphere.radius;
			}
		}
		
#if defined _DEBUG || defined DEBUG
		else	
			assert(currentBoundSphere.center.y > ge_terrain->GetMinHeight());
#endif

		
	}while (collision && dt > 0.0f);
	
	this->transform._41 = currentBoundSphere.center.x;
	this->transform._42 = currentBoundSphere.center.y;
	this->transform._43 = currentBoundSphere.center.z;

#if defined _DEBUG || defined DEBUG
	assert(currentBoundSphere.center.y > ge_terrain->GetMinHeight());
#endif
}