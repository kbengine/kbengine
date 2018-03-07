/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_ENTITY_AUTOLOADER_H
#define KBE_ENTITY_AUTOLOADER_H

#include "common/common.h"

namespace KBEngine{

class InitProgressHandler;
class EntityAutoLoader
{
public:
	EntityAutoLoader(Network::NetworkInterface & networkInterface, InitProgressHandler* pInitProgressHandler);
	~EntityAutoLoader();
	
	bool process();

	void pInitProgressHandler(InitProgressHandler* p)
		{ pInitProgressHandler_ = p; }

	/** 网络接口
		数据库中查询的自动entity加载信息返回
	*/
	void onEntityAutoLoadCBFromDBMgr(Network::Channel* pChannel, MemoryStream& s);

private:
	Network::NetworkInterface & networkInterface_;
	InitProgressHandler* pInitProgressHandler_;

	std::vector< std::vector<ENTITY_SCRIPT_UID> > entityTypes_;

	// 每次取查询结果集的区段
	ENTITY_ID start_;
	ENTITY_ID end_;

	bool querying_;
};


}

#endif // KBE_ENTITY_AUTOLOADER_H
