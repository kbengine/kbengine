/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef KBENGINE_MD5_H
#define KBENGINE_MD5_H

#include "openssl/md5.h"
#include <string>

namespace KBEngine
{

/**
 *	openssl md5µÄ·â×°
 */
class KBE_MD5
{
public:
	KBE_MD5();
	KBE_MD5(const void * data, int numBytes);
	~KBE_MD5();

	void append(const void * data, int numBytes);
	const unsigned char* getDigest();
	std::string getDigestStr();

	void clear();
	
	void final();

	bool operator==( const KBE_MD5 & other ) const;
	bool operator!=( const KBE_MD5 & other ) const
		{ return !(*this == other); }

	bool operator<( const KBE_MD5 & other ) const;

	static std::string getDigest(const void * data, int numBytes);

	bool isFinal() const{ return isFinal_; }

private:
	MD5_CTX state_;
	unsigned char bytes_[16];
	bool isFinal_;
};


}

#endif // KBENGINE_MD5_H
