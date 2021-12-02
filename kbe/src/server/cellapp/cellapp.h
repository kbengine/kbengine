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
		��ش���ӿ� 
	*/
	virtual void handleTimeout(TimerHandle handle, void * arg);
	virtual void handleGameTick();

	/**  
		��ʼ����ؽӿ� 
	*/
	bool initializeBegin();
	bool initializeEnd();
	void finalise();

	virtual ShutdownHandler::CAN_SHUTDOWN_STATE canShutdown();
	virtual void onShutdown(bool first);

	void destroyObjPool();

	float _getLoad() const { return getLoad(); }
	virtual void onUpdateLoad();

	/**  ����ӿ�
		dbmgr��֪�Ѿ�����������baseapp����cellapp�ĵ�ַ
		��ǰapp��Ҫ������ȥ�����ǽ�������
	*/
	virtual void onGetEntityAppFromDbmgr(Network::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);

	/**
		������һ��entity�ص�
	*/
	virtual Entity* onCreateEntity(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid);

	/**  
		����һ��entity 
	*/
	static PyObject* __py_createEntity(PyObject* self, PyObject* args);

	/** 
		��dbmgr����ִ��һ�����ݿ�����
	*/
	static PyObject* __py_executeRawDatabaseCommand(PyObject* self, PyObject* args);
	void executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid, const std::string& dbInterfaceName);
	void onExecuteRawDatabaseCommandCB(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		dbmgr���ͳ�ʼ��Ϣ
		startID: ��ʼ����ENTITY_ID ����ʼλ��
		endID: ��ʼ����ENTITY_ID �ν���λ��
		startGlobalOrder: ȫ������˳�� �������ֲ�ͬ���
		startGroupOrder: ��������˳�� ����������baseapp�еڼ���������
		machineGroupOrder: ��machine����ʵ����˳��, �ṩ�ײ���ĳЩʱ���ж��Ƿ�Ϊ��һ��cellappʱʹ��
	*/
	void onDbmgrInitCompleted(Network::Channel* pChannel, GAME_TIME gametime, 
		ENTITY_ID startID, ENTITY_ID endID, COMPONENT_ORDER startGlobalOrder, COMPONENT_ORDER startGroupOrder, 
		const std::string& digest);

	/** ����ӿ�
		dbmgr�㲥global���ݵĸı�
	*/
	void onBroadcastCellAppDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		baseEntity���󴴽���һ���µ�space��
	*/
	void onCreateCellEntityInNewSpaceFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		baseEntity���󴴽���һ���µ�space��
	*/
	void onRestoreSpaceInCellFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** ����ӿ�
	��������ı�space�鿴��������Ӻ�ɾ�����ܣ�
	�����������²��ҷ������ϲ����ڸõ�ַ�Ĳ鿴�����Զ������������ɾ������ȷ����ɾ��Ҫ��
	*/
	void setSpaceViewer(Network::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		����APP�����ڴ����ѻָ�
	*/
	void requestRestore(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		baseapp���������cellapp�ϴ���һ��entity
	*/
	void onCreateCellEntityFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	void _onCreateCellEntityFromBaseapp(std::string& entityType, ENTITY_ID createToEntityID, ENTITY_ID entityID, 
		MemoryStream* pCellData, bool hasClient, bool inRescore, COMPONENT_ID componentID, SPACE_ID spaceID);

	/** ����ӿ�
		����ĳ��cellEntity
	*/
	void onDestroyCellEntityFromBaseapp(Network::Channel* pChannel, ENTITY_ID eid);

	/** ����ӿ�
		entity�յ�Զ��call����, ��ĳ��app�ϵ�entitycall����
	*/
	void onEntityCall(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** ����ӿ�
		client����entity��cell������baseappת��
	*/
	void onRemoteCallMethodFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		client��������
	*/
	void onUpdateDataFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	void onUpdateDataFromClientForControlledEntity(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		real����������Ե�ghost
	*/
	void onUpdateGhostPropertys(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** ����ӿ�
		ghost�������def����real
	*/
	void onRemoteRealMethodCall(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		real����������Ե�ghost
	*/
	void onUpdateGhostVolatileData(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		base�����ȡcelldata
	*/
	void reqBackupEntityCellData(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		base�����ȡWriteToDB
	*/
	void reqWriteToDBFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		�ͻ���ֱ�ӷ�����Ϣ��cellʵ��
	*/
	void forwardEntityMessageToCellappFromClient(Network::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		��������flags
	*/
	void reqSetFlags(Network::Channel* pChannel, MemoryStream& s);

	/**
		��ȡ��Ϸʱ��
	*/
	static PyObject* __py_gametime(PyObject* self, PyObject* args);

	/**
		�����ɾ��һ��Updatable����
	*/
	bool addUpdatable(Updatable* pObject);
	bool removeUpdatable(Updatable* pObject);

	/**
		hook entitycallcall
	*/
	RemoteEntityMethod* createEntityCallCallEntityRemoteMethod(MethodDescription* pMethodDescription, EntityCallAbstract* pEntityCall);

	/** ����ӿ�
		ĳ��app����鿴��app
	*/
	virtual void lookApp(Network::Channel* pChannel);

	/**
		���µ������еĽű�
	*/
	static PyObject* __py_reloadScript(PyObject* self, PyObject* args);
	virtual void reloadScript(bool fullReload);
	virtual void onReloadScript(bool fullReload);

	/**
		��ȡ�����Ƿ����ڹر���
	*/
	static PyObject* __py_isShuttingDown(PyObject* self, PyObject* args);

	/**
		��ȡ�����ڲ������ַ
	*/
	static PyObject* __py_address(PyObject* self, PyObject* args);

	WitnessedTimeoutHandler	* pWitnessedTimeoutHandler(){ return pWitnessedTimeoutHandler_; }

	/**
		����ӿ�
		��һ��cellapp��entityҪteleport����cellapp�ϵ�space��
	*/
	void reqTeleportToCellApp(Network::Channel* pChannel, MemoryStream& s);
	void reqTeleportToCellAppCB(Network::Channel* pChannel, MemoryStream& s);
	void reqTeleportToCellAppOver(Network::Channel* pChannel, MemoryStream& s);

	/**
		��ȡ������ghost������
	*/
	void pGhostManager(GhostManager* v){ pGhostManager_ = v; }
	GhostManager* pGhostManager() const{ return pGhostManager_; }

	ArraySize spaceSize() const { return (ArraySize)SpaceMemorys::size(); }

	/** 
		���� 
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

	// ���е�cell
	Cells								cells_;

	TelnetServer*						pTelnetServer_;

	WitnessedTimeoutHandler	*			pWitnessedTimeoutHandler_;

	GhostManager*						pGhostManager_;
	
	// APP�ı�־
	uint32								flags_;

	// ͨ�����߲鿴space
	SpaceViewers						spaceViewers_;

	InitProgressHandler*				pInitProgressHandler_;
};

}

#endif // KBE_CELLAPP_H
