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

#ifndef __PENDING_LOGIN_MGR__
#define __PENDING_LOGIN_MGR__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/tasks.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "server/components.hpp"
#include "network/address.hpp"

namespace KBEngine { 

namespace Mercury
{
class NetworkInterface;
class EventDispatcher;
}

/*
	登录到服务器检测成功， 但还没有进入到游戏世界时， 需要将账号缓存一下便于后续处理
*/
class PendingLoginMgr : public Task, 
						public Singleton<PendingLoginMgr>
{
public:
	struct PLInfos
	{
		Mercury::Address addr;
		COMPONENT_CLIENT_TYPE ctype;
		std::string accountName;
		std::string password;
		std::string datas;
		TimeStamp lastProcessTime;
		ENTITY_ID entityID;
		DBID entityDBID;
	};

	typedef std::tr1::unordered_map<std::string, PLInfos*> PTINFO_MAP;
public:
	PendingLoginMgr(Mercury::NetworkInterface & networkInterface);
	~PendingLoginMgr();

	Mercury:: EventDispatcher & dispatcher();

	bool add(PLInfos* infos);
	
	bool process();
	
	PendingLoginMgr::PLInfos* remove(std::string& accountName);
	PendingLoginMgr::PLInfos* find(std::string& accountName);
private:
	Mercury::NetworkInterface & networkInterface_;

	bool start_;
	
	PTINFO_MAP pPLMap_;

};

}

#endif // __PENDING_LOGIN_MGR__
