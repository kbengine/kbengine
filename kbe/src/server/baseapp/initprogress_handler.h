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

#ifndef KBE_BASEAPP_INIT_PROGRESS_HANDLER_H
#define KBE_BASEAPP_INIT_PROGRESS_HANDLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class EntityAutoLoader;
class InitProgressHandler : public Task
{
public:
	InitProgressHandler(Network::NetworkInterface & networkInterface);
	~InitProgressHandler();
	
	bool process();

	void setAutoLoadState(int8 state);

	/** 网络接口
		数据库中查询的自动entity加载信息返回
	*/
	void onEntityAutoLoadCBFromDBMgr(Network::Channel* pChannel, MemoryStream& s);


	void setError();

private:
	Network::NetworkInterface & networkInterface_;
	int delayTicks_;
	EntityAutoLoader* pEntityAutoLoader_;
	int8 autoLoadState_;
	bool error_;
	bool baseappReady_;
};


}

#endif // KBE_BASEAPP_INIT_PROGRESS_HANDLER_H
