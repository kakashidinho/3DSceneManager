#ifndef _IMG_LOADER_
#define _IMG_LOADER_


#define IMG_OK 1
#define IMG_FAIL_NOT_SUPPORTED 2
#define IMG_FAIL_FILE_NOT_EXIST 3
#define IMG_FAIL_MEM_ALLOC 4
#define IMG_FAIL_BAD_FORMAT 5
#define IMG_FAIL_CUBE_MAP_LOAD_NOT_SAME_FORMAT 6
#define IMG_FAIL_CANT_GENERATE_MIPMAPS 7
#define IMG_FAIL_NOT_ENOUGH_CUBE_FACES 8


#define GetRfromRGB16(a) ((a & 0xf800)>> 11)
#define GetGfromRGB16(a) ((a & 0x7e0)>> 5)
#define GetBfromRGB16(a) ((a & 0x1f))
#define RGB16(r,g,b) (((r & 0x1f)<< 11) | ((g & 0x3f)<< 5) | (b & 0x1f))
#define SwapRGB16(color) (RGB16 ( GetBfromRGB16(color), GetGfromRGB16(color), GetRfromRGB16(color) ) )

enum ImgOrigin{
	ORIGIN_BOTTOM_LEFT = 0,
	ORIGIN_BOTTOM_RIGHT = 1,
	ORIGIN_TOP_LEFT = 2,
	ORIGIN_TOP_RIGHT = 3
};


//các color channel theo thứ tự byte trọng số lớn nhất đến nhỏ nhất
enum SurfaceFormat{
	FMT_UNKNOWN=0, //unknown
	FMT_R8G8B8=1, //8 bit red, 8 bit green, 8 bit blue
	FMT_B8G8R8=2, //8 bit red, 8 bit green, 8 bit blue
	FMT_A8R8G8B8=3,//8 bit alpha, 8 bit red, 8 bit green, 8 bit blue
	FMT_X8R8G8B8=4,//8 bit không dùng, 8 bit red, 8 bit green, 8 bit blue
	FMT_R5G6B5=5,//5 bit red, 6 bit green, 5 bit blue
	FMT_B5G6R5=6,//5 bit red, 6 bit green, 5 bit blue
	FMT_L8=7,//8 bit greyscale
	FMT_A8L8=8,//8 bit alpha,8 bit greyscale
	FMT_S3TC_DXT1=9,//dạng nén DXT1
	FMT_S3TC_DXT3=10,//dạng nén DXT3
	FMT_S3TC_DXT5=11,//dạng nén DXT5
	FMT_A8B8G8R8=12,//8 bit alpha, 8 bit blue, 8 bit green, 8 bit red
	FMT_X8B8G8R8=13,//8 bit không dùng,8 bit blue, 8 bit green, 8 bit red
	FMT_R8G8B8A8=14,//8 bit red, 8 bit green, 8 bit blue, 8 bit alpha
	FMT_B8G8R8A8=15,//8 bit blue, 8 bit green, 8 bit red, 8 bit alpha
	FMT_A8=16,//8 bit alpha
	FMT_ETC1=17,//dạng nén ETC1
	FMT_PVRTC_RGB_4BPP=18,//dạng nén PVRTC RGB 4 bit
	FMT_PVRTC_RGB_2BPP=19,//dạng nén PVRTC RGB 2 bit
	FMT_PVRTC_RGBA_4BPP=20,//dạng nén PVRTC RGBA 4 bit
	FMT_PVRTC_RGBA_2BPP=21//dạng nén PVRTC RGBA 2 bit
};

enum OutputRGBLayout
{
	LAYOUT_DONT_CARE,
	LAYOUT_RGB,
	LAYOUT_BGR,
};



#include "HQPrimitiveDataType.h"

#endif