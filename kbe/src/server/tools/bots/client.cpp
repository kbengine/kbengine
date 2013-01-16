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
#include "bots.hpp"
#include "client.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "network/tcp_packet.hpp"
#include "network/bundle.hpp"
#include "network/fixed_messages.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "client_lib/client_interface.hpp"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"


namespace KBEngine{

//-------------------------------------------------------------------------------------
Client::Client(std::string name):
pChannel_(new Mercury::Channel()),
name_(name),
password_(),
error_(C_ERROR_NONE),
state_(C_STATE_INIT),
entityID_(0),
dbid_(0),
ip_(),
port_(),
lastSentActiveTickTime_(timestamp()),
connectedGateway_(false)
{
	pChannel_->incRef();
}

//-------------------------------------------------------------------------------------
Client::~Client()
{
	pChannel_->decRef();
}

//-------------------------------------------------------------------------------------
bool Client::initCreate()
{
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("Client::initNetwork: couldn't create a socket\n");
		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	ENGINE_COMPONENT_INFO& infos = g_kbeSrvConfig.getBots();
	u_int32_t address;

	pEndpoint->convertAddress(infos.login_ip, address);
	if(pEndpoint->connect(htons(infos.login_port), address) == -1)
	{
		ERROR_MSG(boost::format("Client::initNetwork: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}

	Mercury::Address addr(infos.login_ip, infos.login_port);
	pEndpoint->addr(addr);

	pChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	pChannel_->pMsgHandlers(&ClientInterface::messageHandlers);
	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);
	return true;
}

//-------------------------------------------------------------------------------------
bool Client::processSocket(bool expectingPacket)
{
	
	Mercury::TCPPacket* pReceiveWindow = Mercury::TCPPacket::ObjPool().createObject();
	int len = pReceiveWindow->recvFromEndPoint(*pChannel_->endpoint());

	if (len < 0)
	{
		Mercury::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);

		if(rstate == Mercury::PacketReceiver::RECV_STATE_INTERRUPT)
		{
			Bots::getSingleton().pEventPoller()->deregisterForRead(*pChannel_->endpoint());
			pChannel_->destroy();
			Bots::getSingleton().delClient(this);
			return false;
		}

		return rstate == Mercury::PacketReceiver::RECV_STATE_CONTINUE;
	}
	else if(len == 0) // 客户端正常退出
	{
		Mercury::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		Bots::getSingleton().pEventPoller()->deregisterForRead(*pChannel_->endpoint());
		pChannel_->destroy();
		Bots::getSingleton().delClient(this);
		return false;
	}

	pChannel_->addReceiveWindow(pReceiveWindow);
	Mercury::Reason ret = this->processPacket(pChannel_, pReceiveWindow);

	if(ret != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG(boost::format("Client::processSocket: "
					"Throwing %1%\n") %
					Mercury::reasonToString(ret));
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Client::createAccount()
{
	// 创建账号
	Mercury::Bundle bundle;
	bundle.newMessage(LoginappInterface::reqCreateAccount);
	bundle << name_;
	bundle << password_;
	bundle.send(*pChannel_->endpoint());
	return true;
}

//-------------------------------------------------------------------------------------
void Client::onCreateAccountResult(MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;
	std::string datas = "";

	s >> retcode;
	s.readBlob(datas);

	if(retcode != 0)
	{
		error_ = C_ERROR_CREATE_FAILED;
		INFO_MSG(boost::format("Client::onCreateAccountResult: %1% create is failed! code=%2%.\n") % name_ % retcode);
		return;
	}

	state_ = C_STATE_LOGIN;
	INFO_MSG(boost::format("Client::onCreateAccountResult: %1% create is successfully!\n") % name_);
}

//-------------------------------------------------------------------------------------
bool Client::login()
{
	if(error_ != C_ERROR_NONE)
		return false;

	Mercury::Bundle bundle;

	std::string bindatas = "bots client";

	// 提交账号密码请求登录
	bundle.newMessage(LoginappInterface::login);
	CLIENT_CTYPE tclient = CLIENT_TYPE_BOTS;
	bundle << tclient;
	bundle.appendBlob(bindatas);
	bundle << name_;
	bundle << password_;
	bundle.send(*pChannel_->endpoint());
	return true;
}

//-------------------------------------------------------------------------------------
bool Client::initLoginGateWay()
{
	Bots::getSingleton().pEventPoller()->deregisterForRead(*pChannel_->endpoint());
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("Client::initLogin: couldn't create a socket\n");
		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	u_int32_t address;

	pEndpoint->convertAddress(ip_.c_str(), address);
	if(pEndpoint->connect(htons(port_), address) == -1)
	{
		ERROR_MSG(boost::format("Client::initLogin: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}

	Mercury::Address addr(ip_.c_str(), port_);
	pEndpoint->addr(addr);

	pChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);
	return true;
}

//-------------------------------------------------------------------------------------
bool Client::loginGateWay()
{
	// 请求登录网关
	Mercury::Bundle bundle;
	bundle.newMessage(BaseappInterface::loginGateway);
	bundle << name_;
	bundle << password_;
	bundle.send(*pChannel_->endpoint());
	return true;
}

//-------------------------------------------------------------------------------------
void Client::gameTick()
{
	if(pChannel()->endpoint())
	{
		pChannel()->handleMessage(NULL);
		sendTick();
	}

	switch(state_)
	{
		case C_STATE_INIT:

			state_ = C_STATE_PLAY;

			if(!initCreate() || !createAccount())
				return;

			break;
		case C_STATE_LOGIN:

			state_ = C_STATE_PLAY;

			if(!login())
				return;

			break;
		case C_STATE_LOGIN_GATEWAY:

			state_ = C_STATE_PLAY;

			if(!initLoginGateWay() || !loginGateWay())
				return;

			break;
		case C_STATE_PLAY:
			break;
		default:
			KBE_ASSERT(false);
			break;
	};
}

//-------------------------------------------------------------------------------------
void Client::sendTick()
{
	// 向服务器发送tick
	uint64 check = uint64( Mercury::g_channelExternalTimeout * stampsPerSecond() ) / 2;
	if (timestamp() - lastSentActiveTickTime_ > check)
	{
		lastSentActiveTickTime_ = timestamp();

		Mercury::Bundle bundle;
		if(connectedGateway_)
			bundle.newMessage(BaseappInterface::onClientActiveTick);
		else
			bundle.newMessage(LoginappInterface::onClientActiveTick);
		bundle.send(*pChannel_->endpoint());
	}
}

//-------------------------------------------------------------------------------------
bool Client::process()
{
	return false;
}

//-------------------------------------------------------------------------------------	
void Client::onLoginSuccessfully(MemoryStream& s)
{
	std::string accountName, datas;

	s >> accountName;
	s >> ip_;
	s >> port_;
	s.readBlob(datas);

	INFO_MSG(boost::format("Client::onLoginSuccessfully: %1% addr=%2%:%3%!\n") % name_ % ip_ % port_);

	state_ = C_STATE_LOGIN_GATEWAY;
}

//-------------------------------------------------------------------------------------	
void Client::onLoginFailed(MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;
	std::string datas;

	s >> failedcode;
	s.readBlob(datas);

	INFO_MSG(boost::format("Client::onLoginFailed: %1% failedcode=%2%!\n") % name_ % failedcode);

	error_ = C_ERROR_LOGIN_FAILED;
}

//-------------------------------------------------------------------------------------	
void Client::onLoginGatewayFailed(SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(boost::format("Client::onLoginGatewayFailed: %1% failedcode=%2%!\n") % name_ % failedcode);
}

//-------------------------------------------------------------------------------------	
void Client::onCreatedProxies(uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	connectedGateway_ = true;

	entityID_ = eid;
	INFO_MSG(boost::format("Client::onCreatedProxies(%1%): rndUUID=%2% eid=%3% entityType=%4%!\n") % 
		name_ % rndUUID % eid % entityType);
}

//-------------------------------------------------------------------------------------	
void Client::onCreatedEntity(ENTITY_ID eid, std::string& entityType)
{
}

//-------------------------------------------------------------------------------------	
void Client::onEntityGetCell(ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------	
void Client::onEntityEnterWorld(ENTITY_ID eid, SPACE_ID spaceID)
{
}

//-------------------------------------------------------------------------------------	
void Client::onEntityLeaveWorld(ENTITY_ID eid, SPACE_ID spaceID)
{
}

//-------------------------------------------------------------------------------------	
void Client::onEntityEnterSpace(SPACE_ID spaceID, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------	
void Client::onEntityLeaveSpace(SPACE_ID spaceID, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------	
void Client::onEntityDestroyed(ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------
void Client::onRemoteMethodCall(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void Client::onUpdatePropertys(MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
}
