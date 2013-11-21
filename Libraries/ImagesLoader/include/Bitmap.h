#ifndef _BITMAP_
#define _BITMAP_
#include "ImgLoader.h"
#include "JPEG.h"
#include "PNGImg.h"

enum FileType{
	IMG_UNKNOWN=0x0,
	IMG_TGA=0x1,
	IMG_BMP=0x2,
	IMG_DDS=0x4,
	IMG_JPEG=0x8,
	IMG_PNG=0x10,
	IMG_KTX=0x20,
	IMG_PVR=0x40
};

#define SURFACE_COMPLEX_MIPMAP 0x1
#define SURFACE_COMPLEX_CUBE 0x2
#define SURFACE_COMPLEX_CUBE_POS_X 0x4
#define SURFACE_COMPLEX_CUBE_NEG_X 0x8
#define SURFACE_COMPLEX_CUBE_POS_Y 0x10
#define SURFACE_COMPLEX_CUBE_NEG_Y 0x20
#define SURFACE_COMPLEX_CUBE_POS_Z 0x40
#define SURFACE_COMPLEX_CUBE_NEG_Z 0x80
#define SURFACE_COMPLEX_FULL_CUBE_FACES (SURFACE_COMPLEX_CUBE_POS_X | SURFACE_COMPLEX_CUBE_NEG_X | SURFACE_COMPLEX_CUBE_POS_Y | SURFACE_COMPLEX_CUBE_NEG_Y | SURFACE_COMPLEX_CUBE_POS_Z | SURFACE_COMPLEX_CUBE_NEG_Z)
#define SURFACE_COMPLEX_VOLUME 0x100

struct SurfaceComplexity{
	hq_uint32 dwComplexFlags;//độ phức tạp :có mipmap,dạng cube hay volume texture
	hq_uint32 nMipMap;//số lượng mipmap level
	hq_uint32 vDepth;//độ sâu của volume texture
};

typedef struct PVR_Header{ 
	unsigned int  dwHeaderSize;  
	unsigned int  dwHeight;  
	unsigned int  dwWidth;   
	unsigned int  dwMipMapCount;  
	unsigned int  dwpfFlags;  
	unsigned int  dwDataSize;  
	unsigned int  dwBitCount;  
	unsigned int  dwRBitMask;  
	unsigned int  dwGBitMask;  
	unsigned int  dwBBitMask;  
	unsigned int  dwAlphaBitMask;  
	unsigned int  dwPVR;   
	unsigned int  dwNumSurfs;  
} PVR_Header; 

/*-----forward declaration-----*/
struct KTX_Header;
struct ImageHierarchy;
class ImgByteStream;

/*----------------------------
the structure of pixel data:

for each array element
	if cube texture :
		for each face
			for each mipmap level
				image data
	else if volume texture :
		for each mipmap level
			for each volume slice
				image data
---------------------------*/
class Bitmap{
private:

	SurfaceComplexity complex;//độ phức tạp của dữ liệu ảnh,một số file như .DDS có chứa nhiều hơn 1 ảnh trong nó,bao gồm các level mipmap,các mặt của cube texture..v.v.v
	SurfaceFormat format;//format của pixel ảnh
	FileType ftype;//định dạng file ảnh
	ImgOrigin origin;//vị trí pixel đầu tiên
	OutputRGBLayout layout;//layout for 24/32 bits ouput color from image
	OutputRGBLayout layout16;//layout for 16 bits ouput color from image
	hq_uint32 width,height;//chiều rộng,cao
	hq_short16 bits;// số bit trên 1 pixel của file ảnh
	hq_uint32 imgSize;//độ lớn (byte) của dữ liệu ảnh
	hq_ubyte8* pData;//dữ liệu pixel file ảnh
	hq_ubyte8* pTemp;//dữ liệu tạm
#if CUBIC_INTERPOLATION
	hqfloat32 cubicCofactor[4][16];//cached cubic interpolation cofactors for at most 4 channels
	hqfloat32 cubicCell[4];//cached cubic interpolation cell (x1,y1,x2,y2)
#endif
	JPEGImg jpegImg;
	PNGImg pngImg;
	ImgByteStream* stream;
	bool pDataOwner;//is this object the owner of pixel data pointer
	
