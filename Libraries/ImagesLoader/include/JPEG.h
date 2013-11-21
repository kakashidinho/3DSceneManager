#ifndef HQ_JPEG_H
#define HQ_JPEG_H
#include "HQPrimitiveDataType.h"
#include "ImgByteStream.h"

struct jpeg_decompress_struct;
struct jpeg_error_mgr;

class JPEGImg
{
public:
	JPEGImg();
	~JPEGImg();

	int Load(ImgByteStream &stream, hq_ubyte8* & pixelDataOut,
			hq_uint32 &width,hq_uint32 &height,hq_short16 &bits ,
			hq_uint32 &imgSize, bool &isGreyscale);

	bool IsJPEG(ImgByteStream &stream);
	void Clear();
private:
	ImgByteStream *pStream;
	jpeg_decompress_struct * dobj;
	jpeg_error_mgr* jerr;
};



#endif