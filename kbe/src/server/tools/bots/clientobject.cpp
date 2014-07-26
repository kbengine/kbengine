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
#include "clientobject.hpp"
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
#include "entitydef/entitydef.hpp"
#include "client_lib/client_interface.hpp"
#include "cstdkbe/kbeversion.hpp"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"


namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientObject)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientObject)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientObject)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientObject, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
ClientObject::ClientObject(std::string name, Mercury::NetworkInterface& ninterface):
ClientObjectBase(ninterface, getScriptType()),
Mercury::TCPPacketReceiver(),
error_(C_ERROR_NONE),
state_(C_STATE_INIT),
pBlowfishFilter_(0)
{
	name_ = name;
	typeClient_ = CLIENT_TYPE_BOTS;
	extradatas_ = "bots";

	this->pNetworkInterface_ = &ninterface;
}

//-------------------------------------------------------------------------------------
ClientObject::~ClientObject()
{
	SAFE_RELEASE(pBlowfishFilter_);
}

//-------------------------------------------------------------------------------------
bool ClientObject::initCreate()
{
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObject::initNetwork: couldn't create a socket\n");
		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	ENGINE_COMPONENT_INFO& infos = g_kbeSrvConfig.getBots();
	u_int32_t address;

	pEndpoint->convertAddress(infos.login_ip, address);
	if(pEndpoint->connect(htons(infos.login_port), address) == -1)
	{
		ERROR_MSG(boost::format("ClientObject::initNetwork: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}

	Mercury::Address addr(infos.login_ip, infos.login_port);
	pEndpoint->addr(addr);

	pServerChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	pServerChannel_->pMsgHandlers(&ClientInterface::messageHandlers);
	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::hello);
	(*pBundle) << KBEVersion::versionString();

	if(Mercury::g_channelExternalEncryptType == 1)
	{
		pBlowfishFilter_ = new Mercury::BlowfishFilter();
		(*pBundle).appendBlob(pBlowfishFilter_->key());
	}
	else
	{
		std::string key = "";
		(*pBundle).appendBlob(key);
	}

	pServerChannel_->pushBundle(pBundle);

	this->pEndpoint_ = pEndpoint;
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::processSocket(bool expectingPacket)
{
	
	Mercury::TCPPacket* pReceiveWindow = Mercury::TCPPacket::ObjPool().createObject();
	int len = pReceiveWindow->recvFromEndPoint(*pServerChannel_->endpoint());

	if (len < 0)
	{
		Mercury::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);

		if(rstate == Mercury::PacketReceiver::RECV_STATE_INTERRUPT)
		{
			Bots::getSingleton().pEventPoller()->deregisterForRead(*pServerChannel_->endpoint());
			pServerChannel_->destroy();
			Bots::getSingleton().delClient(this);
			return false;
		}

		return rstate == Mercury::PacketReceiver::RECV_STATE_CONTINUE;
	}
	else if(len == 0) // 客户端正常退出
	{
		Mercury::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		Bots::getSingleton().pEventPoller()->deregisterForRead(*pServerChannel_->endpoint());
		pServerChannel_->destroy();
		Bots::getSingleton().delClient(this);
		return false;
	}

	Mercury::Reason ret = this->processPacket(pServerChannel_, pReceiveWindow);

	if(ret != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG(boost::format("ClientObject::processSocket: "
					"Throwing %1%\n") %
					Mercury::reasonToString(ret));
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::initLoginGateWay()
{
	Bots::getSingleton().pEventPoller()->deregisterForRead(*pServerChannel_->endpoint());
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObject::initLogin: couldn't create a socket\n");
		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	u_int32_t address;

	pEndpoint->convertAddress(ip_.c_str(), address);
	if(pEndpoint->connect(htons(port_), address) == -1)
	{
		ERROR_MSG(boost::format("ClientObject::initLogin: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}

	Mercury::Address addr(ip_.c_str(), port_);
	pEndpoint->addr(addr);

	pServerChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);
	connectedGateway_ = true;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::hello);
	(*pBundle) << KBEVersion::versionString();
	
	if(Mercury::g_channelExternalEncryptType == 1)
	{
		pBlowfishFilter_ = new Mercury::BlowfishFilter();
		(*pBundle).appendBlob(pBlowfishFilter_->key());
		pServerChannel_->pFilter(NULL);
	}
	else
	{
		std::string key = "";
		(*pBundle).appendBlob(key);
	}

	pServerChannel_->pushBundle(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObject::gameTick()
{
	if(pServerChannel()->endpoint())
	{
		pServerChannel()->processPackets(NULL);
	}
	else
	{
		if(connectedGateway_)
		{
			EventData_ServerCloased eventdata;
			eventHandler_.fire(&eventdata);
			connectedGateway_ = false;
			canReset_ = true;
		}
	}

	switch(state_)
	{
		case C_STATE_INIT:

			state_ = C_STATE_PLAY;

			if(!initCreate())
				return;

			break;
		case C_STATE_CREATE:

			state_ = C_STATE_PLAY;

			if(!createAccount())
				return;

			break;
		case C_STATE_LOGIN:

			state_ = C_STATE_PLAY;

			if(!login())
				return;

			break;
		case C_STATE_LOGIN_GATEWAY_CREATE:

			state_ = C_STATE_PLAY;

			if(!initLoginGateWay())
				return;

			break;
		case C_STATE_LOGIN_GATEWAY:

			state_ = C_STATE_PLAY;

			if(!loginGateWay())
				return;

			break;
		case C_STATE_PLAY:
			break;
		default:
			KBE_ASSERT(false);
			break;
	};

	tickSend();
}

//-------------------------------------------------------------------------------------	
void ClientObject::onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo, 
		COMPONENT_TYPE componentType)
{
	if(Mercury::g_channelExternalEncryptType == 1)
	{
		pChannel->pFilter(pBlowfishFilter_);
		pBlowfishFilter_ = NULL;
	}

	if(componentType == LOGINAPP_TYPE)
	{
		state_ = C_STATE_CREATE;
	}
	else
	{
		state_ = C_STATE_LOGIN_GATEWAY;
	}
}

//-------------------------------------------------------------------------------------
void ClientObject::onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;

	s >> retcode;
	s.readBlob(extradatas_);

	if(retcode != 0)
	{
		//error_ = C_ERROR_CREATE_FAILED;

		// 继续尝试登录
		state_ = C_STATE_LOGIN;
		INFO_MSG(boost::format("ClientObject::onCreateAccountResult: %1% create is failed! code=%2%.\n") % name_ % SERVER_ERR_STR[retcode]);
		return;
	}

	state_ = C_STATE_LOGIN;
	INFO_MSG(boost::format("ClientObject::onCreateAccountResult: %1% create is successfully!\n") % name_);
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
	std::string accountName;

	s >> accountName;
	s >> ip_;
	s >> port_;
	s.readBlob(extradatas_);

	INFO_MSG(boost::format("ClientObject::onLoginSuccessfully: %1% addr=%2%:%3%!\n") % name_ % ip_ % port_);

	state_ = C_STATE_LOGIN_GATEWAY_CREATE;
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;

	s >> failedcode;
	s.readBlob(extradatas_);

	INFO_MSG(boost::format("ClientObject::onLoginFailed: %1% failedcode=%2%!\n") % name_ % SERVER_ERR_STR[failedcode]);

	error_ = C_ERROR_LOGIN_FAILED;
}

//-------------------------------------------------------------------------------------
}
