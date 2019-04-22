// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef CLIENT_OBJECT_BASE_H
#define CLIENT_OBJECT_BASE_H

#include "event.h"
#include "script_callbacks.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "helper/script_loglevel.h"
#include "pyscript/scriptobject.h"
#include "entitydef/entities.h"
#include "entitydef/common.h"
#include "server/callbackmgr.h"
#include "server/server_errors.h"
#include "math/math.h"

namespace KBEngine{

namespace client{
class Entity;
}

class EntityCall;
class EntityCallAbstract;

namespace Network
{
class Channel;
}

class ClientObjectBase : public script::ScriptObject
{
	/** 
		子类化 将一些py操作填充进派生类 
	*/
	INSTANCE_SCRIPT_HREADER(ClientObjectBase, ScriptObject)	
public:
	ClientObjectBase(Network::NetworkInterface& ninterface, PyTypeObject* pyType = NULL);
	virtual ~ClientObjectBase();

	Network::Channel* pServerChannel() const{ return pServerChannel_; }
	void pServerChannel(Network::Channel* pChannel){ pServerChannel_ = pChannel; }

	virtual void finalise(void);
	virtual void reset(void);
	virtual void canReset(bool v){ canReset_ = v; }

	Entities<client::Entity>* pEntities() const{ return pEntities_; }

	/**
		创建一个entity 
	*/
	client::Entity* createEntity(const char* entityType, PyObject* params,
		bool isInitializeScript = true, ENTITY_ID eid = 0, bool initProperty = true, 
		EntityCall* base = NULL, EntityCall* cell = NULL);

	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }	

	/**
		通过entityID销毁一个entity 
	*/
	virtual bool destroyEntity(ENTITY_ID entityID, bool callScript);

	void tickSend();
	
	virtual Network::Channel* initLoginappChannel(std::string accountName, 
		std::string passwd, std::string ip, KBEngine::uint32 port);
	virtual Network::Channel* initBaseappChannel();

	bool createAccount();
	bool login();
	virtual void onLogin(Network::Bundle* pBundle);

	bool loginBaseapp();
	bool reloginBaseapp();

	int32 appID() const{ return appID_; }
	const char* name(){ return name_.c_str(); }

	ENTITY_ID entityID(){ return entityID_; }
	DBID dbid(){ return dbid_; }

	bool registerEventHandle(EventHandle* pEventHandle);
	bool deregisterEventHandle(EventHandle* pEventHandle);
	
	void fireEvent(const EventData* pEventData);
	
	EventHandler& eventHandler(){ return eventHandler_; }

