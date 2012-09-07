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


#include "address.hpp"

namespace KBEngine { 
namespace Mercury
{
char Address::s_stringBuf[2][32] = {{0},{0}};

int Address::s_currStringBuf = 0;
const Address Address::NONE(0, 0);


//-------------------------------------------------------------------------------------
static ObjectPool<Address> _g_objPool;
ObjectPool<Address>& Address::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
void Address::onReclaimObject()
{
	ip = 0;
	port = 0;
}

//-------------------------------------------------------------------------------------
int Address::writeToString(char * str, int length) const
{
	uint32	hip = ntohl(ip);
	uint16	hport = ntohs(port);

	return kbe_snprintf(str, length,
		"%d.%d.%d.%d:%d",
		(int)(uchar)(hip>>24),
		(int)(uchar)(hip>>16),
		(int)(uchar)(hip>>8),
		(int)(uchar)(hip),
		(int)hport);
}

//-------------------------------------------------------------------------------------
char * Address::c_str() const
{
	char * buf = Address::nextStringBuf();
	this->writeToString(buf, 32);
    return buf;
}

//-------------------------------------------------------------------------------------
const char * Address::ipAsString() const
{
	uint32	hip = ntohl(ip);
	char * buf = Address::nextStringBuf();

	kbe_snprintf(buf, 32, "%d.%d.%d.%d",
		(int)(uchar)(hip>>24),
		(int)(uchar)(hip>>16),
		(int)(uchar)(hip>>8),
		(int)(uchar)(hip));

    return buf;
}

//-------------------------------------------------------------------------------------
char * Address::nextStringBuf()
{
	s_currStringBuf = (s_currStringBuf + 1) % 2;
	return s_stringBuf[ s_currStringBuf ];
}

}
}
