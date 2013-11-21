/*********************************************************************
*Copyright 2010 Le Hoang Quyen. All rights reserved.
*********************************************************************/
// Defines all inline functions
//
#ifndef _3DMATH_INLINE_
#define _3DMATH_INLINE_
#include <iostream>
#include "HQ3DMath.h"
#include <string.h>//for memset
#ifdef LINUX
#include <malloc.h>//for memalign
#endif

#ifdef NEON_MATH
#	define HQ_RETURN_VECTOR4 return *HQA16ByteVector4Ptr
#else
#	define HQ_RETURN_VECTOR4 return HQVector4
#endif

//inline functions
HQ_FORCE_INLINE void HQSincosf( hq_float32 angle, hq_float32* sinOut, hq_float32* cosOut )
{
#ifdef WIN32
   _asm
   {
	   fld [angle]		// push dữ liệu góc angle vào đỉnh FPU register stack
	   fsincos		//tính sin và cos của giá trị lưu ở đỉnh FPU stack,đỉnh stack lúc này sẽ lưu giá trị cos tính được,sin kế tiếp
		  //sau đó giá trị cos tính được được push vào đỉnh stack
       mov eax , cosOut      //load giá trị con trỏ cosOut vào eax
       fstp [eax]		//pop giá trị đỉnh FPU stack và đưa vào giá trị con trỏ cosOut trỏ đến
       mov eax , sinOut		//load giá trị con trỏ sinOut vào eax
       fstp [eax]		//pop giá trị đỉnh FPU stack và đưa vào giá trị con trỏ sinOut trỏ đến
   }
#elif defined LINUX || defined APPLE
	//inline assembly gcc/g++ version 
	asm(
		"fsincos\n\t"
        : "=t"(*cosOut) ,"=u"(*sinOut)
        :"0" (angle)
        );
#else
	*sinOut = sinf(angle);
	*cosOut = cosf(angle);
#endif
}

//Vector

//**********************************************
//tích vector với số thực
//**********************************************
HQ_FORCE_INLINE HQVector4 operator*(const hq_float32 f,const HQVector4& v){
	HQ_RETURN_VECTOR4(f*v.x,f*v.y,f*v.z,v.w);
}
//**********************************************
//vector âm
//**********************************************
HQ_FORCE_INLINE HQVector4 HQVector4::operator -() const{
	HQ_RETURN_VECTOR4(-x,-y,-z,w);
}

//**********************************************
//tích vector  với số thực
//**********************************************
HQ_FORCE_INLINE HQVector4& HQVector4::operator *=(const hq_float32 f){
	x*=f;y*=f;z*=f;
	return *this;
}

HQ_FORCE_INLINE HQVector4 HQVector4::operator *(const hq_float32 f)const{
	HQ_RETURN_VECTOR4(x*f,y*f,z*f,w);
}

HQ_FORCE_INLINE HQVector4* HQVector4Mul(const HQVector4* pV1,hq_float32 f,HQVector4* out)
{
	*out = (*pV1) * f;
	return out;
}
//**********************************************
//thương vector  với số thực
//**********************************************
HQ_FORCE_INLINE HQVector4& HQVector4::operator /=(const hq_float32 f){
	float _1overf = 1.0f / f;
	x *= _1overf ;y *= _1overf ;z *= _1overf ;
	return *this;
}
HQ_FORCE_INLINE HQVector4 HQVector4::operator /(const hq_float32 f)const{
	float _1overf = 1.0f / f;
	HQ_RETURN_VECTOR4(x * _1overf,y * _1overf,z * _1overf,w);
}

