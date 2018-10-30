// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
