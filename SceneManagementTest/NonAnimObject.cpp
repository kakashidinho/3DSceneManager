#include "NonAnimObject.h"
#include "OctreeSceneManager.h"


NonAnimObject::NonAnimObject(OctreeSceneManager *manager , Model *_model, const HQVector4 &_position , float _scale)
:  SceneObject(manager) ,  model(_model)
{
	currentBoundSphere.radius = _model->GetBoundingSphere().radius * _scale;
	currentBoundSphere.center = _position;

	HQMatrix4Scale(_scale , _scale , _scale , &this->transform);
	this->transform._41 = currentBoundSphere.center.x;
	this->transform._42 = currentBoundSphere.center.y;
	this->transform._43 = currentBoundSphere.center.z;
}

NonAnimObject::~NonAnimObject()
{
}


void NonAnimObject::Render()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(this->transform);
	this->model->Render();

	glPopMatrix();
}