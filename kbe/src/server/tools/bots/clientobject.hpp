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

#ifndef __CLIENT_OBJECT_H__
#define __CLIENT_OBJECT_H__

// common include	
// #define NDEBUG
#include "entity.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entitydef.hpp"
#include "entitydef/entities.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/address.hpp"
#include "network/endpoint.hpp"
#include "network/bundle.hpp"
#include "network/tcp_packet.hpp"
#include "network/tcp_packet_receiver.hpp"
#include "pyscript/script.hpp"
#include "pyscript/scriptobject.hpp"
#include "pyscript/pyobject_pointer.hpp"

namespace KBEngine{ 

/*
*/

class ClientObject : public script::ScriptObject, Mercury::TCPPacketReceiver
{
	/** 
		子类化 将一些py操作填充进派生类 
	*/
	INSTANCE_SCRIPT_HREADER(ClientObject, ScriptObject)	
public:
	enum C_ERROR
	{
		C_ERROR_NONE = 0,
		C_ERROR_INIT_NETWORK_FAILED = 1,
		C_ERROR_CREATE_FAILED = 2,
		C_ERROR_LOGIN_FAILED = 3,
		C_ERROR_LOGIN_GATEWAY_FAILED = 4,
	};

	enum C_STATE
	{
		C_STATE_INIT = 0,
		C_STATE_LOGIN = 1,
		C_STATE_LOGIN_GATEWAY = 2,
		C_STATE_PLAY = 3,
	};

	ClientObject(std::string name);
	virtual ~ClientObject();

	bool processSocket(bool expectingPacket);

	bool initCreate();
	bool createAccount();
	bool login();

	bool initLoginGateWay();
	bool loginGateWay();

	void gameTick();

	const char* name(){ return name_.c_str(); }

	Mercury::Channel* pChannel(){ return pChannel_; }

	DECLARE_PY_GET_MOTHOD(pyGetEntities);

	/**
		创建账号成功和失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ACCOUNT_CREATE:创建失败（已经存在）, 
									MERCURY_SUCCESS:账号创建成功

									SERVER_ERROR_CODE failedcode;
		@二进制附带数据:二进制额外数据: uint32长度 + bytearray
	*/
	void onCreateAccountResult(MemoryStream& s);

	ClientObject::C_ERROR lasterror(){ return error_; }

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_SRV_OVERLOAD:服务器负载过重, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginFailed(MemoryStream& s);

	/** 网络接口
	   登录成功
	   @ip: 服务器ip地址
	   @port: 服务器端口
	*/
	virtual void onLoginSuccessfully(MemoryStream& s);

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ILLEGAL_LOGIN:非法登录, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginGatewayFailed(SERVER_ERROR_CODE failedcode);

	/** 网络接口
		服务器端已经创建了一个与客户端关联的代理Entity
	   在登录时也可表达成功回调
	   @datas: 账号entity的信息
	*/
	virtual void onCreatedProxies(uint64 rndUUID, 
		ENTITY_ID eid, std::string& entityType);

	/** 网络接口
		服务器端已经创建了一个Entity
	*/
	virtual void onCreatedEntity(ENTITY_ID eid, std::string& entityType);

	/** 网络接口
		服务器上的entity已经有了一个cell部分
	*/
	virtual void onEntityGetCell(ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经进入游戏世界了
	*/
	virtual void onEntityEnterWorld(ENTITY_ID eid, SPACE_ID spaceID);

	/** 网络接口
		服务器上的entity已经离开游戏世界了
	*/
	virtual void onEntityLeaveWorld(ENTITY_ID eid, SPACE_ID spaceID);

	/** 网络接口
		告诉客户端某个entity销毁了， 此类entity通常是还未onEntityEnterWorld
	*/
	virtual void onEntityDestroyed(ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经进入space了
	*/
	virtual void onEntityEnterSpace(SPACE_ID spaceID, ENTITY_ID eid);

	/** 网络接口
		服务器上的entity已经离开space了
	*/
	virtual void onEntityLeaveSpace(SPACE_ID spaceID, ENTITY_ID eid);

	/** 网络接口
		远程调用entity的方法 
	*/
	virtual void onRemoteMethodCall(MemoryStream& s);

	/** 网络接口
		服务器更新entity属性
	*/
	virtual void onUpdatePropertys(MemoryStream& s);

	void sendTick();

	Entities<Entity>* pEntities()const{ return pEntities_; }

	/**
		创建一个entity 
	*/
	Entity* createEntityCommon(const char* entityType, PyObject* params,
		bool isInitializeScript = true, ENTITY_ID eid = 0, bool initProperty = true);

	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }	

	int32 appID()const{ return appID_; }
	static PyObject* __pyget_pyGetID(ClientObject *self, void *closure){
		return PyLong_FromLong(self->appID());	
	}

protected:
	int32 appID_;

	Mercury::Channel* pChannel_;

	std::string name_;
	std::string password_;

	PyObjectPtr	entryScript_;

	C_ERROR error_;
	C_STATE state_;

	ENTITY_ID entityID_;
	DBID dbid_;

	std::string ip_;
	uint16 port_;

	uint64 lastSentActiveTickTime_;

	bool connectedGateway_;

	// 存储所有的entity的容器
	Entities<Entity>* pEntities_;	

	PY_CALLBACKMGR	pyCallbackMgr_;
};


}
#endif
