#include "Mode2D.h"
#include "Common.h"

//static data
Mode2D::StaticStuff Mode2D::staticStuff;
/*--------------------------*/
void Mode2D::Begin()
{
	glViewport(0 , 0 , WINDOW_WIDTH ,WINDOW_HEIGHT);
	glDepthMask(GL_FALSE);//disable depth writting

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(Mode2D::staticStuff._2DViewMatrix);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(Mode2D::staticStuff._2DProjMatrix);
}

void Mode2D::End()
{	
	glDepthMask(GL_TRUE);//enable depth writting

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void Mode2D::DrawRect(const HQRect & rect , GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D , texture);

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f( 0 , 0 ); glVertex3i(rect.X , rect.Y , 1);
	glTexCoord2f( 0 , 1 ); glVertex3i(rect.X , rect.Y + rect.H, 1);
	glTexCoord2f( 1 , 0 ); glVertex3i(rect.X + rect.W , rect.Y , 1);
	glTexCoord2f( 1 , 1 ); glVertex3i(rect.X + rect.W , rect.Y + rect.H , 1);
	glEnd();
}

/*------------Mode2D::StaticStuff-------*/
Mode2D::StaticStuff::StaticStuff()
{
	HQMatrix4rView(&HQVector4(1,0,0) , &HQVector4(0,-1,0) , &HQVector4(0,0,-1) ,
					&HQVector4(WINDOW_WIDTH / 2.0f , WINDOW_HEIGHT / 2.0f , 0.0f) , 
					&this->_2DViewMatrix);

	HQMatrix4rOrthoProjRH(WINDOW_WIDTH , 
						WINDOW_HEIGHT,
						1.0f , 2.0f, 
						&this->_2DProjMatrix , 
						HQ_RA_OGL);
}