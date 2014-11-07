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
ClientObject::ClientObject(std::string name, Network::NetworkInterface& ninterface):
ClientObjectBase(ninterface, getScriptType()),
Network::TCPPacketReceiver(),
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
void ClientObject::reset(void)
{
	if(pServerChannel_ && pServerChannel_->endpoint())
		Bots::getSingleton().pEventPoller()->deregisterForRead(*pServerChannel_->endpoint());

	std::string name = name_;
	std::string passwd = password_;
	ClientObjectBase::reset();
	
	name_ = name;
	password_ = passwd;
	extradatas_ = "bots";
	state_ = C_STATE_INIT;
}

//-------------------------------------------------------------------------------------
bool ClientObject::initCreate()
{
	Network::EndPoint* pEndpoint = new Network::EndPoint();
	
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
		ERROR_MSG(fmt::format("ClientObject::initNetwork({1}): connect server({2}:{3}) is error({0})!\n",
			kbe_strerror(), name_, infos.login_ip, infos.login_port));

		delete pEndpoint;
		// error_ = C_ERROR_INIT_NETWORK_FAILED;
		state_ = C_STATE_INIT;
		return false;
	}

	Network::Address addr(infos.login_ip, infos.login_port);
	pEndpoint->addr(addr);

	pServerChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	pServerChannel_->pMsgHandlers(&ClientInterface::messageHandlers);
	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::hello);
	(*pBundle) << KBEVersion::versionString() << KBEVersion::scriptVersionString();

	if(Network::g_channelExternalEncryptType == 1)
	{
		pBlowfishFilter_ = new Network::BlowfishFilter();
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
	
	Network::TCPPacket* pReceiveWindow = Network::TCPPacket::ObjPool().createObject();
	int len = pReceiveWindow->recvFromEndPoint(*pServerChannel_->endpoint());

	if (len < 0)
	{
		Network::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);

		if(rstate == Network::PacketReceiver::RECV_STATE_INTERRUPT)
		{
			Bots::getSingleton().pEventPoller()->deregisterForRead(*pServerChannel_->endpoint());
			pServerChannel_->destroy();
			Bots::getSingleton().delClient(this);
			return false;
		}

		return rstate == Network::PacketReceiver::RECV_STATE_CONTINUE;
	}
	else if(len == 0) // ¿Í»§¶ËÕý³£ÍË³ö
	{
		Network::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		Bots::getSingleton().pEventPoller()->deregisterForRead(*pServerChannel_->endpoint());
		pServerChannel_->destroy();
		Bots::getSingleton().delClient(this);
		return false;
	}

	Network::Reason ret = this->processPacket(pServerChannel_, pReceiveWindow);

	if(ret != Network::REASON_SUCCESS)
	{
		ERROR_MSG(fmt::format("ClientObject::processSocket: "
					"Throwing {}\n",
					Network::reasonToString(ret)));
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::initLoginGateWay()
{
	Bots::getSingleton().pEventPoller()->deregisterForRead(*pServerChannel_->endpoint());
	Network::EndPoint* pEndpoint = new Network::EndPoint();
	
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
		ERROR_MSG(fmt::format("ClientObject::initLogin({}): connect server is error({})!\n",
			kbe_strerror(), name_));

		delete pEndpoint;
		// error_ = C_ERROR_INIT_NETWORK_FAILED;
		state_ = C_STATE_LOGIN_GATEWAY_CREATE;
		return false;
	}

	Network::Address addr(ip_.c_str(), port_);
	pEndpoint->addr(addr);

	pServerChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);
	connectedGateway_ = true;

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::hello);
	(*pBundle) << KBEVersion::versionString() << KBEVersion::scriptVersionString();
	
	if(Network::g_channelExternalEncryptType == 1)
	{
		pBlowfishFilter_ = new Network::BlowfishFilter();
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
			state_ = C_STATE_INIT;
			
			DEBUG_MSG(fmt::format("ClientObject({})::tickSend: serverCloased! name({})!\n", 
			this->appID(), this->name()));
		}
	}

	if(locktime() > 0 && timestamp() < locktime())
	{
		return;
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
void ClientObject::onHelloCB_(Network::Channel* pChannel, const std::string& verInfo, 
		const std::string& scriptVerInfo, const std::string& protocolMD5, const std::string& entityDefMD5, 
		COMPONENT_TYPE componentType)
{
	if(Network::g_channelExternalEncryptType == 1)
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
void ClientObject::onCreateAccountResult(Network::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;

	s >> retcode;
	s.readBlob(extradatas_);

	if(retcode != 0)
	{
		//error_ = C_ERROR_CREATE_FAILED;

		// ¼ÌÐø³¢ÊÔµÇÂ¼
		state_ = C_STATE_LOGIN;
		
		INFO_MSG(fmt::format("ClientObject::onCreateAccountResult: {} create is failed! code={}.\n", 
			name_, SERVER_ERR_STR[retcode]));
		
		return;
	}

	state_ = C_STATE_LOGIN;
	INFO_MSG(fmt::format("ClientObject::onCreateAccountResult: {} create is successfully!\n", name_));
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginSuccessfully(Network::Channel * pChannel, MemoryStream& s)
{
	std::string accountName;

	s >> accountName;
	s >> ip_;
	s >> port_;
	s.readBlob(extradatas_);

	INFO_MSG(fmt::format("ClientObject::onLoginSuccessfully: {} addr={}:{}!\n", 
		name_, ip_, port_));

	state_ = C_STATE_LOGIN_GATEWAY_CREATE;
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginFailed(Network::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;

	s >> failedcode;
	s.readBlob(extradatas_);

	INFO_MSG(fmt::format("ClientObject::onLoginFailed: {} failedcode={}!\n", 
		name_, SERVER_ERR_STR[failedcode]));

	// error_ = C_ERROR_LOGIN_FAILED;

	// ¼ÌÐø³¢ÊÔµÇÂ¼
	state_ = C_STATE_LOGIN;
}

//-------------------------------------------------------------------------------------
}
