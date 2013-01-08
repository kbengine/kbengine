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

#ifndef __CELLAPP_H__
#define __CELLAPP_H__
	
// common include	
#include "entity.hpp"
#include "spaces.hpp"
#include "updatables.hpp"
#include "server/entity_app.hpp"
#include "server/forward_messagebuffer.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Cellapp:	public EntityApp<Entity>, 
				public Singleton<Cellapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_LOADING_TICK = TIMEOUT_ENTITYAPP_MAX + 1
	};
	
	Cellapp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Cellapp();

	virtual bool installPyModules();
	virtual void onInstallPyModules();
	virtual bool uninstallPyModules();
	
	bool run();
	
	virtual bool initializeWatcher();

	/**  
		相关处理接口 
	*/
	virtual void handleTimeout(TimerHandle handle, void * arg);
	virtual void handleGameTick();

	/**  
		初始化相关接口 
	*/
	bool initializeBegin();
	bool initializeEnd();
	void finalise();

	/**  网络接口
		dbmgr告知已经启动的其他baseapp或者cellapp的地址
		当前app需要主动的去与他们建立连接
	*/
	virtual void onGetEntityAppFromDbmgr(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, 
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport);

	/**  
		创建一个entity 
	*/
	static PyObject* __py_createEntity(PyObject* self, PyObject* args);

	/** 
		想dbmgr请求执行一个数据库命令
	*/
	static PyObject* __py_executeRawDatabaseCommand(PyObject* self, PyObject* args);
	void executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback);
	void onExecuteRawDatabaseCommandCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		dbmgr发送初始信息
		startID: 初始分配ENTITY_ID 段起始位置
		endID: 初始分配ENTITY_ID 段结束位置
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, GAME_TIME gametime, 
		ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder);

	/** 网络接口
		dbmgr广播global数据的改变
	*/
	void onBroadcastCellAppDataChange(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		baseEntity请求创建在一个新的space中
	*/
	void onCreateInNewSpaceFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		baseapp请求在这个cellapp上创建一个entity
	*/
	void onCreateCellEntityFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	void _onCreateCellEntityFromBaseapp(std::string& entityType, ENTITY_ID createToEntityID, ENTITY_ID entityID, 
		MemoryStream* pCellData, bool hasClient, COMPONENT_ID componentID, SPACE_ID spaceID);

	/** 网络接口
		销毁某个cellEntity
	*/
	void onDestroyCellEntityFromBaseapp(Mercury::Channel* pChannel, ENTITY_ID eid);

	/** 网络接口
		entity收到一封mail, 由某个app上的mailbox发起
	*/
	void onEntityMail(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** 网络接口
		client访问entity的cell方法由baseapp转发
	*/
	void onRemoteCallMethodFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		base请求获取celldata
	*/
	void reqBackupEntityCellData(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		base请求获取WriteToDB
	*/
	void reqWriteToDBFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		客户端直接发送消息给cell实体
	*/
	void forwardEntityMessageToCellappFromClient(Mercury::Channel* pChannel, MemoryStream& s);

	/**
		获取游戏时间
	*/
	static PyObject* __py_gametime(PyObject* self, PyObject* args);

	void updateLoad();
protected:
	GlobalDataClient*					pCellAppData_;									// cellAppData
	ForwardComponent_MessageBuffer		forward_messagebuffer_;

	Updatables updatables_;
};

}
#endif
