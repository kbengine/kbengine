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


#ifndef KBE_BOTS_HPP
#define KBE_BOTS_HPP
	
// common include	
#include "profile.hpp"
#include "create_and_login_handler.hpp"
#include "cstdkbe/timer.hpp"
#include "pyscript/script.hpp"
#include "network/endpoint.hpp"
#include "helper/debug_helper.hpp"
#include "helper/script_loglevel.hpp"
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

class ClientObject;
class PyBots;
class TelnetServer;
typedef SmartPointer<ClientObject> ClientObjectPtr;

class Bots  : public ClientApp
{
public:
	Bots(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Bots();

	virtual bool initialize();
	virtual void finalise();

	virtual bool initializeBegin();
	virtual bool initializeEnd();

	virtual bool installPyModules();
	virtual void onInstallPyModules() {};
	virtual bool uninstallPyModules();
	bool uninstallPyScript();
	bool installEntityDef();

	virtual void handleTimeout(TimerHandle, void * pUser);
	virtual void handleGameTick();

	static Bots& getSingleton(){ 
		return *static_cast<Bots*>(ClientApp::getSingletonPtr()); 
	}

	/**
		设置脚本输出类型前缀
	*/
	static PyObject* __py_setScriptLogType(PyObject* self, PyObject* args);

	bool run(void);

	/**
		由mailbox来尝试获取一个channel的实例
	*/
	virtual Mercury::Channel* findChannelByMailbox(EntityMailbox& mailbox);

	/** 网络接口
		某个app请求查看该app
	*/
	virtual void lookApp(Mercury::Channel* pChannel);

	/** 网络接口
		请求关闭服务器
	*/
	virtual void reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		请求关闭服务器
	*/
	void reqKillServer(Mercury::Channel* pChannel, MemoryStream& s);

	void onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	typedef std::map< Mercury::Channel*, ClientObjectPtr > CLIENTS;
	CLIENTS& clients(){ return clients_; }

	uint32 reqCreateAndLoginTotalCount(){ return reqCreateAndLoginTotalCount_; }
	void reqCreateAndLoginTotalCount(uint32 v){ reqCreateAndLoginTotalCount_ = v; }

	uint32 reqCreateAndLoginTickCount(){ return reqCreateAndLoginTickCount_; }
	void reqCreateAndLoginTickCount(uint32 v){ reqCreateAndLoginTickCount_ = v; }

	float reqCreateAndLoginTickTime(){ return reqCreateAndLoginTickTime_; }
	void reqCreateAndLoginTickTime(float v){ reqCreateAndLoginTickTime_ = v; }

	bool addClient(ClientObject* pClient);
	bool delClient(ClientObject* pClient);

	ClientObject* findClient(Mercury::Channel * pChannel);
	ClientObject* findClientByAppID(int32 appID);

	static PyObject* __py_addBots(PyObject* self, PyObject* args);

	/** 网络接口
	   添加bots
	   @total uint32: 总共添加的个数
	   @ticknum uint32: 每个tick添加多少个
	   @ticktime float: 一个tick的时间
	*/
	virtual void addBots(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
		某个app向本app告知处于活动状态。
	*/
	void onAppActiveTick(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID);

	virtual void onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo, 
		const std::string& scriptVerInfo, COMPONENT_TYPE componentType);

	/** 网络接口
		和服务端的版本不匹配
	*/
	virtual void onVersionNotMatch(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		和服务端的脚本层版本不匹配
	*/
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel, MemoryStream& s);

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
	   重登陆baseapp成功
	*/
	virtual void onReLoginGatewaySuccessfully(Mercury::Channel * pChannel);

	/** 网络接口
		服务器端已经创建了一个与客户端关联的代理Entity
	   在登录时也可表达成功回调
	   @datas: 账号entity的信息
	*/
	virtual void onCreatedProxies(Mercury::Channel * pChannel, uint64 rndUUID, 
		ENTITY_ID eid, std::string& entityType);

	/** 网络接口
		服务器上的entity已经进入游戏世界了
	*/
	virtual void onEntityEnterWorld(Mercury::Channel * pChannel, MemoryStream& s);


	/** 网络接口
		服务器上的entity已经离开游戏世界了
	*/
	virtual void onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid);
	virtual void onEntityLeaveWorldOptimized(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
		告诉客户端某个entity销毁了， 此类entity通常是还未onEntityEnterWorld
	*/
	virtual void onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经进入space了
	*/
	virtual void onEntityEnterSpace(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
		服务器上的entity已经离开space了
	*/
	virtual void onEntityLeaveSpace(Mercury::Channel * pChannel, ENTITY_ID eid);

	/** 网络接口
		远程调用entity的方法 
	*/
	virtual void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onRemoteMethodCallOptimized(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
	   被踢出服务器
	*/
	virtual void onKicked(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
		服务器更新entity属性
	*/
	virtual void onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdatePropertysOptimized(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新avatar基础位置
	*/
	virtual void onUpdateBasePos(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateBasePosXZ(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器强制设置entity的位置与朝向
	*/
	virtual void onSetEntityPosAndDir(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新VolatileData
	*/
	virtual void onUpdateData(Mercury::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_ypr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yp(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_pr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_y(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_p(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_r(Mercury::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xz(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_ypr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yp(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_pr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_y(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_p(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_r(Mercury::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xyz(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_ypr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yp(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_pr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_y(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_p(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_r(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		download stream开始了 
	*/
	virtual void onStreamDataStarted(Mercury::Channel* pChannel, int16 id, uint32 datasize, std::string& descr);

	/** 网络接口
		接收到streamData
	*/
	virtual void onStreamDataRecv(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		download stream完成了 
	*/
	virtual void onStreamDataCompleted(Mercury::Channel* pChannel, int16 id);

	/** 网络接口
		space相关操作接口
		服务端添加了某个space的几何映射
	*/
	void setSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value);
	void delSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key);

	/** 网络接口
		请求查看watcher
	*/
	void queryWatcher(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		console请求开始profile
	*/
	void startProfile(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	virtual void startProfile_(Mercury::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen);

protected:
	PyBots*													pPyBots_;

	CLIENTS													clients_;

	// console请求创建到服务端的bots数量
	uint32													reqCreateAndLoginTotalCount_;
	uint32													reqCreateAndLoginTickCount_;
	float													reqCreateAndLoginTickTime_;

	// 处理创建与登录的handler
	CreateAndLoginHandler*									pCreateAndLoginHandler_;

	Mercury::EventPoller*									pEventPoller_;

	TelnetServer*											pTelnetServer_;
};

}

#endif // KBE_BOTS_HPP
