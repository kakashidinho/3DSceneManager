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

wchar_t * ContainingDirectory(const wchar_t* path, size_t size) // returned string need to be deleted after done using it
{
	size_t parentSize = FindLastPathSeperator(path, size);
	if (parentSize > size)
		return NULL;

	wchar_t *str = new wchar_t[parentSize + 1];

	memcpy(str, path, parentSize);

	str[parentSize] = 0;

	return str;
}

size_t FindLastPathSeperator(const wchar_t* path, size_t size)
{
	const wchar_t* p = path + size;
	while (p > path && (*p != L'\\') && (*p != L'/')) {
		--p;
	}

	if (p == path)
		return size + 1;

	return p - path;
}