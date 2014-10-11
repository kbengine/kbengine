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


#ifndef KBE_BASEAPP_HPP
#define KBE_BASEAPP_HPP
	
// common include	
#include "base.hpp"
#include "proxy.hpp"
#include "profile.hpp"
#include "server/entity_app.hpp"
#include "server/pendingLoginmgr.hpp"
#include "server/forward_messagebuffer.hpp"
#include "network/endpoint.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

namespace Mercury{
	class Channel;
}

class Proxy;
class Backuper;
class Archiver;
class TelnetServer;
class RestoreEntityHandler;

class Baseapp :	public EntityApp<Base>, 
				public Singleton<Baseapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_CHECK_STATUS = TIMEOUT_ENTITYAPP_MAX + 1,
		TIMEOUT_MAX
	};
	
	Baseapp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Baseapp();
	
	virtual bool installPyModules();
	virtual void onInstallPyModules();
	virtual bool uninstallPyModules();

	bool run();
	
	/** 
		��ش���ӿ� 
	*/
	virtual void handleTimeout(TimerHandle handle, void * arg);
	virtual void handleGameTick();
	void handleCheckStatusTick();
	void handleBackup();
	void handleArchive();

	/** 
		��ʼ����ؽӿ� 
	*/
	bool initializeBegin();
	bool initializeEnd();
	void finalise();
	
	virtual bool canShutdown();
	virtual void onShutdownBegin();
	virtual void onShutdown(bool first);
	virtual void onShutdownEnd();

	virtual bool initializeWatcher();

	float getLoad()const { return load_; }
	
	void updateLoad();

	static uint64 checkTickPeriod();

	static int quantumPassedPercent(uint64 curr = timestamp());
	static PyObject* __py_quantumPassedPercent(PyObject* self, PyObject* args);

	virtual void onChannelDeregister(Mercury::Channel * pChannel);

	/**
		һ��cellapp����
	*/
	void onCellAppDeath(Mercury::Channel * pChannel);

	/** ����ӿ�
		dbmgr��֪�Ѿ�����������baseapp����cellapp�ĵ�ַ
		��ǰapp��Ҫ������ȥ�����ǽ�������
	*/
	virtual void onGetEntityAppFromDbmgr(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);
	
	/** ����ӿ�
		ĳ��client��app��֪���ڻ״̬��
	*/
	void onClientActiveTick(Mercury::Channel* pChannel);

	/** 
		������һ��entity�ص�
	*/
	virtual Base* onCreateEntityCommon(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid);

	/** 
		����һ��entity 
	*/
	static PyObject* __py_createBase(PyObject* self, PyObject* args);
	static PyObject* __py_createBaseAnywhere(PyObject* self, PyObject* args);
	static PyObject* __py_createBaseFromDBID(PyObject* self, PyObject* args);
	static PyObject* __py_createBaseAnywhereFromDBID(PyObject* self, PyObject* args);
	
	/**
		����һ���µ�space 
	*/
	void createInNewSpace(Base* base, PyObject* cell);

	/**
		�ָ�һ��space 
	*/
	void restoreSpaceInCell(Base* base);

	/** 
		��һ�����ؽϵ͵�baseapp�ϴ���һ��baseEntity 
	*/
	void createBaseAnywhere(const char* entityType, PyObject* params, PyObject* pyCallback);

	/** �յ�baseappmgr������ĳ��baseappҪ��createBaseAnywhere�������ڱ�baseapp��ִ�� 
		@param entityType	: entity����� entities.xml�еĶ���ġ�
		@param strInitData	: ���entity��������Ӧ�ø�����ʼ����һЩ���ݣ� ��Ҫʹ��pickle.loads���.
		@param componentID	: ���󴴽�entity��baseapp�����ID
	*/
	void onCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s);

	/** 
		��db��ȡ��Ϣ����һ��entity
	*/
	void createBaseFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback);

	/** ����ӿ�
		createBaseFromDBID�Ļص���
	*/
	void onCreateBaseFromDBIDCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 
		��db��ȡ��Ϣ����һ��entity
	*/
	void createBaseAnywhereFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback);

	/** ����ӿ�
		createBaseFromDBID�Ļص���
	*/
	// �����ݿ����Ļص�
	void onCreateBaseAnywhereFromDBIDCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	// ��������������ϴ������entity
	void createBaseAnywhereFromDBIDOtherBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	// ������Ϻ�Ļص�
	void onCreateBaseAnywhereFromDBIDOtherBaseappCallback(Mercury::Channel* pChannel, COMPONENT_ID createByBaseappID, 
							std::string entityType, ENTITY_ID createdEntityID, CALLBACK_ID callbackID, DBID dbid);
	

	/** 
		baseapp ��createBaseAnywhere�Ļص� 
	*/
	void onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	void _onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, CALLBACK_ID callbackID, 
		std::string& entityType, ENTITY_ID eid, COMPONENT_ID componentID);

	/** 
		Ϊһ��baseEntity��ָ����cell�ϴ���һ��cellEntity 
	*/
	void createCellEntity(EntityMailboxAbstract* createToCellMailbox, Base* base);
	
	/** ����ӿ�
		createCellEntityʧ�ܵĻص���
	*/
	void onCreateCellFailure(Mercury::Channel* pChannel, ENTITY_ID entityID);

	/** ����ӿ�
		createCellEntity��cellʵ�崴���ɹ��ص���
	*/
	void onEntityGetCell(Mercury::Channel* pChannel, ENTITY_ID id, COMPONENT_ID componentID, SPACE_ID spaceID);

	/** 
		֪ͨ�ͻ��˴���һ��proxy��Ӧ��ʵ�� 
	*/
	bool createClientProxies(Proxy* base, bool reload = false);

	/** 
		��dbmgr����ִ��һ�����ݿ�����
	*/
	static PyObject* __py_executeRawDatabaseCommand(PyObject* self, PyObject* args);
	void executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid);
	void onExecuteRawDatabaseCommandCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		dbmgr���ͳ�ʼ��Ϣ
		startID: ��ʼ����ENTITY_ID ����ʼλ��
		endID: ��ʼ����ENTITY_ID �ν���λ��
		startGlobalOrder: ȫ������˳�� �������ֲ�ͬ���
		startGroupOrder: ��������˳�� ����������baseapp�еڼ���������
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest);

	/** ����ӿ�
		dbmgr�㲥global���ݵĸı�
	*/
	void onBroadcastBaseAppDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		ע�ὫҪ��¼���˺�, ע����������¼��������
	*/
	void registerPendingLogin(Mercury::Channel* pChannel, std::string& loginName, std::string& accountName, 
		std::string& password, ENTITY_ID entityID, DBID entityDBID, uint32 flags, uint64 deadline, COMPONENT_TYPE componentType);

	/** ����ӿ�
		���û������¼��������
	*/
	void loginGateway(Mercury::Channel* pChannel, std::string& accountName, std::string& password);

	/**
		�߳�һ��Channel
	*/
	void kickChannel(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode);

	/** ����ӿ�
		���µ�¼ ���������ؽ���������ϵ(ǰ����֮ǰ�Ѿ���¼�ˣ� 
		֮��Ͽ��ڷ������ж���ǰ�˵�Entityδ��ʱ���ٵ�ǰ���¿��Կ�����������������Ӳ��ﵽ�ٿظ�entity��Ŀ��)
	*/
	void reLoginGateway(Mercury::Channel* pChannel, std::string& accountName, 
		std::string& password, uint64 key, ENTITY_ID entityID);

	/**
	   ��¼ʧ��
	   @failedcode: ʧ�ܷ����� MERCURY_ERR_SRV_NO_READY:������û��׼����, 
									MERCURY_ERR_ILLEGAL_LOGIN:�Ƿ���¼, 
									MERCURY_ERR_NAME_PASSWORD:�û����������벻��ȷ
	*/
	void loginGatewayFailed(Mercury::Channel* pChannel, std::string& accountName, 
		SERVER_ERROR_CODE failedcode, bool relogin = false);

	/** ����ӿ�
		��dbmgr��ȡ���˺�Entity��Ϣ
	*/
	void onQueryAccountCBFromDbmgr(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/**
		�ͻ����������������
	*/
	void onClientEntityEnterWorld(Proxy* base, COMPONENT_ID componentID);

	/** ����ӿ�
		entity�յ�һ��mail, ��ĳ��app�ϵ�mailbox����(ֻ����������ڲ�ʹ�ã� �ͻ��˵�mailbox���÷�����
		onRemoteCellMethodCallFromClient)
	*/
	void onEntityMail(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** ����ӿ�
		client����entity��cell����
	*/
	void onRemoteCallCellMethodFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		client��������
	*/
	void onUpdateDataFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);


	/** ����ӿ�
		cellapp����entity��cell����
	*/
	void onBackupEntityCellData(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		cellapp writeToDB���
	*/
	void onCellWriteToDBCompleted(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		cellappת��entity��Ϣ��client
	*/
	void forwardMessageToClientFromCellapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		cellappת��entity��Ϣ��ĳ��baseEntity��cellEntity
	*/
	void forwardMessageToCellappFromCellapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/**
		��ȡ��Ϸʱ��
	*/
	static PyObject* __py_gametime(PyObject* self, PyObject* args);

	/** ����ӿ�
		дentity��db�ص�
	*/
	void onWriteToDBCallback(Mercury::Channel* pChannel, ENTITY_ID eid, DBID entityDBID, CALLBACK_ID callbackID, bool success);

	/**
		����proxices����
	*/
	void incProxicesCount(){ ++numProxices_; }

	/**
		����proxices����
	*/
	void decProxicesCount(){ --numProxices_; }

	/**
		���proxices����
	*/
	int32 numProxices()const{ return numProxices_; }

	/**
		���numClients����
	*/
	int32 numClients(){ return this->networkInterface().numExtChannels(); }
	
	/** 
		�����ֵ
	*/
	static PyObject* __py_charge(PyObject* self, PyObject* args);
	void charge(std::string chargeID, DBID dbid, const std::string& datas, PyObject* pycallback);
	void onChargeCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/**
		hook mailboxcall
	*/
	RemoteEntityMethod* createMailboxCallEntityRemoteMethod(MethodDescription* md, EntityMailbox* pMailbox);

	virtual void onHello(Mercury::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	// ����汾��ƥ��
	virtual void onVersionNotMatch(Mercury::Channel* pChannel);

	// ����ű���汾��ƥ��
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel);

	/**
		һ��cell��entity���ָ����
	*/
	void onRestoreEntitiesOver(RestoreEntityHandler* pRestoreEntityHandler);

	/** ����ӿ�
		ĳ��baseapp�ϵ�space�ָ���cell�� �жϵ�ǰbaseapp�Ƿ������entity��Ҫ�ָ�cell
	*/
	void onRestoreSpaceCellFromOtherBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		ĳ��app����鿴��app
	*/
	virtual void lookApp(Mercury::Channel* pChannel);

	/** ����ӿ�
		�ͻ���Э�鵼��
	*/
	void importClientMessages(Mercury::Channel* pChannel);

	/** ����ӿ�
		�ͻ���entitydef����
	*/
	void importClientEntityDef(Mercury::Channel* pChannel);

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

	/**
		ͨ��dbid�����ݿ���ɾ��һ��ʵ��

		�����ݿ�ɾ��ʵ�壬 ���ʵ�岻���������ֱ��ɾ���ص�����true�� ���������ص����ص���entity��mailbox�� �����κ�ԭ�򶼷���false.
	*/
	static PyObject* __py_deleteBaseByDBID(PyObject* self, PyObject* args);

	/** ����ӿ�
		ͨ��dbid�����ݿ���ɾ��һ��ʵ��Ļص�
	*/
	void deleteBaseByDBIDCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		�����email
	*/
	void reqAccountBindEmail(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& password, std::string& email);

	void onReqAccountBindEmailCB(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code);

	/** ����ӿ�
		�����email
	*/
	void reqAccountNewPassword(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& oldpassworld, std::string& newpassword);

	void onReqAccountNewPasswordCB(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
		SERVER_ERROR_CODE failedcode);
protected:
	TimerHandle												loopCheckTimerHandle_;

	GlobalDataClient*										pBaseAppData_;								// globalBases

	// ��¼��¼������������δ������ϵ��˺�
	PendingLoginMgr											pendingLoginMgr_;

	ForwardComponent_MessageBuffer							forward_messagebuffer_;

	// ���ݴ浵���
	KBEShared_ptr< Backuper >								pBackuper_;	
	KBEShared_ptr< Archiver >								pArchiver_;	

	float													load_;

	static uint64											_g_lastTimestamp;

	int32													numProxices_;

	TelnetServer*											pTelnetServer_;

	std::vector< KBEShared_ptr< RestoreEntityHandler > >	pRestoreEntityHandlers_;

	TimerHandle												pResmgrTimerHandle_;
};

}

#endif // KBE_BASEAPP_HPP
