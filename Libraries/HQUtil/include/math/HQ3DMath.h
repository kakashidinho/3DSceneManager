//*************************************************************************************
//cross platform 3D math library
//author :Le Hoang Quyen
//mostly use SSE instructions for x86 processors
//and Neon instructions for ARM processors
//Copyright 2011 Le Hoang Quyen. All rights reserved.
//*************************************************************************************
#ifndef _3D_MATH
#define _3D_MATH

#include "HQUtilMathCommon.h"

//=======================================================
//tính nhanh sin và cos cùng 1 lúc

HQ_FORCE_INLINE void HQSincosf( hq_float32 angle, hq_float32* sinOut, hq_float32* cosOut );

//=======================================================

class HQ_UTIL_MATH_API HQBaseMathClass : public HQA16ByteObject
{
public:
	inline HQBaseMathClass() {HQ_ASSERT_ALIGN16(this)}//ensure that address of this object is 16 bytes aligned , it's a must for SSE and NEON.

#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif

//=======================================================
class HQVector4;
class HQMatrix4;
class HQMatrix3x4;
class HQRay3D;
class HQPlane;
class HQAABB;
class HQOBB;
class HQSphere;
class HQQuaternion;

//=============================================================================
//Vector (x,y,z,w) w=1 =>(x,y,z) là tọa độ điểm ,w=0 =>(x,y,z) là tọa độ vector
//=============================================================================
class HQ_UTIL_MATH_API HQVector4 : public HQBaseMathClass{
public:
	static const HQVector4 posX;//(1 , 0 , 0 , 0)
	static const HQVector4 negX;//(-1 , 0 , 0 , 0)
	static const HQVector4 posY;//(0 , 1 , 0 , 0)
	static const HQVector4 negY;//(0 , -1 , 0 , 0)
	static const HQVector4 posZ;//(0 , 0 , 1 , 0)
	static const HQVector4 negZ;//(0 , 0 , -1 , 0)
	static const HQVector4 origin;//(0 , 0 , 0 , 0)


	union{
		struct{hq_float32 x,y,z,w;};
		hq_float32 v[4];
	};

	HQ_FORCE_INLINE HQVector4(){};//create uninitialized vector
	HQ_FORCE_INLINE HQVector4(bool isPoint) : x(0.0f) , y(0.0f) , z(0.0f)
	{if(isPoint)w=1.0f;else w=0.0f;};
	HQ_FORCE_INLINE HQVector4(hq_float32 _x,hq_float32 _y,hq_float32 _z)
		: x (_x) , y (_y) , z(_z) , w (0.0f)
	{};
	HQ_FORCE_INLINE HQVector4(hq_float32 _x,hq_float32 _y,hq_float32 _z,hq_float32 _w)
		: x (_x) , y (_y) , z(_z) , w(_w)
	{};

	HQ_FORCE_INLINE void Set(hq_float32 _x,hq_float32 _y,hq_float32 _z,hq_float32 _w)
	{x=_x;y=_y;z=_z;w=_w;};
	HQ_FORCE_INLINE void Set(hq_float32 _x,hq_float32 _y,hq_float32 _z)
	{x=_x;y=_y;z=_z;};

	HQVector4& Normalize();

	hq_float32 Length();
	hq_float32 LengthSqr()const;

	HQVector4 operator -()const;

	HQVector4& operator +=(const HQVector4& v2);
	HQVector4& operator -=(const HQVector4& v2);
	HQVector4& operator *=(const hq_float32 f);
	HQVector4& operator /=(const hq_float32 f);
	HQVector4& operator *=(const HQMatrix4& m);

	HQVector4 operator +(const HQVector4& v2)const;
	HQVector4 operator -(const HQVector4& v2)const;
	HQVector4 operator *(const hq_float32 f) const;
	HQVector4 operator /(const hq_float32 f) const;
	HQVector4 operator*(const HQMatrix4& m)const;

	hq_float32 operator *(const HQVector4& v2)const;//dot product operator
	HQVector4 Cross(const HQVector4& v2)const;//cross product
	HQVector4& Cross(const HQVector4& v1,const HQVector4& v2);//cross product, result will be stored in this object

	hq_float32 AngleWith(HQVector4& v2);