	int LoadPixelData();
	void DeletePixelData();
	void CopyPixelDataIfNotOwner();//make a copy of pixel data if this object is not the owner of pixel data pointer
	FileType CheckfileFormat();//đọc định dạng file dữ liệu ảnh

#if CUBIC_INTERPOLATION
	void InvalidateCubicCache();//invalidate cubic interpolation cache
	void CalCubicCofactors(hqfloat32 u, hqfloat32 v, hqfloat32 lineSize, hquint32 channels);//calculate cubic interpolation cofactors. {channels} = 0 means 16 bit RGB
	void CubicFilter(hqubyte8 *pixel, hqfloat32 u, hqfloat32 v, hquint32 lineSize, hquint32 channels);//. {channels} = 0 means 16 bit RGB
#endif
	void BilinearFiler24(hq_ubyte8* pPixel,hq_float32 u,hq_float32 v,const hq_uint32 lineSize);//lọc màu 24 bit rgb
	void BilinearFiler32(hq_ubyte8* pPixel,hq_float32 u,hq_float32 v,const hq_uint32 lineSize);//lọc màu 32 bit rgba
	void BilinearFiler16AL(hq_ubyte8* pPixel,hq_float32 u,hq_float32 v,const hq_uint32 lineSize);//lọc màu 16 bit : 8 bit alpha 8 bit greyscale
	void BilinearFiler16RGB(hq_ubyte8* pPixel,hq_float32 u,hq_float32 v,const hq_uint32 lineSize);//lọc màu 16 bit : 5 bit red ,6 bit green, 5 bit blue
	void BilinearFiler8(hq_ubyte8* pPixel,hq_float32 u,hq_float32 v,const hq_uint32 lineSize);//lọc màu 8 bit greyscale hoặc alpha

	/*-----loader--------*/
	void GetPixelDataFromStream(hqubyte8* pPixelData, hquint32 totalSize);
	/*--dds------*/
	int LoadDDS();
	/*--bmp------*/
	int LoadBMP();
	int loadBMPRawData(hq_ushort16 linePadding,
				const hq_uint32 lineSize);
	int loadBMPBitMaskData(hq_ushort16 linePadding,
				const hq_uint32 lineSize, hquint32 imgDataOffset);
	/*-tga-------*/
	int LoadTGA();
	void CheckOriginTGA();//kiểm tra pixel đầu tiên của file ảnh nằm ở góc nào,bên dưới trái,hay bên trên trái.v..v.v
	int LoadTGARawData();
	int LoadTGARLEData();
	/*---ktx-----*/
	int LoadKTX();
	int LoadKTXData(const KTX_Header &header);
	bool IsKTX();
	int CheckKTXComplex(const KTX_Header &header);
	int CheckKTXOrigin(hquint32 bytesOfKeyValueData, bool swapEndianess);
	int GetKTXFormatAndPixelBits(const KTX_Header &header);
	int CreateKTXPixelDataHolder(const KTX_Header &header, ImageHierarchy &imageHierarchy);

	/*---pvr------*/
	int LoadPVR();
	int LoadPVRData();
	int CheckPVRFormat(const PVR_Header& header);
public:
	Bitmap();
	Bitmap(const Bitmap& source);
	Bitmap(const hq_ubyte8* pPixelData,hq_uint32 width,
		hq_uint32 height,hq_short16 bits,hquint32 imgSize,
		SurfaceFormat format,ImgOrigin origin,
		SurfaceComplexity &surfaceComplex);
	~Bitmap();
	void SetLoadedOutputRGBLayout(OutputRGBLayout layout) {this->layout = layout;}
	void SetLoadedOutputRGB16Layout(OutputRGBLayout layout) {this->layout16 = layout;}

	void Wrap(const hq_ubyte8* pPixelData,hq_uint32 width,
		hq_uint32 height,hq_short16 bits,hquint32 imgSize,
		SurfaceFormat format,ImgOrigin origin,
		SurfaceComplexity &surfaceComplex);
	void Set(hq_ubyte8* pPixelData,hq_uint32 width,
		hq_uint32 height,hq_short16 bits,hquint32 imgSize,
		SurfaceFormat format,ImgOrigin origin,
		SurfaceComplexity &surfaceComplex);
	void ClearData();//xóa dữ liệu file ảnh lưu trong đối tượng này
	
	int Load(const char* filename);
	int LoadFromMemory(const hq_ubyte8* pImgFileData , hq_uint32 size);

	//load 6 cube faces từ 6 file ảnh .
	//Tham số <origin> chỉ tất cả các file ảnh sau khi load xong sẽ chỉnh lại vị trí pixel đầu tiên tương ứng với ví trí <origin>
	//Lưu ý :
	//-Mỗi file ảnh ko dc phép có sẵn hơn 1 mipmap level , và ko dc phép chứa sẳn các mặt của cube map.
	//-Các file ảnh phải cùng pixel format.
	//-Thứ tự file ảnh sẽ dùng để tạo : mặt positive X ,negative X ,positive Y ,negative Y ,positive Z ,negative Z 
 	int LoadCubeFaces(const char *fileNames[6] , ImgOrigin origin , bool generateMipmaps);
	int LoadCubeFacesFromMemory(const hq_ubyte8 *fileDatas[6] , hq_uint32 fileSizes[6], ImgOrigin origin , bool generateMipmaps);

