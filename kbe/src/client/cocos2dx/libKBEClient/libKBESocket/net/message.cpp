/*
 * Message.cpp
 *
 *  Created on: 2011-9-19
 *      Author: lyh
 */

#include "message.h"
#include "../util/byteorder.h"

NS_GC_BEGIN

Message::Message()
{
}

Message::~Message()
{
}
uint32 Message::getOpcode()
{
	return opcode;
}
void Message::setOpcode(uint32 opcode)
{
	this->opcode = opcode;
}
ByteBuffer& Message::getBuffer()
{
	return this->buffer;
}
void Message::setBuffer(ByteBuffer& buffer)
{
	this->buffer = buffer;
}
Message & Message::operator<<(bool value)
{
	buffer << value;
	return *this;
}
Message & Message::operator<<(uint8 value)
{
	buffer << value;
	return *this;
}
Message & Message::operator<<(uint16 value)
{
	buffer << endian_swap2<uint16> (value);
	return *this;
}
Message & Message::operator<<(uint32 value)
{
	buffer << endian_swap4<uint32> (value);
	return *this;
}
Message & Message::operator<<(uint64 value)
{
	buffer << endian_swap8<uint64> (value);
	return *this;
}
Message & Message::operator<<(int8 value)
{
	buffer << value;
	return *this;
}
Message & Message::operator<<(int16 value)
{
	buffer << endian_swap2<int16> (value);
	return *this;
}
Message & Message::operator<<(int32 value)
{
	buffer << endian_swap4<uint32> (value);
	return *this;
}
Message & Message::operator<<(int64 value)
{
	buffer << endian_swap8<int64> (value);
	return *this;
}
Message & Message::operator<<(float value)
{
	buffer << endian_swap4<float> (value);
	return *this;
}
Message & Message::operator<<(double value)
{
	buffer << endian_swap8<double> (value);
	return *this;
}
Message & Message::operator<<(const char* str)
{
	int len = strlen(str);
	buffer << endian_swap2<uint16> (len);
	buffer.append((uint8*)str, len);
	return *this;
}
Message & Message::operator>>(bool & value)
{
	buffer >> value;
	return *this;
}
Message & Message::operator>>(uint8 & value)
{
	buffer >> value;
	return *this;
}
Message & Message::operator>>(uint16 & value)
{
	buffer >> value;
	value = endian_swap2<uint16> (value);
	return *this;
}
Message & Message::operator>>(uint32 & value)
{
	buffer >> value;
	value = endian_swap4<uint32> (value);
	return *this;
}
Message & Message::operator>>(uint64 & value)
{
	buffer >> value;
	value = endian_swap8<uint64> (value);
	return *this;
}
Message & Message::operator>>(int8 & value)
{
	buffer >> value;
	return *this;
}
Message & Message::operator>>(int16 & value)
{
	buffer >> value;
	value = endian_swap2<int16> (value);
	return *this;
}
Message & Message::operator>>(int32 & value)
{
	buffer >> value;
	value = endian_swap4<int32> (value);
	return *this;
}
Message & Message::operator>>(int64 & value)
{
	buffer >> value;
	value = endian_swap8<int64> (value);
	return *this;
}
Message & Message::operator>>(float & value)
{
	buffer >> value;
	value = endian_swap4<float> (value);
	return *this;
}
Message & Message::operator>>(double & value)
{
	buffer >> value;
	value = endian_swap8<double> (value);
	return *this;
}
Message & Message::operator>>(std::string & value)
{
	value.clear();
	uint16 len;
	buffer >> len;
	len = endian_swap2(len);
	if (len==0)
	{
		return *this;
	}
	uint8* dest = new uint8[len];
	memset(dest, 0, len);
	buffer.read(dest, len);
	value.append((const char*) dest, len);
	delete[] dest;
//	len = value.size() / 3 + 1;
//	wchar_t* ws = new wchar_t[len];
//	memset(ws, 0, sizeof(wchar_t) * len);
//	len = UTF8ToUnicode(value.c_str(), ws);
//	value = wstring2string(ws);
	return *this;
}

NS_GC_END