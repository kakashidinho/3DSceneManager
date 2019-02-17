#ifndef COMMON_H
#define COMMON_H
#include <windows.h>
#ifndef CMATH
#	define CMATH
#endif
#include "../Libraries/HQUtil/include/math/HQUtilMath.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

wchar_t * UTF8(const char *multibyteString);//Create wide string from utf8 unicode string.returned string need to be deleted after done using it
wchar_t * ContainingDirectory(const wchar_t* path, size_t size); // returned string need to be deleted after done using it
size_t FindLastPathSeperator(const wchar_t* path, size_t size); // return size + 1 if no separator found

class Terrain;
extern Terrain * ge_terrain;
extern float ge_worldMaxHeight;

#endif