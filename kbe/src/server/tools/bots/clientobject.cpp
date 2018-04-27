// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "bots.h"
#include "clientobject.h"
#include "network/common.h"
#include "network/message_handler.h"
#include "network/tcp_packet.h"
#include "network/bundle.h"
#include "network/fixed_messages.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "server/serverconfig.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/entitydef.h"
#include "client_lib/client_interface.h"
#include "common/kbeversion.h"

#include "baseapp/baseapp_interface.h"
#include "cellapp/cellapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "loginapp/loginapp_interface.h"


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
error_(C_ERROR_NONE),
state_(C_STATE_INIT),
pBlowfishFilter_(0),
pTCPPacketSenderEx_(NULL),
pTCPPacketReceiverEx_(NULL)
{
	name_ = name;
	typeClient_ = CLIENT_TYPE_BOTS;
	clientDatas_ = "bots";
	password_ = ServerConfig::getSingleton().getBots().bots_account_passwd;
}

//-------------------------------------------------------------------------------------
ClientObject::~ClientObject()
{
	SAFE_RELEASE(pBlowfishFilter_);
}

//-------------------------------------------------------------------------------------		
void ClientObject::finalise(void)
{
	reset();
	ClientObjectBase::finalise();
}

//-------------------------------------------------------------------------------------		
void ClientObject::reset(void)
{
	if(pTCPPacketReceiverEx_)
		Bots::getSingleton().networkInterface().dispatcher().deregisterReadFileDescriptor(*pTCPPacketReceiverEx_->pEndPoint());

	if(pServerChannel_ && pServerChannel_->pEndPoint())
	{
		pServerChannel_->stopSend();
		pServerChannel_->pPacketSender(NULL);
	}

	SAFE_RELEASE(pTCPPacketSenderEx_);
	SAFE_RELEASE(pTCPPacketReceiverEx_);

	std::string name = name_;
	std::string passwd = password_;
	ClientObjectBase::reset();
	
	name_ = name;
	password_ = passwd;
	clientDatas_ = "bots";
	state_ = C_STATE_INIT;
}

