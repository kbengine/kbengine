// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CELLAPP_H
#define KBE_CELLAPP_H

#include "entity.h"
#include "spacememorys.h"
#include "cells.h"
#include "space_viewer.h"
#include "updatables.h"
#include "ghost_manager.h"
#include "witnessed_timeout_handler.h"
#include "server/entity_app.h"
#include "server/forward_messagebuffer.h"
	
namespace KBEngine{

class TelnetServer;
class InitProgressHandler;

class Cellapp:	public EntityApp<Entity>, 
				public Singleton<Cellapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_LOADING_TICK = TIMEOUT_ENTITYAPP_MAX + 1
	};
	
	Cellapp(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
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

	virtual ShutdownHandler::CAN_SHUTDOWN_STATE canShutdown();
	virtual void onShutdown(bool first);

	void destroyObjPool();

	float _getLoad() const { return getLoad(); }
	virtual void onUpdateLoad();

	/**  网络接口
		dbmgr告知已经启动的其他baseapp或者cellapp的地址
		当前app需要主动的去与他们建立连接
	*/
	virtual void onGetEntityAppFromDbmgr(Network::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);

	/**
		创建了一个entity回调
	*/
	virtual Entity* onCreateEntity(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid);

	/**  
		创建一个entity 
	*/
	static PyObject* __py_createEntity(PyObject* self, PyObject* args);

	/** 
		向dbmgr请求执行一个数据库命令
	*/
	static PyObject* __py_executeRawDatabaseCommand(PyObject* self, PyObject* args);
	void executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid, const std::string& dbInterfaceName);
	void onExecuteRawDatabaseCommandCB(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		dbmgr发送初始信息
		startID: 初始分配ENTITY_ID 段起始位置
		endID: 初始分配ENTITY_ID 段结束位置
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
		machineGroupOrder: 在machine中真实的组顺序, 提供底层在某些时候判断是否为第一个cellapp时使用
	*/
	void onDbmgrInitCompleted(Network::Channel* pChannel, GAME_TIME gametime, 
		ENTITY_ID startID, ENTITY_ID endID, COMPONENT_ORDER startGlobalOrder, COMPONENT_ORDER startGroupOrder, 
		const std::string& digest);

	/** 网络接口
		dbmgr广播global数据的改变
	*/
	void onBroadcastCellAppDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		baseEntity请求创建在一个新的space中
	*/
	void onCreateCellEntityInNewSpaceFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		baseEntity请求创建在一个新的space中
	*/
	void onRestoreSpaceInCellFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** 网络接口
	工具请求改变space查看器（含添加和删除功能）
	如果是请求更新并且服务器上不存在该地址的查看器则自动创建，如果是删除则明确给出删除要求
	*/
	void setSpaceViewer(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		其他APP请求在此灾难恢复
	*/
	void requestRestore(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		baseapp请求在这个cellapp上创建一个entity
	*/
	void onCreateCellEntityFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	void _onCreateCellEntityFromBaseapp(std::string& entityType, ENTITY_ID createToEntityID, ENTITY_ID entityID, 
		MemoryStream* pCellData, bool hasClient, bool inRescore, COMPONENT_ID componentID, SPACE_ID spaceID);

	/** 网络接口
		销毁某个cellEntity
	*/
	void onDestroyCellEntityFromBaseapp(Network::Channel* pChannel, ENTITY_ID eid);

	/** 网络接口
		entity收到远程call请求, 由某个app上的entitycall发起
	*/
	void onEntityCall(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** 网络接口
		client访问entity的cell方法由baseapp转发
	*/
	void onRemoteCallMethodFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		client更新数据
	*/
	void onUpdateDataFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	void onUpdateDataFromClientForControlledEntity(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		real请求更新属性到ghost
	*/
	void onUpdateGhostPropertys(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** 网络接口
		ghost请求调用def方法real
	*/
	void onRemoteRealMethodCall(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		real请求更新属性到ghost
	*/
	void onUpdateGhostVolatileData(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		base请求获取celldata
	*/
	void reqBackupEntityCellData(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		base请求获取WriteToDB
	*/
	void reqWriteToDBFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		客户端直接发送消息给cell实体
	*/
	void forwardEntityMessageToCellappFromClient(Network::Channel* pChannel, MemoryStream& s);

	/**
		获取游戏时间
	*/
	static PyObject* __py_gametime(PyObject* self, PyObject* args);

	/**
		添加与删除一个Updatable对象
	*/
	bool addUpdatable(Updatable* pObject);
	bool removeUpdatable(Updatable* pObject);

	/**
		hook entitycallcall
	*/
	RemoteEntityMethod* createEntityCallCallEntityRemoteMethod(MethodDescription* pMethodDescription, EntityCallAbstract* pEntityCall);

	/** 网络接口
		某个app请求查看该app
	*/
	virtual void lookApp(Network::Channel* pChannel);

	/**
		重新导入所有的脚本
	*/
	static PyObject* __py_reloadScript(PyObject* self, PyObject* args);
	virtual void reloadScript(bool fullReload);
	virtual void onReloadScript(bool fullReload);

	/**
		获取进程是否正在关闭中
	*/
	static PyObject* __py_isShuttingDown(PyObject* self, PyObject* args);

	/**
		获取进程内部网络地址
	*/
	static PyObject* __py_address(PyObject* self, PyObject* args);

	WitnessedTimeoutHandler	* pWitnessedTimeoutHandler(){ return pWitnessedTimeoutHandler_; }

	/**
		网络接口
		另一个cellapp的entity要teleport到本cellapp上的space中
	*/
	void reqTeleportToCellApp(Network::Channel* pChannel, MemoryStream& s);
	void reqTeleportToCellAppCB(Network::Channel* pChannel, MemoryStream& s);
	void reqTeleportToCellAppOver(Network::Channel* pChannel, MemoryStream& s);

	/**
		获取和设置ghost管理器
	*/
	void pGhostManager(GhostManager* v){ pGhostManager_ = v; }
	GhostManager* pGhostManager() const{ return pGhostManager_; }

	ArraySize spaceSize() const { return (ArraySize)SpaceMemorys::size(); }

	/** 
		射线 
	*/
	int raycast(SPACE_ID spaceID, int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPos);
	static PyObject* __py_raycast(PyObject* self, PyObject* args);

	uint32 flags() const { return flags_; }
	void flags(uint32 v) { flags_ = v; }
	static PyObject* __py_setFlags(PyObject* self, PyObject* args);
	static PyObject* __py_getFlags(PyObject* self, PyObject* args);

protected:
	// cellAppData
	GlobalDataClient*					pCellAppData_;

	ForwardComponent_MessageBuffer		forward_messagebuffer_;

	Updatables							updatables_;

	// 所有的cell
	Cells								cells_;

	TelnetServer*						pTelnetServer_;

	WitnessedTimeoutHandler	*			pWitnessedTimeoutHandler_;

	GhostManager*						pGhostManager_;
	
	// APP的标志
	uint32								flags_;

	// 通过工具查看space
	SpaceViewers						spaceViewers_;

	InitProgressHandler*				pInitProgressHandler_;
};

}

#endif // KBE_CELLAPP_H
