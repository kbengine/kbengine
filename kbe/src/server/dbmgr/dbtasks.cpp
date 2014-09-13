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

#include "dbtasks.hpp"
#include "dbmgr.hpp"
#include "buffered_dbtasks.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/kbe_tables.hpp"
#include "db_mysql/db_exception.hpp"
#include "db_mysql/db_interface_mysql.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "openssl/md5.h"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
#pragma comment(lib, "libeay32_d.lib")
#pragma comment(lib, "ssleay32_d.lib")
#else
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif
#endif


namespace KBEngine{

//-------------------------------------------------------------------------------------
DBTask::DBTask(const Mercury::Address& addr, MemoryStream& datas):
DBTaskBase(),
pDatas_(0),
addr_(addr)
{
	pDatas_ = MemoryStream::ObjPool().createObject();
	*pDatas_ = datas;
}

//-------------------------------------------------------------------------------------
DBTask::~DBTask()
{
	if(pDatas_)
		MemoryStream::ObjPool().reclaimObject(pDatas_);
}

//-------------------------------------------------------------------------------------
bool DBTask::send(Mercury::Bundle& bundle)
{
	Mercury::Channel* pChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);
	
	if(pChannel){
		bundle.send(Dbmgr::getSingleton().networkInterface(), pChannel);
	}
	else{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
DBTask* EntityDBTask::tryGetNextTask()
{
	KBE_ASSERT(_pBuffered_DBTasks != NULL);
	return _pBuffered_DBTasks->tryGetNextTask(this);
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState EntityDBTask::presentMainThread()
{
	return DBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskExecuteRawDatabaseCommand::DBTaskExecuteRawDatabaseCommand(const Mercury::Address& addr, MemoryStream& datas):
DBTask(addr, datas),
componentID_(0),
componentType_(UNKNOWN_COMPONENT_TYPE),
sdatas_(),
callbackID_(0),
error_(),
execret_()
{
}

//-------------------------------------------------------------------------------------
DBTaskExecuteRawDatabaseCommand::~DBTaskExecuteRawDatabaseCommand()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskExecuteRawDatabaseCommand::db_thread_process()
{
	(*pDatas_) >> componentID_ >> componentType_;
	(*pDatas_) >> callbackID_;
	(*pDatas_).readBlob(sdatas_);

	try
	{
		if(!static_cast<DBInterfaceMysql*>(pdbi_)->execute(sdatas_.data(), sdatas_.size(), &execret_))
		{
			error_ = pdbi_->getstrerror();
		}
	}
	catch (std::exception & e)
	{
		DBException& dbe = static_cast<DBException&>(e);
		if(dbe.isLostConnection())
		{
			static_cast<DBInterfaceMysql*>(pdbi_)->processException(e);
			return true;
		}

		error_ = e.what();
	}

	return false;
}

//-------------------------------------------------------------------------------------
DBTaskExecuteRawDatabaseCommandByEntity::DBTaskExecuteRawDatabaseCommandByEntity(const Mercury::Address& addr, MemoryStream& datas, ENTITY_ID entityID):
EntityDBTask(addr, datas, entityID, 0),
componentID_(0),
componentType_(UNKNOWN_COMPONENT_TYPE),
sdatas_(),
callbackID_(0),
error_(),
execret_()
{
}

//-------------------------------------------------------------------------------------
DBTaskExecuteRawDatabaseCommandByEntity::~DBTaskExecuteRawDatabaseCommandByEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskExecuteRawDatabaseCommandByEntity::db_thread_process()
{
	(*pDatas_) >> componentID_ >> componentType_;
	(*pDatas_) >> callbackID_;
	(*pDatas_).readBlob(sdatas_);

	try
	{
		if(!static_cast<DBInterfaceMysql*>(pdbi_)->execute(sdatas_.data(), sdatas_.size(), &execret_))
		{
			error_ = pdbi_->getstrerror();
		}
	}
	catch (std::exception & e)
	{
		DBException& dbe = static_cast<DBException&>(e);
		if(dbe.isLostConnection())
		{
			static_cast<DBInterfaceMysql*>(pdbi_)->processException(e);
			return true;
		}

		error_ = e.what();
	}

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskExecuteRawDatabaseCommandByEntity::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::DBTaskExecuteRawDatabaseCommandByEntity:%1%.\n") % sdatas_.c_str());

	// 如果不需要回调则结束
	if(callbackID_ <= 0)
		return EntityDBTask::presentMainThread();

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	if(componentType_ == BASEAPP_TYPE)
		(*pBundle).newMessage(BaseappInterface::onExecuteRawDatabaseCommandCB);
	else if(componentType_ == CELLAPP_TYPE)
		(*pBundle).newMessage(CellappInterface::onExecuteRawDatabaseCommandCB);
	else
	{
		KBE_ASSERT(false && "no support!\n");
	}

	int32 packetsLength = execret_.opsize();
	const Mercury::MessageHandler& msgHandler = CellappInterface::onExecuteRawDatabaseCommandCB;

	if(packetsLength > const_cast<Mercury::MessageHandler*>(&msgHandler)->msglenMax())
	{
		error_ = (boost::format("DBTaskExecuteRawDatabaseCommandByEntity::presentMainThread: msglen exceeds the limit! msglen=(%1%) > maxlen(%2%).") %
			packetsLength % const_cast<Mercury::MessageHandler*>(&msgHandler)->msglenMax()).str();

		ERROR_MSG(error_);
	}

	(*pBundle) << callbackID_;
	(*pBundle) << error_;

	if(error_.size() <= 0)
		(*pBundle).append(execret_);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentType_, componentID_);

	if(cinfos && cinfos->pChannel)
	{
		packetsLength = (*pBundle).packetsLength(true);

		if(packetsLength > const_cast<Mercury::MessageHandler*>(&msgHandler)->msglenMax())
		{
			ERROR_MSG(boost::format("DBTaskExecuteRawDatabaseCommandByEntity::presentMainThread: msglen exceeds the limit! msglen=(%1%) > maxlen(%2%).") %
				packetsLength % const_cast<Mercury::MessageHandler*>(&msgHandler)->msglenMax());
		}
		else
		{
			(*pBundle).send(Dbmgr::getSingleton().networkInterface(), cinfos->pChannel);
		}
	}
	else
	{
		ERROR_MSG(boost::format("DBTaskExecuteRawDatabaseCommandByEntity::presentMainThread: %1% not found.") %
			COMPONENT_NAME_EX(componentType_));
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskExecuteRawDatabaseCommand::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::executeRawDatabaseCommand:%1%.\n") % sdatas_.c_str());

	// 如果不需要回调则结束
	if(callbackID_ <= 0)
		return thread::TPTask::TPTASK_STATE_COMPLETED;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	if(componentType_ == BASEAPP_TYPE)
		(*pBundle).newMessage(BaseappInterface::onExecuteRawDatabaseCommandCB);
	else if(componentType_ == CELLAPP_TYPE)
		(*pBundle).newMessage(CellappInterface::onExecuteRawDatabaseCommandCB);
	else
	{
		KBE_ASSERT(false && "no support!\n");
	}

	(*pBundle) << callbackID_;
	(*pBundle) << error_;

	if(error_.size() <= 0)
		(*pBundle).append(execret_);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentType_, componentID_);

	if(cinfos && cinfos->pChannel)
	{
		(*pBundle).send(Dbmgr::getSingleton().networkInterface(), cinfos->pChannel);
	}
	else
	{
		ERROR_MSG(boost::format("DBTaskExecuteRawDatabaseCommand::presentMainThread: %1% not found.") %
			COMPONENT_NAME_EX(componentType_));
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskWriteEntity::DBTaskWriteEntity(const Mercury::Address& addr, 
									 COMPONENT_ID componentID, ENTITY_ID eid, 
									 DBID entityDBID, MemoryStream& datas):
EntityDBTask(addr, datas, eid, entityDBID),
componentID_(componentID),
eid_(eid),
entityDBID_(entityDBID),
sid_(0),
callbackID_(0),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskWriteEntity::~DBTaskWriteEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskWriteEntity::db_thread_process()
{
	(*pDatas_) >> sid_ >> callbackID_;

	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	bool writeEntityLog = (entityDBID_ == 0);

	uint32 ip = 0;
	uint16 port = 0;

	if(writeEntityLog)
	{
		(*pDatas_) >> ip >> port;
	}

	entityDBID_ = EntityTables::getSingleton().writeEntity(pdbi_, entityDBID_, pDatas_, pModule);
	success_ = entityDBID_ > 0;

	if(writeEntityLog && success_)
	{
		success_ = false;

		// 先写log， 如果写失败则可能这个entity已经在线
		KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
						(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
		KBE_ASSERT(pELTable);

		success_ = pELTable->logEntity(pdbi_, inet_ntoa((struct in_addr&)ip), port, entityDBID_, 
			componentID_, eid_, pModule->getUType());

		if(!success_)
		{
			entityDBID_ = 0;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskWriteEntity::presentMainThread()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	DEBUG_MSG(fmt::format("Dbmgr::writeEntity: {0}({1}).\n", pModule->getName(), entityDBID_));

	// 返回写entity的结果， 成功或者失败

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onWriteToDBCallback);
	BaseappInterface::onWriteToDBCallbackArgs4::staticAddToBundle((*pBundle), 
		eid_, entityDBID_, callbackID_, success_);

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(fmt::format("DBTaskWriteEntity::presentMainThread: channel({0}) not found.\n", addr_.c_str()));
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	
	return EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskRemoveEntity::DBTaskRemoveEntity(const Mercury::Address& addr, 
									 COMPONENT_ID componentID, ENTITY_ID eid, 
									 DBID entityDBID, MemoryStream& datas):
EntityDBTask(addr, datas, eid, entityDBID),
componentID_(componentID),
eid_(eid),
entityDBID_(entityDBID),
sid_(0)
{
}

//-------------------------------------------------------------------------------------
DBTaskRemoveEntity::~DBTaskRemoveEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskRemoveEntity::db_thread_process()
{
	(*pDatas_) >> sid_;

	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);
	pELTable->eraseEntityLog(pdbi_, entityDBID_, sid_);

	EntityTables::getSingleton().removeEntity(pdbi_, entityDBID_, EntityDef::findScriptModule(sid_));
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskRemoveEntity::presentMainThread()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	DEBUG_MSG(boost::format("Dbmgr::removeEntity: %1%(%2%).\n") % pModule->getName() % entityDBID_);
	return EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskDeleteBaseByDBID::DBTaskDeleteBaseByDBID(const Mercury::Address& addr, COMPONENT_ID componentID, 
		DBID entityDBID, CALLBACK_ID callbackID, ENTITY_SCRIPT_UID sid):
DBTask(addr),
componentID_(componentID),
callbackID_(callbackID),
entityDBID_(entityDBID),
sid_(sid),
success_(false),
entityID_(0),
entityInAppID_(0)
{
}

//-------------------------------------------------------------------------------------
DBTaskDeleteBaseByDBID::~DBTaskDeleteBaseByDBID()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskDeleteBaseByDBID::db_thread_process()
{
	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));

	KBE_ASSERT(pELTable);

	bool haslog = false;
	KBEEntityLogTable::EntityLog entitylog;

	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);

	haslog = pELTable->queryEntity(pdbi_, entityDBID_, entitylog, pModule->getUType());

	// 如果有在线纪录
	if(haslog)
	{
		success_ = false;
		entityInAppID_ = entitylog.componentID;
		entityID_ = entitylog.entityID;
		KBE_ASSERT(entityID_ > 0 && entityInAppID_ > 0);
		return false;
	}

	EntityTables::getSingleton().removeEntity(pdbi_, entityDBID_, pModule);
	success_ = true;
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskDeleteBaseByDBID::presentMainThread()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	DEBUG_MSG(boost::format("Dbmgr::DBTaskDeleteBaseByDBID: %1%(%2%), entityInAppID(%3%).\n") % 
		pModule->getName() % entityDBID_ % entityInAppID_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::deleteBaseByDBIDCB);

	(*pBundle) << success_ << entityID_ << entityInAppID_ << callbackID_ << sid_ << entityDBID_;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskDeleteBaseByDBID::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskCreateAccount::DBTaskCreateAccount(const Mercury::Address& addr, 
										 std::string& registerName,
										 std::string& accountName, 
										 std::string& password, 
										 std::string& postdatas, 
										std::string& getdatas):
DBTask(addr),
registerName_(registerName),
accountName_(accountName),
password_(password),
postdatas_(postdatas),
getdatas_(getdatas),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskCreateAccount::~DBTaskCreateAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskCreateAccount::db_thread_process()
{
	ACCOUNT_INFOS info;
	success_ = DBTaskCreateAccount::writeAccount(pdbi_, accountName_, password_, postdatas_, info) && info.dbid > 0;
	return false;
}

//-------------------------------------------------------------------------------------
bool DBTaskCreateAccount::writeAccount(DBInterface* pdbi, const std::string& accountName, 
									   const std::string& passwd, const std::string& datas, ACCOUNT_INFOS& info)
{
	info.dbid = 0;
	if(accountName.size() == 0)
	{
		return false;
	}

	// 寻找dblog是否有此账号， 如果有则创建失败
	// 如果没有则向account表新建一个entity数据同时在accountlog表写入一个log关联dbid
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	ScriptDefModule* pModule = EntityDef::findScriptModule(DBUtil::accountScriptName());

	if(pTable->queryAccount(pdbi, accountName, info) && (info.flags & ACCOUNT_FLAG_NOT_ACTIVATED) <= 0)
	{
		if(pdbi->getlasterror() > 0)
		{
			WARNING_MSG(boost::format("DBTaskCreateAccount::writeAccount(): queryAccount error: %1%\n") % 
				pdbi->getstrerror());
		}

		return false;
	}

	bool hasset = (info.dbid != 0);
	if(!hasset)
	{
		info.flags = g_kbeSrvConfig.getDBMgr().accountDefaultFlags;
		info.deadline = g_kbeSrvConfig.getDBMgr().accountDefaultDeadline;
	}

	DBID entityDBID = info.dbid;
	
	if(entityDBID == 0)
	{
		// 防止多线程问题， 这里做一个拷贝。
		MemoryStream copyAccountDefMemoryStream(pTable->accountDefMemoryStream());

		entityDBID = EntityTables::getSingleton().writeEntity(pdbi, 0, 
				&copyAccountDefMemoryStream, pModule);
	}

	KBE_ASSERT(entityDBID > 0);

	info.name = accountName;
	info.password = passwd;
	info.dbid = entityDBID;
	info.datas = datas;
	
	if(!hasset)
	{
		if(!pTable->logAccount(pdbi, info))
		{
			if(pdbi->getlasterror() > 0)
			{
				WARNING_MSG(boost::format("DBTaskCreateAccount::writeAccount(): logAccount error:%1%\n") % 
					pdbi->getstrerror());
			}

			return false;
		}
	}
	else
	{
		if(!pTable->setFlagsDeadline(pdbi, accountName, info.flags & ~ACCOUNT_FLAG_NOT_ACTIVATED, info.deadline))
		{
			if(pdbi->getlasterror() > 0)
			{
				WARNING_MSG(boost::format("DBTaskCreateAccount::writeAccount(): logAccount error:%1%\n") % 
					pdbi->getstrerror());
			}

			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskCreateAccount::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::reqCreateAccount:%1%.\n") % registerName_.c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onReqCreateAccountResult);
	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	if(!success_)
		failedcode = SERVER_ERR_ACCOUNT_CREATE_FAILED;

	(*pBundle) << failedcode << registerName_ << password_;
	(*pBundle).appendBlob(getdatas_);

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskCreateAccount::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
std::string genmail_code(const std::string& str)
{
	std::string datas = KBEngine::StringConv::val2str(KBEngine::genUUID64());
	datas += KBE_MD5::getDigest(str.data(), str.length());

	srand(getSystemTime());
	datas += KBEngine::StringConv::val2str(rand());
	return datas;
}

//-------------------------------------------------------------------------------------
DBTaskCreateMailAccount::DBTaskCreateMailAccount(const Mercury::Address& addr, 
										 std::string& registerName,
										 std::string& accountName, 
										 std::string& password, 
										 std::string& postdatas, 
										std::string& getdatas):
DBTask(addr),
registerName_(registerName),
accountName_(accountName),
password_(password),
postdatas_(postdatas),
getdatas_(getdatas),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskCreateMailAccount::~DBTaskCreateMailAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskCreateMailAccount::db_thread_process()
{
	ACCOUNT_INFOS info;
	if(accountName_.size() == 0)
	{
		ERROR_MSG("DBTaskCreateMailAccount::db_thread_process(): accountName is NULL!\n");
		return false;
	}

	// 寻找dblog是否有此账号， 如果有则创建失败
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);
	
	info.flags = 0;
	if(pTable->queryAccount(pdbi_, accountName_, info)  && (info.flags & ACCOUNT_FLAG_NOT_ACTIVATED) <= 0)
	{
		if(pdbi_->getlasterror() > 0)
		{
			WARNING_MSG(boost::format("DBTaskCreateMailAccount::db_thread_process(): queryAccount is error: %1%\n") % 
				pdbi_->getstrerror());
		}

		return false;
	}

	// 生成激活码并存储激活码到数据库
	// 发送smtp邮件到邮箱， 用户点击确认后即可激活
	getdatas_ = genmail_code(password_);
	KBEEmailVerificationTable* pTable1 = static_cast<KBEEmailVerificationTable*>(EntityTables::getSingleton().findKBETable("kbe_email_verification"));
	KBE_ASSERT(pTable1);
	
	info.datas = getdatas_;
	info.name = registerName_;
	info.password = password_;
	info.flags |= ACCOUNT_FLAG_NOT_ACTIVATED; 
	info.email = info.name;
	info.dbid = KBEngine::genUUID64();

	try
	{
		pTable->logAccount(pdbi_, info);
	}
	catch (...)
	{
		WARNING_MSG(boost::format("DBTaskCreateMailAccount::db_thread_process(): logAccount(kbe_accountinfos) is error: %1%\n%2%\n") % 
			pdbi_->getstrerror() % pdbi_->lastquery());
	}

	password_ = KBE_MD5::getDigest(password_.data(), password_.length());

	success_ = pTable1->logAccount(pdbi_, (int8)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT, 
		registerName_, password_, getdatas_);

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskCreateMailAccount::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::reqCreateMailAccount:%1%, success=%2%.\n") % registerName_ % success_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onReqCreateMailAccountResult);
	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	if(!success_)
		failedcode = SERVER_ERR_ACCOUNT_CREATE_FAILED;

	(*pBundle) << failedcode << registerName_ << password_;
	(*pBundle).appendBlob(getdatas_);

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskCreateMailAccount::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskActivateAccount::DBTaskActivateAccount(const Mercury::Address& addr, 
										 std::string& code):
DBTask(addr),
code_(code),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskActivateAccount::~DBTaskActivateAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskActivateAccount::db_thread_process()
{
	ACCOUNT_INFOS info;

	KBEEmailVerificationTable* pTable1 = 
		static_cast<KBEEmailVerificationTable*>(EntityTables::getSingleton().findKBETable("kbe_email_verification"));

	KBE_ASSERT(pTable1);

	success_ = pTable1->activateAccount(pdbi_, code_, info);
	if(!success_)
	{
		ERROR_MSG(boost::format("DBTaskActivateAccount::db_thread_process(): activateAccount(%2%) error: %1%\n") % 
				pdbi_->getstrerror() % code_);
		return false;
	}

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskActivateAccount::presentMainThread()
{
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onAccountActivated);


	(*pBundle) << code_ << success_;
	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskActivateAccount::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}


//-------------------------------------------------------------------------------------
DBTaskReqAccountResetPassword::DBTaskReqAccountResetPassword(const Mercury::Address& addr, std::string& accountName):
DBTask(addr),
code_(),
email_(),
accountName_(accountName),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskReqAccountResetPassword::~DBTaskReqAccountResetPassword()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskReqAccountResetPassword::db_thread_process()
{
	ACCOUNT_INFOS info;
	if(accountName_.size() == 0)
	{
		return false;
	}

	// 生成激活码并存储激活码到数据库
	// 发送smtp邮件到邮箱， 用户点击确认后即可激活
	KBEEmailVerificationTable* pTable1 = 
		static_cast<KBEEmailVerificationTable*>(EntityTables::getSingleton().findKBETable("kbe_email_verification"));
	KBE_ASSERT(pTable1);

	info.datas = genmail_code(accountName_);
	info.name = accountName_;
	email_ = accountName_;
	code_ = info.datas;
	success_ = pTable1->logAccount(pdbi_, (int8)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD, accountName_, "", code_);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskReqAccountResetPassword::presentMainThread()
{
	DEBUG_MSG(boost::format("DBTaskReqAccountResetPassword::presentMainThread: accountName=%1%, code_=%2%, success=%3%.\n") 
		% accountName_ % code_ % success_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onReqAccountResetPasswordCB);
	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	if(!success_)
		failedcode = SERVER_ERR_OP_FAILED;

	(*pBundle) << accountName_;
	(*pBundle) << email_;
	(*pBundle) << failedcode;
	(*pBundle) << code_;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskReqAccountResetPassword::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskAccountResetPassword::DBTaskAccountResetPassword(const Mercury::Address& addr, std::string& accountName, 
		std::string& newpassword, std::string& code):
DBTask(addr),
code_(code),
accountName_(accountName),
newpassword_(newpassword),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountResetPassword::~DBTaskAccountResetPassword()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountResetPassword::db_thread_process()
{
	KBEEmailVerificationTable* pTable1 = 
		static_cast<KBEEmailVerificationTable*>(EntityTables::getSingleton().findKBETable("kbe_email_verification"));

	KBE_ASSERT(pTable1);

	success_ = pTable1->resetpassword(pdbi_, accountName_, newpassword_, code_);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskAccountResetPassword::presentMainThread()
{
	DEBUG_MSG(boost::format("DBTaskAccountResetPassword::presentMainThread: code(%1%), success=%2%.\n") 
		% code_ % success_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onAccountResetPassword);

	(*pBundle) << code_;
	(*pBundle) << success_;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskAccountResetPassword::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskReqAccountBindEmail::DBTaskReqAccountBindEmail(const Mercury::Address& addr, ENTITY_ID entityID, std::string& accountName, 
		std::string password,std::string& email):
DBTask(addr),
code_(),
password_(password),
accountName_(accountName),
email_(email),
success_(false),
entityID_(entityID)
{
}

//-------------------------------------------------------------------------------------
DBTaskReqAccountBindEmail::~DBTaskReqAccountBindEmail()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskReqAccountBindEmail::db_thread_process()
{
	ACCOUNT_INFOS info;
	if(accountName_.size() == 0)
	{
		return false;
	}

	// 生成激活码并存储激活码到数据库
	// 发送smtp邮件到邮箱， 用户点击确认后即可激活
	KBEEmailVerificationTable* pTable1 = 
		static_cast<KBEEmailVerificationTable*>(EntityTables::getSingleton().findKBETable("kbe_email_verification"));
	KBE_ASSERT(pTable1);

	info.datas = genmail_code(accountName_);
	info.name = accountName_;
	code_ = info.datas;
	success_ = pTable1->logAccount(pdbi_, (int8)KBEEmailVerificationTable::V_TYPE_BIND_MAIL, accountName_, email_, code_);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskReqAccountBindEmail::presentMainThread()
{
	DEBUG_MSG(boost::format("DBTaskReqAccountBindEmail::presentMainThread: code(%1%), success=%2%.\n") 
		% code_ % success_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onReqAccountBindEmailCB);
	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	if(!success_)
		failedcode = SERVER_ERR_OP_FAILED;

	(*pBundle) << entityID_; 
	(*pBundle) << accountName_;
	(*pBundle) << email_;
	(*pBundle) << failedcode;
	(*pBundle) << code_;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskReqAccountBindEmail::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskAccountBindEmail::DBTaskAccountBindEmail(const Mercury::Address& addr, std::string& accountName, 
		std::string& code):
DBTask(addr),
code_(code),
accountName_(accountName),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountBindEmail::~DBTaskAccountBindEmail()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountBindEmail::db_thread_process()
{
	KBEEmailVerificationTable* pTable1 = 
		static_cast<KBEEmailVerificationTable*>(EntityTables::getSingleton().findKBETable("kbe_email_verification"));

	KBE_ASSERT(pTable1);

	success_ = pTable1->bindEMail(pdbi_, accountName_, code_);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskAccountBindEmail::presentMainThread()
{
	DEBUG_MSG(boost::format("DBTaskAccountBindEmail::presentMainThread: code(%1%), success=%2%.\n") 
		% code_ % success_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onAccountBindedEmail);

	(*pBundle) << code_;
	(*pBundle) << success_;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskAccountBindEmail::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskAccountNewPassword::DBTaskAccountNewPassword(const Mercury::Address& addr, ENTITY_ID entityID, std::string& accountName, 
		std::string& oldpassword_, std::string& newpassword):
DBTask(addr),
accountName_(accountName),
oldpassword_(oldpassword_), newpassword_(newpassword),
success_(false),
entityID_(entityID)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountNewPassword::~DBTaskAccountNewPassword()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountNewPassword::db_thread_process()
{
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	ACCOUNT_INFOS info;
	if(!pTable->queryAccount(pdbi_, accountName_, info))
	{
		return false;
	}

	if(info.dbid == 0 || info.flags != ACCOUNT_FLAG_NORMAL)
		return false;

	if(kbe_stricmp(info.password.c_str(), KBE_MD5::getDigest(oldpassword_.data(), oldpassword_.length()).c_str()) != 0)
	{
		return false;
	}

	success_ = pTable->updatePassword(pdbi_, accountName_, KBE_MD5::getDigest(newpassword_.data(), newpassword_.length()));
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskAccountNewPassword::presentMainThread()
{
	DEBUG_MSG(boost::format("DBTaskAccountNewPassword::presentMainThread: success=%1%.\n") 
		% success_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onReqAccountNewPasswordCB);

	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	if(!success_)
		failedcode = SERVER_ERR_OP_FAILED;

	(*pBundle) << entityID_;
	(*pBundle) << accountName_;
	(*pBundle) << failedcode;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskAccountNewPassword::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return thread::TPTask::TPTASK_STATE_COMPLETED;
}

//-------------------------------------------------------------------------------------
DBTaskQueryAccount::DBTaskQueryAccount(const Mercury::Address& addr, std::string& accountName, std::string& password, 
		COMPONENT_ID componentID, ENTITY_ID entityID, DBID entityDBID, uint32 ip, uint16 port):
EntityDBTask(addr, entityID, entityDBID),
accountName_(accountName),
password_(password),
success_(false),
s_(),
dbid_(entityDBID),
componentID_(componentID),
entityID_(entityID),
error_(),
ip_(ip),
port_(port),
flags_(0),
deadline_(0)
{
}

//-------------------------------------------------------------------------------------
DBTaskQueryAccount::~DBTaskQueryAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskQueryAccount::db_thread_process()
{
	if(accountName_.size() == 0)
	{
		error_ = "accountName_ is NULL";
		return false;
	}

	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	ACCOUNT_INFOS info;
	info.name = "";
	info.password = "";
	info.dbid = dbid_;

	if(dbid_ == 0)
	{
		if(!pTable->queryAccount(pdbi_, accountName_, info))
		{
			error_ = "pTable->queryAccount() is failed!";
			
			if(pdbi_->getlasterror() > 0)
			{
				error_ += pdbi_->getstrerror();
			}
	
			return false;
		}

		if(info.dbid == 0 || info.flags != ACCOUNT_FLAG_NORMAL)
		{
			error_ = "dbid is 0 or flags != ACCOUNT_FLAG_NORMAL";
			return false;
		}

		if(kbe_stricmp(info.password.c_str(), KBE_MD5::getDigest(password_.data(), password_.length()).c_str()) != 0)
		{
			error_ = "password is error";
			return false;
		}
	}

	ScriptDefModule* pModule = EntityDef::findScriptModule(g_kbeSrvConfig.getDBMgr().dbAccountEntityScriptType);
	success_ = EntityTables::getSingleton().queryEntity(pdbi_, info.dbid, &s_, pModule);

	if(!success_ && pdbi_->getlasterror() > 0)
	{
		error_ += "queryEntity: ";
		error_ += pdbi_->getstrerror();
	}

	dbid_ = info.dbid;

	if(!success_)
		return false;

	success_ = false;

	// 先写log， 如果写失败则可能这个entity已经在线
	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);
	
	success_ = pELTable->logEntity(pdbi_, inet_ntoa((struct in_addr&)ip_), port_, dbid_, 
		componentID_, entityID_, pModule->getUType());

	if(!success_ && pdbi_->getlasterror() > 0)
	{
		error_ += "logEntity: ";
		error_ += pdbi_->getstrerror();
	}

	flags_ = info.flags;
	deadline_ = info.deadline;

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskQueryAccount::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::queryAccount:%1%, success=%2%, flags=%3%, deadline=%4%.\n") 
		% accountName_.c_str() % success_ % flags_ % deadline_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onQueryAccountCBFromDbmgr);
	(*pBundle) << accountName_;
	(*pBundle) << password_;
	(*pBundle) << dbid_;
	(*pBundle) << success_;
	(*pBundle) << entityID_;
	(*pBundle) << flags_;
	(*pBundle) << deadline_;

	if(success_)
	{
		pBundle->append(s_);
	}
	else
	{
		(*pBundle) << error_;
	}

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskQueryAccount::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskAccountOnline::DBTaskAccountOnline(const Mercury::Address& addr, std::string& accountName,
		COMPONENT_ID componentID, ENTITY_ID entityID):
EntityDBTask(addr, entityID, 0),
accountName_(accountName),
componentID_(componentID)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountOnline::~DBTaskAccountOnline()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountOnline::db_thread_process()
{
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskAccountOnline::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::onAccountOnline:componentID:%1%, entityID:%2%.\n") % componentID_ % EntityDBTask_entityID());

	/*
	// 如果没有连接db则从log中查找账号是否还在线
	if(!pDBInterface_)
	{
		PROXICES_ONLINE_LOG::iterator iter = proxicesOnlineLogs_.find(accountName_);
		if(iter != proxicesOnlineLogs_.end())
		{
			iter->second.cid = componentID_;
			iter->second.eid = entityID_;
		}
		else
		{
			proxicesOnlineLogs_[accountName].cid = componentID_;
			proxicesOnlineLogs_[accountName].eid = entityID_;
		}
	}
	*/

	return EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskEntityOffline::DBTaskEntityOffline(const Mercury::Address& addr, DBID dbid, ENTITY_SCRIPT_UID sid):
EntityDBTask(addr, 0, dbid),
sid_(sid)
{
}

//-------------------------------------------------------------------------------------
DBTaskEntityOffline::~DBTaskEntityOffline()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskEntityOffline::db_thread_process()
{
	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);
	pELTable->eraseEntityLog(pdbi_, EntityDBTask_entityDBID(), sid_);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskEntityOffline::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::onEntityOffline:%1%, entityType=%2%.\n") % EntityDBTask_entityDBID() % sid_);
	return EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskAccountLogin::DBTaskAccountLogin(const Mercury::Address& addr, 
									   std::string& loginName, 
									   std::string& accountName, 
									   std::string& password, 
									   bool success,
									   std::string& postdatas, 
									   std::string& getdatas):
DBTask(addr),
loginName_(loginName),
accountName_(accountName),
password_(password),
postdatas_(postdatas),
getdatas_(getdatas),
success_(success),
componentID_(0),
entityID_(0),
dbid_(0),
flags_(0),
deadline_(0)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountLogin::~DBTaskAccountLogin()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountLogin::db_thread_process()
{
	// 如果billing已经判断不成功就没必要继续下去
	if(!success_)
	{
		return false;
	}

	success_ = false;

	if(accountName_.size() == 0)
	{
		return false;
	}

	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);

	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	ACCOUNT_INFOS info;
	info.dbid = 0;
	info.flags = 0;
	info.deadline = 0;
	if(!pTable->queryAccount(pdbi_, accountName_, info))
	{
		flags_ = info.flags;
		deadline_ = info.deadline;
		if(ACCOUNT_TYPE(g_kbeSrvConfig.getLoginApp().account_type) != ACCOUNT_TYPE_NORMAL)
		{
			if (email_isvalid(accountName_.c_str()))
			{
				ERROR_MSG(boost::format("DBTaskAccountLogin::db_thread_process(): account[%1%] is email, autocreate failed!\n") % 
					accountName_);
				return false;
			}
		}

		if(g_kbeSrvConfig.getDBMgr().notFoundAccountAutoCreate)
		{
			if(!DBTaskCreateAccount::writeAccount(pdbi_, accountName_, password_, postdatas_, info) || info.dbid == 0 || info.flags != ACCOUNT_FLAG_NORMAL)
			{
				ERROR_MSG(boost::format("DBTaskAccountLogin::db_thread_process(): not found account[%1%], autocreate failed!\n") % 
					accountName_);
				return false;
			}

			INFO_MSG(boost::format("DBTaskAccountLogin::db_thread_process(): not found account[%1%], autocreate successfully!\n") % 
				accountName_);

			if(kbe_stricmp(g_kbeSrvConfig.billingSystemAccountType(), "normal") == 0)
			{
				info.password = KBE_MD5::getDigest(password_.data(), password_.length());
			}
		}
		else
		{
			ERROR_MSG(boost::format("DBTaskAccountLogin::db_thread_process(): not found account[%1%], login failed!\n") % 
				accountName_);
			return false;
		}
	}

	if(info.dbid == 0 || info.flags != ACCOUNT_FLAG_NORMAL)
		return false;

	if(kbe_stricmp(g_kbeSrvConfig.billingSystemAccountType(), "normal") == 0)
	{
		if(kbe_stricmp(info.password.c_str(), KBE_MD5::getDigest(password_.data(), password_.length()).c_str()) != 0)
		{
			success_ = false;
			return false;
		}
	}
	
	pTable->updateCount(pdbi_, info.dbid);

	success_ = false;
	KBEEntityLogTable::EntityLog entitylog;

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	ScriptDefModule* pModule = EntityDef::findScriptModule(dbcfg.dbAccountEntityScriptType);
	success_ = !pELTable->queryEntity(pdbi_, info.dbid, entitylog, pModule->getUType());

	// 如果有在线纪录
	if(!success_)
	{
		componentID_ = entitylog.componentID;
		entityID_ = entitylog.entityID;
	}

	dbid_ = info.dbid;
	flags_ = info.flags;
	deadline_ = info.deadline;
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskAccountLogin::presentMainThread()
{
	DEBUG_MSG(fmt::format("Dbmgr::onAccountLogin:loginName{0}, accountName={1}, success={2}, componentID={3}, dbid={4}, flags={5}, deadline={6}.\n", 
		loginName_,
		accountName_,
		success_,
		componentID_,
		dbid_,
		flags_,
		deadline_
		));

	// 一个用户登录， 构造一个数据库查询指令并加入到执行队列， 执行完毕将结果返回给loginapp
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onLoginAccountQueryResultFromDbmgr);

	(*pBundle) << success_;
	(*pBundle) << loginName_;
	(*pBundle) << accountName_;
	(*pBundle) << password_;
	(*pBundle) << componentID_;   // 如果大于0则表示账号还存活在某个baseapp上
	(*pBundle) << entityID_;
	(*pBundle) << dbid_;
	(*pBundle) << flags_;
	(*pBundle) << deadline_;
	(*pBundle).appendBlob(getdatas_);

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskAccountLogin::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return DBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskQueryEntity::DBTaskQueryEntity(const Mercury::Address& addr, int8 queryMode, std::string& entityType, DBID dbid, 
		COMPONENT_ID componentID, CALLBACK_ID callbackID, ENTITY_ID entityID):
EntityDBTask(addr, entityID, dbid),
queryMode_(queryMode),
entityType_(entityType),
dbid_(dbid),
componentID_(componentID),
callbackID_(callbackID),
success_(false),
s_(),
entityID_(entityID),
wasActive_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskQueryEntity::~DBTaskQueryEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskQueryEntity::db_thread_process()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(entityType_.c_str());
	success_ = EntityTables::getSingleton().queryEntity(pdbi_, dbid_, &s_, pModule);

	if(success_)
	{
		// 先写log， 如果写失败则可能这个entity已经在线
		KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
						(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
		KBE_ASSERT(pELTable);

		try
		{
			success_ = pELTable->logEntity(pdbi_, addr_.ipAsString(), addr_.port, dbid_, 
				componentID_, entityID_, pModule->getUType());
		}
		catch (std::exception & e)
		{
			DBException& dbe = static_cast<DBException&>(e);
			if(dbe.isLostConnection())
			{
				static_cast<DBInterfaceMysql*>(pdbi_)->processException(e);
				return true;
			}
			else
				success_ = false;
		}

		if(!success_)
			wasActive_ = true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskQueryEntity::presentMainThread()
{
	DEBUG_MSG(boost::format("Dbmgr::DBTaskQueryEntity:%1%, dbid=%2%, entityID=%3%, wasActive=%4%, queryMode=%5%, success=%6%.\n") % 
		entityType_ % dbid_ % entityID_ % wasActive_ % ((int)queryMode_) % success_);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	if(queryMode_ == 0)
		pBundle->newMessage(BaseappInterface::onCreateBaseFromDBIDCallback);
	else if(queryMode_ == 1)
		pBundle->newMessage(BaseappInterface::onCreateBaseAnywhereFromDBIDCallback);

	(*pBundle) << entityType_;
	(*pBundle) << dbid_;
	(*pBundle) << callbackID_;
	(*pBundle) << success_;
	(*pBundle) << entityID_;
	(*pBundle) << wasActive_;

	if(success_)
	{
		pBundle->append(s_);
	}

	if(!this->send((*pBundle)))
	{
		ERROR_MSG(boost::format("DBTaskQueryEntity::presentMainThread: channel(%1%) not found.\n") % addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	return EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
}
