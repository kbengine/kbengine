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


#ifndef __KBENGINE_BLOWFISH_HPP__
#define __KBENGINE_BLOWFISH_HPP__

#include "openssl/blowfish.h"
#include <string>

namespace KBEngine { 


class KBEBlowfish
{
public:
	static const int BLOCK_SIZE = 64 / 8;

	static const int MIN_KEY_SIZE = 32 / 8;
	static const int MAX_KEY_SIZE = 448 / 8;
	static const int DEFAULT_KEY_SIZE = 128 / 8;

	typedef std::string Key;

	virtual ~KBEBlowfish();
	KBEBlowfish(const Key & key);
	
	const Key & key() const { return key_; }
	const char * readableKey() const;
	bool isGood() const { return isGood_; }

	int encrypt(const unsigned char * src, unsigned char * dest, int length);
	int decrypt(const unsigned char * src, unsigned char * dest, int length);

	BF_KEY * pBFKey() { return (BF_KEY*)pBFKey_; }
protected:
	bool initKey();

	Key key_;
	int keySize_;
	bool isGood_;

	void * pBFKey_;
};

}

#endif // __KBENGINE_BLOWFISH_HPP__
