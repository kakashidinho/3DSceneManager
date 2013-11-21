/*********************************************************************
*Copyright 2011 Le Hoang Quyen. All rights reserved.
*********************************************************************/
#ifndef HQ_PRIMITIVE_DATA_TYPE_H
#define HQ_PRIMITIVE_DATA_TYPE_H

typedef int hq_int32 , hqint32 , hqint , hq_int;
typedef unsigned int hq_uint32 , hquint32 , hquint , hq_uint;
typedef short hq_short16 , hqshort16 ,hq_short , hqshort;
typedef unsigned short hq_ushort16 , hqushort16 ,hq_ushort , hqushort;
typedef char hq_byte8 , hqbyte8 , hq_byte , hqbyte;
typedef unsigned char hq_ubyte8 , hqubyte8 , hq_ubyte , hqubyte;
typedef float hq_float32 , hqfloat32 ,hq_float ,hqfloat;
typedef double hq_float64 , hqfloat64 ,hq_double , hqdouble;

typedef long long hq_int64 , hqint64;
typedef unsigned long long hq_uint64 , hquint64;


typedef enum HQBool
{
	HQ_FALSE=0,
	HQ_TRUE=1,
	HQ_BOOL_FORCE_DWORD=0xffffffff
} HQBoolType , hq_bool , hqbool;


#endif