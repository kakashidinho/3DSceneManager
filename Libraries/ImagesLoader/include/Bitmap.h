/*********************************************************************
*Copyright 2010 Le Hoang Quyen. All rights reserved.
*********************************************************************/
#ifndef _BITMAP_
#define _BITMAP_
#include "ImgLoader.h"


enum FileType{
	IMG_UNKNOWN=0x0,
	IMG_TGA=0x1,
	IMG_BMP=0x2,
	IMG_DDS=0x4
};
//các color channel theo thứ tự byte trọng số lớn nhất đến nhỏ nhất
enum SurfaceFormat{
	FMT_UNKNOWN=0, //unknown
	FMT_R8G8B8=1, //8 bit red, 8 bit green, 8 bit blue
	FMT_A8R8G8B8=2,//8 bit alpha, 8 bit red, 8 bit green, 8 bit blue
	FMT_X8R8G8B8=3,//8 bit không dùng, 8 bit red, 8 bit green, 8 bit blue
	FMT_R5G6B5=4,//5 bit red, 6 bit green, 5 bit blue
	FMT_L8=5,//8 bit greyscale
	FMT_A8L8=6,//8 bit alpha,8 bit greyscale
	FMT_S3TC_DXT1=7,//dạng nén DXT1
	FMT_S3TC_DXT3=8,//dạng nén DXT3
	FMT_S3TC_DXT5=9,//dạng nén DXT5
	FMT_A8B8G8R8=10,//8 bit alpha, 8 bit blue, 8 bit green, 8 bit red
	FMT_X8B8G8R8=11,//8 bit không dùng,8 bit blue, 8 bit green, 8 bit red
	FMT_R8G8B8A8=12,//8 bit red, 8 bit green, 8 bit blue, 8 bit alpha
	FMT_B8G8R8A8=13,//8 bit blue, 8 bit green, 8 bit red, 8 bit alpha
	FMT_A8=14//8 bit alpha
};

#define SURFACE_COMPLEX_MIPMAP 0x1
#define SURFACE_COMPLEX_CUBE 0x2
#define SURFACE_COMPLEX_CUBE_POS_X 0x4
#define SURFACE_COMPLEX_CUBE_NEG_X 0x8
#define SURFACE_COMPLEX_CUBE_POS_Y 0x10
#define SURFACE_COMPLEX_CUBE_NEG_Y 0x20
#define SURFACE_COMPLEX_CUBE_POS_Z 0x40
#define SURFACE_COMPLEX_CUBE_NEG_Z 0x80
#define SURFACE_COMPLEX_VOLUME 0x100

struct ComplexSurface{
	unsigned int dwComplexFlags;//độ phức tạp :có mipmap,dạng cube hay volume texture
	unsigned int nMipMap;//số lượng mipmap level
	unsigned int vDepth;//độ sâu của volume texture
};

class Bitmap{
private:
	ComplexSurface complex;//độ phức tạp của dữ liệu ảnh,một số file như .DDS có chứa nhiều hơn 1 ảnh trong nó,bao gồm các level mipmap,các mặt của cube texture..v.v.v
	SurfaceFormat format;//format của pixel ảnh
	FileType ftype;//định dạng file ảnh
	ImgOrigin origin;//vị trí pixel đầu tiên
	unsigned int width,height;//chiều rộng,cao
	short bits;// số bit trên 1 pixel của file ảnh
	unsigned int imgSize;//độ lớn (byte) của dữ liệu ảnh
	unsigned char* pData;//dữ liệu pixel file ảnh
	unsigned char* pTemp;//dữ liệu tạm

	FileType checkfileFormat(unsigned int fileSize);//đọc đuôi file ảnh