//-------------------------------------------------------------------------------------
bool ClientObject::initCreate()
{
	Network::EndPoint* pEndpoint = Network::EndPoint::createPoolObject();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObject::initNetwork: couldn't create a socket\n");
		Network::EndPoint::reclaimPoolObject(pEndpoint);
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	ENGINE_COMPONENT_INFO& infos = g_kbeSrvConfig.getBots();
	u_int32_t address;

	Network::Address::string2ip(infos.login_ip, address);
	if(pEndpoint->connect(htons(infos.login_port), address) == -1)
	{
		ERROR_MSG(fmt::format("ClientObject::initNetwork({1}): connect server({2}:{3}) is error({0})!\n",
			kbe_strerror(), name_, infos.login_ip, infos.login_port));

		Network::EndPoint::reclaimPoolObject(pEndpoint);
		// error_ = C_ERROR_INIT_NETWORK_FAILED;
		state_ = C_STATE_INIT;
		return false;
	}

	Network::Address addr(infos.login_ip, infos.login_port);
	pEndpoint->addr(addr);

	pServerChannel_->pEndPoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	pServerChannel_->pMsgHandlers(&ClientInterface::messageHandlers);

	pTCPPacketSenderEx_ = new Network::TCPPacketSenderEx(*pEndpoint, this->networkInterface_, this);
	pTCPPacketReceiverEx_ = new Network::TCPPacketReceiverEx(*pEndpoint, this->networkInterface_, this);
	Bots::getSingleton().networkInterface().dispatcher().registerReadFileDescriptor((*pEndpoint), pTCPPacketReceiverEx_);
	
	//不在这里注册
	//Bots::getSingleton().networkInterface().dispatcher().registerWriteFileDescriptor((*pEndpoint), pTCPPacketSenderEx_);
	pServerChannel_->pPacketSender(pTCPPacketSenderEx_);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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

	pEndpoint->send(pBundle);
	Network::Bundle::reclaimPoolObject(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::initLoginBaseapp()
{
	if(pTCPPacketReceiverEx_)
		Bots::getSingleton().networkInterface().dispatcher().deregisterReadFileDescriptor(*pTCPPacketReceiverEx_->pEndPoint());

	pServerChannel_->stopSend();
	pServerChannel_->pPacketSender(NULL);

	SAFE_RELEASE(pTCPPacketSenderEx_);
	SAFE_RELEASE(pTCPPacketReceiverEx_);

	Network::EndPoint* pEndpoint = Network::EndPoint::createPoolObject();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObject::initLogin: couldn't create a socket\n");
		Network::EndPoint::reclaimPoolObject(pEndpoint);
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	u_int32_t address;

	Network::Address::string2ip(ip_.c_str(), address);
	if(pEndpoint->connect(htons(port_), address) == -1)
	{
		ERROR_MSG(fmt::format("ClientObject::initLogin({}): connect server is error({})!\n",
			kbe_strerror(), name_));

		Network::EndPoint::reclaimPoolObject(pEndpoint);
		// error_ = C_ERROR_INIT_NETWORK_FAILED;
		state_ = C_STATE_LOGIN_BASEAPP_CREATE;
		return false;
	}

	Network::Address addr(ip_.c_str(), port_);
	pEndpoint->addr(addr);
	pServerChannel_->pEndPoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	pTCPPacketSenderEx_ = new Network::TCPPacketSenderEx(*pEndpoint, this->networkInterface_, this);
	pTCPPacketReceiverEx_ = new Network::TCPPacketReceiverEx(*pEndpoint, this->networkInterface_, this);
	Bots::getSingleton().networkInterface().dispatcher().registerReadFileDescriptor((*pEndpoint), pTCPPacketReceiverEx_);

	//不在这里注册
	//Bots::getSingleton().networkInterface().dispatcher().registerWriteFileDescriptor((*pEndpoint), pTCPPacketSenderEx_);
	pServerChannel_->pPacketSender(pTCPPacketSenderEx_);

	connectedBaseapp_ = true;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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

	pEndpoint->send(pBundle);
	Network::Bundle::reclaimPoolObject(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObject::gameTick()
{
	if(pServerChannel()->pEndPoint())
	{
		if(pServerChannel()->isCondemn())
		{
			destroy();
			return;
		}
		
		pServerChannel()->processPackets(NULL);
	}
	else
	{
		if(connectedBaseapp_)
		{
			EventData_ServerCloased eventdata;
			eventHandler_.fire(&eventdata);
			connectedBaseapp_ = false;
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
		case C_STATE_LOGIN_BASEAPP_CREATE:

			state_ = C_STATE_PLAY;

			if(!initLoginBaseapp())
				return;

			break;
		case C_STATE_LOGIN_BASEAPP:

			state_ = C_STATE_PLAY;

			if(!loginBaseapp())
				return;

			break;
		case C_STATE_PLAY:
			break;	
		case C_STATE_DESTROYED:
			return;
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
		pServerChannel_->pFilter(pBlowfishFilter_);
		pBlowfishFilter_ = NULL;
	}

	if(componentType == LOGINAPP_TYPE)
	{
		state_ = C_STATE_CREATE;
	}
	else
	{
		state_ = C_STATE_LOGIN_BASEAPP;
	}
}

//-------------------------------------------------------------------------------------
void ClientObject::onCreateAccountResult(Network::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;

	s >> retcode;
	s.readBlob(serverDatas_);

	if(retcode != 0)
	{
		//error_ = C_ERROR_CREATE_FAILED;

		// 继续尝试登录
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
	s.readBlob(serverDatas_);

	INFO_MSG(fmt::format("ClientObject::onLoginSuccessfully: {} addr={}:{}!\n", 
		name_, ip_, port_));

	state_ = C_STATE_LOGIN_BASEAPP_CREATE;
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginFailed(Network::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;

	s >> failedcode;
	s.readBlob(serverDatas_);

	INFO_MSG(fmt::format("ClientObject::onLoginFailed: {} failedcode={}!\n", 
		name_, SERVER_ERR_STR[failedcode]));

	// error_ = C_ERROR_LOGIN_FAILED;

	// 继续尝试登录
	state_ = C_STATE_LOGIN;
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginBaseappFailed(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	ClientObjectBase::onLoginBaseappFailed(pChannel, failedcode);
	destroy();
}

//-------------------------------------------------------------------------------------
void ClientObject::onLogin(Network::Bundle* pBundle)
{
}

//-------------------------------------------------------------------------------------
}
