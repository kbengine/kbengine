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


#ifndef KBE_CELLAPPMGR_H
#define KBE_CELLAPPMGR_H
	
#include "cellapp.h"
#include "server/kbemain.h"
#include "server/serverapp.h"
#include "server/idallocate.h"
#include "server/serverconfig.h"
#include "server/forward_messagebuffer.h"
#include "common/timer.h"
#include "network/endpoint.h"

namespace KBEngine{


class Cellappmgr :	public ServerApp, 
					public Singleton<Cellappmgr>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1
	};
	
	Cellappmgr(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Cellappmgr();
	
	bool run();
	
	virtual void onChannelDeregister(Network::Channel * pChannel);

	void handleTimeout(TimerHandle handle, void * arg);
	void handleGameTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();

	/** 找出一个最空闲的cellapp */
	COMPONENT_ID findFreeCellapp(void);
	void updateBestCellapp();

	/** 网络接口
		baseEntity请求创建在一个新的space中
	*/
	void reqCreateInNewSpace(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		baseEntity请求创建在一个新的space中
	*/
	void reqRestoreSpaceInCell(Network::Channel* pChannel, MemoryStream& s);
	
	/** 网络接口
		消息转发， 由某个app想通过本app将消息转发给某个app。
	*/
	void forwardMessage(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		更新cellapp情况。
	*/
	void updateCellapp(Network::Channel* pChannel, COMPONENT_ID componentID, ENTITY_ID numEntities, float load, uint32 flags);

	/** 网络接口
		cellapp同步自己的初始化信息
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onCellappInitProgress(Network::Channel* pChannel, COMPONENT_ID cid, float progress);

	bool componentsReady();
	bool componentReady(COMPONENT_ID cid);

	void removeCellapp(COMPONENT_ID cid);
	Cellapp& getCellapp(COMPONENT_ID cid);
	std::map< COMPONENT_ID, Cellapp >& cellapps();

	uint32 numLoadBalancingApp();

protected:
	TimerHandle							gameTimer_;
	ForwardAnywhere_MessageBuffer		forward_anywhere_cellapp_messagebuffer_;
	ForwardComponent_MessageBuffer		forward_cellapp_messagebuffer_;

	COMPONENT_ID						bestCellappID_;

	std::map< COMPONENT_ID, Cellapp >	cellapps_;
	std::vector<COMPONENT_ID>			cellapp_cids_;
};

} 

#endif // KBE_CELLAPPMGR_H
