// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_BASEAPPMGR_H
#define KBE_BASEAPPMGR_H

#include "baseapp.h"
#include "server/kbemain.h"
#include "server/serverapp.h"
#include "server/idallocate.h"
#include "server/serverconfig.h"
#include "server/forward_messagebuffer.h"
#include "common/timer.h"
#include "network/endpoint.h"

namespace KBEngine{

class Baseappmgr :	public ServerApp, 
					public Singleton<Baseappmgr>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1
	};
	
	Baseappmgr(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Baseappmgr();
	
	bool run();
	
	virtual void onChannelDeregister(Network::Channel * pChannel);
	virtual void onAddComponent(const Components::ComponentInfos* pInfos);

	void handleTimeout(TimerHandle handle, void * arg);
	void handleGameTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();

	COMPONENT_ID findFreeBaseapp();
	void updateBestBaseapp();

	/** 网络接口
		baseapp::createEntityAnywhere查询当前最好的组件ID
	*/
	void reqCreateEntityAnywhereFromDBIDQueryBestBaseappID(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		收到baseapp::createEntityAnywhere请求在某个空闲的baseapp上创建一个baseEntity
		@param sp: 这个数据包中存储的是 entityType	: entity的类别， entities.xml中的定义的。
										strInitData	: 这个entity被创建后应该给他初始化的一些数据， 
													  需要使用pickle.loads解包.
										componentID	: 请求创建entity的baseapp的组件ID
	*/
	void reqCreateEntityAnywhere(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
	收到baseapp::createEntityRemotely请求在某个空闲的baseapp上创建一个baseEntity
	@param sp: 这个数据包中存储的是 entityType	: entity的类别， entities.xml中的定义的。
	strInitData	: 这个entity被创建后应该给他初始化的一些数据，
	需要使用pickle.loads解包.
	componentID	: 请求创建entity的baseapp的组件ID
	*/
	void reqCreateEntityRemotely(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		收到baseapp::createEntityAnywhereFromDBID请求在某个空闲的baseapp上创建一个baseEntity
	*/
	void reqCreateEntityAnywhereFromDBID(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		收到baseapp::createEntityRemotelyFromDBID请求在某个空闲的baseapp上创建一个baseEntity
	*/
	void reqCreateEntityRemotelyFromDBID(Network::Channel* pChannel, MemoryStream& s);
	
	/** 网络接口
		消息转发， 由某个app想通过本app将消息转发给某个app。
	*/
	void forwardMessage(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		一个新登录的账号获得合法登入baseapp的权利， 现在需要将账号注册给baseapp
		使其允许在此baseapp上登录。
	*/
	void registerPendingAccountToBaseapp(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		一个新登录的账号获得合法登入baseapp的权利， 现在需要将账号注册给指定的baseapp
		使其允许在此baseapp上登录。
	*/
	void registerPendingAccountToBaseappAddr(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		baseapp将自己的地址发送给loginapp并转发给客户端。
	*/
	void onPendingAccountGetBaseappAddr(Network::Channel* pChannel, 
								  std::string& loginName, std::string& accountName, 
								  std::string& addr, uint16 tcp_port, uint16 udp_port);

	/** 网络接口
		更新baseapp情况。
	*/
	void updateBaseapp(Network::Channel* pChannel, COMPONENT_ID componentID,
								ENTITY_ID numEntitys, ENTITY_ID numProxices, float load, uint32 flags);

	/** 网络接口
		baseapp同步自己的初始化信息
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onBaseappInitProgress(Network::Channel* pChannel, COMPONENT_ID cid, float progress);

	/** 
		将分配的baseapp地址发送给loginapp并转发给客户端。
	*/
	void sendAllocatedBaseappAddr(Network::Channel* pChannel, 
								  std::string& loginName, std::string& accountName, 
								  const std::string& addr, uint16 tcp_port, uint16 udp_port);

	bool componentsReady();
	bool componentReady(COMPONENT_ID cid);

	std::map< COMPONENT_ID, Baseapp >& baseapps();

	uint32 numLoadBalancingApp();

	/** 网络接口
		查询所有相关进程负载信息
	*/
	void queryAppsLoads(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		baseapp请求绑定email（返回时需要找到loginapp的地址）
	*/
	void reqAccountBindEmailAllocCallbackLoginapp(Network::Channel* pChannel, COMPONENT_ID reqBaseappID, ENTITY_ID entityID, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code);

	/** 网络接口
		请求绑定email, loginapp返回需要找到loginapp的地址
	*/
	void onReqAccountBindEmailCBFromLoginapp(Network::Channel* pChannel, COMPONENT_ID reqBaseappID, ENTITY_ID entityID, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code, std::string& loginappCBHost, uint16 loginappCBPort);

protected:
	TimerHandle													gameTimer_;

	ForwardAnywhere_MessageBuffer								forward_anywhere_baseapp_messagebuffer_;
	ForwardComponent_MessageBuffer								forward_baseapp_messagebuffer_;

	COMPONENT_ID												bestBaseappID_;

	std::map< COMPONENT_ID, Baseapp >							baseapps_;

	KBEUnordered_map< std::string, COMPONENT_ID >				pending_logins_;

	float														baseappsInitProgress_;
};

}

#endif // KBE_BASEAPPMGR_H
