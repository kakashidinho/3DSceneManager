#ifndef HQ_PNG_H
#define HQ_PNG_H

#include "ImgByteStream.h"
#include "ImgLoader.h"

struct png_struct_def;
struct png_info_def;

class PNGImg
{
public:
	PNGImg();
	~PNGImg();

	int Load(ImgByteStream &stream, hq_ubyte8* & pixelDataOut,
			hq_uint32 &width,hq_uint32 &height,hq_short16 &bits ,
			hq_uint32 &imgSize, SurfaceFormat &format, 
			OutputRGBLayout layout);

	bool IsPNG(ImgByteStream &stream);
	void Clear();
private:
	ImgByteStream *pStream;
	png_struct_def* png_ptr;
	png_info_def* info_ptr;
};



#endif