	void BilinearFiler24(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//lọc màu 24 bit rgb
	void BilinearFiler32(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//lọc màu 32 bit rgba
	void BilinearFiler16AL(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//lọc màu 16 bit : 8 bit alpha 8 bit greyscale
	void BilinearFiler16RGB(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//lọc màu 16 bit : 5 bit red ,6 bit green, 5 bit blue
	void BilinearFiler8(unsigned char* pPixel,float u,float v,const unsigned int lineSize);//lọc màu 8 bit greyscale hoặc alpha
public:
	Bitmap();
	Bitmap(const Bitmap& source);
	Bitmap(unsigned char* pPixelData,unsigned int width,
		unsigned int height,short bits,
		SurfaceFormat format,ImgOrigin origin,
		ComplexSurface &surfaceComplex);
	~Bitmap();
	void Set(unsigned char* pPixelData,unsigned int width,
		unsigned int height,short bits,
		SurfaceFormat format,ImgOrigin origin,
		ComplexSurface &surfaceComplex);
	void ClearData();//xóa dữ liệu file ảnh lưu trong đối tượng này

	int Load(const char* filename);
	//load 6 cube faces từ 6 file ảnh .
	//Tham số <origin> chỉ tất cả các file ảnh sau khi load xong sẽ chỉnh lại vị trí pixel đầu tiên tương ứng với ví trí <origin>
	//Lưu ý :
	//-Mỗi file ảnh ko dc phép có sẵn hơn 1 mipmap level , và ko dc phép chứa sẳn các mặt của cube map.
	//-Các file ảnh phải cùng pixel format.
	//-Thứ tự file ảnh sẽ dùng để tạo : mặt positive X ,negative X ,positive Y ,negative Y ,positive Z ,negative Z 
 	int LoadCubeFaces(const char *fileNames[6] , ImgOrigin origin , bool generateMipmaps);

	unsigned char* GetPixelData()const{return pData;};//truy vấn dữ liệu pixel ảnh lưu trong đối tượng này
	unsigned int GetWidth()const{return width;};//truy vấn chiều rộng ảnh
	unsigned int GetHeight()const{return height;};//truy vấn chiều cao ảnh
	short GetBits()const{return bits;};//truy vấn số bit trên 1 pixel ảnh
	unsigned int GetImgSize()const{return imgSize;};//truy vấn độ lớn của dữ liệu ảnh
	ImgOrigin GetPixelOrigin()const{return origin;};//truy vấn vị trí pixel đầu tiên nằm ở đâu, góc dưới trái,hay trên phải ..v.v.v.
	SurfaceFormat GetSurfaceFormat()const{return format;};//truy vấn pixel format
	ComplexSurface GetSurfaceComplex()const{return complex;};//truy vấn độ phức tạp của dữ liệu ảnh

	unsigned int GetFirstLevelSize();
	unsigned int CalculateSize(unsigned int width,unsigned int height);//tính size của dữ liệu ảnh có định dạng giống dữ liệu ảnh đang lưu, nếu cho chiều cao và rộng như tham số
	unsigned int CalculateRowSize(unsigned int width);//tính size của 1 hàng của dữ liệu ảnh có định dạng giống dữ liệu ảnh đang lưu, nếu cho chiều cao như tham số

	void GetErrorDesc(int errCode,char *buffer);//xem thông tin về error Code
	//sửa ảnh
	int FlipVertical();//đảo ngược dữ liệu pixel theo chiều dọc
	int FlipHorizontal();//đảo ngược dữ liệu pixel theo chiều ngang
	int SetPixelOrigin(ImgOrigin origin);
	int FlipRGB();//đảo các thành phần RGB->BGR hoặc ngược lại ,hoặc đảo thứ tự little endian -> big endian trong dạng format R5G6B5
	int FlipRGBA();//đảo BGRA->ARGB hoặc ngược lại,hoặc RGBA->ABGR hoặc ngược lại,  pixel data phải ở dạng 32 bit
	int Scalef(float wFactor,float hFactor);//phóng to/thu nhỏ hình ảnh với tỷ lệ wFactor và hFactor
	int Scalei(unsigned int newWidth,unsigned int newHeight);//phóng to/thu nhỏ hình ảnh đến chiều rộng newWidth và chiều cao newHeight
	int DeCompressDXT();//giải nén dạng nén DXT
	int RGB16ToRGBA();//chuyển dạng R5G6B5 thành A8R8G8B8
	int L8ToAL16();//chuyển dạng 8 bit greyscale thành dạng 8 bit greyscale và 8 bit alpha
	int RGB24ToRGBA();//chuyển dạng R8G8B8 thành A8R8G8B8
	int AL16ToRGBA();//chuyển dạng 8 bit greyscale và 8 bit alpha thành A8R8G8B8
	int L8ToRGB();//chuyển dạng 8 bit greyscale thành R8G8B8
	int GenerateMipmaps();//tạo bộ mipmap


};
#endif