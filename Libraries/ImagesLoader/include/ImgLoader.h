/*********************************************************************
*Copyright 2010 Le Hoang Quyen. All rights reserved.
*********************************************************************/
#ifndef _IMG_LOADER_
#define _IMG_LOADER_


#define IMG_OK 1
#define IMG_FAIL_NOT_SUPPORTED 2
#define IMG_FAIL_FILE_NOT_EXIST 3
#define IMG_FAIL_MEM_ALLOC 4
#define IMG_FAIL_BAD_FORMAT 5
#define IMG_FAIL_CUBE_MAP_LOAD_NOT_SAME_FORMAT 6
#define IMG_FAIL_CANT_GENERATE_MIPMAPS 7


#define GetRfromRGB16(a) ((a & 0xf800)>> 11)
#define GetGfromRGB16(a) ((a & 0x7e0)>> 5)
#define GetBfromRGB16(a) ((a & 0x1f))
#define RGB16(r,g,b) (((r & 0x1f)<< 11) | ((g & 0x3f)<< 5) | (b & 0x1f))

enum ImgOrigin{
	ORIGIN_BOTTOM_LEFT = 0,
	ORIGIN_BOTTOM_RIGHT = 1,
	ORIGIN_TOP_LEFT = 2,
	ORIGIN_TOP_RIGHT = 3
};

#include "Bitmap.h"

#endif