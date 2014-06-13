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

#ifndef __KBENGINE_RSA__
#define __KBENGINE_RSA__

#include <string>

namespace KBEngine
{


/**
 *	openssl rsaµÄ·â×°
 */
class KBE_RSA
{
public:
	KBE_RSA(const std::string& pubkeyname, 
		const std::string& prikeyname);

	KBE_RSA();

	virtual ~KBE_RSA();

	bool generateKey(const std::string& pubkeyname, 
		const std::string& prikeyname, int keySize = 1024, int e = 65537);

	int encrypt(const std::string& instr, std::string& outCertifdata);
	int decrypt(const std::string& inCertifdata, std::string& outstr);

	static void hexCertifData(const std::string& inCertifdata);

	bool loadPublic(const std::string& keyname);
	bool loadPrivate(const std::string& keyname);

	virtual bool isGood()const { return rsa_public != NULL && rsa_private != NULL; }
protected:
	void* rsa_public, *rsa_private;
};


}

#endif // __KBENGINE_RSA__