HQ_FORCE_INLINE HQVector4* HQVector4Div(const HQVector4* pV1,hq_float32 f,HQVector4* out)
{
	*out = (*pV1) / f;
	return out;
}
//**********************************************
//tổng 2 vector
//**********************************************
HQ_FORCE_INLINE HQVector4& HQVector4::operator +=(const HQVector4 &v2){
	x+=v2.x;y+=v2.y;z+=v2.z;
	return *this;
}
HQ_FORCE_INLINE HQVector4 HQVector4::operator +(const HQVector4& v2)const{
	HQ_RETURN_VECTOR4(x+v2.x,y+v2.y,z+v2.z,w);
}
HQ_FORCE_INLINE HQVector4* HQVector4Add(const HQVector4* pV1,const HQVector4 *pV2,HQVector4* out)
{
	*out = (*pV1) + (*pV2);
	return out;
}
//**********************************************
//hiệu 2 vector
//**********************************************
HQ_FORCE_INLINE HQVector4& HQVector4::operator -=(const HQVector4 &v2){
	x-=v2.x;y-=v2.y;z-=v2.z;
	return *this;
}
HQ_FORCE_INLINE HQVector4 HQVector4::operator -(const HQVector4& v2)const{
	HQ_RETURN_VECTOR4(x-v2.x,y-v2.y,z-v2.z,w);
}
HQ_FORCE_INLINE HQVector4* HQVector4Sub(const HQVector4* pV1,const HQVector4 *pV2,HQVector4* out)
{
	*out = (*pV1) - (*pV2);
	return out;
}
//**********************************************
//tích vô hướng
//**********************************************
HQ_FORCE_INLINE hq_float32 HQVector4::operator *(const HQVector4 &v2) const{
#if	defined CMATH || defined NEON_MATH
	return x*v2.x+y*v2.y+z*v2.z;
	
#elif defined SSE4_MATH
	hq_float32 f;//kết quả cần trả về
	float4 m0 , m1;
	m0 = _mm_load_ps(this->v);//copy vector data to 128 bit xmm register
	m1 = _mm_load_ps(v2.v);//copy vector 2 data to 128 bit xmm register

	m0 = _mm_dp_ps(m0 , m1 , SSE4_DP_MASK(0,1,1,1,0,0,0,1)) ;
	
	_mm_store_ss(&f,m0);
	return f;//trả về f
#else/*SSE*/
	hq_float32 f;//kết quả cần trả về
	float4 m0,m1,m2;
	m0=_mm_load_ps(this->v);//copy vector data to 128 bit xmm register
	m1=_mm_load_ps(v2.v);//copy vector2 data to 128 bit xmm register
	m0=_mm_mul_ps(m0,m1);//x1*x2 y1*y2 z1*z2 w1*w2
	m1 = hq_mm_copy_ps(m0,SSE_SHUFFLEL(1,0,0,3));//	y1*y2 x1*x2 x1*x2 w1*w2
	m2 = hq_mm_copy_ps(m0,SSE_SHUFFLEL(2,2,1,3));//	z1*z2 z1*z2 y1*y2 w1*w2 

	m0=_mm_add_ss(m0,m1);//x1*x2 + y1*y2		_		_		_
	m0=_mm_add_ss(m0,m2);//x1*x2 + y1*y2 + z1*z2		_		_		_
	
	_mm_store_ss(&f,m0);
	return f;//trả về f
#endif
}
//**********************************************
//bình phương độ dài vector
//**********************************************
HQ_FORCE_INLINE hq_float32 HQVector4::LengthSqr()const{
#if	defined CMATH || defined NEON_MATH
	return x*x+y*y+z*z;
#elif defined SSE4_MATH
	hq_float32 f;//kết quả cần trả về
	float4 m0;
	m0 = _mm_load_ps(this->v);//copy vector data to 128 bit xmm register
	m0 = _mm_dp_ps(m0 , m0 , SSE4_DP_MASK(0,1,1,1,0,0,0,1)) ;//binary format 01110001
	
	_mm_store_ss(&f,m0);
	return f;//trả về f
#else/*SSE*/
	hq_float32 f;//kết quả cần trả về
	float4 m0,m1,m2;
	m0=_mm_load_ps(this->v);//copy vector data to 128 bit xmm register
	m0=_mm_mul_ps(m0,m0);//nhân vector với chính nó x^2 y^2 z^2 w^2
	m1 = hq_mm_copy_ps(m0,SSE_SHUFFLEL(1,0,0,3));//	y^2 x^2 x^2 w^2
	m2 = hq_mm_copy_ps(m0,SSE_SHUFFLEL(2,2,1,3));//	z^2 z^2 y^2 w^2 

	m0=_mm_add_ss(m0,m1);//x^2+y^2		_		_		_
	m0=_mm_add_ss(m0,m2);//x^2+y^2+z^2		_		_		_
	
	_mm_store_ss(&f,m0);
	return f;//trả về f
#endif
}

