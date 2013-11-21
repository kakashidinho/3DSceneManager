/*********************************************************************
*Copyright 2010 Le Hoang Quyen. All rights reserved.
*********************************************************************/
#ifndef HQ_ASSERT_H
#define HQ_ASSERT_H
#include <assert.h>

#ifdef WIN32
#include <crtdbg.h>
#include <windows.h>
#endif


#ifndef HQ_ASSERT
#	if defined DEBUG || defined _DEBUG
#		ifdef WIN32
#			define HQ_ASSERT(e) {_ASSERTE(e);}
#		else
#			define HQ_ASSERT(e) { assert(e);}
#		endif
#	else
#		define HQ_ASSERT(e)
#	endif
#endif


#endif