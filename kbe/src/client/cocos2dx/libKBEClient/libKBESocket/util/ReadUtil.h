#pragma once

#include "basic_types.h"
#include <string>

NS_GC_BEGIN

int readInt8(unsigned char* &data, int8& value);//返回读取长度
int readShort(unsigned char* &data, short& value, bool isSwap=true);
int readUint16(unsigned char* &data, uint16& value, bool isSwap=true);
int readInt(unsigned char* &data, int& value, bool isSwap=true);
int readInt64(unsigned char* &data, int64& value, bool isSwap=true);
int readFloat(unsigned char* &data, float& value, bool isSwap=true);
int readDouble(unsigned char* &data, double& value, bool isSwap=true);
int readBool(unsigned char* &data, bool& value);
int readChar(unsigned char* &data, char& value);
int readUChar(unsigned char* &data, unsigned char& value);
int readString(unsigned char* &data, std::string& value, bool isSwap=true);
int readCSString(unsigned char* &data, std::string& value, bool isSwap=true);//读取c#文件写的string

inline void my_memcpy(unsigned char* dst, unsigned char* src, size_t len)
{
	while (len--)
	{
		*dst++ = *src++;
	}
}
template<typename T> inline T read(unsigned char* &data)
{
	T r;
	my_memcpy((unsigned char*)&r, data, sizeof(T));
	data = data+sizeof(T);
	return r;
}

NS_GC_END