//**********************************************
//độ dài vector
//**********************************************
HQ_FORCE_INLINE hq_float32 HQVector4::Length(){
#if	defined CMATH || defined NEON_MATH
	return sqrtf(x * x + y * y + z * z);
#elif defined SSE4_MATH
	hq_float32 f;//kết quả cần trả về
	float4 m0;
	m0 = _mm_load_ps(this->v);//copy vector data to 128 bit xmm register
	m0 = _mm_dp_ps(m0 , m0 , SSE4_DP_MASK(0,1,1,1,0,0,0,1)) ;//binary format 01110001
	
	m0=_mm_sqrt_ss(m0);//căn phần tử đầu

	_mm_store_ss(&f,m0);
	return f;//trả về f
#else/*SSE*/
	hq_float32 f;//kết quả cần trả về
	float4 m0,m1,m2;
	m0=_mm_load_ps(this->v);//copy vector data to 128 bit xmm register
	m0=_mm_mul_ps(m0,m0);//nhân vector với chính nó x^2 y^2 z^2 w^2
	m1 = hq_mm_copy_ps(m0,SSE_SHUFFLEL(1,0,0,3));//	y^2 x^2 x^2 w^2
	m2 = hq_mm_copy_ps(m0,SSE_SHUFFLEL(2,2,1,3));//	z^2 z^2 y^2 w^2 

	m0=_mm_add_ss(m0,m1);//x^2+y^2		_		_		_
	m0=_mm_add_ss(m0,m2);//x^2+y^2+z^2		_		_		_
	
	m0=_mm_sqrt_ss(m0);//căn phần tử đầu
	
	_mm_store_ss(&f,m0);
	return f;//trả về f
#endif
}


