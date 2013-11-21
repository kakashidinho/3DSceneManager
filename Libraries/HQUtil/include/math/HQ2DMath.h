/*********************************************************************
*Copyright 2010 Le Hoang Quyen. All rights reserved.
*********************************************************************/
#ifndef _2DMATH_
#define _2DMATH_
#include "HQUtilMathCommon.h"

class HQ_UTIL_MATH_API HQPoint
{
public:
	hq_int32 X;
	hq_int32 Y;
};

class HQ_UTIL_MATH_API HQRect
{
public:
	hq_int32 X;
	hq_int32 Y;
	hq_int32 W;
	hq_int32 H;
	bool IsPointInside(hq_int32 X,hq_int32 Y) const;
};

#include "HQ2DMathInline.h"
#endif