	static PyObject* __pyget_pyGetEntities(PyObject *self, void *closure)
	{
		ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);
		Py_INCREF(pClientObjectBase->pEntities());
		return pClientObjectBase->pEntities(); 
	}

	static PyObject* __pyget_pyGetID(PyObject *self, void *closure){
		
		ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);
		return PyLong_FromLong(pClientObjectBase->appID());	
	}

	static PyObject* __py_getPlayer(PyObject *self, void *args);
	
	static PyObject* __py_callback(PyObject* self, PyObject* args);
	static PyObject* __py_cancelCallback(PyObject* self, PyObject* args);

	static PyObject* __py_getWatcher(PyObject* self, PyObject* args);
	static PyObject* __py_getWatcherDir(PyObject* self, PyObject* args);

	static PyObject* __py_disconnect(PyObject* self, PyObject* args);

	/**
		如果entitiessize小于256
		通过索引位置来获取entityID
		否则直接取ID
	*/
	ENTITY_ID readEntityIDFromStream(MemoryStream& s);

	/**
		由entityCall来尝试获取一个channel的实例
	*/
	virtual Network::Channel* findChannelByEntityCall(EntityCallAbstract& entityCall);

	/**
		通过entity的ID尝试寻找它的实例
	*/
	virtual PyObject* tryGetEntity(COMPONENT_ID componentID, ENTITY_ID entityID);

	/** 网络接口
		客户端与服务端第一次建立交互, 服务端返回
	*/
	virtual void onHelloCB_(Network::Channel* pChannel, const std::string& verInfo,
		const std::string& scriptVerInfo, const std::string& protocolMD5, 
		const std::string& entityDefMD5, COMPONENT_TYPE componentType);

	virtual void onHelloCB(Network::Channel* pChannel, MemoryStream& s);

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
	virtual void onReloginBaseappFailed(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode);

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
	void onRemoteMethodCall_(ENTITY_ID eid, MemoryStream& s);

	/** 网络接口
	   被踢出服务器
	*/
	virtual void onKicked(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
		服务器更新entity属性
	*/
	virtual void onUpdatePropertys(Network::Channel* pChannel, MemoryStream& s);
	virtual void onUpdatePropertysOptimized(Network::Channel* pChannel, MemoryStream& s);
	void onUpdatePropertys_(ENTITY_ID eid, MemoryStream& s);

	/** 网络接口
		服务器强制设置entity的位置与朝向
	*/
	virtual void onSetEntityPosAndDir(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新avatar基础位置和朝向
	*/
	virtual void onUpdateBasePos(Network::Channel* pChannel, float x, float y, float z);
	virtual void onUpdateBasePosXZ(Network::Channel* pChannel, float x, float z);
	virtual void onUpdateBaseDir(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新VolatileData
	*/
	virtual void onUpdateData(Network::Channel* pChannel, MemoryStream& s);

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
	
	void _updateVolatileData(ENTITY_ID entityID, float x, float y, float z, float roll, 
		float pitch, float yaw, int8 isOnGround, bool isOptimized);

	/** 
		更新玩家到服务端 
	*/
	virtual void updatePlayerToServer();

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
		服务器告诉客户端：你当前（取消）控制谁的位移同步
	*/
	virtual void onControlEntity(Network::Channel* pChannel, int32 eid, int8 p_isControlled);

	/** 网络接口
		接收到ClientMessages(通常是web等才会应用到)
	*/
	virtual void onImportClientMessages(Network::Channel* pChannel, MemoryStream& s){}

	/** 网络接口
		接收到entitydef(通常是web等才会应用到)
	*/
	virtual void onImportClientEntityDef(Network::Channel* pChannel, MemoryStream& s){}
	
	/** 网络接口
		错误码描述导出(通常是web等才会应用到)
	*/
	virtual void onImportServerErrorsDescr(Network::Channel* pChannel, MemoryStream& s){}

	/** 网络接口
	接收导入sdk消息(通常是开发期使用，更新客户端sdk用)
	*/
	virtual void onImportClientSDK(Network::Channel* pChannel, MemoryStream& s) {}

	/** 网络接口
		重置账号密码请求返回
	*/
	virtual void onReqAccountResetPasswordCB(Network::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** 网络接口
		请求绑定邮箱返回
	*/
	virtual void onReqAccountBindEmailCB(Network::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** 网络接口
		请求修改密码返回
	*/
	virtual void onReqAccountNewPasswordCB(Network::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** 
		获得player实例
	*/
	client::Entity* pPlayer();

	void setTargetID(ENTITY_ID id){ 
		targetID_ = id; 
		onTargetChanged();
	}
	ENTITY_ID getTargetID() const{ return targetID_; }
	virtual void onTargetChanged(){}

	ENTITY_ID getViewEntityID(ENTITY_ID id);
	ENTITY_ID getViewEntityIDFromStream(MemoryStream& s);
	ENTITY_ID getViewEntityIDByAliasID(uint8 id);

	/** 
		space相关操作接口
		服务端添加了某个space的几何映射
	*/
	virtual void addSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath);
	virtual void onAddSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath){}
	virtual void onLoadedSpaceGeometryMapping(SPACE_ID spaceID){
		isLoadedGeometry_ = true;
	}

	const std::string& getGeometryPath();
	
	virtual void initSpaceData(Network::Channel* pChannel, MemoryStream& s);
	virtual void setSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value);
	virtual void delSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key);
	bool hasSpaceData(const std::string& key);
	const std::string& getSpaceData(const std::string& key);
	static PyObject* __py_GetSpaceData(PyObject* self, PyObject* args);
	void clearSpace(bool isAll);

	Timers & timers() { return timers_; }
	void handleTimers();

	ScriptCallbacks & scriptCallbacks() { return scriptCallbacks_; }

	void locktime(uint64 t){ locktime_ = t; }
	uint64 locktime() const{ return locktime_; }

	virtual void onServerClosed();

	uint64 rndUUID() const{ return rndUUID_; }

	Network::NetworkInterface* pNetworkInterface()const { return &networkInterface_; }

	/** 网络接口
		服务器心跳返回
	*/
	void onAppActiveTickCB(Network::Channel* pChannel);

	/**
		允许脚本assert底层
	*/
	static PyObject* __py_assert(PyObject* self, PyObject* args);

protected:				
	int32													appID_;

	// 服务端网络通道
	Network::Channel*										pServerChannel_;

	// 存储所有的entity的容器
	Entities<client::Entity>*								pEntities_;	
	std::vector<ENTITY_ID>									pEntityIDAliasIDList_;

	PY_CALLBACKMGR											pyCallbackMgr_;

	ENTITY_ID												entityID_;
	SPACE_ID												spaceID_;

	DBID													dbid_;

	std::string												ip_;
	uint16													tcp_port_;
	uint16													udp_port_;

	std::string												baseappIP_;
	uint16													baseappPort_;

	uint64													lastSentActiveTickTime_;
	uint64													lastSentUpdateDataTime_;

	bool													connectedBaseapp_;
	bool													canReset_;

	std::string												name_;
	std::string												password_;

	std::string												clientDatas_;
	std::string												serverDatas_;

	CLIENT_CTYPE											typeClient_;

	typedef std::map<ENTITY_ID, KBEShared_ptr<MemoryStream> > BUFFEREDMESSAGE;
	BUFFEREDMESSAGE											bufferedCreateEntityMessage_;

	EventHandler											eventHandler_;

	Network::NetworkInterface&								networkInterface_;

	// 当前客户端所选择的目标
	ENTITY_ID												targetID_;

	// 是否加载过地形数据
	bool													isLoadedGeometry_;

	SPACE_DATA												spacedatas_;

	Timers													timers_;
	ScriptCallbacks											scriptCallbacks_;

	uint64													locktime_;
	
	// 用于重登陆网关时的key
	uint64													rndUUID_; 

    // 受本客户端控制的entity列表
    std::list<client::Entity *>                             controlledEntities_;
};



}
#endif