	operator hq_float32*() {return v;}//casting operator
	operator const hq_float32*() const {return v;}//casting operator

#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif

HQ_UTIL_MATH_API HQ_FORCE_INLINE HQVector4 operator*(const hq_float32 f,const HQVector4& v);
HQ_UTIL_MATH_API HQ_FORCE_INLINE HQVector4* HQVector4Add(const HQVector4* pV1,const HQVector4 *pV2,HQVector4* out);
HQ_UTIL_MATH_API HQ_FORCE_INLINE HQVector4* HQVector4Sub(const HQVector4* pV1,const HQVector4 *pV2,HQVector4* out);
HQ_UTIL_MATH_API HQ_FORCE_INLINE HQVector4* HQVector4Mul(const HQVector4* pV1,hq_float32 f,HQVector4* out);
HQ_UTIL_MATH_API HQ_FORCE_INLINE HQVector4* HQVector4Div(const HQVector4* pV1,hq_float32 f,HQVector4* out);


HQ_UTIL_MATH_API HQVector4* HQVector4Normalize(const HQVector4* in,HQVector4* out);
HQ_UTIL_MATH_API HQVector4* HQVector4Cross(const HQVector4* v1,const HQVector4 *v2,HQVector4* out);
HQ_UTIL_MATH_API HQVector4* HQVector4Transform(const HQVector4* v1,const HQMatrix4* mat,HQVector4* out);
HQ_UTIL_MATH_API HQVector4* HQVector4Transform(const HQVector4* v1,const HQMatrix3x4* mat,HQVector4* out);
HQ_UTIL_MATH_API HQVector4* HQVector4Transform(const HQVector4* v1,const HQQuaternion* quat,HQVector4* out);
HQ_UTIL_MATH_API HQVector4* HQVector4Transform(const HQVector4* v1,const HQOBB* box,HQVector4* out);//biến đổi vào không gian tọa độ có các trục là các trục của hình hộp OBB
HQ_UTIL_MATH_API HQVector4* HQVector4TransformNormal(const HQVector4* v1,const HQMatrix4* mat,HQVector4* out);//biến đổi vector (x,y,z,0) với ma trận 4x4 row major
HQ_UTIL_MATH_API HQVector4* HQVector4TransformNormal(const HQVector4* v1,const HQMatrix3x4* mat,HQVector4* out);//biến đổi vector (x,y,z,0) với 1 ma trận  3x4 (coi như ma trận 4x4 column major với hàng cuối là 0 0 0 1)
HQ_UTIL_MATH_API HQVector4* HQVector4TransformNormal(const HQVector4* v1,const HQOBB* box,HQVector4* out);//biến đổi vector (x,y,z,0) vào không gian tọa độ có các trục là các trục của hình hộp OBB
HQ_UTIL_MATH_API HQVector4* HQVector4TransformCoord(const HQVector4* v1,const HQMatrix4* mat,HQVector4* out);//biến đổi vector (x,y,z,1) với ma trận 4x4 row major
HQ_UTIL_MATH_API HQVector4* HQVector4TransformCoord(const HQVector4* v1,const HQMatrix3x4* mat,HQVector4* out);//biến đổi vector (x,y,z,1) với 1 ma trận  3x4 (coi như ma trận 4x4 column major với hàng cuối là 0 0 0 1)
HQ_UTIL_MATH_API HQVector4* HQVector4TransformCoord(const HQVector4* v1,const HQOBB* box,HQVector4* out);//biến đổi vector (x,y,z,1) vào không gian tọa độ có các trục là các trục của hình hộp OBB
HQ_UTIL_MATH_API HQVector4* HQVector4MultiTransform(const HQVector4* v, hq_uint32 numVec, const HQMatrix4* mat,HQVector4* out);//biến đổi <numVec> vectors bằng 1 ma trận row major
HQ_UTIL_MATH_API HQVector4* HQVector4MultiTransform(const HQVector4* v, hq_uint32 numVec, const HQMatrix3x4* mat,HQVector4* out);//biến đổi <numVec> vectors bằng 1 ma trận row major
HQ_UTIL_MATH_API HQVector4* HQVector4MultiTransformNormal(const HQVector4* v, hq_uint32 numVec, const HQMatrix4* mat,HQVector4* out);//biến đổi <numVec> vectors dạng (x,y,z,0) bằng 1 ma trận row major
HQ_UTIL_MATH_API HQVector4* HQVector4MultiTransformCoord(const HQVector4* v, hq_uint32 numVec, const HQMatrix4* mat,HQVector4* out);//biến đổi <numVec> vectors dạng (x,y,z,1) bằng 1 ma trận row major
HQ_UTIL_MATH_API HQVector4* HQVector4MultiTransformNormal(const HQVector4* v, hq_uint32 numVec, const HQMatrix3x4* mat,HQVector4* out);//biến đổi <numVec> vectors dạng (x,y,z,0) bằng 1 ma trận  3x4 (coi như ma trận 4x4 column major với hàng cuối là 0 0 0 1)
HQ_UTIL_MATH_API HQVector4* HQVector4MultiTransformCoord(const HQVector4* v, hq_uint32 numVec, const HQMatrix3x4* mat,HQVector4* out);//biến đổi <numVec> vectors dạng (x,y,z,1) bằng 1 ma trận  3x4 (coi như ma trận 4x4 column major với hàng cuối là 0 0 0 1)
HQ_UTIL_MATH_API void HQPrintVector4(const HQVector4* pV);

//=======================================================
//Matrix 4x4 - default row major
//=======================================================
class HQ_UTIL_MATH_API HQMatrix4: public HQBaseMathClass{
public:
	union{
		struct{
			hq_float32 _11,_12,_13,_14,
					  _21,_22,_23,_24,
					  _31,_32,_33,_34,
					  _41,_42,_43,_44;
		};
		hq_float32 m[16];
		hq_float32 mt[4][4];
	};
	HQ_FORCE_INLINE HQMatrix4();//construct an identity matrix
	HQ_FORCE_INLINE HQMatrix4(const void * null) {}//this constructor does nothing
	HQ_FORCE_INLINE HQMatrix4(hq_float32 _11, hq_float32 _12, hq_float32 _13, hq_float32 _14,
				hq_float32 _21, hq_float32 _22, hq_float32 _23, hq_float32 _24,
				hq_float32 _31, hq_float32 _32, hq_float32 _33, hq_float32 _34,
				hq_float32 _41, hq_float32 _42, hq_float32 _43, hq_float32 _44);

	HQ_FORCE_INLINE hq_float32& operator[] (hq_uint32 index)
	{
		return m[index];
	};

	HQ_FORCE_INLINE hq_float32& operator() (hq_uint32 row,hq_uint32 col)
	{
		return mt[row][col];
	};

	HQ_FORCE_INLINE const hq_float32& operator[] (hq_uint32 index)const
	{
		return m[index];
	};

	HQ_FORCE_INLINE const hq_float32& operator() (hq_uint32 row,hq_uint32 col)const
	{
		return mt[row][col];
	};

	HQMatrix4& Identity();
	HQMatrix4& Transpose();
	HQMatrix4& Inverse();
	HQMatrix4& Translate(hq_float32 x,hq_float32 y,hq_float32 z);
	HQMatrix4& RotateX(hq_float32 angle);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
	HQMatrix4& RotateY(hq_float32 angle);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
	HQMatrix4& RotateZ(hq_float32 angle);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
	HQMatrix4& RotateAxis(HQVector4& axis,hq_float32 angle);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ ,vector trục quay sẽ dc chuẩn hóa
	HQMatrix4& RotateAxisUnit(const HQVector4& axis,hq_float32 angle);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ,Vector chỉ phương của trục phải đã chuẩn hóa
	HQMatrix4& Scale(hq_float32 sx,hq_float32 sy,hq_float32 sz);
	HQMatrix4& Scale(hq_float32 s[3]);


	HQVector4 operator *(const HQVector4& v)const;
	HQMatrix4 operator *(const HQMatrix4& m)const;

	HQMatrix4& operator *=(const HQMatrix4& m);

