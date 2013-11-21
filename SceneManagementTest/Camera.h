#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "Terrain.h"

/*-------------------
first person camera
------------------*/
class Camera : public HQA16ByteObject
{
public:
	Camera (float maxVelocity ,float sideMaxVelocity , float cameraHeight);
	
	const HQMatrix4 &GetViewMatrix() const { return view;}
	const HQVector4 &GetPosition() const {return position;}
	const HQVector4 &GetDirection() const {return direction;}
	float GetYaw() const {return yaw;}
	float GetPitch() const {return pitch;}

	void Move(float velocity);
	void Strafe(float velocity);//velocity > 0 => move right , velocity < 0 => move left 
	void RotateX(float angle);//angle > 0 => turn left , angle < 0 => turn right
	void RotateY(float angle);//angle > 0 => look up , angle < 0 => look down
	
	inline void SetCameraHeight(float height)
	{
		cameraHeight = height;
	}
	void Update();
private:
	float velocity ,maxVelocity , sideVelocity , sideMaxVelocity , cameraHeight;
	HQVector4 direction , up , right , position;
	float yaw , pitch;


	HQMatrix4 view;
};

#endif