//--------------------------------------------------
//Matrix4
HQ_FORCE_INLINE HQMatrix4::HQMatrix4()
	: _11(1.0f) , _12(0.0f) , _13(0.0f) , _14(0.0f) ,
	  _21(0.0f) , _22(1.0f) , _23(0.0f) , _24(0.0f) , 
	  _31(0.0f) , _32(0.0f) , _33(1.0f) , _34(0.0f) ,
	  _41(0.0f) , _42(0.0f) , _43(0.0f) , _44(1.0f)
{
}
HQ_FORCE_INLINE HQMatrix4::HQMatrix4(hq_float32 m11, hq_float32 m12, hq_float32 m13, hq_float32 m14, 
				hq_float32 m21, hq_float32 m22, hq_float32 m23, hq_float32 m24, 
				hq_float32 m31, hq_float32 m32, hq_float32 m33, hq_float32 m34, 
				hq_float32 m41, hq_float32 m42, hq_float32 m43, hq_float32 m44)
	: _11(m11) , _12(m12) , _13(m13) , _14(m14) ,
	  _21(m21) , _22(m22) , _23(m23) , _24(m24) , 
	  _31(m31) , _32(m32) , _33(m33) , _34(m34) ,
	  _41(m41) , _42(m42) , _43(m43) , _44(m44)
{
}
//***********************************************
//ma trận đơn vị
//***********************************************
HQ_FORCE_INLINE HQMatrix4& HQMatrix4::Identity(){
	_12 = _13 = _14 = 
	_21 = _23 = _24 = 
	_31 = _32 = _34 =
	_41 = _42 = _43 = 0.0f;
	_11=_22=_33=_44=1.0f;
	return *this;
}
HQ_FORCE_INLINE HQMatrix4* HQMatrix4Identity(HQMatrix4* inout){
	inout->_12 = inout->_13 = inout->_14 = 
	inout->_21 = inout->_23 = inout->_24 = 
	inout->_31 = inout->_32 = inout->_34 =
	inout->_41 = inout->_42 = inout->_43 = 0.0f;
	inout->_11=inout->_22=inout->_33=inout->_44=1.0f;
	return inout;
}
//******************************************************
//ma trận chuyển vị
//******************************************************
HQ_FORCE_INLINE HQMatrix4& HQMatrix4::Transpose(){
#ifdef CMATH
	swapf(_12,_21);
	swapf(_13,_31);
	swapf(_14,_41);
	swapf(_23,_32);
	swapf(_24,_42);
	swapf(_34,_43);
#elif defined NEON_MATH
	HQNeonMatrix4Transpose(this->m , this->m);
#else/*SSE*/
	float4 m0 , m1 , m2 , m3 , m4 , m5 , m6 , m7;

	m0 = _mm_load_ps(&this->m[0]);//11 12 13 14
	m1 = _mm_load_ps(&this->m[4]);//21 22 23 24
	m2 = _mm_load_ps(&this->m[8]);//31 32 33 34
	m3 = _mm_load_ps(&this->m[12]);//41 42 43 44

	m5 = _mm_unpacklo_ps(m0 , m1);//11 21 12 22
	m6 = _mm_unpacklo_ps(m2 , m3);//31 41 32 42
	m4 = _mm_shuffle_ps(m5 , m6 , SSE_SHUFFLEL(0 , 1 , 0 , 1));//11 21 31 41
	m5 = _mm_shuffle_ps(m5 , m6 , SSE_SHUFFLEL(2 , 3 , 2 , 3));//12 22 32 42

	m7 = _mm_unpackhi_ps(m0 , m1);//13 23 14 24
	m0 = _mm_unpackhi_ps(m2 , m3);//33 43 34 44
	m6 = _mm_shuffle_ps(m7 , m0 , SSE_SHUFFLEL(0 , 1 , 0 , 1));//13 23 33 43
	m7 = _mm_shuffle_ps(m7 , m0 , SSE_SHUFFLEL(2 , 3 , 2 , 3));//14 24 34 44
	
	_mm_store_ps(this->m , m4);
	_mm_store_ps(this->m + 4, m5);
	_mm_store_ps(this->m + 8, m6);
	_mm_store_ps(this->m + 12, m7);
#endif
	return *this;
}
HQ_FORCE_INLINE HQMatrix4* HQMatrix4Transpose(const HQMatrix4 *in, HQMatrix4 *out){
#ifdef CMATH
	if(in==out)
		out->Transpose();
	else{
		out->_11=in->_11;
		out->_12=in->_21;
		out->_13=in->_31;
		out->_14=in->_41;

		out->_21=in->_12;
		out->_22=in->_22;
		out->_23=in->_32;
		out->_24=in->_42;

		out->_31=in->_13;
		out->_32=in->_23;
		out->_33=in->_33;
		out->_34=in->_43;

		out->_41=in->_14;
		out->_42=in->_24;
		out->_43=in->_34;
		out->_44=in->_44;
	}
#elif defined NEON_MATH
	
	HQNeonMatrix4Transpose(in->m , out->m);
	
#else/*SSE*/
	float4 m0 , m1 , m2 , m3 , m4 , m5 , m6 , m7;

	m0 = _mm_load_ps(&in->m[0]);//11 12 13 14
	m1 = _mm_load_ps(&in->m[4]);//21 22 23 24
	m2 = _mm_load_ps(&in->m[8]);//31 32 33 34
	m3 = _mm_load_ps(&in->m[12]);//41 42 43 44

	m5 = _mm_unpacklo_ps(m0 , m1);//11 21 12 22
	m6 = _mm_unpacklo_ps(m2 , m3);//31 41 32 42
	m4 = _mm_shuffle_ps(m5 , m6 , SSE_SHUFFLEL(0 , 1 , 0 , 1));//11 21 31 41
	m5 = _mm_shuffle_ps(m5 , m6 , SSE_SHUFFLEL(2 , 3 , 2 , 3));//12 22 32 42

	m7 = _mm_unpackhi_ps(m0 , m1);//13 23 14 24
	m0 = _mm_unpackhi_ps(m2 , m3);//33 43 34 44
	m6 = _mm_shuffle_ps(m7 , m0 , SSE_SHUFFLEL(0 , 1 , 0 , 1));//13 23 33 43
	m7 = _mm_shuffle_ps(m7 , m0 , SSE_SHUFFLEL(2 , 3 , 2 , 3));//14 24 34 44
	
	_mm_store_ps(out->m , m4);
	_mm_store_ps(out->m + 4, m5);
	_mm_store_ps(out->m + 8, m6);
	_mm_store_ps(out->m + 12, m7);
#endif
	return out;
}

//Matrix3
HQ_FORCE_INLINE HQMatrix3x4::HQMatrix3x4()
	: _11(1.0f) , _12(0.0f) , _13(0.0f), _14(0.0f),
	  _21(0.0f) , _22(1.0f) , _23(0.0f), _24(0.0f),
	  _31(0.0f) , _32(0.0f) , _33(1.0f) ,_34(0.0f) 
{
}
HQ_FORCE_INLINE HQMatrix3x4::HQMatrix3x4(hq_float32 m11, hq_float32 m12, hq_float32 m13, hq_float32 m14,
				hq_float32 m21, hq_float32 m22, hq_float32 m23, hq_float32 m24,
				hq_float32 m31, hq_float32 m32, hq_float32 m33, hq_float32 m34)
	: _11(m11) , _12(m12) , _13(m13), _14(m14),
	  _21(m21) , _22(m22) , _23(m23), _24(m24) ,
	  _31(m31) , _32(m32) , _33(m33), _34(m34)
{
}

