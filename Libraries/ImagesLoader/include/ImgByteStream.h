#ifndef IMG_BYTE_STREAM_H
#define IMG_BYTE_STREAM_H

#include "HQPrimitiveDataType.h"

#if defined HQ_WIN_PHONE_PLATFORM || defined HQ_WIN_STORE_PLATFORM
#include "../HQEngine/winstore/HQWinStoreFileSystem.h"
#endif

#include <stdio.h>

class ImgByteStream
{
public:
	ImgByteStream();
	~ImgByteStream();

	void CreateByteStreamFromMemory(const hqubyte8 * memory, size_t size);
	bool CreateByteStreamFromFile(const char *fileName);

	void Rewind();
	void Clear();
	void Seek(size_t pos);
	void Advance(size_t offset);
	
	bool IsMemoryMode() const {return isMemoryMode;}
	const hqubyte8 * GetMemorySrc() const {return memory;}

	hqint32 GetByte();//return EOF if reach end of stream
	bool GetBytes(void *bytes, size_t maxSize);
	size_t TryGetBytes(void *buffer, size_t elemSize, size_t numElems);//return number of element successfully read
	size_t GetStreamSize() const {return streamSize;}
private:

	bool isMemoryMode;
	size_t streamSize;
	union{
#if defined HQ_WIN_PHONE_PLATFORM || defined HQ_WIN_STORE_PLATFORM
		HQWinStoreFileSystem::BufferedDataReader *file;
#else
		FILE *file;
#endif
		struct{
			const hqubyte* memory;
			size_t iterator;
		};
	};
};

#endif