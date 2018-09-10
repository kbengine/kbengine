// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_DBTASKS_H
#define KBE_DBTASKS_H

#include "common/common.h"
#include "common/memorystream.h"
#include "common/timestamp.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"
#include "entitydef/entitydef.h"
#include "network/address.h"
#include "db_interface/db_tasks.h"
#include "server/server_errors.h"

namespace KBEngine{ 

class DBInterface;
class Buffered_DBTasks;
struct ACCOUNT_INFOS;

/*
	���ݿ��߳����������
*/

class DBTask : public DBTaskBase
{
public:
	DBTask(const Network::Address& addr, MemoryStream& datas);

	DBTask(const Network::Address& addr):
	DBTaskBase(),
	pDatas_(0),
	addr_(addr)
	{
	}

	DBTask():
	DBTaskBase(),
	pDatas_(0),
	addr_()
	{
	}
	
	virtual ~DBTask();

	bool send(Network::Bundle* pBundle);

	virtual std::string name() const {
		return "DBTask";
	}

protected:
	MemoryStream* pDatas_;
	Network::Address addr_;
};

/*
	EntityDBTask
*/
class EntityDBTask : public DBTask
{
public:
	EntityDBTask(const Network::Address& addr, MemoryStream& datas, ENTITY_ID entityID, DBID entityDBID):
	DBTask(addr, datas),
	_entityID(entityID),
	_entityDBID(entityDBID),
	_pBuffered_DBTasks(NULL)
	{
	}
	
	EntityDBTask(const Network::Address& addr, ENTITY_ID entityID, DBID entityDBID):
	DBTask(addr),
	_entityID(entityID),
	_entityDBID(entityDBID),
	_pBuffered_DBTasks(NULL)
	{
	}
	
	virtual ~EntityDBTask(){}
	
	ENTITY_ID EntityDBTask_entityID() const { return _entityID; }
	DBID EntityDBTask_entityDBID() const { return _entityDBID; }
	
	void pBuffered_DBTasks(Buffered_DBTasks* v){ _pBuffered_DBTasks = v; }
	virtual thread::TPTask::TPTaskState presentMainThread();

	DBTask* tryGetNextTask();

	virtual std::string name() const {
		return "EntityDBTask";
	}

private:
	ENTITY_ID _entityID;
	DBID _entityDBID;
	Buffered_DBTasks* _pBuffered_DBTasks;
};

/**
	ִ��һ��sql���
*/
class DBTaskExecuteRawDatabaseCommand : public DBTask
{
public:
	DBTaskExecuteRawDatabaseCommand(const Network::Address& addr, MemoryStream& datas);
	virtual ~DBTaskExecuteRawDatabaseCommand();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskExecuteRawDatabaseCommand";
	}

protected:
	COMPONENT_ID componentID_;
	COMPONENT_TYPE componentType_;
	std::string sdatas_;
	CALLBACK_ID callbackID_;
	std::string error_;
	MemoryStream* pExecret_;
};


/**
	ִ��һ��sql���
*/
class DBTaskExecuteRawDatabaseCommandByEntity : public EntityDBTask
{
public:
	DBTaskExecuteRawDatabaseCommandByEntity(const Network::Address& addr, MemoryStream& datas, ENTITY_ID entityID);
	virtual ~DBTaskExecuteRawDatabaseCommandByEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskExecuteRawDatabaseCommandByEntity";
	}

protected:
	COMPONENT_ID componentID_;
	COMPONENT_TYPE componentType_;
	std::string sdatas_;
	CALLBACK_ID callbackID_;
	std::string error_;
	MemoryStream* pExecret_;
};

/**
	�����ݿ�дentity�� ����entityʱҲ���������
*/
class DBTaskWriteEntity : public EntityDBTask
{
public:
	DBTaskWriteEntity(const Network::Address& addr, COMPONENT_ID componentID, 
		ENTITY_ID eid, DBID entityDBID, MemoryStream& datas);

	virtual ~DBTaskWriteEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskWriteEntity";
	}

protected:
	COMPONENT_ID componentID_;
	ENTITY_ID eid_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
	CALLBACK_ID callbackID_;
	int8 shouldAutoLoad_;
	bool success_;
};

/**
	�����ݿ���ɾ��entity
*/
class DBTaskRemoveEntity : public EntityDBTask
{
public:
	DBTaskRemoveEntity(const Network::Address& addr, COMPONENT_ID componentID, 
		ENTITY_ID eid, DBID entityDBID, MemoryStream& datas);

	virtual ~DBTaskRemoveEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskRemoveEntity";
	}

protected:
	COMPONENT_ID componentID_;
	ENTITY_ID eid_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
};

/**
	�����ݿ���ɾ��entity
*/
class DBTaskDeleteEntityByDBID : public DBTask
{
public:
	DBTaskDeleteEntityByDBID(const Network::Address& addr, COMPONENT_ID componentID, 
		DBID entityDBID, CALLBACK_ID callbackID, ENTITY_SCRIPT_UID sid);