//HQPlane

//*******************************************
//basic methods
//*******************************************
HQ_FORCE_INLINE HQPlane::HQPlane(const HQVector4 &Normal, const HQVector4& Point)
: N(Normal)  
{
	this->D=-(N * Point);
}
HQ_FORCE_INLINE HQPlane::HQPlane(const HQVector4 &Normal, const hq_float32 Distance)
: N(Normal) , D(Distance)
{
}
HQ_FORCE_INLINE HQPlane::HQPlane(const HQVector4 &p0, const HQVector4 &p1, const HQVector4 &p2 , bool normalize)
{
	HQ_DECL_STACK_2VECTOR4(e1 , e2);
	e1=p1-p0;
	e2=p2-p0;
	N.Cross(e1,e2);
	if (normalize)
		N.Normalize();
	D=-(N*p0);
}

HQ_FORCE_INLINE void HQPlane::Set(const HQVector4 &Normal, const HQVector4& Point){
	this->N=Normal;
	this->D=-(N * Point);
}
HQ_FORCE_INLINE void HQPlane::Set(const HQVector4 &Normal, const hq_float32 Distance){
	this->N=Normal;
	this->D=Distance;
}
HQ_FORCE_INLINE void HQPlane::Set(const HQVector4 &p0, const HQVector4 &p1, const HQVector4 &p2 , bool normalize){
	HQ_DECL_STACK_2VECTOR4(e1 , e2);
	e1=p1-p0;
	e2=p2-p0;
	N.Cross(e1,e2);
	if (normalize)
		N.Normalize();
	D=-(N*p0);
}
//***************************************************************************
//khoảng cách từ điểm p đến mặt phẳng ,vector pháp tuyến phải đã dc chuẩn hóa
//***************************************************************************
HQ_FORCE_INLINE hq_float32 HQPlane::Distance(const HQVector4 &p)const{
	return fabs((N*p)+D);
}
/*----------------------------------------------------------------------
kiểm tra mặt phẳng cắt ray
-----------------------------------------------------------------------*/
HQ_FORCE_INLINE bool HQPlane::Intersect(const HQRay3D& ray,hq_float32 *pT,bool Cull) const//<cull> = true - không tính plane mà góc giữa pháp vector & hướng tia < 90 độ
{
	return ray.Intersect(*this ,pT ,Cull);
}
HQ_FORCE_INLINE bool HQPlane::Intersect(const HQRay3D& ray,hq_float32 *pT,hq_float32 maxT,bool Cull) const//<cull> = true - không tính plane mà góc giữa pháp vector & hướng tia < 90 độ
{
	return ray.Intersect(*this ,pT , maxT , Cull);
}
/*----------------------------------------------------------------------
kiểm tra mặt phẳng cắt hình cầu
-----------------------------------------------------------------------*/
HQ_FORCE_INLINE bool HQPlane::Intersect(const HQSphere& sphere)const
{
	return this->Distance(sphere.center) <= sphere.radius;
}

//*********************************************************************
//kiểm tra điểm ở mặt trước (phía mà pháp vector hướng đến) hay mặt sau
//trả về 1 nếu mặt trước,2 nếu mặt sau và 0 nếu nằm trên
//*********************************************************************
HQ_FORCE_INLINE HQPlane::Side HQPlane::CheckSide(const HQVector4 &p)const{
	hq_float32 f=N*p+D;
	if(f>EPSILON) return FRONT_PLANE;
	else if (f<-EPSILON) return BACK_PLANE;
	return IN_PLANE;
}

//------------------HQAABB----------------------------------------------------

HQ_FORCE_INLINE void HQAABB::Construct(const HQSphere &sphere)
{
	vMin.x = sphere.center.x - sphere.radius;
	vMin.y = sphere.center.y - sphere.radius;
	vMin.z = sphere.center.z - sphere.radius;

	vMax.x = sphere.center.x + sphere.radius;
	vMax.y = sphere.center.y + sphere.radius;
	vMax.z = sphere.center.z + sphere.radius;
}


