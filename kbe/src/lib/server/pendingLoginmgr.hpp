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

#ifndef KBE_PENDING_LOGIN_MGR_HPP
#define KBE_PENDING_LOGIN_MGR_HPP

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
	��¼�����������ɹ��� ����û�н��뵽��Ϸ����ʱ�� ��Ҫ���˺Ż���һ�±��ں�������
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
		}

		Mercury::Address addr;
		COMPONENT_CLIENT_TYPE ctype;
		std::string accountName;
		std::string password;
		std::string datas;
		TimeStamp lastProcessTime;
		ENTITY_ID entityID;
		DBID entityDBID;
		uint32 flags;
		uint64 deadline;
	};

	typedef KBEUnordered_map<std::string, PLInfos*> PTINFO_MAP;
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

#endif // KBE_PENDING_LOGIN_MGR_HPP
