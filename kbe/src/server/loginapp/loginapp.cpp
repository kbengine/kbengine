/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "jwsmtp.h"
#include "loginapp.h"
#include "profile.h"	
#include "http_cb_handler.h"
#include "loginapp_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "common/kbeversion.h"
#include "server/components.h"
#include "server/telnet_server.h"
#include "server/sendmail_threadtasks.h"
#include "client_lib/client_interface.h"
#include "network/encryption_filter.h"

#include "baseapp/baseapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "dbmgr/dbmgr_interface.h"


namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Loginapp);

//-------------------------------------------------------------------------------------
Loginapp::Loginapp(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	PythonApp(dispatcher, ninterface, componentType, componentID),
	mainProcessTimer_(),
	pendingCreateMgr_(ninterface),
	pendingLoginMgr_(ninterface),
	digest_(),
	pHttpCBHandler(NULL),
	initProgress_(0.f),
	pTelnetServer_(NULL)
{
}

//-------------------------------------------------------------------------------------
Loginapp::~Loginapp()
{
	SAFE_RELEASE(pHttpCBHandler);
}

//-------------------------------------------------------------------------------------	
void Loginapp::onShutdownBegin()
{
	PythonApp::onShutdownBegin();
	
	// ֪ͨ�ű�
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS0(getEntryScript().get(), const_cast<char*>("onLoginAppShutDown"));
}

//-------------------------------------------------------------------------------------	
void Loginapp::onShutdownEnd()
{
	PythonApp::onShutdownEnd();
}

//-------------------------------------------------------------------------------------
bool Loginapp::run()
{
	return PythonApp::run();
}

//-------------------------------------------------------------------------------------
void Loginapp::handleTimeout(TimerHandle handle, void * arg)
{
	PythonApp::handleTimeout(handle, arg);

	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_TICK:
			this->handleMainTick();
			break;
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::handleMainTick()
{
	threadPool_.onMainThreadTick();
	networkInterface().processChannels(&LoginappInterface::messageHandlers);
	pendingLoginMgr_.process();
	pendingCreateMgr_.process();
}

//-------------------------------------------------------------------------------------
void Loginapp::onChannelDeregister(Network::Channel * pChannel)
{
	// ������ⲿͨ������
	if(!pChannel->isInternal())
	{
		const std::string& extra = pChannel->extra();

		// ֪ͨdbmgr�Ӷ���������������� ����ӵ��
		if(extra.size() > 0)
		{
			Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
			Components::ComponentInfos* dbmgrinfos = NULL;

			if(cts.size() > 0)
				dbmgrinfos = &(*cts.begin());

			if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
			{
			}
			else
			{
				Network::Bundle* pBundle = Network::Bundle::createPoolObject();
				(*pBundle).newMessage(DbmgrInterface::eraseClientReq);
				(*pBundle) << extra;
				dbmgrinfos->pChannel->send(pBundle);
			}
		}
	}

	PythonApp::onChannelDeregister(pChannel);
}

//-------------------------------------------------------------------------------------
bool Loginapp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Loginapp::inInitialize()
{
	return PythonApp::inInitialize();
}

//-------------------------------------------------------------------------------------
bool Loginapp::initializeEnd()
{
	PythonApp::initializeEnd();

	// ���һ��timer�� ÿ����һЩ״̬
	mainProcessTimer_ = this->dispatcher().addTimer(1000000 / 50, this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	// ���нű����������
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onLoginAppReady"), 
										const_cast<char*>(""));

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
	
	pTelnetServer_ = new TelnetServer(&this->dispatcher(), &this->networkInterface());
	pTelnetServer_->pScript(&this->getScript());

	bool ret = pTelnetServer_->start(g_kbeSrvConfig.getLoginApp().telnet_passwd,
		g_kbeSrvConfig.getLoginApp().telnet_deflayer,
		g_kbeSrvConfig.getLoginApp().telnet_port);

	Components::getSingleton().extraData4(pTelnetServer_->port());
	return ret;
}

//-------------------------------------------------------------------------------------		
void Loginapp::onInstallPyModules()
{
	PyObject * module = getScript().getModule();

	for (int i = 0; i < SERVER_ERR_MAX; i++)
	{
		if(PyModule_AddIntConstant(module, SERVER_ERR_STR[i], i))
		{
			ERROR_MSG( fmt::format("Loginapp::onInstallPyModules: Unable to set KBEngine.{}.\n", SERVER_ERR_STR[i]));
		}
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::finalise()
{
	mainProcessTimer_.cancel();
	PythonApp::finalise();
}

//-------------------------------------------------------------------------------------
void Loginapp::onDbmgrInitCompleted(Network::Channel* pChannel, COMPONENT_ORDER startGlobalOrder, 
	COMPONENT_ORDER startGroupOrder, const std::string& digest)
{
	if(pChannel->isExternal())
		return;

	INFO_MSG(fmt::format("Loginapp::onDbmgrInitCompleted:startGlobalOrder={0}, startGroupOrder={1}, digest={2}.\n",
		startGlobalOrder, startGroupOrder, digest));

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;
	g_componentGlobalOrder = startGlobalOrder;
	g_componentGroupOrder = startGroupOrder;
	digest_ = digest;

	// �ٴ�ͬ���Լ�������Ϣ(startGlobalOrder, startGroupOrder��)��machine
	Components::getSingleton().broadcastSelf();

	if(startGroupOrder_ == 1)
		pHttpCBHandler = new HTTPCBHandler();
}

//-------------------------------------------------------------------------------------
void Loginapp::onClientActiveTick(Network::Channel* pChannel)
{
	if(!pChannel->isExternal())
		return;

	onAppActiveTick(pChannel, CLIENT_TYPE, 0);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(ClientInterface::onAppActiveTickCB);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
bool Loginapp::_createAccount(Network::Channel* pChannel, std::string& accountName, 
								 std::string& password, std::string& datas, ACCOUNT_TYPE type)
{
	AUTO_SCOPED_PROFILE("createAccount");

	ACCOUNT_TYPE oldType = type;

	if(!g_kbeSrvConfig.getDBMgr().account_registration_enable)
	{
		ERROR_MSG(fmt::format("Loginapp::_createAccount({}): not available! modify kbengine[_defs].xml->dbmgr->account_registration.\n",
			accountName));

		std::string retdatas = "";
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
		SERVER_ERROR_CODE retcode = SERVER_ERR_ACCOUNT_REGISTER_NOT_AVAILABLE;
		(*pBundle) << retcode;
		(*pBundle).appendBlob(retdatas);
		pChannel->send(pBundle);
		return false;
	}

	accountName = KBEngine::strutil::kbe_trim(accountName);
	password = KBEngine::strutil::kbe_trim(password);

	if(accountName.size() > ACCOUNT_NAME_MAX_LENGTH)
	{
		ERROR_MSG(fmt::format("Loginapp::_createAccount: accountName too big, size={}, limit={}.\n",
			accountName.size(), ACCOUNT_NAME_MAX_LENGTH));

		return false;
	}

	if(password.size() > ACCOUNT_PASSWD_MAX_LENGTH)
	{
		ERROR_MSG(fmt::format("Loginapp::_createAccount: password too big, size={}, limit={}.\n",
			password.size(), ACCOUNT_PASSWD_MAX_LENGTH));

		return false;
	}

	if(datas.size() > ACCOUNT_DATA_MAX_LENGTH)
	{
		ERROR_MSG(fmt::format("Loginapp::_createAccount: bindatas too big, size={}, limit={}.\n",
			datas.size(), ACCOUNT_DATA_MAX_LENGTH));

		return false;
	}
	
	std::string retdatas = "";
	if(shuttingdown_ != SHUTDOWN_STATE_STOP)
	{
		WARNING_MSG(fmt::format("Loginapp::_createAccount: shutting down, create {} failed!\n", accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
		SERVER_ERROR_CODE retcode = SERVER_ERR_IN_SHUTTINGDOWN;
		(*pBundle) << retcode;
		(*pBundle).appendBlob(retdatas);
		pChannel->send(pBundle);
		return false;
	}

	PendingLoginMgr::PLInfos* ptinfos = pendingCreateMgr_.find(const_cast<std::string&>(accountName));
	if(ptinfos != NULL)
	{
		WARNING_MSG(fmt::format("Loginapp::_createAccount: pendingCreateMgr has {}, request create failed!\n", 
			accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
		SERVER_ERROR_CODE retcode = SERVER_ERR_BUSY;
		(*pBundle) << retcode;
		(*pBundle).appendBlob(retdatas);
		pChannel->send(pBundle);
		return false;
	}
	
	{
		// �������ɽű�����
		SERVER_ERROR_CODE retcode = SERVER_SUCCESS;
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
											const_cast<char*>("onRequestCreateAccount"), 
											const_cast<char*>("ssy#"), 
											accountName.c_str(),
											password.c_str(),
											datas.c_str(), datas.length());

		if(pyResult != NULL)
		{
			if(PySequence_Check(pyResult) && PySequence_Size(pyResult) == 4)
			{
				char* sname;
				char* spassword;
			    char *extraDatas;
			    Py_ssize_t extraDatas_size = 0;
				
				if(PyArg_ParseTuple(pyResult, "H|s|s|y#",  &retcode, &sname, &spassword, &extraDatas, &extraDatas_size) == -1)
				{
					ERROR_MSG(fmt::format("Loginapp::_createAccount: {}.onRequestLogin, Return value error! accountName={}\n", 
						g_kbeSrvConfig.getLoginApp().entryScriptFile, accountName));

					retcode = SERVER_ERR_OP_FAILED;
				}
				else
				{
					accountName = sname;
					password = spassword;

					if (extraDatas && extraDatas_size > 0)
						datas.assign(extraDatas, extraDatas_size);
					else
						SCRIPT_ERROR_CHECK();
				}
			}
			else
			{
				ERROR_MSG(fmt::format("Loginapp::_createAccount: {}.onRequestLogin, Return value error, must be errorcode or tuple! accountName={}\n", 
					g_kbeSrvConfig.getLoginApp().entryScriptFile, accountName));

				retcode = SERVER_ERR_OP_FAILED;
			}
			
			Py_DECREF(pyResult);
		}
		else
		{
			SCRIPT_ERROR_CHECK();
			retcode = SERVER_ERR_OP_FAILED;
		}
			
		if(retcode != SERVER_SUCCESS)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
			(*pBundle) << retcode;
			(*pBundle).appendBlob(retdatas);
			pChannel->send(pBundle);
			return false;
		}
		else
		{
			if(accountName.size() == 0)
			{
				ERROR_MSG(fmt::format("Loginapp::_createAccount: accountName is empty!\n"));

				retcode = SERVER_ERR_NAME;
				Network::Bundle* pBundle = Network::Bundle::createPoolObject();
				(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
				(*pBundle) << retcode;
				(*pBundle).appendBlob(retdatas);
				pChannel->send(pBundle);
				return false;
			}
		}
	}

	if(type == ACCOUNT_TYPE_SMART)
	{
		if (email_isvalid(accountName.c_str()))
		{
			type = ACCOUNT_TYPE_MAIL;
		}
		else
		{
			if(!validName(accountName))
			{
				ERROR_MSG(fmt::format("Loginapp::_createAccount: invalid accountName({})\n",
					accountName));

				Network::Bundle* pBundle = Network::Bundle::createPoolObject();
				(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
				SERVER_ERROR_CODE retcode = SERVER_ERR_NAME;
				(*pBundle) << retcode;
				(*pBundle).appendBlob(retdatas);
				pChannel->send(pBundle);
				return false;
			}

			type = ACCOUNT_TYPE_NORMAL;
		}
	}
	else if(type == ACCOUNT_TYPE_NORMAL)
	{
		if(!validName(accountName))
		{
			ERROR_MSG(fmt::format("Loginapp::_createAccount: invalid accountName({})\n",
				accountName));

			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
			SERVER_ERROR_CODE retcode = SERVER_ERR_NAME;
			(*pBundle) << retcode;
			(*pBundle).appendBlob(retdatas);
			pChannel->send(pBundle);
			return false;
		}
	}
	else if (!email_isvalid(accountName.c_str()))
    {
		/*
		std::string user_name, domain_name;
        user_name = regex_replace(accountName, _g_mail_pattern, std::string("$1") );
        domain_name = regex_replace(accountName, _g_mail_pattern, std::string("$2") );
		*/
		WARNING_MSG(fmt::format("Loginapp::_createAccount: invalid mail={}\n", 
			accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
		SERVER_ERROR_CODE retcode = SERVER_ERR_NAME_MAIL;
		(*pBundle) << retcode;
		(*pBundle).appendBlob(retdatas);
		pChannel->send(pBundle);
		return false;
    }

	DEBUG_MSG(fmt::format("Loginapp::_createAccount: accountName={}, passwordsize={}, type={}, oldType={}.\n",
		accountName.c_str(), password.size(), type, oldType));

	ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	ptinfos->datas = datas;
	ptinfos->addr = pChannel->addr();
	pendingCreateMgr_.add(ptinfos);

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(fmt::format("Loginapp::_createAccount: create({}), not found dbmgr!\n", 
			accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
		SERVER_ERROR_CODE retcode = SERVER_ERR_SRV_NO_READY;
		(*pBundle) << retcode;
		(*pBundle).appendBlob(retdatas);
		pChannel->send(pBundle);
		return false;
	}

	pChannel->extra(accountName);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::reqCreateAccount);
	uint8 uatype = uint8(type);
	(*pBundle) << accountName << password << uatype;
	(*pBundle).appendBlob(datas);
	dbmgrinfos->pChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void Loginapp::reqCreateAccount(Network::Channel* pChannel, MemoryStream& s)
{
	std::string accountName, password, datas;

	s >> accountName >> password;
	s.readBlob(datas);
	
	if(!_createAccount(pChannel, accountName, password, datas, ACCOUNT_TYPE(g_serverConfig.getLoginApp().account_type)))
		return;
}

//-------------------------------------------------------------------------------------
void Loginapp::reqCreateMailAccount(Network::Channel* pChannel, MemoryStream& s)
{
	std::string accountName, password, datas, retdatas = "";

	s >> accountName >> password;
	s.readBlob(datas);

	DEBUG_MSG(fmt::format("Loginapp::reqCreateMailAccount: accountName={}, passwordsize={}.\n",
		accountName.c_str(), password.size()));

	if(!_createAccount(pChannel, accountName, password, datas, ACCOUNT_TYPE_MAIL))
		return;
}

//-------------------------------------------------------------------------------------
void Loginapp::onReqCreateAccountResult(Network::Channel* pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;
	std::string accountName;
	std::string password;
	std::string retdatas = "";

	s >> failedcode >> accountName >> password;
	s.readBlob(retdatas);

	// �������ɽű�����
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onCreateAccountCallbackFromDB"), 
										const_cast<char*>("sHy#"), 
										accountName.c_str(),
										failedcode,
										retdatas.c_str(), retdatas.length());

	if(pyResult != NULL)
	{
		Py_DECREF(pyResult);
	}
	else
	{
		SCRIPT_ERROR_CHECK();
	}

	DEBUG_MSG(fmt::format("Loginapp::onReqCreateAccountResult: accountName={}, failedcode={}.\n",
		accountName.c_str(), failedcode));

	PendingLoginMgr::PLInfos* ptinfos = pendingCreateMgr_.remove(accountName);
	if(ptinfos == NULL)
		return;

	Network::Channel* pClientChannel = this->networkInterface().findChannel(ptinfos->addr);
	if(pClientChannel == NULL)
		return;

	pClientChannel->extra("");

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
	(*pBundle) << failedcode;
	(*pBundle).appendBlob(retdatas);

	pClientChannel->send(pBundle);
	SAFE_RELEASE(ptinfos);
}

//-------------------------------------------------------------------------------------
void Loginapp::onReqCreateMailAccountResult(Network::Channel* pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;
	std::string accountName;
	std::string password;
	std::string retdatas = "";

	s >> failedcode >> accountName >> password;
	s.readBlob(retdatas);

	DEBUG_MSG(fmt::format("Loginapp::onReqCreateMailAccountResult: accountName={}, failedcode={}.\n",
		accountName.c_str(), failedcode));

	if(failedcode == SERVER_SUCCESS)
	{
		Components::COMPONENTS& loginapps = Components::getSingleton().getComponents(LOGINAPP_TYPE);

		std::string http_host = "localhost";
		if(startGroupOrder_ == 1)
		{
			if(strlen((const char*)&g_kbeSrvConfig.getLoginApp().externalAddress) > 0)
				http_host = g_kbeSrvConfig.getBaseApp().externalAddress;
			else
				http_host = inet_ntoa((struct in_addr&)Loginapp::getSingleton().networkInterface().extaddr().ip);
		}
		else
		{
			Components::COMPONENTS::iterator iter = loginapps.begin();
			for(; iter != loginapps.end(); ++iter)
			{
				if((*iter).groupOrderid == 1)
				{
					if(strlen((const char*)&(*iter).externalAddressEx) > 0)
						http_host = (*iter).externalAddressEx;
					else
						http_host = inet_ntoa((struct in_addr&)(*iter).pExtAddr->ip);
				}
			}
		}

		threadPool_.addTask(new SendActivateEMailTask(accountName, retdatas, 
			http_host, 
			g_kbeSrvConfig.getLoginApp().http_cbport));
	}

	PendingLoginMgr::PLInfos* ptinfos = pendingCreateMgr_.remove(accountName);
	if(ptinfos == NULL)
		return;

	Network::Channel* pClientChannel = this->networkInterface().findChannel(ptinfos->addr);
	if(pClientChannel == NULL)
		return;

	pClientChannel->extra("");
	retdatas = "";

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onCreateAccountResult);
	(*pBundle) << failedcode;
	(*pBundle).appendBlob(retdatas);

	pClientChannel->send(pBundle);

	SAFE_RELEASE(ptinfos);
}

//-------------------------------------------------------------------------------------
void Loginapp::onAccountActivated(Network::Channel* pChannel, std::string& code, bool success)
{
	DEBUG_MSG(fmt::format("Loginapp::onAccountActivated: code={}, success={}\n", code, success));
	if(!pHttpCBHandler)
	{
		WARNING_MSG("Loginapp::onAccountActivated: pHttpCBHandler is NULL!\n");
		return;
	}

	pHttpCBHandler->onAccountActivated(code, success);
}

//-------------------------------------------------------------------------------------
void Loginapp::onAccountBindedEmail(Network::Channel* pChannel, std::string& code, bool success)
{
	DEBUG_MSG(fmt::format("Loginapp::onAccountBindedEmail: code={}, success={}\n", code, success));
	if(!pHttpCBHandler)
	{
		WARNING_MSG("Loginapp::onAccountBindedEmail: pHttpCBHandler is NULL!\n");
		return;
	}

	pHttpCBHandler->onAccountBindedEmail(code, success);
}

//-------------------------------------------------------------------------------------
void Loginapp::onAccountResetPassword(Network::Channel* pChannel, std::string& code, bool success)
{
	DEBUG_MSG(fmt::format("Loginapp::onAccountResetPassword: code={}, success={}\n", code, success));
	if(!pHttpCBHandler)
	{
		WARNING_MSG("Loginapp::onAccountResetPassword: pHttpCBHandler is NULL!\n");
		return;
	}

	pHttpCBHandler->onAccountResetPassword(code, success);
}

//-------------------------------------------------------------------------------------
void Loginapp::reqAccountResetPassword(Network::Channel* pChannel, std::string& accountName)
{
	AUTO_SCOPED_PROFILE("reqAccountResetPassword");

	accountName = KBEngine::strutil::kbe_trim(accountName);
	INFO_MSG(fmt::format("Loginapp::reqAccountResetPassword: accountName({})\n",
		accountName));

	if (!g_kbeSrvConfig.getDBMgr().account_registration_enable)
	{
		ERROR_MSG(fmt::format("Loginapp::reqAccountResetPassword({}): not available! modify kbengine[_defs].xml->dbmgr->account_resetPassword.\n",
			accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onReqAccountResetPasswordCB);
		SERVER_ERROR_CODE retcode = SERVER_ERR_ACCOUNT_RESET_PASSWORD_NOT_AVAILABLE;
		(*pBundle) << retcode;
		pChannel->send(pBundle);
		return;
	}

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(fmt::format("Loginapp::_createAccount: create({}), not found dbmgr!\n", 
			accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onReqAccountResetPasswordCB);
		SERVER_ERROR_CODE retcode = SERVER_ERR_SRV_NO_READY;
		(*pBundle) << retcode;
		pChannel->send(pBundle);
		return;
	}

	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(DbmgrInterface::accountReqResetPassword);
		(*pBundle) << accountName;
		dbmgrinfos->pChannel->send(pBundle);
	}

	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onReqAccountResetPasswordCB);
		SERVER_ERROR_CODE retcode = SERVER_SUCCESS;
		(*pBundle) << retcode;
		pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::onReqAccountResetPasswordCB(Network::Channel* pChannel, std::string& accountName, std::string& email,
	SERVER_ERROR_CODE failedcode, std::string& code)
{
	INFO_MSG(fmt::format("Loginapp::onReqAccountResetPasswordCB: {}, email={}, failedcode={}!\n", 
		accountName, email, failedcode));

	if(failedcode == SERVER_SUCCESS)
	{
		Components::COMPONENTS& loginapps = Components::getSingleton().getComponents(LOGINAPP_TYPE);

		std::string http_host = "localhost";
		if(startGroupOrder_ == 1)
		{
			if(strlen((const char*)&g_kbeSrvConfig.getLoginApp().externalAddress) > 0)
				http_host = g_kbeSrvConfig.getBaseApp().externalAddress;
			else
				http_host = inet_ntoa((struct in_addr&)Loginapp::getSingleton().networkInterface().extaddr().ip);
		}
		else
		{
			Components::COMPONENTS::iterator iter = loginapps.begin();
			for(; iter != loginapps.end(); ++iter)
			{
				if((*iter).groupOrderid == 1)
				{
					if(strlen((const char*)&(*iter).externalAddressEx) > 0)
						http_host = (*iter).externalAddressEx;
					else
						http_host = inet_ntoa((struct in_addr&)(*iter).pExtAddr->ip);
				}
			}
		}

		threadPool_.addTask(new SendResetPasswordEMailTask(email, code, 
			http_host,  
			g_kbeSrvConfig.getLoginApp().http_cbport));
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::onReqAccountBindEmailAllocCallbackLoginapp(Network::Channel* pChannel, COMPONENT_ID reqBaseappID, ENTITY_ID entityID, std::string& accountName, std::string& email,
	SERVER_ERROR_CODE failedcode, std::string& code)
{
	if (pChannel->isExternal())
		return;

	INFO_MSG(fmt::format("Loginapp::onReqAccountBindEmailAllocCallbackLoginapp: {}, email={}, failedcode={}! reqBaseappID={}\n",
		accountName, email, failedcode, reqBaseappID));

	Components::COMPONENTS& loginapps = Components::getSingleton().getComponents(LOGINAPP_TYPE);

	std::string http_host = "localhost";
	if (startGroupOrder_ == 1)
	{
		if (strlen((const char*)&g_kbeSrvConfig.getLoginApp().externalAddress) > 0)
			http_host = g_kbeSrvConfig.getBaseApp().externalAddress;
		else
			http_host = inet_ntoa((struct in_addr&)Loginapp::getSingleton().networkInterface().extaddr().ip);
	}
	else
	{
		Components::COMPONENTS::iterator iter = loginapps.begin();
		for (; iter != loginapps.end(); ++iter)
		{
			if ((*iter).groupOrderid == 1)
			{
				if (strlen((const char*)&(*iter).externalAddressEx) > 0)
					http_host = (*iter).externalAddressEx;
				else
					http_host = inet_ntoa((struct in_addr&)(*iter).pExtAddr->ip);
			}
		}
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	(*pBundle).newMessage(BaseappmgrInterface::onReqAccountBindEmailCBFromLoginapp);

	BaseappmgrInterface::onReqAccountBindEmailCBFromLoginappArgs8::staticAddToBundle((*pBundle), reqBaseappID,
		entityID, accountName, email, failedcode, code, http_host, g_kbeSrvConfig.getLoginApp().http_cbport);

	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Loginapp::login(Network::Channel* pChannel, MemoryStream& s)
{
	AUTO_SCOPED_PROFILE("login");

	COMPONENT_CLIENT_TYPE ctype;
	CLIENT_CTYPE tctype = UNKNOWN_CLIENT_COMPONENT_TYPE;
	std::string loginName;
	std::string password;
	std::string datas;
	bool forceInternalLogin = false;

	// ǰ�����
	s >> tctype;
	ctype = static_cast<COMPONENT_CLIENT_TYPE>(tctype);
	
	// ��������
	s.readBlob(datas);

	// �ʺŵ�¼��
	s >> loginName;

	// ����
	s >> password;

	loginName = KBEngine::strutil::kbe_trim(loginName);
	if(loginName.size() == 0)
	{
		INFO_MSG("Loginapp::login: loginName is NULL.\n");
		_loginFailed(pChannel, loginName, SERVER_ERR_NAME, datas, true);
		s.done();
		return;
	}

	if(loginName.size() > ACCOUNT_NAME_MAX_LENGTH)
	{
		INFO_MSG(fmt::format("Loginapp::login: loginName is too long, size={}, limit={}.\n",
			loginName.size(), ACCOUNT_NAME_MAX_LENGTH));
		
		_loginFailed(pChannel, loginName, SERVER_ERR_NAME, datas, true);
		s.done();
		return;
	}

	if(password.size() > ACCOUNT_PASSWD_MAX_LENGTH)
	{
		INFO_MSG(fmt::format("Loginapp::login: password is too long, size={}, limit={}.\n",
			password.size(), ACCOUNT_PASSWD_MAX_LENGTH));
		
		_loginFailed(pChannel, loginName, SERVER_ERR_PASSWORD, datas, true);
		s.done();
		return;
	}
	
	if(datas.size() > ACCOUNT_DATA_MAX_LENGTH)
	{
		INFO_MSG(fmt::format("Loginapp::login: bindatas is too long, size={}, limit={}.\n",
			datas.size(), ACCOUNT_DATA_MAX_LENGTH));
		
		_loginFailed(pChannel, loginName, SERVER_ERR_OP_FAILED, datas, true);
		s.done();
		return;
	}

	// ���ȱ���baseappmgr��dbmgr���Ѿ�׼������ˡ�
	Components::ComponentInfos* baseappmgrinfos = Components::getSingleton().getBaseappmgr();
	if(baseappmgrinfos == NULL || baseappmgrinfos->pChannel == NULL || baseappmgrinfos->cid == 0)
	{
		datas = "";
		_loginFailed(pChannel, loginName, SERVER_ERR_SRV_NO_READY, datas, true);
		s.done();
		return;
	}

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		datas = "";
		_loginFailed(pChannel, loginName, SERVER_ERR_SRV_NO_READY, datas, true);
		s.done();
		return;
	}

	if(!g_kbeSrvConfig.getDBMgr().allowEmptyDigest)
	{
		std::string clientDigest;

		if(s.length() > 0)
			s >> clientDigest;

		if(clientDigest.size() > 0)
		{
			if(clientDigest != digest_)
			{
				INFO_MSG(fmt::format("Loginapp::login: loginName({}), digest not match. curr({}) != dbmgr({})\n",
					loginName, clientDigest, digest_));

				datas = "";
				_loginFailed(pChannel, loginName, SERVER_ERR_ENTITYDEFS_NOT_MATCH, datas, true);
				return;
			}
		}
		else
		{
			//WARNING_MSG(fmt::format("Loginapp::login: loginName={} no check entitydefs!\n", loginName));
		}
	}

	// ����ǻ����˵�½�����������ǿ��ʹ���ڲ���ַ��½����Ҫ��ȡ�����־
	// ��ϸ�������ļ��е�forceInternalLogin
	if (ctype == CLIENT_TYPE_BOTS)
	{
		if (s.length() > 0)
			s >> forceInternalLogin;
	}

	s.done();

	if(shuttingdown_ != SHUTDOWN_STATE_STOP)
	{
		INFO_MSG(fmt::format("Loginapp::login: shutting down, {} login failed!\n", loginName));

		datas = "";
		_loginFailed(pChannel, loginName, SERVER_ERR_IN_SHUTTINGDOWN, datas, true);
		return;
	}

	if(initProgress_ < 1.f)
	{
		datas = fmt::format("initProgress: {}", initProgress_);
		_loginFailed(pChannel, loginName, SERVER_ERR_SRV_STARTING, datas, true);
		return;
	}
	
	// �������ɽű�����
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onRequestLogin"), 
										const_cast<char*>("ssby#"), 
										loginName.c_str(),
										password.c_str(),
										tctype,
										datas.c_str(), datas.length());

	if(pyResult != NULL)
	{
		bool login_check = true;
		if(PySequence_Check(pyResult) && PySequence_Size(pyResult) == 5)
		{
			char* sname;
			char* spassword;
		    char *extraDatas;
		    Py_ssize_t extraDatas_size = 0;
			SERVER_ERROR_CODE error;
			
			if(PyArg_ParseTuple(pyResult, "H|s|s|b|y#",  &error, &sname, &spassword, &tctype, &extraDatas, &extraDatas_size) == -1)
			{
				ERROR_MSG(fmt::format("Loginapp::login: {}.onRequestLogin, Return value error! loginName={}\n", 
					g_kbeSrvConfig.getLoginApp().entryScriptFile, loginName));

				login_check = false;
				_loginFailed(pChannel, loginName, SERVER_ERR_OP_FAILED, datas, true);
			}
			
			if(login_check)
			{
				loginName = sname;
				password = spassword;

				if (extraDatas && extraDatas_size > 0)
					datas.assign(extraDatas, extraDatas_size);
				else
					SCRIPT_ERROR_CHECK();
			}
			
			if(error != SERVER_SUCCESS)
			{
				login_check = false;
				_loginFailed(pChannel, loginName, error, datas, true);
			}
			
			if(loginName.size() == 0)
			{
				INFO_MSG("Loginapp::login: loginName is NULL.\n");
				_loginFailed(pChannel, loginName, SERVER_ERR_NAME, datas, true);
				s.done();
				return;
			}
		}
		else
		{
			ERROR_MSG(fmt::format("Loginapp::login: {}.onRequestLogin, Return value error, must be errorcode or tuple! loginName={}\n", 
				g_kbeSrvConfig.getLoginApp().entryScriptFile, loginName));

			login_check = false;
			_loginFailed(pChannel, loginName, SERVER_ERR_OP_FAILED, datas, true);
		}
		
		Py_DECREF(pyResult);
		
		if(!login_check)
			return;
	}
	else
	{
		SCRIPT_ERROR_CHECK();
		_loginFailed(pChannel, loginName, SERVER_ERR_OP_FAILED, datas, true);
	}

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(loginName);
	if(ptinfos != NULL)
	{
		datas = "";
		_loginFailed(pChannel, loginName, SERVER_ERR_BUSY, datas, true);
		return;
	}

	ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->ctype = ctype;
	ptinfos->datas = datas;
	ptinfos->accountName = loginName;
	ptinfos->password = password;
	ptinfos->addr = pChannel->addr();
	ptinfos->forceInternalLogin = forceInternalLogin;
	pendingLoginMgr_.add(ptinfos);

	if(ctype < UNKNOWN_CLIENT_COMPONENT_TYPE || ctype >= CLIENT_TYPE_END)
		ctype = UNKNOWN_CLIENT_COMPONENT_TYPE;

	INFO_MSG(fmt::format("Loginapp::login: new client[{0}], loginName={1}, datas={2}.\n",
		COMPONENT_CLIENT_NAME[ctype], loginName, datas));

	pChannel->extra(loginName);

	// ��dbmgr��ѯ�û��Ϸ���
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::onAccountLogin);
	(*pBundle) << loginName << password;
	(*pBundle).appendBlob(datas);
	dbmgrinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Loginapp::_loginFailed(Network::Channel* pChannel, std::string& loginName, SERVER_ERROR_CODE failedcode, std::string& datas, bool force)
{
	INFO_MSG(fmt::format("Loginapp::loginFailed: loginName={0} login is failed. failedcode={1}, datas={2}.\n",
		loginName, SERVER_ERR_STR[failedcode], datas));
	
	PendingLoginMgr::PLInfos* infos = NULL;

	if(!force)
	{
		infos = pendingLoginMgr_.remove(loginName);
		if(infos == NULL)
			return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onLoginFailed);
	(*pBundle) << failedcode;
	(*pBundle).appendBlob(datas);

	if(pChannel)
	{
		pChannel->send(pBundle);
	}
	else 
	{
		if(infos)
		{
			Network::Channel* pClientChannel = this->networkInterface().findChannel(infos->addr);
			if(pClientChannel)
				pClientChannel->send(pBundle);
			else
				Network::Bundle::reclaimPoolObject(pBundle);
		}
		else
		{
			ERROR_MSG(fmt::format("Loginapp::_loginFailed: infos({}) is NULL!\n", 
				loginName));

			Network::Bundle::reclaimPoolObject(pBundle);
		}
	}

	SAFE_RELEASE(infos);
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryResultFromDbmgr(Network::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	std::string loginName, accountName, password, datas;
	SERVER_ERROR_CODE retcode = SERVER_SUCCESS;
	COMPONENT_ID componentID;
	ENTITY_ID entityID;
	DBID dbid;
	uint32 flags;
	uint64 deadline;

	s >> retcode;

	// ��¼���ȵ�¼ʱ�ͻ�����������ƣ� �˺�������dbmgr��ѯ�õ�������
	// �����������һ���˺Ŷ�����ϵͳ���߶���������˺�ϵͳ���������
	// accountNameΪ����Ϸ�������˺����󶨵���������
	// �ͻ��˵õ�baseapp��ַ��ͬʱҲ�᷵������˺�����
	// �ͻ��˵�½baseappӦ��ʹ������˺����Ƶ�½
	s >> loginName;
	s >> accountName;

	s >> password;
	s >> componentID;
	s >> entityID;
	s >> dbid;
	s >> flags;
	s >> deadline;

	s.readBlob(datas);

	//DEBUG_MSG(fmt::format("Loginapp::onLoginAccountQueryResultFromDbmgr: loginName={}.\n",
	//	loginName));

	if((flags & ACCOUNT_FLAG_LOCK) > 0)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_ACCOUNT_LOCK, datas);
		return;
	}

	if((flags & ACCOUNT_FLAG_NOT_ACTIVATED) > 0)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_ACCOUNT_NOT_ACTIVATED, datas);
		return;
	}

	if(deadline > 0 && ::time(NULL) - deadline <= 0)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_ACCOUNT_DEADLINE, datas);
		return;
	}

	PendingLoginMgr::PLInfos* infos = pendingLoginMgr_.find(loginName);
	if(infos == NULL)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_SRV_OVERLOAD, datas);
		return;
	}

	// �������ɽű�����
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onLoginCallbackFromDB"), 
										const_cast<char*>("ssHy#"), 
										loginName.c_str(),
										accountName.c_str(),
										retcode,
										datas.c_str(), datas.length());

	if(pyResult != NULL)
	{
		Py_DECREF(pyResult);
	}
	else
	{
		SCRIPT_ERROR_CHECK();
	}
	
	infos->datas = datas;

	Network::Channel* pClientChannel = this->networkInterface().findChannel(infos->addr);
	if(pClientChannel)
		pClientChannel->extra("");

	if(retcode != SERVER_SUCCESS && entityID == 0 && componentID == 0)
	{
		_loginFailed(NULL, loginName, retcode, datas);
		return;
	}

	// ���baseappmgr��ַ��
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::ComponentInfos* baseappmgrinfos = NULL;
	if(cts.size() > 0)
		baseappmgrinfos = &(*cts.begin());

	if(baseappmgrinfos == NULL || baseappmgrinfos->pChannel == NULL || baseappmgrinfos->cid == 0)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_SRV_NO_READY, datas);
		return;
	}

	// �������0��˵����ǰ�˺���Ȼ�����ĳ��baseapp��
	if(componentID > 0)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(BaseappmgrInterface::registerPendingAccountToBaseappAddr);
		(*pBundle) << componentID << loginName << accountName << password << entityID << dbid << flags << deadline << infos->ctype << infos->forceInternalLogin;
		(*pBundle).appendBlob(infos->datas);
		baseappmgrinfos->pChannel->send(pBundle);
		return;
	}
	else
	{
		// ע�ᵽbaseapp���һ�ȡbaseapp�ĵ�ַ
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(BaseappmgrInterface::registerPendingAccountToBaseapp);

		(*pBundle) << loginName;
		(*pBundle) << accountName;
		(*pBundle) << password;
		(*pBundle) << dbid;
		(*pBundle) << flags;
		(*pBundle) << deadline;
		(*pBundle) << infos->ctype;
		(*pBundle) << infos->forceInternalLogin;
		(*pBundle).appendBlob(infos->datas);
		baseappmgrinfos->pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr(Network::Channel* pChannel, std::string& loginName, 
															std::string& accountName, std::string& addr, uint16 port)
{
	if(pChannel->isExternal())
		return;
	
	if(addr.size() == 0)
	{
		ERROR_MSG(fmt::format("Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr:accountName={}, not found baseapp, Please check the baseappmgr errorlog!\n", 
			loginName));
		
		std::string datas;
		_loginFailed(NULL, loginName, SERVER_ERR_SRV_NO_READY, datas);
	}

	Network::Address address(addr, ntohs(port));

	DEBUG_MSG(fmt::format("Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr:accountName={0}, addr={1}.\n", 
		loginName, address.c_str()));

	// ������Բ���ɾ���� ��Ȼʹ�䱣��һ��ʱ�����ͬһʱ��ͬʱ��¼�������Ӱ��
	PendingLoginMgr::PLInfos* infos = pendingLoginMgr_.remove(loginName);
	if(infos == NULL)
		return;
	
	infos->lastProcessTime = timestamp();
	Network::Channel* pClientChannel = this->networkInterface().findChannel(infos->addr);

	if(pClientChannel == NULL)
	{
		SAFE_RELEASE(infos);
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onLoginSuccessfully);
	uint16 fport = ntohs(port);
	(*pBundle) << accountName;
	(*pBundle) << addr;
	(*pBundle) << fport;
	(*pBundle).appendBlob(infos->datas);
	pClientChannel->send(pBundle);

	SAFE_RELEASE(infos);
}

//-------------------------------------------------------------------------------------
void Loginapp::onHello(Network::Channel* pChannel, 
						const std::string& verInfo, 
						const std::string& scriptVerInfo, 
						const std::string& encryptedKey)
{
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	
	pBundle->newMessage(ClientInterface::onHelloCB);
	(*pBundle) << KBEVersion::versionString();
	(*pBundle) << KBEVersion::scriptVersionString();
	(*pBundle) << Network::MessageHandlers::getDigestStr();
	(*pBundle) << digest_;
	(*pBundle) << g_componentType;

	// ����Ϣ��������ܣ������趨�Ѽ��ܺ����ٴμ��ܣ�����һ��send��Ϣ���������������ǽ���epoll֪ͨʱ��������������һ�����ڲ��ԣ����滷��������֣�
	// webЭ�����Ҫ���ܣ����Բ�������Ϊtrue
	if (pChannel->type() != KBEngine::Network::Channel::CHANNEL_WEB)
		pBundle->pCurrPacket()->encrypted(true);

	pChannel->send(pBundle);

	if(Network::g_channelExternalEncryptType > 0)
	{
		if(encryptedKey.size() > 3)
		{
			// �滻Ϊһ�����ܵĹ�����
			pChannel->pFilter(Network::createEncryptionFilter(Network::g_channelExternalEncryptType, encryptedKey));
		}
		else
		{
			WARNING_MSG(fmt::format("Loginapp::onHello: client is not encrypted, addr={}\n"
				, pChannel->c_str()));
		}
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::onVersionNotMatch(Network::Channel* pChannel)
{
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	
	pBundle->newMessage(ClientInterface::onVersionNotMatch);
	(*pBundle) << KBEVersion::versionString();
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Loginapp::onScriptVersionNotMatch(Network::Channel* pChannel)
{
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	
	pBundle->newMessage(ClientInterface::onScriptVersionNotMatch);
	(*pBundle) << KBEVersion::scriptVersionString();
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Loginapp::importClientMessages(Network::Channel* pChannel)
{
	static Network::Bundle bundle;
	
	if(bundle.empty())
	{
		std::map< Network::MessageID, Network::ExposedMessageInfo > clientMessages;
		{
			const Network::MessageHandlers::MessageHandlerMap& msgHandlers = ClientInterface::messageHandlers.msgHandlers();
			Network::MessageHandlers::MessageHandlerMap::const_iterator iter = msgHandlers.begin();
			for(; iter != msgHandlers.end(); ++iter)
			{
				Network::MessageHandler* pMessageHandler = iter->second;

				Network::ExposedMessageInfo& info = clientMessages[iter->first];
				info.id = iter->first;
				info.name = pMessageHandler->name;
				info.msgLen = pMessageHandler->msgLen;
				info.argsType = (int8)pMessageHandler->pArgs->type();

				KBEngine::strutil::kbe_replace(info.name, "::", "_");
				std::vector<std::string>::iterator iter1 = pMessageHandler->pArgs->strArgsTypes.begin();
				for(; iter1 !=  pMessageHandler->pArgs->strArgsTypes.end(); ++iter1)
				{
					info.argsTypes.push_back((uint8)datatype2id((*iter1)));
				}
			}
		}

		std::map< Network::MessageID, Network::ExposedMessageInfo > messages;
		{
			const Network::MessageHandlers::MessageHandlerMap& msgHandlers = LoginappInterface::messageHandlers.msgHandlers();
			Network::MessageHandlers::MessageHandlerMap::const_iterator iter = msgHandlers.begin();
			for(; iter != msgHandlers.end(); ++iter)
			{
				Network::MessageHandler* pMessageHandler = iter->second;
				if(!iter->second->exposed)
					continue;

				Network::ExposedMessageInfo& info = messages[iter->first];
				info.id = iter->first;
				info.name = pMessageHandler->name;
				info.msgLen = pMessageHandler->msgLen;
				
				KBEngine::strutil::kbe_replace(info.name, "::", "_");
				std::vector<std::string>::iterator iter1 = pMessageHandler->pArgs->strArgsTypes.begin();
				for(; iter1 !=  pMessageHandler->pArgs->strArgsTypes.end(); ++iter1)
				{
					info.argsTypes.push_back((uint8)datatype2id((*iter1)));
				}
			}
		}
	
		bundle.newMessage(ClientInterface::onImportClientMessages);
		
		uint16 size = (uint16)(messages.size() + clientMessages.size());
		bundle << size;

		std::map< Network::MessageID, Network::ExposedMessageInfo >::iterator iter = clientMessages.begin();
		for(; iter != clientMessages.end(); ++iter)
		{
			uint8 argsize = (uint8)iter->second.argsTypes.size();
			bundle << iter->second.id << iter->second.msgLen << iter->second.name << iter->second.argsType << argsize;

			std::vector<uint8>::iterator argiter = iter->second.argsTypes.begin();
			for(; argiter != iter->second.argsTypes.end(); ++argiter)
			{
				bundle << (*argiter);
			}
		}

		iter = messages.begin();
		for(; iter != messages.end(); ++iter)
		{
			uint8 argsize = (uint8)iter->second.argsTypes.size();
			bundle << iter->second.id << iter->second.msgLen << iter->second.name << iter->second.argsType << argsize;

			std::vector<uint8>::iterator argiter = iter->second.argsTypes.begin();
			for(; argiter != iter->second.argsTypes.end(); ++argiter)
			{
				bundle << (*argiter);
			}
		}
	}

	pChannel->send(new Network::Bundle(bundle));
}

//-------------------------------------------------------------------------------------
void Loginapp::importServerErrorsDescr(Network::Channel* pChannel)
{
	static Network::Bundle bundle;
	
	if(bundle.empty())
	{
		std::map<uint16, std::pair< std::string, std::string> > errsDescrs;

		TiXmlNode *rootNode = NULL;
		SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes("server/server_errors.xml").c_str()));

		if(!xml->isGood())
		{
			ERROR_MSG(fmt::format("ServerConfig::loadConfig: load {} is failed!\n",
				"server/server_errors.xml"));

			return;
		}

		rootNode = xml->getRootNode();
		if(rootNode == NULL)
		{
			// root�ڵ���û���ӽڵ���
			return;
		}

		XML_FOR_BEGIN(rootNode)
		{
			TiXmlNode* node = xml->enterNode(rootNode->FirstChild(), "id");
			TiXmlNode* node1 = xml->enterNode(rootNode->FirstChild(), "descr");
			errsDescrs[xml->getValInt(node)] = std::make_pair< std::string, std::string>(xml->getKey(rootNode), xml->getVal(node1));
		}
		XML_FOR_END(rootNode);

		bundle.newMessage(ClientInterface::onImportServerErrorsDescr);
		std::map<uint16, std::pair< std::string, std::string> >::iterator iter = errsDescrs.begin();
		uint16 size = (uint16)errsDescrs.size();

		bundle << size;
		for(; iter != errsDescrs.end(); ++iter)
		{
			bundle << iter->first;
			bundle.appendBlob(iter->second.first.data(), (ArraySize)iter->second.first.size());
			bundle.appendBlob(iter->second.second.data(), (ArraySize)iter->second.second.size());
		}
	}

	pChannel->send(new Network::Bundle(bundle));
}

//-------------------------------------------------------------------------------------
void Loginapp::onBaseappInitProgress(Network::Channel* pChannel, float progress)
{
	if(progress > 1.f)
	{
		INFO_MSG(fmt::format("Loginapp::onBaseappInitProgress: progress={}.\n", 
			(progress > 1.f ? 1.f : progress)));
	}

	initProgress_ = progress;
}

//-------------------------------------------------------------------------------------

}