//******************************************
//kiểm tra nằm trong
//******************************************
HQ_FORCE_INLINE bool HQAABB::ContainsPoint(const HQVector4 &p)const{
	if(p.x>vMax.x) return false;
	if(p.x<vMin.x) return false;
	if(p.y>vMax.y) return false;
	if(p.y<vMin.y) return false;
	if(p.z>vMax.z) return false;
	if(p.z<vMin.z) return false;
	return true;
}
HQ_FORCE_INLINE bool HQAABB::ContainsPoint(const HQFloat3& p)const
{
	if(p.x>vMax.x) return false;
	if(p.x<vMin.x) return false;
	if(p.y>vMax.y) return false;
	if(p.y<vMin.y) return false;
	if(p.z>vMax.z) return false;
	if(p.z<vMin.z) return false;
	return true;
}

HQ_FORCE_INLINE bool HQAABB::ContainsAABB(const HQAABB &aabb)const{
	if(vMin.x>aabb.vMin.x||vMax.x<aabb.vMax.x)
		return false;
	if(vMin.y>aabb.vMin.y||vMax.y<aabb.vMax.y)
		return false;
	if(vMin.z>aabb.vMin.z||vMax.z<aabb.vMax.z)
		return false;
	return true;
}


HQ_FORCE_INLINE bool HQAABB::ContainsSphere(const HQSphere &sphere) const
{
	if (vMin.x > sphere.center.x - sphere.radius)
		return false;
	if (vMin.y > sphere.center.y - sphere.radius)
		return false;
	if (vMin.z > sphere.center.z - sphere.radius)
		return false;

	if (vMax.x < sphere.center.x + sphere.radius)
		return false;
	if (vMax.y < sphere.center.y + sphere.radius)
		return false;
	if (vMax.z < sphere.center.z + sphere.radius)
		return false;

	return true;
}

/*--------------kiểm tra cắt----------------*/

HQ_FORCE_INLINE bool HQAABB::Intersect(const HQRay3D& ray,hq_float32 *pT) const
{
	return ray.Intersect(*this , pT);
}
HQ_FORCE_INLINE bool HQAABB::Intersect(const HQRay3D& ray,hq_float32 *pT,hq_float32 maxT) const
{
	return ray.Intersect(*this , pT , maxT);
}
HQ_FORCE_INLINE bool HQAABB::Intersect(const HQPlane& plane)const
{
	return plane.Intersect(*this);
}

HQ_FORCE_INLINE bool HQAABB::Intersect(const HQOBB& obb) const
{
	return obb.Intersect(*this);
}

//***************************************
//kiểm tra 2 hình hộp cắt nhau
//***************************************
HQ_FORCE_INLINE bool HQAABB::Intersect(const HQAABB &aabb)const{
	if(vMin.x>aabb.vMax.x||vMax.x<aabb.vMin.x)
		return false;
	if(vMin.y>aabb.vMax.y||vMax.y<aabb.vMin.y)
		return false;
	if(vMin.z>aabb.vMax.z||vMax.z<aabb.vMin.z)
		return false;
	return true;
}

//------------------HQOBB
HQ_FORCE_INLINE bool HQOBB::Intersect(const HQRay3D& ray,hq_float32 *pT) const
{
	return ray.Intersect(*this , pT);
}
HQ_FORCE_INLINE bool HQOBB::Intersect(const HQRay3D& ray,hq_float32 *pT,hq_float32 maxT) const
{
	return ray.Intersect(*this , pT , maxT);
}
HQ_FORCE_INLINE bool HQOBB::Intersect(const HQPlane& plane)const
{
	return plane.Intersect(*this);
}

//------------------HQSphere
HQ_FORCE_INLINE bool HQSphere::Intersect(const HQRay3D& ray,hq_float32 *pT1,hq_float32 *pT2) const//<pT1> - lưu thời điểm cắt gần nhất , <pT2> - lưu thời điểm cắt xa nhất
{
	return ray.Intersect(*this , pT1 , pT2);
}
HQ_FORCE_INLINE bool HQSphere::Intersect(const HQRay3D& ray,hq_float32 *pT1,hq_float32 *pT2,hq_float32 maxT) const//<pT1> - lưu thời điểm cắt gần nhất , <pT2> - lưu thời điểm cắt xa nhất
{
	return ray.Intersect(*this , pT1 , pT2 , maxT);
}

HQ_FORCE_INLINE bool HQSphere::Intersect(const HQPlane& plane)const
{
	return plane.Intersect(*this);
}

HQ_FORCE_INLINE bool HQSphere::Intersect(const HQAABB &box) const
{
	return box.Intersect(*this);
}

