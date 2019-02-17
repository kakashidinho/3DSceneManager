/*
Copyright (C) 2010-2013  Le Hoang Quyen (lehoangq@gmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the MIT license.  See the file
COPYING.txt included with this distribution for more information.


*/

#ifndef HQ_PLATFORM_DEF_H
#define HQ_PLATFORM_DEF_H

#ifdef WIN32//win32
#	if defined WINAPI_FAMILY
#		include <winapifamily.h>
#	endif
#	if defined WINAPI_FAMILY && defined WINAPI_FAMILY_PHONE_APP && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#		if !defined HQ_WIN_PHONE_PLATFORM
#			define HQ_WIN_PHONE_PLATFORM
#		endif
#	elif defined WINAPI_FAMILY && defined WINAPI_FAMILY_APP && WINAPI_FAMILY == WINAPI_FAMILY_APP
#		ifndef HQ_WIN_STORE_PLATFORM
#			define HQ_WIN_STORE_PLATFORM
#		endif
#	else
#		ifndef HQ_WIN_DESKTOP_PLATFORM
#			define HQ_WIN_DESKTOP_PLATFORM
#		endif
#	endif
#elif defined __APPLE__
#	include <TargetConditionals.h>
#	if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR //ios
#		import <Availability.h>
#		ifndef HQ_OPENGLES
#			define HQ_OPENGLES
#		endif
#		define HQ_IPHONE_PLATFORM
#	else //Mac OS
#		define HQ_MAC_PLATFORM
#	endif
#elif defined ANDROID//android
#	ifndef HQ_OPENGLES
#		define HQ_OPENGLES
#	endif
#	define HQ_ANDROID_PLATFORM
#elif defined WINAPI_FAMILY 
#	include <winapifamily.h>
#	if defined WINAPI_FAMILY_PHONE_APP && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#		ifndef HQ_WIN_PHONE_PLATFORM
#			define HQ_WIN_PHONE_PLATFORM
#		endif
#	elif defined WINAPI_FAMILY_APP && WINAPI_FAMILY == WINAPI_FAMILY_APP
#		ifndef HQ_WIN_STORE_PLATFORM
#			define HQ_WIN_STORE_PLATFORM
#		endif
#	elif defined WINAPI_FAMILY_DESKTOP_APP && WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
#		ifndef HQ_WIN_DESKTOP_PLATFORM
#			define HQ_WIN_DESKTOP_PLATFORM
#		endif
#	endif
#else//linux
#	define HQ_LINUX_PLATFORM
#endif

#ifndef HQ_MALLOC
#	define HQ_MALLOC malloc
#endif
#ifndef HQ_REALLOC
#	define HQ_REALLOC realloc
#endif
#ifndef HQ_FREE
#	define HQ_FREE free
#endif


#ifdef __cplusplus
#ifndef HQ_NEW
#	define HQ_NEW new
#endif
#ifndef HQ_DELETE
#	define HQ_DELETE delete
#endif
#endif//#ifdef __cplusplus

#endif
