#include "Camera.h"
#include "Common.h"

Camera :: Camera (float maxVelocity ,float sideMaxVelocity , float cameraHeight)
{
	this->maxVelocity = fabs(maxVelocity);
	this->sideMaxVelocity = fabs(sideMaxVelocity);
	this->velocity = 0.0f;
	this->sideVelocity = 0.0f;
	this->cameraHeight = cameraHeight;

	this->position.x = (ge_terrain->GetStartX() + ge_terrain->GetMaxX()) / 2.0f;
	this->position.z = (ge_terrain->GetStartZ() + ge_terrain->GetMaxZ()) / 2.0f;
	this->position.y = ge_terrain->GetHeight(ge_terrain->GetNumVertZ() / 2, ge_terrain->GetNumVertX() / 2) 
		+ this->cameraHeight;
	this->position.w = 1.0f;

	this->pitch = 0;
	this->yaw = - 3 * HQPiFamily::_PIOVER4;
}


void Camera :: Move(float velocity)
{
	this->velocity = velocity;
}
void Camera :: Strafe(float velocity)
{
	this->sideVelocity = velocity;
}
void Camera :: RotateX(float angle)//angle > 0 => turn left , angle < 0 => turn right
{
	if (angle > HQPiFamily::_2PI)
		angle = HQPiFamily::_2PI;
	this->yaw += angle;
}
void Camera :: RotateY(float angle)//angle > 0 => look up , angle < 0 => look down
{
	if (angle > HQPiFamily::_2PI)
		angle = HQPiFamily::_2PI;
	this->pitch += angle;
}
void Camera :: Update()
{
	HQMatrix4 matrix;
	if (pitch > HQPiFamily::_PIOVER2)
		pitch = HQPiFamily::_PIOVER2;
	else if (pitch < -HQPiFamily::_PIOVER2)
		pitch = -HQPiFamily::_PIOVER2;
	
	if (yaw > HQPiFamily::_2PI)
		yaw -= HQPiFamily::_2PI;
	else if (yaw < 0.0f)
		yaw += HQPiFamily::_2PI;
	
	if (fabs(this->velocity) > this->maxVelocity)
	{
		if (this->velocity < 0.0f)
			this->velocity = -this->maxVelocity;
		else
			this->velocity = this->maxVelocity;
	}

	if (fabs(this->sideVelocity) > this->sideMaxVelocity)
	{
		if (this->sideVelocity < 0.0f)
			this->sideVelocity = -this->sideMaxVelocity;
		else
			this->sideVelocity = this->sideMaxVelocity;
	}

	this->direction.Set(0,0,-1);
	this->up.Set(0,1,0);
	this->right.Set(1,0,0);
	
	matrix.RotateY(yaw );
	HQVector4TransformNormal(&direction , &matrix , &direction);
	HQVector4TransformNormal(&right , &matrix , &right);

	matrix.RotateAxisUnit(right , pitch);
	HQVector4TransformNormal(&up , &matrix, &up);
	HQVector4TransformNormal(&direction , &matrix, &direction);

	/*-------correct float errors------------*/
	direction.Normalize();
	right.Cross(direction , up );
	right.Normalize();
	up.Cross(right , direction);
	up.Normalize();

	/*---------------------------------------*/
	if (this->velocity != 0.0f)
		this->position += this->direction * this->velocity;
	if (this->sideVelocity != 0.0f)
		this->position += this->sideVelocity * this->right;
	/*-----out of terrain check-----------*/
	if (this->position.x < ge_terrain->GetStartX() + 0.0001f)
		this->position.x = ge_terrain->GetStartX();
	else if (this->position.x > ge_terrain->GetMaxX() - 0.0001f)
		this->position.x = ge_terrain->GetMaxX();

	if (this->position.z < ge_terrain->GetStartZ()  + 0.0001f)
		this->position.z = ge_terrain->GetStartZ();
	else if (this->position.z > ge_terrain->GetMaxZ()  - 0.0001f)
		this->position.z = ge_terrain->GetMaxZ();
	
	/*---------------get height----------------------*/
	this->position.y = ge_terrain->GetHeight(this->position.x  , this->position.z ) + this->cameraHeight;
	if (this->position.y > ge_worldMaxHeight - 1.0001f)
		this->position.y = ge_worldMaxHeight - 1.0001f;

	HQVector4 CameraOz = -direction;
	HQMatrix4rView(&right , &up , &CameraOz , &position , &view);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view);
}