	bool IsCompressed() const;
	bool IsPVRTC() const;
	void GetPVRHeader(PVR_Header &header) const;
	hq_ubyte8* GetPixelData(){return pData;};//truy vấn dữ liệu pixel ảnh lưu trong đối tượng này
	const hq_ubyte8* GetPixelData()const{return pData;};//truy vấn dữ liệu pixel ảnh lưu trong đối tượng này
	hq_uint32 GetWidth()const{return width;};//truy vấn chiều rộng ảnh
	hq_uint32 GetHeight()const{return height;};//truy vấn chiều cao ảnh
	hq_short16 GetBits()const{return bits;};//truy vấn số bit trên 1 pixel ảnh
	hq_uint32 GetImgSize()const{return imgSize;};//truy vấn độ lớn của dữ liệu ảnh
	ImgOrigin GetPixelOrigin()const{return origin;};//truy vấn vị trí pixel đầu tiên nằm ở đâu, góc dưới trái,hay trên phải ..v.v.v.
	SurfaceFormat GetSurfaceFormat()const{return format;};//truy vấn pixel format
	SurfaceComplexity GetSurfaceComplex()const{return complex;};//truy vấn độ phức tạp của dữ liệu ảnh

	hq_uint32 GetFirstLevelSize() const;
	hq_uint32 CalculateSize(hq_uint32 width,hq_uint32 height) const;//tính size của dữ liệu ảnh có định dạng giống dữ liệu ảnh đang lưu, nếu cho chiều cao và rộng như tham số
	hq_uint32 CalculateRowSize(hq_uint32 width) const;//tính size của 1 hàng pixel/ block của dữ liệu ảnh có định dạng giống dữ liệu ảnh đang lưu, nếu cho chiều rộng như tham số

	void GetErrorDesc(int errCode,char *buffer);//xem thông tin về error Code
	//sửa ảnh
	int FlipVertical();//đảo ngược dữ liệu pixel theo chiều dọc
	int FlipHorizontal();//đảo ngược dữ liệu pixel theo chiều ngang
	int SetPixelOrigin(ImgOrigin origin);
	int FlipRGB();//đảo các thành phần RGB->BGR hoặc ngược lại ,hoặc đảo thứ tự little endian -> big endian trong dạng format R5G6B5
	int FlipRGBA();//đảo BGRA->ARGB hoặc ngược lại,hoặc RGBA->ABGR hoặc ngược lại,  pixel data phải ở dạng 32 bit
	int Scalef(hq_float32 wFactor,hq_float32 hFactor);//phóng to/thu nhỏ hình ảnh với tỷ lệ wFactor và hFactor
	int Scalei(hq_uint32 newWidth,hq_uint32 newHeight);//phóng to/thu nhỏ hình ảnh đến chiều rộng newWidth và chiều cao newHeight
	int DeCompress(bool flipRGB = false);//giải nén ra _R8G8B8/ _B8G8R8
	int DeCompressDXT(bool flipRGB = false);//giải nén dạng nén DXT1 thành A8R8G8B8/ A8B8G8R8, DXT3/5 thành A8R8G8B8/ A8B8G8R8
	int DeCompressETC(bool flipRGB = false);//giải nén ETC1 ra R8G8B8/B8G8R8

	int RGBAToRGB24(bool flipRGB = false);//chuyển A8R8G8B8(X8R8G8B8) thành R8G8B8/B8G8R8 hoặc A8B8G8R8(X8B8G8R8) thành B8G8R8/R8G8B8
	int RGB16ToRGB24(bool flipRGB = false);//chuyển dạng R5G6B5 thành R8G8B8/B8G8R8 nếu flipRGB hoặc B5G6R5 thành B8G8R8/R8G8B8 nếu flipRGB
	int RGB16ToRGBA(bool flipRGB = false);//chuyển dạng R5G6B5 thành A8R8G8B8/A8B8G8R8 nếu flipRGB hoặc B5G6R5 thành A8B8G8R8/A8R8G8B8 nếu flipRGB
	int L8ToAL16();//chuyển dạng 8 bit greyscale thành dạng 8 bit greyscale và 8 bit alpha
	int RGB24ToRGBA(bool flipRGB = false);//chuyển dạng R8G8B8 thành A8R8G8B8/A8B8G8R8 nếu flipRGB hoặc B8G8R8 thành A8B8G8R8/A8R8G8B8 nếu flipRGB
	int AL16ToRGBA(bool flipRGB = false);//chuyển dạng 8 bit greyscale và 8 bit alpha thành A8R8G8B8 hoặc A8B8G8R8 nếu flipRGB = true
	int L8ToRGB(bool flipRGB = false);//chuyển dạng 8 bit greyscale thành R8G8B8 hoặc B8G8R8 nếu flipRGB
	int L8ToRGBA(bool flipRGB = false);//chuyển dạng 8 bit greyscale thành A8R8G8B8 hoặc A8B8G8R8 nếu flipRGB
	int GenerateMipmaps();//tạo bộ mipmap


};
#endif