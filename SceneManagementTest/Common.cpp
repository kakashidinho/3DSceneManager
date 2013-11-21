#include "windows.h"
#include "Common.h"


wchar_t * UTF8(const char *multibyteString)
{
	unsigned int size = MultiByteToWideChar(CP_UTF8 , 0 , multibyteString , -1 , NULL , 0);

	wchar_t *str = new wchar_t[size];
	if (str == NULL)
		return NULL;

	MultiByteToWideChar(CP_UTF8 , 0 , multibyteString , -1 , str , size);

	return str;
}