#include "ReadUtil.h"
#include "byteorder.h"

NS_GC_BEGIN

int readChar(unsigned char* &data, char& value)
{
	value = read<char>(data);
	return sizeof(char);
}
int readUChar(unsigned char* &data, unsigned char& value)
{
	value = read<unsigned char>(data);
	return sizeof(unsigned char);
}
int readBool(unsigned char* &data, bool& value)
{
	value = read<char>(data)>0?true:false;
	return sizeof(bool);
}
int readInt8(unsigned char* &data, int8& value)
{
	value = read<int8>(data);
	return sizeof(int8);
}
int readShort(unsigned char* &data, short& value, bool isSwap/* =true */)
{
	value = read<short>(data);
	if (isSwap)
	{
		value = endian_swap2<short> (value);
	}
	return sizeof(short);
}
int readUint16(unsigned char* &data, uint16& value, bool isSwap/* =true */)
{
	value = read<uint16>(data);
	if (isSwap)
	{
		value = endian_swap2<uint16> (value);
	}
	return sizeof(uint16);
}
int readInt(unsigned char* &data, int& value, bool isSwap/* =true */)
{
	value = read<int>(data);
	if (isSwap)
	{
		value = endian_swap4<int>(value);
	}
	return sizeof(int);
}
int readFloat(unsigned char* &data, float& value, bool isSwap/* =true */)
{
	value = read<float>(data);
	if (isSwap)
	{
		value = endian_swap4<float>(value);
	}
	return sizeof(float);
}
int readDouble(unsigned char* &data, double& value, bool isSwap/* =true */)
{
	value = read<double>(data);
	if (isSwap)
	{
		value = endian_swap4<double>(value);
	}
	return sizeof(double);
}
int readInt64(unsigned char* &data, int64& value, bool isSwap/* =true */)
{
	value = read<int64>(data);
	if (isSwap)
	{
		value = endian_swap4<int64>(value);
	}
	return sizeof(int64);
}
int readString(unsigned char* &data, std::string& value, bool isSwap/* =true */)
{
	short len;
	readShort(data, len, isSwap);
	uint8* dest = new uint8[len];
	memset(dest, 0, len);
	memcpy(dest, data, len);
	value.clear();
	value.append((const char*)dest, len);
	data = data+len;//data后移
	delete[] dest;
	return (len+sizeof(short));
}
int readCSString(unsigned char* &data, std::string& value, bool isSwap)
{
	int stringLength = 0;
	bool stringLengthParsed = false;
	int step = 0;
	int addPos = 0;//读取了的长度
	char part;
	while(!stringLengthParsed)
	{
		addPos += readChar(data, part);
		stringLengthParsed = (((int)part >> 7) == 0);
		int partCutter = part & 127;
		part = (char)partCutter;
		int toAdd = (int)part << (step*7);
		stringLength += toAdd;
		step++;
	}
	uint8* dest = new uint8[stringLength];
	memset(dest, 0, stringLength);
	memcpy(dest, data, stringLength);
	value.clear();
	value.append((const char*)dest, stringLength);
	data = data+stringLength;//data后移
	delete[] dest;

	addPos += stringLength;
	return addPos;
}

NS_GC_END