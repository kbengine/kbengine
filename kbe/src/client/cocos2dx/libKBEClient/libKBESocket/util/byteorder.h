#ifndef TALK_BASE_BYTEORDER_H__
#define TALK_BASE_BYTEORDER_H__

#include "basic_types.h"
#include <string>

NS_GC_BEGIN

uint16 endian_swap(uint16 value);
uint32 endian_swap(uint32 value);
float endian_swap(float value);
double endian_swap(double value);
short endian_swap(short value);
int endian_swap(int value);

template<typename T> inline T endian_swap(T value)
{
	size_t size = sizeof(value);
	switch (size)
	{
	case 2:
		return endian_swap2(value);
	case 4:
		return endian_swap4(value);
	case 8:
		return endian_swap8(value);
	default:
		return value;
	}
}
template<typename T> inline T endian_swap2(T value)
{
	char sBuf[2];
	char* temp;
	memset(sBuf, 0, sizeof(sBuf));
	temp = (char*) (&value);
	sBuf[0] = temp[1];
	sBuf[1] = temp[0];
	T *w = (T*) (&sBuf);
	return *w;
}
template<typename T> inline T endian_swap4(T value)
{
	char sBuf[4];
	char* temp;
	memset(sBuf, 0, sizeof(sBuf));
	temp = (char*) (&value);
	sBuf[0] = temp[3];
	sBuf[1] = temp[2];
	sBuf[2] = temp[1];
	sBuf[3] = temp[0];
	T *w = (T*) (&sBuf);
	return *w;
}
template<typename T> inline T endian_swap8(T value)
{
	char sBuf[8];
	char* temp;
	memset(sBuf, 0, sizeof(sBuf));
	temp = (char*) (&value);
	sBuf[0] = temp[7];
	sBuf[1] = temp[6];
	sBuf[2] = temp[5];
	sBuf[3] = temp[4];
	sBuf[4] = temp[3];
	sBuf[5] = temp[2];
	sBuf[6] = temp[1];
	sBuf[7] = temp[0];
	T *w = (T*) (&sBuf);
	return *w;
}

NS_GC_END

#endif