	virtual ~DBTaskDeleteEntityByDBID();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskDeleteEntityByDBID";
	}

protected:
	COMPONENT_ID componentID_;
	CALLBACK_ID callbackID_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
	bool success_;
	ENTITY_ID entityID_;
	COMPONENT_ID entityInAppID_;
};

/**
	�����ݿ����Զ�����ʵ��
*/
class DBTaskEntityAutoLoad : public DBTask
{
public:
	DBTaskEntityAutoLoad(const Network::Address& addr, COMPONENT_ID componentID, 
		ENTITY_SCRIPT_UID entityType, ENTITY_ID start, ENTITY_ID end);

	virtual ~DBTaskEntityAutoLoad();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskEntityAutoLoad";
	}

protected:
	COMPONENT_ID componentID_;
	ENTITY_SCRIPT_UID entityType_;
	ENTITY_ID start_;
	ENTITY_ID end_;
	std::vector<DBID> outs_;
};

/**
	ͨ��dbid��ѯһ��ʵ���Ƿ�����ݿ���
*/
class DBTaskLookUpEntityByDBID : public DBTask
{
public:
	DBTaskLookUpEntityByDBID(const Network::Address& addr, COMPONENT_ID componentID, 
		DBID entityDBID, CALLBACK_ID callbackID, ENTITY_SCRIPT_UID sid);

	virtual ~DBTaskLookUpEntityByDBID();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskLookUpEntityByDBID";
	}

protected:
	COMPONENT_ID componentID_;
	CALLBACK_ID callbackID_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
	bool success_;
	ENTITY_ID entityID_;
	COMPONENT_ID entityInAppID_;
	COMPONENT_ID serverGroupID_;
};

/**
	����һ���˺ŵ����ݿ�
*/
class DBTaskCreateAccount : public DBTask
{
public:
	DBTaskCreateAccount(const Network::Address& addr, std::string& registerName, std::string& accountName, 
		std::string& password, std::string& postdatas, std::string& getdatas);
	virtual ~DBTaskCreateAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	static bool writeAccount(DBInterface* pdbi, const std::string& accountName, 
		const std::string& passwd, const std::string& datas, ACCOUNT_INFOS& info);

	virtual std::string name() const {
		return "DBTaskCreateAccount";
	}

protected:
	std::string registerName_; 
	std::string accountName_;
	std::string password_;
	std::string postdatas_, getdatas_;
	bool success_;
	
};

/**
	����һ��email�˺�
*/
class DBTaskCreateMailAccount : public DBTask
{
public:
	DBTaskCreateMailAccount(const Network::Address& addr, std::string& registerName, std::string& accountName, 
		std::string& password, std::string& postdatas, std::string& getdatas);
	virtual ~DBTaskCreateMailAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskCreateMailAccount";
	}

protected:
	std::string registerName_; 
	std::string accountName_;
	std::string password_;
	std::string postdatas_, getdatas_;
	bool success_;
	
};

/**
	����һ��email�˺�
*/
class DBTaskActivateAccount : public DBTask
{
public:
	DBTaskActivateAccount(const Network::Address& addr, std::string& code);
	virtual ~DBTaskActivateAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskActivateAccount";
	}

protected:
	std::string code_; 
	bool success_;
	
};

/**
	���������˺�
*/
class DBTaskReqAccountResetPassword : public DBTask
{
public:
	DBTaskReqAccountResetPassword(const Network::Address& addr, std::string& accountName);
	virtual ~DBTaskReqAccountResetPassword();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskReqAccountResetPassword";
	}

protected:
	std::string code_; 
	std::string email_;
	std::string accountName_;
	bool success_;
	
};

/**
	��������˺�
*/
class DBTaskAccountResetPassword : public DBTask
{
public:
	DBTaskAccountResetPassword(const Network::Address& addr, std::string& accountName, 
		std::string& newpassword, std::string& code);
	virtual ~DBTaskAccountResetPassword();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskAccountResetPassword";
	}

protected:
	std::string code_; 
	std::string accountName_;
	std::string newpassword_;
	bool success_;
	
};

/**
	�����email
*/
class DBTaskReqAccountBindEmail : public DBTask
{
public:
	DBTaskReqAccountBindEmail(const Network::Address& addr, ENTITY_ID entityID, std::string& accountName, 
		std::string password,std::string& email);
	virtual ~DBTaskReqAccountBindEmail();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskReqAccountBindEmail";
	}

protected:
	std::string code_; 
	std::string password_; 
	std::string accountName_;
	std::string email_; 
	bool success_;
	ENTITY_ID entityID_;
	
};

