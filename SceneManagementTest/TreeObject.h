#ifndef TREE_OBJ_H
#define TREE_OBJ_H

#include "NonAnimObject.h"
class TreeObject : public NonAnimObject
{
public:
	TreeObject (OctreeSceneManager *manager ,
		Model *model , 
		unsigned int row , unsigned int col , //index of the cell in terrain in which of the tree'root is
		float scale = 1.0f
		 );

	~TreeObject() {}


};

#endif