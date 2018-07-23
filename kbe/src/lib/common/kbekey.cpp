// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
