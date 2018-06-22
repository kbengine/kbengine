// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_CELLAPPMGR_H
#define KBE_CELLAPPMGR_H
	
#include "cellapp.h"
#include "space_viewer.h"
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
	void reqCreateCellEntityInNewSpace(Network::Channel* pChannel, MemoryStream& s);

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
	void onCellappInitProgress(Network::Channel* pChannel, COMPONENT_ID cid, float progress, 
		COMPONENT_ORDER componentGlobalOrder, COMPONENT_ORDER componentGroupOrder);

	bool componentsReady();
	bool componentReady(COMPONENT_ID cid);

	void removeCellapp(COMPONENT_ID cid);
	Cellapp& getCellapp(COMPONENT_ID cid);
	std::map< COMPONENT_ID, Cellapp >& cellapps();

	uint32 numLoadBalancingApp();

	/* 以groupOrderID为排序基准，
	   増加一个cellapp component id到cellapp_cids_列表中
	*/
	void addCellappComponentID(COMPONENT_ID cid);

	/** 网络接口
	查询所有相关进程负载信息
	*/
	void queryAppsLoads(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
	查询所有相关进程space信息
	*/
	void querySpaces(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
	更新相关进程space信息，注意：此spaceData并非API文档中描述的spaceData
	是指space的一些信息
	*/
	void updateSpaceData(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
	工具请求改变space查看器（含添加和删除功能）
	如果是请求更新并且服务器上不存在该地址的查看器则自动创建，如果是删除则明确给出删除要求
	*/
	void setSpaceViewer(Network::Channel* pChannel, MemoryStream& s);

protected:
	TimerHandle							gameTimer_;
	ForwardAnywhere_MessageBuffer		forward_anywhere_cellapp_messagebuffer_;
	ForwardComponent_MessageBuffer		forward_cellapp_messagebuffer_;

	COMPONENT_ID						bestCellappID_;

	std::map< COMPONENT_ID, Cellapp >	cellapps_;
	std::vector<COMPONENT_ID>			cellapp_cids_;

	// 通过工具查看space
	SpaceViewers						spaceViewers_;
};

} 

#endif // KBE_CELLAPPMGR_H
