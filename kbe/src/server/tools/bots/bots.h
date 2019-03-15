// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_BOTS_H
#define KBE_BOTS_H
	
// common include	
#include "profile.h"
#include "create_and_login_handler.h"
#include "common/timer.h"
#include "pyscript/script.h"
#include "network/endpoint.h"
#include "helper/debug_helper.h"
#include "helper/script_loglevel.h"
#include "xml/xml.h"	
#include "common/singleton.h"
#include "common/smartpointer.h"
#include "common/timer.h"
#include "network/interfaces.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"
#include "client_lib/clientapp.h"
#include "pyscript/pyobject_pointer.h"
#include "entitydef/entitydef.h"

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

class Bots  : public ClientApp
{
public:
	Bots(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
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
		由entityCall来尝试获取一个channel的实例
	*/
	virtual Network::Channel* findChannelByEntityCall(EntityCallAbstract& entityCall);

	/**
	通过entity的ID尝试寻找它的实例
	*/
	virtual PyObject* tryGetEntity(COMPONENT_ID componentID, ENTITY_ID entityID);

	/** 网络接口
		某个app请求查看该app
	*/
	virtual void lookApp(Network::Channel* pChannel);

	/** 网络接口
		请求关闭服务器
	*/
	virtual void reqCloseServer(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		请求关闭服务器
	*/
	void reqKillServer(Network::Channel* pChannel, MemoryStream& s);

	void onExecScriptCommand(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	typedef std::map< Network::Channel*, ClientObject* > CLIENTS;
	CLIENTS& clients(){ return clients_; }

	uint32 reqCreateAndLoginTotalCount(){ return reqCreateAndLoginTotalCount_; }
	void reqCreateAndLoginTotalCount(uint32 v){ reqCreateAndLoginTotalCount_ = v; }

	uint32 reqCreateAndLoginTickCount(){ return reqCreateAndLoginTickCount_; }
	void reqCreateAndLoginTickCount(uint32 v){ reqCreateAndLoginTickCount_ = v; }

	float reqCreateAndLoginTickTime(){ return reqCreateAndLoginTickTime_; }
	void reqCreateAndLoginTickTime(float v){ reqCreateAndLoginTickTime_ = v; }

	bool addClient(ClientObject* pClient);
	bool delClient(ClientObject* pClient);
	bool delClient(Network::Channel * pChannel);

	ClientObject* findClient(Network::Channel * pChannel);
	ClientObject* findClientByAppID(int32 appID);

	static PyObject* __py_addBots(PyObject* self, PyObject* args);

	/** 网络接口
	   添加bots
	   @total uint32: 总共添加的个数
	   @ticknum uint32: 每个tick添加多少个
	   @ticktime float: 一个tick的时间
	*/
	virtual void addBots(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
		某个app向本app告知处于活动状态。
	*/
	void onAppActiveTick(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID);

	virtual void onHelloCB_(Network::Channel* pChannel, const std::string& verInfo,
		const std::string& scriptVerInfo, const std::string& protocolMD5, 
		const std::string& entityDefMD5, COMPONENT_TYPE componentType);

	/** 网络接口
		和服务端的版本不匹配
	*/
	virtual void onVersionNotMatch(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		和服务端的脚本层版本不匹配
	*/
	virtual void onScriptVersionNotMatch(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		创建账号成功和失败回调
	   @failedcode: 失败返回码 NETWORK_ERR_SRV_NO_READY:服务器没有准备好, 
									NETWORK_ERR_ACCOUNT_CREATE:创建失败（已经存在）, 
									NETWORK_SUCCESS:账号创建成功

									SERVER_ERROR_CODE failedcode;
		@二进制附带数据:二进制额外数据: uint32长度 + bytearray
	*/
	virtual void onCreateAccountResult(Network::Channel * pChannel, MemoryStream& s);

	Network::EventPoller* pEventPoller(){ return pEventPoller_; }

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 NETWORK_ERR_SRV_NO_READY:服务器没有准备好, 
									NETWORK_ERR_SRV_OVERLOAD:服务器负载过重, 
									NETWORK_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginFailed(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录成功
	   @ip: 服务器ip地址
	   @port: 服务器端口
	*/
	virtual void onLoginSuccessfully(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 NETWORK_ERR_SRV_NO_READY:服务器没有准备好, 
									NETWORK_ERR_ILLEGAL_LOGIN:非法登录, 
									NETWORK_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginBaseappFailed(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
	   重登陆baseapp成功
	*/
	virtual void onReloginBaseappSuccessfully(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
		服务器端已经创建了一个与客户端关联的代理Entity
	   在登录时也可表达成功回调
	   @datas: 账号entity的信息
	*/
	virtual void onCreatedProxies(Network::Channel * pChannel, uint64 rndUUID, 
		ENTITY_ID eid, std::string& entityType);

	/** 网络接口
		服务器上的entity已经进入游戏世界了
	*/
	virtual void onEntityEnterWorld(Network::Channel * pChannel, MemoryStream& s);


	/** 网络接口
		服务器上的entity已经离开游戏世界了
	*/
	virtual void onEntityLeaveWorld(Network::Channel * pChannel, ENTITY_ID eid);
	virtual void onEntityLeaveWorldOptimized(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
		告诉客户端某个entity销毁了， 此类entity通常是还未onEntityEnterWorld
	*/
	virtual void onEntityDestroyed(Network::Channel * pChannel, ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经进入space了
	*/
	virtual void onEntityEnterSpace(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
		服务器上的entity已经离开space了
	*/
	virtual void onEntityLeaveSpace(Network::Channel * pChannel, ENTITY_ID eid);

	/** 网络接口
		远程调用entity的方法 
	*/
	virtual void onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s);
	virtual void onRemoteMethodCallOptimized(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
	   被踢出服务器
	*/
	virtual void onKicked(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
		服务器更新entity属性
	*/
	virtual void onUpdatePropertys(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdatePropertysOptimized(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新avatar基础位置和朝向
	*/
	virtual void onUpdateBasePos(Network::Channel* pChannel, float x, float y, float z);
	virtual void onUpdateBasePosXZ(Network::Channel* pChannel, float x, float z);
	virtual void onUpdateBaseDir(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器强制设置entity的位置与朝向
	*/
	virtual void onSetEntityPosAndDir(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新VolatileData
	*/
	virtual void onUpdateData(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		非优化高精度同步
	*/
	virtual void onUpdateData_ypr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yp(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_pr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_y(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_p(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_r(Network::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xz(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_ypr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yp(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_pr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_y(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_p(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_r(Network::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xyz(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_ypr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yp(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_pr(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_y(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_p(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_r(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		优化的位置同步
	*/
	virtual void onUpdateData_ypr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yp_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_pr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_y_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_p_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_r_optimized(Network::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xz_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_ypr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yp_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_pr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_y_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_p_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_r_optimized(Network::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xyz_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_ypr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yp_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_pr_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_y_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_p_optimized(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_r_optimized(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		download stream开始了 
	*/
	virtual void onStreamDataStarted(Network::Channel* pChannel, int16 id, uint32 datasize, std::string& descr);

	/** 网络接口
		接收到streamData
	*/
	virtual void onStreamDataRecv(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		download stream完成了 
	*/
	virtual void onStreamDataCompleted(Network::Channel* pChannel, int16 id);

	/** 网络接口
		space相关操作接口
		服务端添加了某个space的几何映射
	*/
	virtual void initSpaceData(Network::Channel* pChannel, MemoryStream& s);
	virtual void setSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value);
	virtual void delSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key);

	/** 网络接口
		请求查看watcher
	*/
	void queryWatcher(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		console请求开始profile
	*/
	void startProfile(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	virtual void startProfile_(Network::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen);

	/** 网络接口
	    服务器告诉客户端：你当前（取消）控制谁的位移同步
	*/
	virtual void onControlEntity(Network::Channel* pChannel, int32 entityID, int8 isControlled);

	/** 网络接口
		服务器心跳返回
	*/
	void onAppActiveTickCB(Network::Channel* pChannel);

protected:
	PyBots*													pPyBots_;

	CLIENTS													clients_;

	// console请求创建到服务端的bots数量
	uint32													reqCreateAndLoginTotalCount_;
	uint32													reqCreateAndLoginTickCount_;
	float													reqCreateAndLoginTickTime_;

	// 处理创建与登录的handler
	CreateAndLoginHandler*									pCreateAndLoginHandler_;

	Network::EventPoller*									pEventPoller_;

	TelnetServer*											pTelnetServer_;
};

}

#endif // KBE_BOTS_H