/**
	��ɰ�email
*/
class DBTaskAccountBindEmail : public DBTask
{
public:
	DBTaskAccountBindEmail(const Network::Address& addr, std::string& accountName, 
		std::string& code);
	virtual ~DBTaskAccountBindEmail();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskAccountBindEmail";
	}

protected:
	std::string code_; 
	std::string accountName_;
	bool success_;
};

/**
	����������
*/
class DBTaskAccountNewPassword : public DBTask
{
public:
	DBTaskAccountNewPassword(const Network::Address& addr, ENTITY_ID entityID, std::string& accountName, 
		std::string& oldpassword_, std::string& newpassword);
	virtual ~DBTaskAccountNewPassword();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskAccountNewPassword";
	}

protected:
	std::string accountName_;
	std::string oldpassword_, newpassword_;
	bool success_;
	ENTITY_ID entityID_;
};

/**
	baseapp�����ѯaccount��Ϣ
*/
class DBTaskQueryAccount : public EntityDBTask
{
public:
	DBTaskQueryAccount(const Network::Address& addr, std::string& accountName, std::string& password, bool needCheckPassword,
		COMPONENT_ID componentID, ENTITY_ID entityID, DBID entityDBID, uint32 ip, uint16 port);
	virtual ~DBTaskQueryAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskQueryAccount";
	}

protected:
	std::string accountName_;
	std::string password_;
	bool success_;
	MemoryStream* s_;
	DBID dbid_;
	COMPONENT_ID componentID_;
	ENTITY_ID entityID_;
	std::string error_;
	uint32 ip_;
	uint16 port_;

	uint32 flags_;
	uint64 deadline_;

	std::string bindatas_;
	bool needCheckPassword_;
};

/**
	�˺�����
*/
class DBTaskAccountOnline : public EntityDBTask
{
public:
	DBTaskAccountOnline(const Network::Address& addr, std::string& accountName,
		COMPONENT_ID componentID, ENTITY_ID entityID);
	virtual ~DBTaskAccountOnline();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskAccountOnline";
	}

protected:
	std::string accountName_;
	COMPONENT_ID componentID_;
};


/**
	entity����
*/
class DBTaskEntityOffline : public EntityDBTask
{
public:
	DBTaskEntityOffline(const Network::Address& addr, DBID dbid, ENTITY_SCRIPT_UID sid);
	virtual ~DBTaskEntityOffline();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskEntityOffline";
	}

protected:
	ENTITY_SCRIPT_UID sid_;
};


/**
	һ�����û���¼�� ��Ҫ���Ϸ���
*/
class DBTaskAccountLogin : public DBTask
{
public:
	DBTaskAccountLogin(const Network::Address& addr, std::string& loginName, 
		std::string& accountName, std::string& password, SERVER_ERROR_CODE retcode, std::string& postdatas, 
		std::string& getdatas, bool needCheckPassword);

	virtual ~DBTaskAccountLogin();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskAccountLogin";
	}

protected:
	std::string loginName_;
	std::string accountName_;
	std::string password_;
	std::string postdatas_, getdatas_;
	SERVER_ERROR_CODE retcode_;
	COMPONENT_ID componentID_;
	ENTITY_ID entityID_;
	DBID dbid_;
	uint32 flags_;
	uint64 deadline_;
	bool needCheckPassword_;
	COMPONENT_ID serverGroupID_;
};

/**
	baseapp�����ѯentity��Ϣ
*/
class DBTaskQueryEntity : public EntityDBTask
{
public:
	DBTaskQueryEntity(const Network::Address& addr, int8 queryMode, std::string& entityType, DBID dbid, 
		COMPONENT_ID componentID, CALLBACK_ID callbackID, ENTITY_ID entityID);

	virtual ~DBTaskQueryEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskQueryEntity";
	}

protected:
	int8 queryMode_;
	std::string entityType_;
	DBID dbid_;
	COMPONENT_ID componentID_;
	CALLBACK_ID callbackID_;
	bool success_;
	MemoryStream* s_;
	ENTITY_ID entityID_;

	// ���ʵ���Ѿ�������������ָ��ʵ������app
	bool wasActive_;
	COMPONENT_ID wasActiveCID_;
	ENTITY_ID wasActiveEntityID_;
	
	COMPONENT_ID serverGroupID_;
};

/**
	д��������־
*/
class DBTaskServerLog : public DBTask
{
public:
	DBTaskServerLog();
	virtual ~DBTaskServerLog();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskServerLog";
	}

protected:
};

/**
	����ĳ��baseapp��¼��entitylog
*/
class DBTaskEraseBaseappEntityLog : public DBTask
{
public: 
	DBTaskEraseBaseappEntityLog(COMPONENT_ID componentID);
	virtual ~DBTaskEraseBaseappEntityLog();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual std::string name() const {
		return "DBTaskEraseBaseappEntityLog";
	}

protected:
	COMPONENT_ID componentID_;
	bool success_;

};

}

#endif // KBE_DBTASKS_H
