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

#include "md5.hpp"

namespace KBEngine
{

KBE_MD5::KBE_MD5()
{
	MD5_Init(&state_);
	isFinal_ = false;
}

//-------------------------------------------------------------------------------------
KBE_MD5::~KBE_MD5()
{
}

//-------------------------------------------------------------------------------------
void KBE_MD5::append(const void * data, int numBytes)
{
	MD5_Update(&state_, (const unsigned char*)data, numBytes);
}

//-------------------------------------------------------------------------------------
const unsigned char* KBE_MD5::getDigest()
{
	if(!isFinal_)
	{
		MD5_Final(bytes_, &state_);
		isFinal_ = true;
	}

	return bytes_;
}

//-------------------------------------------------------------------------------------
void KBE_MD5::clear()
{
	memset(this, 0, sizeof(*this));
}

//-------------------------------------------------------------------------------------
bool KBE_MD5::operator==(const KBE_MD5 & other) const
{
	return memcmp(this->bytes_, other.bytes_, sizeof(bytes_)) == 0;
}

//-------------------------------------------------------------------------------------
bool KBE_MD5::operator<(const KBE_MD5 & other) const
{
	return memcmp(this->bytes_, other.bytes_, sizeof(bytes_)) < 0;
}


//-------------------------------------------------------------------------------------
} 
