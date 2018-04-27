// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBENGINE_KEY_H
#define KBENGINE_KEY_H

#include "rsa.h"
#include "common/singleton.h"

namespace KBEngine
{


/**
 *	引擎的key管理
 */
class KBEKey : public KBE_RSA, public Singleton<KBEKey>
{
public:
	KBEKey(const std::string& pubkeyname, 
		const std::string& prikeyname);

	KBEKey();
	virtual ~KBEKey();

	virtual bool isGood() const;
};


}

#endif // KBENGINE_KEY_H
