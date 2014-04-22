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


#ifndef __CLIENT_OBJECT_BASE_H__
#define __CLIENT_OBJECT_BASE_H__

#include "event.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"
#include "helper/script_loglevel.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/entities.hpp"
#include "entitydef/common.hpp"
#include "server/callbackmgr.hpp"
#include "server/server_errors.hpp"
#include "math/math.hpp"

namespace KBEngine{

namespace client{
class Entity;
}

class EntityMailbox;

namespace Mercury
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
	ClientObjectBase(Mercury::NetworkInterface& ninterface, PyTypeObject* pyType = NULL);
	virtual ~ClientObjectBase();

	Mercury::Channel* pServerChannel()const{ return pServerChannel_; }

	void finalise(void);
	virtual void reset(void);

	Entities<client::Entity>* pEntities()const{ return pEntities_; }

	/**
		创建一个entity 
	*/
	client::Entity* createEntityCommon(const char* entityType, PyObject* params,
		bool isInitializeScript = true, ENTITY_ID eid = 0, bool initProperty = true, 
		EntityMailbox* base = NULL, EntityMailbox* cell = NULL);

	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }	

	/**
		通过entityID销毁一个entity 
	*/
	virtual bool destroyEntity(ENTITY_ID entityID, bool callScript);

	void tickSend();
	
	virtual Mercury::Channel* initLoginappChannel(std::string accountName, std::string passwd, std::string ip, KBEngine::uint32 port);
	virtual Mercury::Channel* initBaseappChannel();

	bool createAccount();
	bool login();
	bool loginGateWay();

	int32 appID()const{ return appID_; }
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
	
	/**
		如果entitiessize小于256
		通过索引位置来获取entityID
		否则直接取ID
	*/
	ENTITY_ID readEntityIDFromStream(MemoryStream& s);

	/**
		由mailbox来尝试获取一个channel的实例
	*/
	virtual Mercury::Channel* findChannelByMailbox(EntityMailbox& mailbox);

	/** 网络接口
		客户端与服务端第一次建立交互, 服务端返回
	*/
	virtual void onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo, 
		COMPONENT_TYPE componentType);

	virtual void onHelloCB(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		创建账号成功和失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ACCOUNT_CREATE:创建失败（已经存在）, 
									MERCURY_SUCCESS:账号创建成功

									SERVER_ERROR_CODE failedcode;
		@二进制附带数据:二进制额外数据: uint32长度 + bytearray
	*/
	virtual void onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s);

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
		服务器上的entity已经进入游戏世界了
	*/
	virtual void onEntityEnterWorld(Mercury::Channel * pChannel, ENTITY_ID eid, 
		ENTITY_SCRIPT_UID scriptType, SPACE_ID spaceID);

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
	   被踢出服务器
	*/
	virtual void onKicked(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
		服务器更新entity属性
	*/
	virtual void onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器强制设置entity的位置与朝向
	*/
	virtual void onSetEntityPosAndDir(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		服务器更新avatar基础位置
	*/
	virtual void onUpdateBasePos(Mercury::Channel* pChannel, MemoryStream& s);

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
	
	void _updateVolatileData(ENTITY_ID entityID, float x, float y, float z, float roll, float pitch, float yaw);

	/** 
		更新玩家到服务端 
	*/
	virtual void updatePlayerToServer();

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
		接收到ClientMessages(通常是web等才会应用到)
	*/
	virtual void onImportClientMessages(Mercury::Channel* pChannel, MemoryStream& s){}

	/** 网络接口
		接收到entitydef(通常是web等才会应用到)
	*/
	virtual void onImportClientEntityDef(Mercury::Channel* pChannel, MemoryStream& s){}
	
	/** 网络接口
		错误码描述导出(通常是web等才会应用到)
	*/
	virtual void onImportMercuryErrorsDescr(Mercury::Channel* pChannel, MemoryStream& s){}

	/** 网络接口
		重置账号密码请求返回
	*/
	virtual void onReqAccountResetPasswordCB(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** 网络接口
		请求绑定邮箱返回
	*/
	virtual void onReqAccountBindEmailCB(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** 网络接口
		请求修改密码返回
	*/
	virtual void onReqAccountNewPasswordCB(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** 
		获得player实例
	*/
	client::Entity* pPlayer();
	void setPlayerPosition(float x, float y, float z){ entityPos_ = Position3D(x, y, z); }
	void setPlayerDirection(float roll, float pitch, float yaw){ entityDir_ = Direction3D(roll, pitch, yaw); }

	void setTargetID(ENTITY_ID id){ 
		targetID_ = id; 
		onTargetChanged();
	}
	ENTITY_ID getTargetID()const{ return targetID_; }
	virtual void onTargetChanged(){}

	/** 
		space相关操作接口
		服务端添加了某个space的几何映射
	*/
	void addSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath);
	virtual void onAddSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath){}
	virtual void onLoadedSpaceGeometryMapping(SPACE_ID spaceID){
		isLoadedGeometry_ = true;
	}

	const std::string& getGeometryPath();
	
	void initSpaceData(Mercury::Channel* pChannel, MemoryStream& s);
	void setSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value);
	void delSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key);
	bool hasSpaceData(const std::string& key);
	const std::string& getSpaceData(const std::string& key);
	static PyObject* __py_GetSpaceData(PyObject* self, PyObject* args);
protected:				
	int32													appID_;

	// 服务端网络通道
	Mercury::Channel*										pServerChannel_;

	// 存储所有的entity的容器
	Entities<client::Entity>*								pEntities_;	
	std::vector<ENTITY_ID>									pEntityIDAliasIDList_;

	PY_CALLBACKMGR											pyCallbackMgr_;

	ENTITY_ID												entityID_;
	SPACE_ID												spaceID_;

	Position3D												entityPos_;
	Direction3D												entityDir_;

	DBID													dbid_;

	std::string												ip_;
	uint16													port_;

	uint64													lastSentActiveTickTime_;
	uint64													lastSentUpdateDataTime_;

	bool													connectedGateway_;
	bool													canReset_;

	std::string												name_;
	std::string												password_;
	std::string												extradatas_;

	CLIENT_CTYPE											typeClient_;

	typedef std::map<ENTITY_ID, KBEShared_ptr<MemoryStream> > BUFFEREDMESSAGE;
	BUFFEREDMESSAGE											bufferedCreateEntityMessage_;

	EventHandler											eventHandler_;

	Mercury::NetworkInterface&								ninterface_;

	// 当前客户端所选择的目标
	ENTITY_ID												targetID_;

	// 是否加载过地形数据
	bool													isLoadedGeometry_;

	SPACE_DATA												spacedatas_;
};

}
#endif