HQ_FORCE_INLINE bool HQSphere::Intersect(const HQOBB &box) const
{
	return box.Intersect(*this);
}

HQ_FORCE_INLINE bool HQSphere::Intersect(const HQSphere & sphere) const
{
	HQ_DECL_STACK_VECTOR4(distanceVector);
	
	distanceVector = this->center - sphere.center;

	return distanceVector.LengthSqr() <= (this->radius * this->radius);
}



//HQQuaternion

//***************************************
//phép cộng
//***************************************
HQ_FORCE_INLINE HQQuaternion& HQQuaternion::operator +=(const HQQuaternion &quat){
#ifdef CMATH
	x+=quat.x;
	y+=quat.y;
	z+=quat.z;
	w+=quat.w;
#elif defined NEON_MATH
	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "vld1.32	{d2 , d3} , [%1, : 128]			\n\t"
				 "vadd.f32	q2 , q0 , q1				\n\t"
				 "vst1.32	{d4 , d5} , [%0, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (quat.q)
				 :"q0" , "q1" , "q2" , "memory"
	);
#else /*SSE*/
	float4 m0 , m1;
	m0 = _mm_load_ps(this->q);
	m1 = _mm_load_ps(quat.q);

	m0 = _mm_add_ps(m0 , m1);

	_mm_store_ps(this->q , m0);
#endif
	return *this;
}
HQ_FORCE_INLINE HQQuaternion HQQuaternion::operator +(const HQQuaternion &quat) const{
#ifdef CMATH
	return HQQuaternion(x+quat.x,y+quat.y,z+quat.z,w+quat.w);
#elif defined NEON_MATH
	HQA16ByteQuaternionPtr result;
	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "vld1.32	{d2 , d3} , [%1, : 128]			\n\t"
				 "vadd.f32	q2 , q0 , q1				\n\t"
				 "vst1.32	{d4 , d5} , [%2, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (quat.q) , "r"(result->q)
				 :"q0" , "q1" , "q2" , "memory"
				 );
	return *result;
#else /*SSE*/
	HQQuaternion result;

	float4 m0 , m1;
	m0 = _mm_load_ps(this->q);
	m1 = _mm_load_ps(quat.q);

	m0 = _mm_add_ps(m0 , m1);

	_mm_store_ps(result.q , m0);

	return result;
#endif
}
//***************************************
//phép trừ
//***************************************
HQ_FORCE_INLINE HQQuaternion& HQQuaternion::operator -=(const HQQuaternion &quat){
#ifdef CMATH
	x-=quat.x;
	y-=quat.y;
	z-=quat.z;
	w-=quat.w;
#elif defined NEON_MATH
	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "vld1.32	{d2 , d3} , [%1, : 128]			\n\t"
				 "vsub.f32	q2 , q0 , q1				\n\t"
				 "vst1.32	{d4 , d5} , [%0, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (quat.q)
				 :"q0" , "q1" , "q2" , "memory"
				 );
#else /*SSE*/
	float4 m0 , m1;
	m0 = _mm_load_ps(this->q);
	m1 = _mm_load_ps(quat.q);

	m0 = _mm_sub_ps(m0 , m1);

	_mm_store_ps(this->q , m0);
#endif
	return *this;
}
HQ_FORCE_INLINE HQQuaternion HQQuaternion::operator -(const HQQuaternion &quat) const{
#ifdef CMATH
	return HQQuaternion(x-quat.x,y-quat.y,z-quat.z,w-quat.w);
#elif defined NEON_MATH
	HQA16ByteQuaternionPtr result;
	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "vld1.32	{d2 , d3} , [%1, : 128]			\n\t"
				 "vsub.f32	q2 , q0 , q1				\n\t"
				 "vst1.32	{d4 , d5} , [%2, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (quat.q) , "r"(result->q)
				 :"q0" , "q1" , "q2" , "memory"
				 );
	return *result;
#else /*SSE*/
	HQQuaternion result;

	float4 m0 , m1;
	m0 = _mm_load_ps(this->q);
	m1 = _mm_load_ps(quat.q);

	m0 = _mm_sub_ps(m0 , m1);

	_mm_store_ps(result.q , m0);

	return result;
#endif
}


HQ_FORCE_INLINE HQQuaternion& HQQuaternion::operator *=(const hq_float32 f){
#ifdef CMATH
	x *= f;
	y *= f;
	z *= f;
	w *= f;
#elif defined NEON_MATH
	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "flds		s4 ,		[%1]				\n\t"
				 "vmul.f32	q2 , q0 , d2[0]					\n\t"
				 "vst1.32	{d4 , d5} , [%0, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (f)
				 :"q0" , "q1" , "q2" , "memory"
				 );
