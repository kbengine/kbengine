/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __KBE_ADDRESS__
#define __KBE_ADDRESS__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/objectpool.hpp"
#include "network/common.hpp"

namespace KBEngine { 
namespace Mercury
{
class Address  : public PoolObject
{
public:
	static const Address NONE;

	typedef std::tr1::shared_ptr< SmartPoolObject< Address > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();
	static ObjectPool<Address>& ObjPool();
	void onReclaimObject();

	Address();
	Address(uint32 ipArg, uint16 portArg);
	uint32	ip;
	uint16	port;

	int writeToString(char * str, int length) const;

	operator char*() const { return this->c_str(); }

	char * c_str() const;
	const char * ipAsString() const;
	bool isNone() const	{ return this->ip == 0; }
private:
	static char s_stringBuf[2][32];
	static int s_currStringBuf;
	static char * nextStringBuf();
};

inline Address::Address():
ip(0),
port(0)
{
}

inline Address::Address(uint32 ipArg, uint16 portArg) :
	ip(ipArg),
	port(portArg)
{
} 

inline bool operator==(const Address & a, const Address & b)
{
	return (a.ip == b.ip) && (a.port == b.port);
}


inline bool operator!=(const Address & a, const Address & b)
{
	return (a.ip != b.ip) || (a.port != b.port);
}

/**
 * 	This operator compares two addresses. It is needed
 * 	for using an address as a key in an STL map.
 *
 * 	@return true if a is less than b, false otherwise.
 */
inline bool operator<(const Address & a, const Address & b)
{
	return (a.ip < b.ip) || (a.ip == b.ip && (a.port < b.port));
}


}
}
#endif // __EVENT_POLLER__
