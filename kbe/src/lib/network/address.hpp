/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBE_ADDRESS__
#define __KBE_ADDRESS__

#include "cstdkbe/cstdkbe.hpp"
#include "network/common.hpp"

namespace KBEngine { 
namespace Mercury
{
class Address
{
public:
	static const Address NONE;

	Address();
	Address(uint32 ipArg, uint16 portArg);
	uint32	ip;
	uint16	port;

	int writeToString(char * str, int length) const;
	
	operator char*() const	{ return this->c_str(); }

	char * c_str() const;
	const char * ipAsString() const;
	bool isNone() const			{ return this->ip == 0; }
private:
	static char s_stringBuf[ 2 ][32];
	static int s_currStringBuf;
	static char * nextStringBuf();
};

inline Address::Address()
{
}

inline Address::Address(uint32 ipArg, uint16 portArg) :
	ip(ipArg),
	port(portArg)
{
}

inline bool operator==(const Mercury::Address & a, const Mercury::Address & b)
{
	return (a.ip == b.ip) && (a.port == b.port);
}


inline bool operator!=(const Mercury::Address & a, const Mercury::Address & b)
{
	return (a.ip != b.ip) || (a.port != b.port);
}

/**
 * 	This operator compares two addresses. It is needed
 * 	for using an address as a key in an STL map.
 *
 * 	@return true if a is less than b, false otherwise.
 */
inline bool operator<(const Mercury::Address & a, const Mercury::Address & b)
{
	return (a.ip < b.ip) || (a.ip == b.ip && (a.port < b.port));
}


}
}
#endif // __EVENT_POLLER__