#else
	float4 m0=_mm_load_ps(q);
	float4 m1=_mm_load_ps1(&f);
	m0=_mm_mul_ps(m0,m1);
	_mm_store_ps(q,m0);
#endif
	return *this;
}
HQ_FORCE_INLINE HQQuaternion HQQuaternion::operator *(const hq_float32 f)const{
#ifdef CMATH
	HQQuaternion result;

	result.x = x * f;
	result.y = y * f;
	result.z = z * f;
	result.w = w * f;

	return result;
#elif defined NEON_MATH
	HQA16ByteQuaternionPtr result;
	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "flds		s4 ,		[%1]				\n\t"
				 "vmul.f32	q2 , q0 , d2[0]					\n\t"
				 "vst1.32	{d4 , d5} , [%2, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (f) , "r"(result->q)
				 :"q0" , "q1" , "q2" , "memory"
				 );

	return *result;
#else
	HQQuaternion result;
	float4 m0=_mm_load_ps(q);
	float4 m1=_mm_load_ps1(&f);
	m0=_mm_mul_ps(m0,m1);
	_mm_store_ps(result.q,m0);

	return result;
#endif
}
HQ_FORCE_INLINE HQQuaternion& HQQuaternion::operator /=(const hq_float32 f){
	hq_float32 rF=1.0f/f;
#ifdef CMATH
	x *= rF;
	y *= rF;
	z *= rF;
	w *= rF;
#elif defined NEON_MATH
	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "flds		s4 ,		[%1]				\n\t"
				 "vmul.f32	q2 , q0 , d2[0]					\n\t"
				 "vst1.32	{d4 , d5} , [%0, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (rF)
				 :"q0" , "q1" , "q2" , "memory"
				 );
#else
	float4 m0=_mm_load_ps(q);
	float4 m1=_mm_load_ps1(&rF);
	m0=_mm_mul_ps(m0,m1);
	_mm_store_ps(q,m0);
#endif
	return *this;
}
HQ_FORCE_INLINE HQQuaternion HQQuaternion::operator /(const hq_float32 f)const{
	hq_float32 rF=1.0f/f;
#ifdef CMATH
	HQQuaternion result;

	result.x = x * rF;
	result.y = y * rF;
	result.z = z * rF;
	result.w = w * rF;

	return result;
#elif defined NEON_MATH
	HQA16ByteQuaternionPtr result;

	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "flds		s4 ,		[%1]				\n\t"
				 "vmul.f32	q2 , q0 , d2[0]					\n\t"
				 "vst1.32	{d4 , d5} , [%2, : 128]			\n\t"
				 :
				 :"r"(this->q) , "r" (rF) , "r"(result->q)
				 :"q0" , "q1" , "q2" , "memory"
				 );

	return *result;
#else
	HQQuaternion result;

	float4 m0=_mm_load_ps(q);
	float4 m1=_mm_load_ps1(&rF);
	m0=_mm_mul_ps(m0,m1);
	_mm_store_ps(result.q,m0);

	return result;
#endif
}
HQ_FORCE_INLINE HQ_UTIL_MATH_API HQQuaternion operator *(const hq_float32 f,const HQQuaternion& quat)
{
#ifdef CMATH
	HQQuaternion result;

	result.x = quat.x * f;
	result.y = quat.y * f;
	result.z = quat.z * f;
	result.w = quat.w * f;

	return result;
#elif defined NEON_MATH
	HQA16ByteQuaternionPtr result;

	asm volatile(
				 "vld1.32	{d0 , d1} , [%0, : 128]			\n\t"
				 "flds		s4 ,		[%1]				\n\t"
				 "vmul.f32	q2 , q0 , d2[0]					\n\t"
				 "vst1.32	{d4 , d5} , [%2, : 128]			\n\t"
				 :
				 :"r"(quat.q) , "r" (f) , "r"(result->q)
				 :"q0" , "q1" , "q2" , "memory"
				 );

	return *result;
#else
	HQQuaternion result;

	float4 m0=_mm_load_ps(quat.q);
	float4 m1=_mm_load_ps1(&f);
	m0=_mm_mul_ps(m0,m1);
	_mm_store_ps(result.q,m0);

	return result;
#endif
}




#endif