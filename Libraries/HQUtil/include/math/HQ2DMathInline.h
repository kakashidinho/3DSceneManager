/*********************************************************************
*Copyright 2010 Le Hoang Quyen. All rights reserved.
*********************************************************************/
#ifndef _2DMATH_INL_
#define _2DMATH_INL_
inline bool HQRect::IsPointInside(hq_int32 X,hq_int32 Y) const
{
	if(X < this->X || X > (this->X + this->W))
		return false;
	if(Y < this->Y || Y > (this->Y + this->H))
		return false;
	return true;
}
#endif