	operator hq_float32*() {return m;}//casting operator
	operator const hq_float32*() const {return m;}//casting operator

#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif



HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Multiply(const HQMatrix4* pM1,const HQMatrix4* pM2,HQMatrix4* pOut);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Multiply(const HQMatrix4* pM1,const HQMatrix3x4* pM2,HQMatrix4* pOut); //<pM2> coi như ma trận 4x4 với hàng cuối là (0,0,0,1)
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4MultiMultiply(const HQMatrix4* pM, hq_uint32 numMatrices ,HQMatrix4* pOut);//multiplication of multiple matrices (numMatrices > 1)
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Inverse(const HQMatrix4* pM,hq_float32* Determinant,HQMatrix4* pOut);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Inverse(const HQMatrix4* in,HQMatrix4*out);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Identity(HQMatrix4* inout);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Transpose(const HQMatrix4* in,HQMatrix4 *out);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Scale(hq_float32 sx,hq_float32 sy,hq_float32 sz,HQMatrix4*out);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4Scale(hq_float32 s[3],HQMatrix4*out);
HQ_UTIL_MATH_API void HQPrintMatrix4(const HQMatrix4* pMat);//in ma trận lên màn hình

/*--------------those funcions assume matrices are in row major---------------*/
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rTranslate(hq_float32 x,hq_float32 y,hq_float32 z,HQMatrix4* out);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rRotateX(hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rRotateY(hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rRotateZ(hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rRotateAxis(HQVector4& axis,hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ,vector trục quay sẽ dc chuẩn hóa
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rRotateAxisUnit(const HQVector4& axis,hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ,Vector chỉ phương của trục phải đã chuẩn hóa

HQ_UTIL_MATH_API HQVector4* HQMatrix4rMulVec(const HQMatrix4* mat,const HQVector4* v1,HQVector4* out);//ma trận nhân với vector

HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rView(const HQVector4* pX,const HQVector4* pY,const HQVector4* pZ,const HQVector4* pPos,HQMatrix4 *out);//tạo ma trận phép nhìn với các trục cơ sở của camera Ox Oy Oz và tọa độ điểm đặt
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rLookAtLH(const HQVector4 *pEye,const HQVector4* pAt,const HQVector4* pUp,HQMatrix4*out);//tạo ma trận phép nhìn trong không gian bàn tay trái với điểm đặt camera (mắt) ,điểm nhìn đến,world 's vector up
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rOrthoProjLH(const hq_float32 width,const hq_float32 height,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu trực giao trong không gian bàn tay trái
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rPerspectiveProjLH(const hq_float32 vFOV,const hq_float32 aspect,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu phối cảnh trong không gian bàn tay trái

HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rLookAtRH(const HQVector4 *pEye,const HQVector4* pAt,const HQVector4* pUp,HQMatrix4*out);//tạo ma trận phép nhìn trong không gian bàn tay phải với điểm đặt camera (mắt) ,điểm nhìn đến,world 's vector up
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rOrthoProjRH(const hq_float32 width,const hq_float32 height,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu trực giao trong không gian bàn tay phải
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4rPerspectiveProjRH(const hq_float32 vFOV,const hq_float32 aspect,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu phối cảnh trong không gian bàn tay phải
/*--------------------------------------------------------------------------------------------------------
truy vấn 6 mặt phẳng tạo thành thể tích nhìn pháp vector của 6 mặt phẵng này hướng ra ngoài thể tính nhìn
<pViewProjMatrix> - con trỏ đến ma trận tích của ma trận nhìn và ma trận chiếu
----------------------------------------------------------------------------------------*/
HQ_UTIL_MATH_API void HQMatrix4rGetFrustum(const HQMatrix4 * pViewProjMatrix , HQPlane planes[6] , HQRenderAPI API);


/*--------------those funcions assume matrices are in column major---------------*/
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cTranslate(hq_float32 x,hq_float32 y,hq_float32 z,HQMatrix4* out);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cRotateX(hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cRotateY(hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cRotateZ(hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cRotateAxis(HQVector4& axis,hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ,vector trục quay sẽ dc chuẩn hóa
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cRotateAxisUnit(const HQVector4& axis,hq_float32 angle,HQMatrix4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ,Vector chỉ phương của trục phải đã chuẩn hóa

HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cView(const HQVector4* pX,const HQVector4* pY,const HQVector4* pZ,const HQVector4* pPos,HQMatrix4 *out);//tạo ma trận phép nhìn với các trục cơ sở của camera Ox Oy Oz và tọa độ điểm đặt
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cLookAtLH(const HQVector4 *pEye,const HQVector4* pAt,const HQVector4* pUp,HQMatrix4*out);//tạo ma trận phép nhìn trong không gian bàn tay trái với điểm đặt camera (mắt) ,điểm nhìn đến,world 's vector up
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cOrthoProjLH(const hq_float32 width,const hq_float32 height,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu trực giao trong không gian bàn tay trái
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cPerspectiveProjLH(const hq_float32 vFOV,const hq_float32 aspect,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu phối cảnh trong không gian bàn tay trái

HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cLookAtRH(const HQVector4 *pEye,const HQVector4* pAt,const HQVector4* pUp,HQMatrix4*out);//tạo ma trận phép nhìn trong không gian bàn tay phải với điểm đặt camera (mắt) ,điểm nhìn đến,world 's vector up
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cOrthoProjRH(const hq_float32 width,const hq_float32 height,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu trực giao trong không gian bàn tay phải
HQ_UTIL_MATH_API HQMatrix4* HQMatrix4cPerspectiveProjRH(const hq_float32 vFOV,const hq_float32 aspect,const hq_float32 zNear,const hq_float32 zFar,HQMatrix4*out,HQRenderAPI API);//tạo ma trận chiếu phối cảnh trong không gian bàn tay phải
/*--------------------------------------------------------------------------------------------------------
truy vấn 6 mặt phẳng tạo thành thể tích nhìn pháp vector của 6 mặt phẵng này hướng ra ngoài thể tính nhìn
<pViewProjMatrix> - con trỏ đến ma trận tích của ma trận nhìn và ma trận chiếu
----------------------------------------------------------------------------------------*/
HQ_UTIL_MATH_API void HQMatrix4cGetFrustum(const HQMatrix4 * pViewProjMatrix , HQPlane planes[6] , HQRenderAPI API);


/*-------------------*/


//=======================================================
//Matrix 3x4
//=======================================================
class HQ_UTIL_MATH_API HQMatrix3x4: public HQBaseMathClass{
public:
	union{
		struct{
			hq_float32 _11,_12,_13,_14,
					  _21,_22,_23,_24,
					  _31,_32,_33,_34;
		};
		hq_float32 m[12];
		hq_float32 mt[3][4];
	};

	HQ_FORCE_INLINE HQMatrix3x4();//construct an identity matrix
	HQ_FORCE_INLINE HQMatrix3x4(const void * null) {}//this constructor does nothing
	HQ_FORCE_INLINE HQMatrix3x4(hq_float32 _11, hq_float32 _12, hq_float32 _13,hq_float32 _14,
				hq_float32 _21, hq_float32 _22, hq_float32 _23, hq_float32 _24,
				hq_float32 _31, hq_float32 _32, hq_float32 _33 ,hq_float32 _34);

	HQ_FORCE_INLINE hq_float32& operator[] (hq_uint32 index)
	{
		return m[index];
	};

	HQ_FORCE_INLINE hq_float32& operator() (hq_uint32 row,hq_uint32 col)
	{
		return mt[row][col];
	};

	HQ_FORCE_INLINE const hq_float32& operator[] (hq_uint32 index)const
	{
		return m[index];
	};

	HQ_FORCE_INLINE const hq_float32& operator() (hq_uint32 row,hq_uint32 col)const
	{
		return mt[row][col];
	};

	HQMatrix3x4 operator *(const HQMatrix3x4& m)const;//same as multiplication of two 4x4 matrices that have last column {0,0,0,1}

	HQMatrix3x4& operator *=(const HQMatrix3x4& m);//same as multiplication of two 4x4 matrices that have last column {0,0,0,1}

	operator hq_float32*() {return m;}//casting operator
	operator const hq_float32*() const {return m;}//casting operator

#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif

HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4Multiply(const HQMatrix3x4* pM1,const HQMatrix3x4* pM2,HQMatrix3x4* pOut);
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4MultiMultiply(const HQMatrix3x4* pM, hq_uint32 numMatrices ,HQMatrix3x4* pOut);//multiplication of multiple matrices (numMatrices > 1)
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4Scale(hq_float32 sx,hq_float32 sy,hq_float32 sz,HQMatrix3x4*out);
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4Scale(hq_float32 s[3],HQMatrix3x4*out);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix3x4Inverse(const HQMatrix3x4* pM,hq_float32* Determinant,HQMatrix4* pOut);
HQ_UTIL_MATH_API HQMatrix4* HQMatrix3x4Inverse(const HQMatrix3x4* in,HQMatrix4*out);
HQ_UTIL_MATH_API void HQPrintMatrix3x4(const HQMatrix3x4* pMat);//in ma trận lên màn hình
/*-------------those funcions assume matrices are in column major-----------*/
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4cTranslate(hq_float32 x,hq_float32 y,hq_float32 z,HQMatrix3x4* out);
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4cRotateX(hq_float32 angle,HQMatrix3x4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4cRotateY(hq_float32 angle,HQMatrix3x4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4cRotateZ(hq_float32 angle,HQMatrix3x4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4cRotateAxis(HQVector4& axis,hq_float32 angle,HQMatrix3x4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ,vector trục quay sẽ dc chuẩn hóa
HQ_UTIL_MATH_API HQMatrix3x4* HQMatrix3x4cRotateAxisUnit(const HQVector4& axis,hq_float32 angle,HQMatrix3x4* out);//tay trái - chiều kim đồng hồ ,tay phải -ngược chiều kim đồng hồ,Vector chỉ phương của trục phải đã chuẩn hóa

//=======================================================
//HQRay3D - tia
//=======================================================

class HQ_UTIL_MATH_API HQRay3D: public HQBaseMathClass {
public:
	HQVector4 O;//origin
	HQVector4 D;//direction
	HQ_FORCE_INLINE HQRay3D(){};
	HQ_FORCE_INLINE HQRay3D(hq_float32 oX,hq_float32 oY,hq_float32 oZ,hq_float32 dX,hq_float32 dY,hq_float32 dZ)
		: O (oX , oY , oZ , 1.0f) ,
		  D (dX , dY , dZ , 0.0f)
	{
	};
	HQ_FORCE_INLINE HQRay3D(const HQVector4& _ori,const HQVector4& _dir)
		: O (_ori.x,_ori.y,_ori.z,1.0f) ,
		  D (_dir.x,_dir.y,_dir.z,0.0f)
	{
	};

	HQ_FORCE_INLINE void Set(const HQVector4& _ori,const HQVector4& _dir){
		O.Set(_ori.x,_ori.y,_ori.z,1.0f);
		D.Set(_dir.x,_dir.y,_dir.z,0.0f);
	};

	void Transform(const HQMatrix4& mat);
	void Transform(const HQMatrix3x4& mat);
	//tọa độ điểm cắt : V0 + (*pU) * (V1 - V0) + (*pV) * (V2 - V0) = O * (*pT) * D.Nếu <Cull> = true , không tính tam giác có góc giữa pháp vector & hướng tia < 90 độ.Với pháp vector tam giác = (V1 - V0) x (V2 - V0)
	bool Intersect(const HQVector4& V0,const HQVector4& V1,const HQVector4& V2,hq_float32* pU,hq_float32* pV,hq_float32 *pT,bool Cull=true)const;
	//tọa độ điểm cắt : V0 + (*pU) * (V1 - V0) + (*pV) * (V2 - V0)  = O * (*pT) * D.Nếu <Cull> = true , không tính tam giác có góc giữa pháp vector & hướng tia < 90 độ.Với pháp vector tam giác = (V1 - V0) x (V2 - V0)
	bool Intersect(const HQVector4& V0,const HQVector4& V1,const HQVector4& V2,hq_float32* pU,hq_float32* pV,hq_float32 *pT,hq_float32 maxT,bool Cull=true)const;
	bool Intersect(const HQPlane& plane,hq_float32 *pT,bool Cull=true) const;//<cull> = true - không tính plane mà góc giữa pháp vector & hướng tia < 90 độ
	bool Intersect(const HQPlane& plane,hq_float32 *pT,hq_float32 maxT,bool Cull=true) const;//<cull> = true - không tính plane mà góc giữa pháp vector & hướng tia < 90 độ
	bool Intersect(const HQAABB& aabb,hq_float32 *pT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQAABB& aabb,hq_float32 *pT,hq_float32 maxT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQOBB& obb,hq_float32 *pT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQOBB& obb,hq_float32 *pT,hq_float32 maxT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQSphere& sphere,hq_float32 *pT1,hq_float32 *pT2) const;//<pT1> - lưu thời điểm cắt gần nhất , <pT2> - lưu thời điểm cắt xa nhất
	bool Intersect(const HQSphere& sphere,hq_float32 *pT1,hq_float32 *pT2,hq_float32 maxT) const;//<pT1> - lưu thời điểm cắt gần nhất , <pT2> - lưu thời điểm cắt xa nhất
#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif

void HQRayTransform(const HQRay3D* pRIn,const HQMatrix4 *pMat,HQRay3D* pROut);
void HQRayTransform(const HQRay3D* pRIn,const HQMatrix3x4 *pMat,HQRay3D* pROut);
//=======================================================
//HQPlane - mặt phẳng
//=======================================================

class HQ_UTIL_MATH_API HQPlane: public HQBaseMathClass{
public:
	enum Side
	{
		IN_PLANE = 0,
		FRONT_PLANE = 1,
		BACK_PLANE = 2,
		BOTH_SIDE = 3
	};

	//phương trình mặt phẳng N*P + D =0  (P là tọa độ điểm bất kỳ nằm trên mặt phẳng)
	HQVector4 N;//vector pháp tuyến
	hq_float32 D;//khoảng cách (có dấu) từ mặt phẳng đến gốc tọa độ

	HQ_FORCE_INLINE HQPlane(){};
	HQPlane(const HQVector4& p0,const HQVector4& p1,const HQVector4& p2 , bool normalize = false);
	HQPlane(const HQVector4& Normal,const HQVector4& Point);
	HQPlane(const HQVector4& Normal,const hq_float32 Distance);

	void Set(const HQVector4& p0,const HQVector4& p1,const HQVector4& p2 , bool normalize = false);
	void Set(const HQVector4& Normal,const HQVector4& Point);
	void Set(const HQVector4& Normal,const hq_float32 Distance);

	HQPlane& Normalize();

	hq_float32 Distance(const HQVector4& p)const;//tìm khoảng cách từ điểm p đến mặt phẳng,vector pháp tuyến phải đã được chuẩn hóa

	Side CheckSide(const HQVector4& p)const;//kiểm tra điểm ở mặt trước (phía pháp vector hướng đến) hay mặt sau,trả về FRONT_PLANE nếu mặt trước,BACK_PLANE nếu mặt sau và IN_PLANE nếu nằm trên

	bool Intersect(const HQRay3D& ray,hq_float32 *pT,bool Cull=true) const;//<cull> = true - không tính plane mà góc giữa pháp vector & hướng tia < 90 độ
	bool Intersect(const HQRay3D& ray,hq_float32 *pT,hq_float32 maxT,bool Cull=true) const;//<cull> = true - không tính plane mà góc giữa pháp vector & hướng tia < 90 độ
	bool Intersect(const HQVector4 &p0,const HQVector4& p1,const HQVector4& p2)const;
	bool Intersect(const HQPlane& plane,HQRay3D *rIntersection)const;
	bool Intersect(const HQAABB& aabb)const;
	bool Intersect(const HQOBB& obb)const;//pháp vector của mặt phẳng phải đã chuẩn hóa
	bool Intersect(const HQSphere& sphere)const;//vector pháp tuyến phải đã được chuẩn hóa

#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif

/*--------------------------*/
enum HQVisibility
{
	HQ_CLIPPED = 3,
	HQ_CULLED = 4,
	HQ_VISIBLE = 5
};
//=============================================================================
//HQAABB - axis aligned bounding box - hình hộp có các cạnh song song trục tọa độ
//=============================================================================


class HQ_UTIL_MATH_API HQAABB: public HQBaseMathClass{
public:
	HQVector4 vMax,vMin;

	HQ_FORCE_INLINE HQAABB(){};

	void Construct(const HQOBB &obb);//tạo hình hộp aabb bao ngoài obb
	void Construct(const HQSphere &sphere);//tạo hình hộp aabb bao ngoài hình cầu
	//kiểm tra hình hộp bị cắt hay nằm ngoài thể tính nhìn (hợp bởi các mặt phẳng tham số).Trả về HQ_CULLED nếu nằm ngoài,HQ_CLIPPED nếu 1 phần nằm trong , và HQ_VISIBLE nếu hoàn toàn nằm trong.
	//pháp vector các mặt phẳng hướng ra ngoài thể tích nhìn
	HQVisibility Cull(const HQPlane* planes,const hq_int32 numPlanes);
	void GetPlanes(HQPlane * planesOut)const;//truy vấn các mặt phẳng của hình hộp theo thứ tự Ox+-,Oy+-,Oz+-
	bool ContainsPoint(const HQVector4& p)const;//kiểm tra điểm có nằm trong hình hộp không
	bool ContainsPoint(const HQFloat3& p)const;//kiểm tra điểm có nằm trong hình hộp không
	bool ContainsSegment(const HQVector4& p0,const HQVector4& p1)const;//kiểm tra đoạn thẳng có nằm trong hình hộp không
	bool ContainsSegment(const HQRay3D& ray,const hq_float32 t)const;//kiểm tra đoạn thẳng có nằm trong hình hộp không
	bool ContainsAABB(const HQAABB &aabb) const;
	bool ContainsOBB(const HQOBB &obb) const;//kiểm tra hình hộp obb có nằm trong hình hộp aabb hay không
	bool ContainsSphere(const HQSphere &sphere) const;//kiểm tra hình cầu có nằm trong hình hộp aabb hay không

	bool Intersect(const HQRay3D& ray,hq_float32 *pT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQRay3D& ray,hq_float32 *pT,hq_float32 maxT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQPlane& plane)const;
	bool Intersect(const HQAABB& aabb)const;
	bool Intersect(const HQOBB& obb) const;
	bool Intersect(const HQSphere &sphere) const;
#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif

//==============================================================================
//HQOBB - oriented bounding box - hình hộp không có các cạnh song song trục tọa độ
//==============================================================================
class HQ_UTIL_MATH_API HQOBB: public HQBaseMathClass{
public:
	HQVector4 vA[3]; //3 trục cơ sở của hình hộp
	HQVector4 vCenter;//điểm trung tâm
	hq_float32 fA[3];//nửa độ dài các cạnh theo 3 trục cơ sở

	void Transform(const HQOBB& source,const HQMatrix4& mat);//biến đổi
	void Transform(const HQOBB& source,const HQMatrix3x4& mat);//biến đổi

	//kiểm tra hình hộp nằm ngoài hay cắt 1 phần thể tính nhìn (tạo bởi các mặt phẳng)
	//pháp vector các mặt phẳng hướng ra ngoài thể tích nhìn và phải đã chuẩn hóa
	HQVisibility Cull(const HQPlane* planes,const hq_int32 numPlanes) const;

	bool Intersect(const HQRay3D& ray,hq_float32 *pT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQRay3D& ray,hq_float32 *pT,hq_float32 maxT) const;//<pT> - lưu thời điểm cắt
	bool Intersect(const HQPlane& plane)const;
	bool Intersect(const HQVector4 & p0,const HQVector4 & p1,const HQVector4 & p2) const;
	bool Intersect(const HQAABB& aabb) const;
	bool Intersect(const HQOBB& obb) const;
	bool Intersect(const HQSphere &sphere) const;
#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif

//==========================================================================
//HQSphere - hình cầu
//==========================================================================
class HQ_UTIL_MATH_API HQSphere: public HQBaseMathClass{
public:
	HQVector4 center;
	hq_float32 radius;
	//kiểm tra hình cấu bị cắt hay nằm ngoài thể tính nhìn (hợp bởi các mặt phẳng tham số).Trả về HQ_CULLED nếu nằm ngoài,HQ_CLIPPED nếu 1 phần nằm trong , và HQ_VISIBLE nếu hoàn toàn nằm trong.
	//pháp vector các mặt phẳng hướng ra ngoài thể tích nhìn và phải đã chuẩn hóa
	HQVisibility Cull(const HQPlane* planes,const hq_int32 numPlanes) const;

	bool Intersect(const HQRay3D& ray,hq_float32 *pT1,hq_float32 *pT2) const;//<pT1> - lưu thời điểm cắt gần nhất , <pT2> - lưu thời điểm cắt xa nhất
	bool Intersect(const HQRay3D& ray,hq_float32 *pT1,hq_float32 *pT2,hq_float32 maxT) const;//<pT1> - lưu thời điểm cắt gần nhất , <pT2> - lưu thời điểm cắt xa nhất
	bool Intersect(const HQPlane& plane)const;
	bool Intersect(const HQVector4 & p0,const HQVector4 & p1,const HQVector4 & p2) const;
	bool Intersect(const HQAABB &box) const;
	bool Intersect(const HQOBB &box) const;
	bool Intersect(const HQSphere & sphere) const;

#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif


//=====================================================
class HQ_UTIL_MATH_API HQQuaternion: public HQBaseMathClass{
public:
	union{
		struct{hq_float32 x,y,z,w;};
		hq_float32 q[4];
	};
	HQ_FORCE_INLINE HQQuaternion() : x(0.0f ) , y(0.0f ) , z(0.0f ) , w(1.0f ){};//create unit quaternion (0,0,0,1)
	HQ_FORCE_INLINE HQQuaternion(const void* null){}//do nothing
	HQ_FORCE_INLINE HQQuaternion(hq_float32 _x,hq_float32 _y,hq_float32 _z,hq_float32 _w)
		: x( _x ) , y( _y ) , z( _z ) , w( _w ){};
	HQ_FORCE_INLINE void Set(hq_float32 _x,hq_float32 _y,hq_float32 _z,hq_float32 _w){
		x=_x;y=_y;z=_z;w=_w;
	};
	void QuatToMatrix4r(HQMatrix4* matOut);//tạo ma trận quay row major từ quaternion ,quaternion sẽ dc chuẩn hóa
	void QuatUnitToMatrix4r(HQMatrix4* matOut)const;//tạo ma trận quay row major từ quaternion ,quaternion phải đã chuẩn hóa
	void QuatFromMatrix4r(const HQMatrix4 &matIn);//tạo quaternion từ matrix row major
	void QuatToMatrix3x4c(HQMatrix3x4* matOut);//tạo ma trận quay 3x4 column major từ quaternion ,quaternion sẽ dc chuẩn hóa
	void QuatUnitToMatrix3x4c(HQMatrix3x4* matOut)const;//tạo ma trận quay 3x4 column major từ quaternion ,quaternion phải đã chuẩn hóa
	void QuatFromMatrix3x4c(const HQMatrix3x4 &matIn);//tạo quaternion từ matrix 3x4 column major
	void QuatFromRollPitchYaw(hq_float32 roll,hq_float32 pitch,hq_float32 yaw);
	void QuatFromRotAxis(hq_float32 angle,HQVector4& axis);//tạo quaternion phép biến đổi quay quanh trục axis góc angle,vector trục sẽ dc chuẩn hóa
	void QuatFromRotAxisUnit(hq_float32 angle,const HQVector4& axis);//tạo quaternion phép biến đổi quay quanh trục axis góc angle,vector chỉ phương của trục phải đã chuẩn hóa
	void QuatFromRotAxisOx(hq_float32 angle);
	void QuatFromRotAxisOy(hq_float32 angle);
	void QuatFromRotAxisOz(hq_float32 angle);
	void QuatToRotAxis(hq_float32* pAngle,HQVector4* pAxis);//truy vấn góc quay và trục quay từ quaternion,quaternion sẽ dc chuẩn hóa
	void QuatUnitToRotAxis(hq_float32* pAngle,HQVector4* pAxis)const;//truy vấn góc quay và trục quay từ quaternion ,quaternion phải đã chuẩn hóa
	hq_float32 GetMagnitude();//sqrt(x^2+y^2+z^2+w^2)
	hq_float32 GetSumSquares();//(x^2+y^2+z^2+w^2)
	hq_float32 Dot(const HQQuaternion& q2)const;
	HQQuaternion& Slerp(const HQQuaternion& quat1,const HQQuaternion& quat2,hq_float32 t);//nội suy giữa 2 quaternion đã chuẩn hóa,kết quả lưu trong đối tượng này
	HQQuaternion& Slerp(const HQQuaternion& quat2,hq_float32 t);//nội suy giữa quaternion này và quaternion <quat2> ,cả 2 quaternion phải đã chuẩn hóa
	HQQuaternion& Normalize();
	HQQuaternion& Inverse();

	HQ_FORCE_INLINE HQQuaternion operator-()const
        {return HQQuaternion(-x,-y,-z,w);};//-x -y -z w

	HQQuaternion& operator *=(const HQQuaternion& quat);
	HQQuaternion operator *(const HQQuaternion& quat)const;
	HQQuaternion& operator *=(const hq_float32 f);
	HQQuaternion operator *(const hq_float32 f)const;

	HQQuaternion& operator /=(const hq_float32 f);
	HQQuaternion operator /(const hq_float32 f)const;

	HQQuaternion& operator +=(const HQQuaternion& quat);
	HQQuaternion operator +(const HQQuaternion& quat)const;

	HQQuaternion& operator -=(const HQQuaternion& quat);
	HQQuaternion operator -(const HQQuaternion& quat)const;
#if !defined WIN32
}  HQ_ALIGN16 ;
#else
};
#endif
HQ_UTIL_MATH_API HQQuaternion operator *(const hq_float32 f,const HQQuaternion& quat);
HQ_UTIL_MATH_API HQQuaternion* HQQuatNormalize(const HQQuaternion* in,HQQuaternion* out);
HQ_UTIL_MATH_API HQQuaternion* HQQuatInverse(const HQQuaternion* in,HQQuaternion* out);
HQ_UTIL_MATH_API HQQuaternion* HQQuatMultiply(const HQQuaternion* q1,const HQQuaternion* q2 , HQQuaternion* out);



/*-----------------------------------*/
typedef HQVector4 *LPHQVector4;
typedef HQMatrix4 *LPHQMatrix4;
typedef HQMatrix3x4 *LPHQMatrix3x4;
typedef HQRay3D *LPHQRay3D;
typedef HQPlane  *LPHQPlane;
typedef HQAABB *LPHQAABB;
typedef HQOBB *LPHQOBB;
typedef HQQuaternion *LPHQQuaternion;

#ifdef WIN32
typedef HQ_UTIL_MATH_API HQA16ByteStoragePtr<HQVector4> HQBaseA16ByteVector4Ptr;
typedef HQ_UTIL_MATH_API HQA16ByteStoragePtr<HQMatrix4> HQBaseA16ByteMatrix4Ptr;
typedef HQ_UTIL_MATH_API HQA16ByteStoragePtr<HQMatrix3x4> HQBaseA16ByteMatrix3x4Ptr;
typedef HQ_UTIL_MATH_API HQA16ByteStoragePtr<HQQuaternion> HQBaseA16ByteQuaternionPtr;
#else
typedef HQA16ByteStoragePtr<HQVector4> HQBaseA16ByteVector4Ptr;
typedef HQA16ByteStoragePtr<HQMatrix4> HQBaseA16ByteMatrix4Ptr;
typedef HQA16ByteStoragePtr<HQMatrix3x4> HQBaseA16ByteMatrix3x4Ptr;
typedef HQA16ByteStoragePtr<HQQuaternion> HQBaseA16ByteQuaternionPtr;
#endif

/*--------------------------------------------------------*/
class HQ_UTIL_MATH_API HQA16ByteVector4Ptr : public HQBaseA16ByteVector4Ptr
{
public:
	HQ_FORCE_INLINE HQA16ByteVector4Ptr() : HQBaseA16ByteVector4Ptr(){};
	HQ_FORCE_INLINE HQA16ByteVector4Ptr(const HQVector4 &src) : HQBaseA16ByteVector4Ptr(src){};
	HQ_FORCE_INLINE HQA16ByteVector4Ptr(bool isPoint) : HQBaseA16ByteVector4Ptr(NULL)
	{
		this->ptr = new (this->ptr) HQVector4(isPoint);
	}
	HQ_FORCE_INLINE HQA16ByteVector4Ptr(hq_float32 _x,hq_float32 _y,hq_float32 _z): HQBaseA16ByteVector4Ptr(NULL)
	{
		this->ptr = new (this->ptr) HQVector4(_x , _y , _z);
	};
	HQ_FORCE_INLINE HQA16ByteVector4Ptr(hq_float32 _x,hq_float32 _y,hq_float32 _z , hq_float32 _w): HQBaseA16ByteVector4Ptr(NULL)
	{
		this->ptr = new (this->ptr) HQVector4(_x , _y , _z , _w);
	};
};

class HQ_UTIL_MATH_API HQA16ByteMatrix4Ptr : public HQBaseA16ByteMatrix4Ptr
{
public:
	HQA16ByteMatrix4Ptr() : HQBaseA16ByteMatrix4Ptr() {};//create identity matrix
	HQA16ByteMatrix4Ptr(const HQMatrix4 &src) : HQBaseA16ByteMatrix4Ptr(src){};
	HQA16ByteMatrix4Ptr(const void * null) : HQBaseA16ByteMatrix4Ptr(NULL){};//create uninitialized matrix
	HQA16ByteMatrix4Ptr(hq_float32 _11, hq_float32 _12, hq_float32 _13, hq_float32 _14,
			hq_float32 _21, hq_float32 _22, hq_float32 _23, hq_float32 _24,
			hq_float32 _31, hq_float32 _32, hq_float32 _33, hq_float32 _34,
			hq_float32 _41, hq_float32 _42, hq_float32 _43, hq_float32 _44): HQBaseA16ByteMatrix4Ptr(NULL)
	{
		this->ptr = new (this->ptr) HQMatrix4(_11 , _12 , _13 , _14 ,
											_21 , _22 , _23 , _24 ,
											_31 , _32 , _33 , _34 ,
											_41 , _42 , _43 , _44 );
	}

	HQ_FORCE_INLINE hq_float32& operator[] (hq_uint32 index)
	{
		return this->ptr->m[index];
	};

	HQ_FORCE_INLINE hq_float32& operator() (hq_uint32 row,hq_uint32 col)
	{
		return this->ptr->mt[row][col];
	};

	HQ_FORCE_INLINE const hq_float32& operator[] (hq_uint32 index)const
	{
		return this->ptr->m[index];
	};

	HQ_FORCE_INLINE const hq_float32& operator() (hq_uint32 row,hq_uint32 col)const
	{
		return this->ptr->mt[row][col];
	};

};

class HQ_UTIL_MATH_API HQA16ByteMatrix3x4Ptr : public HQBaseA16ByteMatrix3x4Ptr
{
public:
	HQA16ByteMatrix3x4Ptr() : HQBaseA16ByteMatrix3x4Ptr() {};
	HQA16ByteMatrix3x4Ptr(const HQMatrix3x4 &src) : HQBaseA16ByteMatrix3x4Ptr(src){};
	HQA16ByteMatrix3x4Ptr(const void * null) : HQBaseA16ByteMatrix3x4Ptr(NULL){};//create uninitialized matrix
	HQA16ByteMatrix3x4Ptr(hq_float32 _11, hq_float32 _12, hq_float32 _13, hq_float32 _14,
			hq_float32 _21, hq_float32 _22, hq_float32 _23, hq_float32 _24,
			hq_float32 _31, hq_float32 _32, hq_float32 _33, hq_float32 _34): HQBaseA16ByteMatrix3x4Ptr(NULL)
	{
		this->ptr = new (this->ptr) HQMatrix3x4(_11 , _12 , _13 , _14 ,
											_21 , _22 , _23 , _24 ,
											_31 , _32 , _33 , _34 );
	}

	HQ_FORCE_INLINE hq_float32& operator[] (hq_uint32 index)
	{
		return this->ptr->m[index];
	};

	HQ_FORCE_INLINE hq_float32& operator() (hq_uint32 row,hq_uint32 col)
	{
		return this->ptr->mt[row][col];
	};

	HQ_FORCE_INLINE const hq_float32& operator[] (hq_uint32 index)const
	{
		return this->ptr->m[index];
	};

	HQ_FORCE_INLINE const hq_float32& operator() (hq_uint32 row,hq_uint32 col)const
	{
		return this->ptr->mt[row][col];
	};
};

class HQ_UTIL_MATH_API HQA16ByteQuaternionPtr : public HQBaseA16ByteQuaternionPtr
{
public:
	HQ_FORCE_INLINE HQA16ByteQuaternionPtr() : HQBaseA16ByteQuaternionPtr(){};//create unit quaternion (0,0,0,1)
	HQ_FORCE_INLINE HQA16ByteQuaternionPtr(const HQQuaternion &src) : HQBaseA16ByteQuaternionPtr(src){};
	HQ_FORCE_INLINE HQA16ByteQuaternionPtr(const void *null) :  HQBaseA16ByteQuaternionPtr(NULL) {}//create uninitialized quaternion
	HQ_FORCE_INLINE HQA16ByteQuaternionPtr(hq_float32 _x,hq_float32 _y,hq_float32 _z , hq_float32 _w): HQBaseA16ByteQuaternionPtr(NULL)
	{
		this->ptr = new (this->ptr) HQQuaternion(_x , _y , _z , _w);
	};
};



//========================================================
//constants
#ifdef SSE_MATH
extern const float4 _3Halves_1Zero;//{0.5 , 0.5 ,0.5 ,0}
extern const float4 _4Threes;// {3.0f , 3.0f ,3.0f ,3.0f};
extern const float4 _4Zeros;// { 0 , 0 , 0 , 0}
extern const float4 _4Ones;// { 1 , 1 , 1 , 1}
#endif
//========================================================


#ifdef HQ_EXPLICIT_ALIGN
#	define HQ_DECL_STACK_VECTOR4_ARRAY(var , size)  \
		HQA16ByteStorageArrayPtr<HQVector4 , size> var

#	define HQ_DECL_STACK_VECTOR4(var)  \
		HQA16ByteVector4Ptr p##var;\
		HQVector4 &var = *p##var

#	define HQ_DECL_STACK_VECTOR4_CTOR_PARAMS(var , params)  \
		HQA16ByteVector4Ptr p##var params;\
		HQVector4 &var = *p##var;

#	define HQ_DECL_STACK_QUATERNION(var)  \
		HQA16ByteQuaternionPtr p##var;\
		HQQuaternion &var = *p##var

#	define HQ_DECL_STACK_QUATERNION_CTOR_PARAMS(var , params)  \
		HQA16ByteQuaternionPtr p##var params;\
		HQQuaternion &var = *p##var;


#	define HQ_DECL_STACK_2VECTOR4(var1 , var2)  \
		HQA16ByteStorageArrayPtr<HQVector4 , 2> p2Vec##var1;\
		HQVector4 &var1 = p2Vec##var1[0] ;\
		HQVector4 &var2 = p2Vec##var1[1]

#	define HQ_DECL_STACK_3VECTOR4(var1 , var2 , var3)  \
		HQA16ByteStorageArrayPtr<HQVector4 , 3> p3Vec##var1;\
		HQVector4 &var1 = p3Vec##var1[0] ;\
		HQVector4 &var2 = p3Vec##var1[1] ;\
		HQVector4 &var3 = p3Vec##var1[2]

#	define HQ_DECL_STACK_4VECTOR4(var1 , var2 , var3 , var4)  \
		HQA16ByteStorageArrayPtr<HQVector4 , 4> p4Vec##var1;\
		HQVector4 &var1 = p4Vec##var1[0] ;\
		HQVector4 &var2 = p4Vec##var1[1] ;\
		HQVector4 &var3 = p4Vec##var1[2] ;\
		HQVector4 &var4 = p4Vec##var1[3]

#	define HQ_DECL_STACK_5VECTOR4(var1 , var2 , var3 , var4 , var5)  \
		HQA16ByteStorageArrayPtr<HQVector4 , 5> p5Vec##var1;\
		HQVector4 &var1 = p5Vec##var1[0] ;\
		HQVector4 &var2 = p5Vec##var1[1] ;\
		HQVector4 &var3 = p5Vec##var1[2] ;\
		HQVector4 &var4 = p5Vec##var1[3] ;\
		HQVector4 &var5 = p5Vec##var1[4]

#	define HQ_DECL_STACK_MATRIX4(var)  \
		HQA16ByteMatrix4Ptr pMat##var;\
		HQMatrix4 & var = *pMat##var;
#	define HQ_DECL_STACK_MATRIX4_CTOR_PARAMS(var , params)  \
		HQA16ByteMatrix4Ptr pMat##var params;\
		HQMatrix4 & var = *pMat##var;

#	define HQ_DECL_STACK_MATRIX3X4(var)  \
		HQA16ByteMatrix3x4Ptr pMat3x4##var;\
		HQMatrix3x4 & var = *pMat3x4##var;

#	define HQ_DECL_STACK_MATRIX3X4_CTOR_PARAMS(var , params)  \
		HQA16ByteMatrix3x4Ptr pMat3x4##var params;\
		HQMatrix3x4 & var = *pMat3x4##var;

#	define HQ_DECL_STACK_VAR_ARRAY(type , var , size) \
		HQA16ByteStorageArrayPtr< type , size> var

#	define HQ_DECL_STACK_VAR(type , var) \
		HQA16ByteStoragePtr<type> pVar##var;\
		type var = *pVar##var

#	define HQ_DECL_STACK_VAR_CTOR_PARAMS(type , var , params) \
		HQA16ByteStoragePtr<type> pVar##var (NULL);\
		new (pVar##var) type params;\
		type var = *pVar##var

#else
#	define HQ_DECL_STACK_VECTOR4_ARRAY(var , size)  \
		HQVector4 var[size]

#	define HQ_DECL_STACK_VECTOR4(var)  \
		HQVector4 var

#	define HQ_DECL_STACK_VECTOR4_CTOR_PARAMS(var , params)  \
		HQVector4 var params

#	define HQ_DECL_STACK_QUATERNION(var)  \
		HQQuaternion var

#	define HQ_DECL_STACK_QUATERNION_CTOR_PARAMS(var , params)  \
		HQQuaternion var params

#	define HQ_DECL_STACK_2VECTOR4(var1 , var2)  \
		HQVector4 var1 ;\
		HQVector4 var2

#	define HQ_DECL_STACK_3VECTOR4(var1 , var2 , var3)  \
		HQVector4 var1  ;\
		HQVector4 var2 ;\
		HQVector4 var3

#	define HQ_DECL_STACK_4VECTOR4(var1 , var2 , var3 , var4)  \
		HQVector4 var1  ;\
		HQVector4 var2 ;\
		HQVector4 var3 ;\
		HQVector4 var4

#	define HQ_DECL_STACK_5VECTOR4(var1 , var2 , var3 , var4 , var5)  \
		HQVector4 var1  ;\
		HQVector4 var2 ;\
		HQVector4 var3 ;\
		HQVector4 var4 ;\
		HQVector4 var5

#	define HQ_DECL_STACK_MATRIX4(var)  \
		HQMatrix4 var

#	define HQ_DECL_STACK_MATRIX4_CTOR_PARAMS(var , params)  \
		HQMatrix4 var params

#	define HQ_DECL_STACK_MATRIX3X4(var)  \
		HQMatrix3x4 var

#	define HQ_DECL_STACK_MATRIX3X4_CTOR_PARAMS(var , params)  \
		HQMatrix3x4 var params

#	define HQ_DECL_STACK_VAR_ARRAY(type , var , size) \
		type var [size]

#	define HQ_DECL_STACK_VAR(type , var) \
		type var

#	define HQ_DECL_STACK_VAR_CTOR_PARAMS(type , var , params) \
		type var params

#endif


#include "HQ3DMathInline.h"



#endif

