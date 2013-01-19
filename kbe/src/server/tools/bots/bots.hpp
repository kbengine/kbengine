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


#ifndef __BOTS_H__
#define __BOTS_H__
	
// common include	
#include "profile.hpp"
#include "create_and_login_handler.hpp"
#include "cstdkbe/timer.hpp"
#include "pyscript/script.hpp"
#include "network/endpoint.hpp"
#include "helper/debug_helper.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_poller.hpp"
#include "client_lib/clientapp.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "entitydef/entitydef.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class ClientAppEx;
typedef SmartPointer<ClientAppEx> ClientAppPtr;

class Bots : public ClientApp
{
public:
	Bots(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Bots();

	virtual bool initialize();
	virtual void finalise();

	void registerScript(PyTypeObject*);
	int registerPyObjectToScript(const char* attrName, PyObject* pyObj);
	int unregisterPyObjectToScript(const char* attrName);

	bool installPyScript();
	virtual bool installPyModules();
	virtual void onInstallPyModules() {};
	virtual bool uninstallPyModules();
	bool uninstallPyScript();
	bool installEntityDef();

	virtual void handleTimeout(TimerHandle, void * pUser);
	void handleGameTick();

	static Bots& getSingleton(){ 
		return *static_cast<Bots*>(ClientApp::getSingletonPtr()); 
	}

	bool run(void);

	/**
		通过entityID销毁一个entity 
	*/
	virtual bool destroyEntity(ENTITY_ID entityID){ return true; }

	/** 网络接口
		某个app请求查看该app
	*/
	virtual void lookApp(Mercury::Channel* pChannel);

	/** 网络接口
		请求关闭服务器
	*/
	virtual void reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s);

	void onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	KBEngine::script::Script& getScript(){ return script_; }

	typedef std::map< Mercury::Channel*, ClientAppPtr > CLIENTS;
	CLIENTS& clients(){ return clients_; }

	uint32 reqCreateAndLoginTotalCount(){ return reqCreateAndLoginTotalCount_; }
	uint32 reqCreateAndLoginTickCount(){ return reqCreateAndLoginTickCount_; }
	float reqCreateAndLoginTickTime(){ return reqCreateAndLoginTickTime_; }

	bool addClient(ClientAppEx* pClient);
	bool delClient(ClientAppEx* pClient);

	ClientAppEx* findClient(Mercury::Channel * pChannel);

	/** 网络接口
		创建账号成功和失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ACCOUNT_CREATE:创建失败（已经存在）, 
									MERCURY_SUCCESS:账号创建成功

									SERVER_ERROR_CODE failedcode;
		@二进制附带数据:二进制额外数据: uint32长度 + bytearray
	*/
	virtual void onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s);

	Mercury::EventPoller* pEventPoller(){ return pEventPoller_; }

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_SRV_OVERLOAD:服务器负载过重, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录成功
	   @ip: 服务器ip地址
	   @port: 服务器端口
	*/
	virtual void onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ILLEGAL_LOGIN:非法登录, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
		服务器端已经创建了一个与客户端关联的代理Entity
	   在登录时也可表达成功回调
	   @datas: 账号entity的信息
	*/
	virtual void onCreatedProxies(Mercury::Channel * pChannel, uint64 rndUUID, 
		ENTITY_ID eid, std::string& entityType);

	/** 网络接口
		服务器端已经创建了一个Entity
	*/
	virtual void onCreatedEntity(Mercury::Channel * pChannel, ENTITY_ID eid, std::string& entityType);

	/** 网络接口
		服务器上的entity已经有了一个cell部分
	*/
	virtual void onEntityGetCell(Mercury::Channel * pChannel, ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经进入游戏世界了
	*/
	virtual void onEntityEnterWorld(Mercury::Channel * pChannel, ENTITY_ID eid, SPACE_ID spaceID);

	/** 网络接口
		服务器上的entity已经离开游戏世界了
	*/
	virtual void onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid, SPACE_ID spaceID);

	/** 网络接口
		告诉客户端某个entity销毁了， 此类entity通常是还未onEntityEnterWorld
	*/
	virtual void onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经进入space了
	*/
	virtual void onEntityEnterSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经离开space了
	*/
	virtual void onEntityLeaveSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid);

	/** 网络接口
		远程调用entity的方法 
	*/
	virtual void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新entity属性
	*/
	virtual void onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s);
protected:
	KBEngine::script::Script								script_;
	std::vector<PyTypeObject*>								scriptBaseTypes_;

	TimerHandle												gameTimer_;

	CLIENTS													clients_;

	// console请求创建到服务端的bots数量
	uint32													reqCreateAndLoginTotalCount_;
	uint32													reqCreateAndLoginTickCount_;
	float													reqCreateAndLoginTickTime_;

	// 处理创建与登录的handler
	CreateAndLoginHandler*									pCreateAndLoginHandler_;

	Mercury::EventPoller*									pEventPoller_;
};

}
#endif
