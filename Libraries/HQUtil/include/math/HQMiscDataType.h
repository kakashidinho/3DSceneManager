/*********************************************************************
*Copyright 2010 Le Hoang Quyen. All rights reserved.
*********************************************************************/
#ifndef HQ_MISC_DATA_TYPE_H
#define HQ_MISC_DATA_TYPE_H

#include "HQPrimitiveDataType.h"

struct HQFloat3
{
	HQFloat3() {}
	HQFloat3(hq_float32 _x , hq_float32 _y , hq_float32 _z)
		: x(_x) , y(_y) , z(_z)
	{}
	
	void Duplicate(hq_float32 duplicatedVal)
	{
		x = y = z = duplicatedVal;
	}

	void Set(hq_float32 _x , hq_float32 _y , hq_float32 _z)
	{
		x = _x ; 
		y = _y ; 
		z = _z ;
	}
	
	operator float * () {return f;}
	operator const float * () const {return f;}

	union{
		struct{
			hq_float32 x , y , z;
		};
		hq_float32 f[3];
	};
};

#endif