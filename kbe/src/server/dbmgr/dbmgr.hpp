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

#ifndef __DBMGR_H__
#define __DBMGR_H__
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "server/globaldata_client.hpp"
#include "server/globaldata_server.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
#include "server/idallocate.hpp"

//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Dbmgr :	public ServerApp, 
					public TimerHandler, 
					public Singleton<Dbmgr>
{
public:
	enum TimeOutType
	{
		TIMEOUT_TICK,
		TIMEOUT_CHECK_STATUS
	};
	
	Dbmgr(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Dbmgr();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleMainTick();
	void handleCheckStatusTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();

	/** 获取ID服务器指针 */
	IDServer<ENTITY_ID>& idServer(void){ return idServer_; }

	/** 网络接口
		请求分配一个ENTITY_ID段
	*/
	void onReqAllocEntityID(Mercury::Channel* pChannel, int8 componentType, COMPONENT_ID componentID);

	/* 网络接口
		注册一个新激活的baseapp或者cellapp或者dbmgr
		通常是一个新的app被启动了， 它需要向某些组件注册自己。
	*/
	virtual void onRegisterNewApp(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, 
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport);


	/** 网络接口
		dbmgr广播global数据的改变
	*/
	void onGlobalDataClientLogon(Mercury::Channel* pChannel, COMPONENT_TYPE componentType);
	void onBroadcastGlobalDataChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete);
	void onBroadcastGlobalBasesChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete);
	void onBroadcastCellAppDataChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete);
protected:
	TimerHandle				loopCheckTimerHandle_;
	TimerHandle				mainProcessTimer_;

	IDServer<ENTITY_ID>		idServer_;									// entityID分配服务端

	GlobalDataServer*					pGlobalData_;								// globalData
	GlobalDataServer*					pGlobalBases_;								// globalBases
	GlobalDataServer*					pCellAppData_;								// cellAppData
};

}
#endif
