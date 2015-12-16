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

#include "kbekey.h"
#include "common.h"
#include "helper/debug_helper.h"


namespace KBEngine
{
KBE_SINGLETON_INIT(KBEKey);

//-------------------------------------------------------------------------------------
KBEKey::KBEKey(const std::string& pubkeyname, const std::string& prikeyname):
KBE_RSA()
{
	if(pubkeyname.size() > 0 || prikeyname.size() > 0)
	{
		KBE_ASSERT(pubkeyname.size() > 0);
		
		if(g_componentType != CLIENT_TYPE)
		{
			KBE_ASSERT(prikeyname.size() > 0);

			bool key1 = loadPrivate(prikeyname);
			bool key2 = loadPublic(pubkeyname);
			KBE_ASSERT(key1 == key2);

			if(!key1 && !key2)
			{
				bool ret = generateKey(pubkeyname, prikeyname);
				KBE_ASSERT(ret);
				key1 = loadPrivate(prikeyname);
				key2 = loadPublic(pubkeyname);
				KBE_ASSERT(key1 && key2);
			}
		}
		else
		{
			bool key = loadPublic(pubkeyname);
			KBE_ASSERT(key);
		}
	}
}

//-------------------------------------------------------------------------------------
KBEKey::KBEKey():
KBE_RSA()
{
}

//-------------------------------------------------------------------------------------
KBEKey::~KBEKey()
{
}

//-------------------------------------------------------------------------------------
bool KBEKey::isGood() const
{
	if(g_componentType == CLIENT_TYPE)
	{
		return rsa_public != NULL;
	}

	return rsa_public != NULL && rsa_private != NULL;
}

//-------------------------------------------------------------------------------------
} 
