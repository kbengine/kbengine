// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#ifndef KBE_STRING_CONV_H
#define KBE_STRING_CONV_H
#include "common/platform.h"

namespace KBEngine{
namespace StringConv
{

template<typename T>
T str2value(const std::string& s) 
{
	std::istringstream is(s);
	T t;
	is >> t;
	return t;
}

template<typename T>
T str2value(const char* s) 
{
	std::istringstream is(s);
	T t;
	is >> t;
	return t;
}

template <class TYPE>
inline std::string val2str(const TYPE& value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

inline void str2value(float& value, const char * pstr)
{
	char* endptr;
	value = strtof(pstr, &endptr);

	if (*endptr)
		throw std::runtime_error("not a number");
}

inline void str2value(double& value, const char * pstr)
{
	char* endptr;
	value = strtod(pstr, &endptr);
	if (*endptr)
		throw std::runtime_error("not a number");
}

inline void str2value(int32& value, const char * pstr)
{
	char* endptr;
	value = strtol(pstr, &endptr, 10);
	if (*endptr)
		throw std::runtime_error("not a number");
}

inline void str2value(int8& value, const char * pstr)
{
	int32 i;
	str2value(i, pstr);
	value = int8(i);
	if (value != i)
		throw std::runtime_error("out of range");
}

inline void str2value(int16& value, const char * pstr)
{
	int32 i;
	str2value(i, pstr);
	value = int16(i);
	if (value != i)
		throw std::runtime_error("out of range");
}

inline void str2value(uint32& value, const char * pstr)
{
	char* endptr;
	value = strtoul(pstr, &endptr, 10);
	if (*endptr)
		throw std::runtime_error("not a number");
}

inline void str2value(uint8& value, const char * pstr)
{
	uint32 ui;
	str2value(ui, pstr);
	value = uint8(ui);
	if (value != ui)
		throw std::runtime_error("out of range");
}

inline void str2value(uint16& value, const char * pstr)
{
	uint32 ui;
	str2value(ui, pstr);
	value = uint16(ui);
	if (value != ui)
		throw std::runtime_error("out of range");
}

inline void str2value(int64& value, const char * pstr)
{
	char* endptr;
	value = strtoll(pstr, &endptr, 10);
	if (*endptr)
		throw std::runtime_error("not a number");
}

inline void str2value(uint64& value, const char * pstr)
{
	char* endptr;
	value = strtoull(pstr, &endptr, 10);
	if (*endptr)
		throw std::runtime_error("not a number");
}


}
}

#endif // KBE_STRING_CONV_H


