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

#ifndef KBE_PENDING_LOGIN_MGR_H
#define KBE_PENDING_LOGIN_MGR_H

#include "common/common.h"
#include "common/tasks.h"
#include "common/timer.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "server/components.h"
#include "network/address.h"

namespace KBEngine { 

namespace Network
{
class NetworkInterface;
class EventDispatcher;
}

/*
	登录到服务器检测成功， 但还没有进入到游戏世界时， 需要将账号缓存一下便于后续处理
*/
class PendingLoginMgr : public Task
{
public:
	struct PLInfos
	{
		PLInfos()
		{
			ctype = UNKNOWN_CLIENT_COMPONENT_TYPE;
			flags = 0;
			deadline = 0;
			entityDBID = 0;
			entityID = 0;
			lastProcessTime = 0;
			forceInternalLogin = false;
			needCheckPassword = true;
		}

		Network::Address addr;
		COMPONENT_CLIENT_TYPE ctype;
		std::string accountName;
		std::string password;
		std::string datas;
		TimeStamp lastProcessTime;
		ENTITY_ID entityID;
		DBID entityDBID;
		uint32 flags;
		uint64 deadline;
		bool forceInternalLogin;
		bool needCheckPassword;
	};

	typedef KBEUnordered_map<std::string, PLInfos*> PTINFO_MAP;

public:
	PendingLoginMgr(Network::NetworkInterface & networkInterface);
	~PendingLoginMgr();

	Network::EventDispatcher & dispatcher();

	bool add(PLInfos* infos);
	
	bool process();
	
	PendingLoginMgr::PLInfos* remove(std::string& accountName);
	PendingLoginMgr::PLInfos* find(std::string& accountName);

	void removeNextTick(std::string& accountName);

private:
	Network::NetworkInterface & networkInterface_;

	bool start_;
	
	PTINFO_MAP pPLMap_;

};

}

#endif // KBE_PENDING_LOGIN_MGR_H
