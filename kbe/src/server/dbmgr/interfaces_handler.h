/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_INTERFACES_HANDLER_H
#define KBE_INTERFACES_HANDLER_H

// common include	
// #define NDEBUG
#include "dbtasks.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"
#include "thread/threadpool.h"

namespace KBEngine{ 

namespace Network{
	class EndPoint;
}

/*
	处理计费、第三方运营账号、注册登录系统等挂接
*/
class InterfacesHandler
{
public:
	InterfacesHandler();
	virtual ~InterfacesHandler();
	
	virtual bool initialize() = 0;

	virtual void eraseClientReq(Network::Channel* pChannel, std::string& logkey) = 0;

	virtual bool createAccount(Network::Channel* pChannel, std::string& registerName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE uatype) = 0;


	virtual bool loginAccount(Network::Channel* pChannel, std::string& loginName, 
		std::string& password, std::string& datas) = 0;

	virtual void onCreateAccountCB(KBEngine::MemoryStream& s) = 0;

	virtual void onLoginAccountCB(KBEngine::MemoryStream& s) = 0;

	virtual void charge(Network::Channel* pChannel, KBEngine::MemoryStream& s) = 0;
	virtual void onChargeCB(KBEngine::MemoryStream& s) = 0;

	virtual void accountActivate(Network::Channel* pChannel, std::string& scode) = 0;
	virtual void accountReqResetPassword(Network::Channel* pChannel, std::string& accountName) = 0;
	virtual void accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode) = 0;
	virtual void accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& password, std::string& email) = 0;
	virtual void accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode) = 0;
	virtual void accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& password, std::string& newpassword) = 0;

protected:
};

class InterfacesHandler_Dbmgr : public InterfacesHandler
{
public:
	InterfacesHandler_Dbmgr();
	virtual ~InterfacesHandler_Dbmgr();
	
	virtual bool initialize(){ return true; }

	virtual void eraseClientReq(Network::Channel* pChannel, std::string& logkey);

	virtual bool createAccount(Network::Channel* pChannel, std::string& registerName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE uatype);

	virtual bool loginAccount(Network::Channel* pChannel, std::string& loginName, 
		std::string& password, std::string& datas);

	virtual void onCreateAccountCB(KBEngine::MemoryStream& s);

	virtual void onLoginAccountCB(KBEngine::MemoryStream& s);

	virtual void charge(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	virtual void onChargeCB(KBEngine::MemoryStream& s);

	virtual void accountActivate(Network::Channel* pChannel, std::string& scode);
	virtual void accountReqResetPassword(Network::Channel* pChannel, std::string& accountName);
	virtual void accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode);
	virtual void accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& password, std::string& email);
	virtual void accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode);
	virtual void accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& password, std::string& newpassword);

protected:
};

class InterfacesHandler_Interfaces : public InterfacesHandler_Dbmgr, public thread::TPTask
{
public:
	InterfacesHandler_Interfaces();
	virtual ~InterfacesHandler_Interfaces();
	
	virtual bool initialize();

	virtual void eraseClientReq(Network::Channel* pChannel, std::string& logkey);

	virtual bool createAccount(Network::Channel* pChannel, std::string& registerName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE uatype);

	virtual bool loginAccount(Network::Channel* pChannel, std::string& loginName, 
		std::string& password, std::string& datas);

	virtual void onCreateAccountCB(KBEngine::MemoryStream& s);

	virtual void onLoginAccountCB(KBEngine::MemoryStream& s);

	virtual void charge(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	virtual void onChargeCB(KBEngine::MemoryStream& s);

	virtual void accountActivate(Network::Channel* pChannel, std::string& scode);
	virtual void accountReqResetPassword(Network::Channel* pChannel, std::string& accountName);
	virtual void accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode);
	virtual void accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& password, std::string& email);
	virtual void accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode);
	virtual void accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& password, std::string& newpassword);

	bool reconnect();

	virtual bool process();

protected:

};

class InterfacesHandlerFactory
{
public:
	static InterfacesHandler* create(std::string type);
};

}

#endif // KBE_INTERFACES_HANDLER_H
