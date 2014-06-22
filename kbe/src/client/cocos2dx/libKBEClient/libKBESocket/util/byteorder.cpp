#include "byteorder.h"

NS_GC_BEGIN

uint16 endian_swap(uint16 value)
{
	return (value >> 8) | (value << 8);
}

uint32 endian_swap(uint32 value)
{
	return (value >> 24) | ((value << 8) & 0x00FF0000) | ((value >> 8)
		& 0x0000FF00) | (value << 24);
}

float endian_swap(float value)
{
	char sBuf[4];
	char* temp;
	memset(sBuf, 0, sizeof(sBuf));
	temp = (char*) (&value);
	sBuf[0] = temp[3];
	sBuf[1] = temp[2];
	sBuf[2] = temp[1];
	sBuf[3] = temp[0];
	float *w = (float*) (&sBuf);
	return *w;
}

double endian_swap(double value)
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
	double *w = (double*) (&sBuf);
	return *w;
}
short endian_swap(short value)
{
	return ((value >> 8) & 0x00FF) | (value << 8);
}

int endian_swap(int value)
{
	return (value << 24) | ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000);
}

NS_GC_END