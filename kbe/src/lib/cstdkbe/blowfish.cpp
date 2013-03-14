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

#include "blowfish.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
KBEBlowfish::KBEBlowfish(const Key & key):
key_(key),
keySize_(key.size()),
isGood_(false),
pBFKey_(0)
{
	if (initKey())
	{
		DEBUG_MSG(boost::format("Using Blowfish key: %1%\n") % this->readableKey());
	}
}

//-------------------------------------------------------------------------------------
KBEBlowfish::~KBEBlowfish()
{
	delete this->pBFKey();
	pBFKey_ = NULL;
}

//-------------------------------------------------------------------------------------
bool KBEBlowfish::initKey()
{
	pBFKey_ = new BF_KEY;

	if ((MIN_KEY_SIZE <= keySize_) && (keySize_ <= MAX_KEY_SIZE))
	{
		BF_set_key(this->pBFKey(), key_.size(), (unsigned char*)key_.c_str() );
		isGood_ = true;
	}
	else
	{
		ERROR_MSG(boost::format("KBEBlowfish::initKey: "
			"invalid length %1%\n") %
			keySize_ );

		isGood_ = false;
	}

	return isGood_;
}

//-------------------------------------------------------------------------------------
const char * KBEBlowfish::readableKey() const
{
	static char buf[1024];
	char *c = buf;

	for (int i=0; i < keySize_; i++)
	{
		c += sprintf(c, "%02hhX ", (unsigned char)key_[i]);
	}

	c[-1] = '\0';
	return buf;
}

//-------------------------------------------------------------------------------------
int KBEBlowfish::encrypt( const unsigned char * src, unsigned char * dest,
	int length )
{
	// BLOCK_SIZEµÄÕûÊý±¶
	if(length % BLOCK_SIZE != 0)
	{
		CRITICAL_MSG(boost::format("Blowfish::encrypt: "
			"Input length (%1%) is not a multiple of block size (%2%)\n") %
			length % (BLOCK_SIZE));
	}

	uint64 * pPrevBlock = NULL;
	for (int i=0; i < length; i += BLOCK_SIZE)
	{
		if (pPrevBlock)
		{
			*(uint64*)(dest + i) = *(uint64*)(src + i) ^ (*pPrevBlock);
		}
		else
		{
			*(uint64*)(dest + i) = *(uint64*)(src + i);
		}

		BF_ecb_encrypt(dest + i, dest + i, this->pBFKey(), BF_ENCRYPT);
		pPrevBlock = (uint64*)(src + i);
	}

	return length;
}

//-------------------------------------------------------------------------------------
int KBEBlowfish::decrypt( const unsigned char * src, unsigned char * dest,
	int length )
{
	if (length % BLOCK_SIZE != 0)
	{
		WARNING_MSG(boost::format("Blowfish::decrypt:"
			"Input stream size (%1%) is not a multiple of the block size (%2%)\n") %
			length % (BLOCK_SIZE));

		return -1;
	}

	uint64 * pPrevBlock = NULL;
	for (int i=0; i < length; i += BLOCK_SIZE)
	{
		BF_ecb_encrypt(src + i, dest + i, this->pBFKey(), BF_DECRYPT);

		if (pPrevBlock)
		{
			*(uint64*)(dest + i) ^= *pPrevBlock;
		}

		pPrevBlock = (uint64*)(dest + i);
	}

	return length;
}

//-------------------------------------------------------------------------------------

}
