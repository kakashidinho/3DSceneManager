#include "TreeObject.h"
#include "OctreeSceneManager.h"
#include "Common.h"
#include "Terrain.h"

TreeObject :: TreeObject(OctreeSceneManager *manager ,
		Model *model , 
		unsigned int row , unsigned int col ,  
		float scale)
		:NonAnimObject(
		manager , 
		model , 
		HQVector4(0 , 0 , 0) , 
		scale)
{
	HQVector4 surfaceNormal , cross;
	HQMatrix4 rotation;
	bool insideWorld ;
	float rootX , rootZ;

	do {
		insideWorld = true;

		rootX = ge_terrain->GetStartX() + ge_terrain->GetCellSizeX() * col + ge_terrain->GetCellSizeX() / 4.0f;
		rootZ = ge_terrain->GetStartZ() + ge_terrain->GetCellSizeZ() * row + ge_terrain->GetCellSizeZ() / 4.0f;

		RelativePostionInfo info;
		ge_terrain->GetRelativePosInfo(rootX , rootZ ,info);
		ge_terrain->GetSurfaceNormal(info , surfaceNormal);

		this->currentBoundSphere.center = HQVector4(rootX , ge_terrain->GetHeight(info) , rootZ) 
			+ surfaceNormal * (model->GetBoundingBox().vMax.y - model->GetBoundingBox().vMin.y) * scale * 0.45f;
		
		//check if tree bounding sphere is inside world
		if (currentBoundSphere.center.x + currentBoundSphere.radius > ge_terrain->GetMaxX() - 0.001f)
		{
			insideWorld = false;
			col--;
		}
		else if (currentBoundSphere.center.x - currentBoundSphere.radius < ge_terrain->GetStartX() + 0.001f)
		{
			insideWorld = false;
			col++;
		}
		if (currentBoundSphere.center.z + currentBoundSphere.radius > ge_terrain->GetMaxZ() - 0.001f)
		{
			insideWorld = false;
			row--;
		}
		else if (currentBoundSphere.center.z - currentBoundSphere.radius < ge_terrain->GetStartZ() + 0.001f)
		{
			insideWorld = false;
			row++;
		}
	}
	while (!insideWorld);

	cross.Cross(HQVector4(0 , 1 , 0) , surfaceNormal);
	rotation.RotateAxisUnit(cross , cross.Length());
		
		
	this->transform = rotation * this->transform;

	this->transform._41 = currentBoundSphere.center.x;
	this->transform._42 = currentBoundSphere.center.y;
	this->transform._43 = currentBoundSphere.center.z;

	if (manager != NULL)
		manager->InsertObject(this);
}