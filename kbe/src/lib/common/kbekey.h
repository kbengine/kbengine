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
