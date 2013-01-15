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


#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"


namespace KBEngine{

//-------------------------------------------------------------------------------------
Client::Client(std::string name):
pChannel_(NULL),
name_(name),
password_()
{
}

//-------------------------------------------------------------------------------------
Client::~Client()
{
}

//-------------------------------------------------------------------------------------
bool Client::initNetwork()
{
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("Client::initNetwork: couldn't create a socket\n");
		delete pEndpoint;
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
		return false;
	}

	Mercury::Address addr(infos.login_ip, infos.login_port);
	pEndpoint->addr(addr);
	pChannel_ = new Mercury::Channel(Bots::getSingleton().getNetworkInterface(), pEndpoint, Mercury::Channel::EXTERNAL);

	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	if(!Bots::getSingleton().getNetworkInterface().registerChannel(pChannel_))
		return false;

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
	bundle.send(Bots::getSingleton().getNetworkInterface(), pChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
bool Client::login()
{
	/*
	Mercury::MessageID msgID = 0;
	Mercury::MessageLength msgLength = 0;

	Mercury::Bundle bundle;


	// 接收创建结果
	// 创建账号成功 failedcode == 0
	Mercury::TCPPacket packet;
	packet.resize(65535);
	int len = endpoint_.recv(packet.data(), 65535);

	packet.wpos(len);
	uint16 failedcode = 0;
	packet >> msgID;
	packet >> failedcode;
	DEBUG_MSG(boost::format("Client::onCreateAccountResult: 创建账号[%s]%s size(%d) failedcode=%u.\n") % 
		name_.c_str() % (failedcode == 0 ? "成功" : "失败") % len % failedcode);
	
	if(failedcode > 0)
		return false;

	// 提交账号密码请求登录
	Mercury::Bundle bundle2;
	bundle2.newMessage(LoginappInterface::login);
	int8 tclient = 1;
	bundle2 << tclient;
	bundle2 << "phone";
	bundle2 << name_;
	bundle2 << password_;
	bundle2.send(endpoint_);

	// 获取返回的网关ip地址
	packet.clear(false);
	len = endpoint_.recv(packet.data(), 65535);
	packet.wpos(len);
	uint16 iport;
	std::string ip;
	packet >> msgID;
	packet >> msgLength;
	packet >> ip;
	packet >> iport;
	DEBUG_MSG(boost::format("Client::onLoginSuccessfully: 获取返回的网关ip地址 size(%d) msgID=%u, ip:%s, port=%u.\n") %
		len % msgID % ip.c_str() % iport);
	
	// 连接网关
	endpoint_.close();
	endpoint_.socket(SOCK_STREAM);
	u_int32_t address;
	endpoint_.convertAddress(ip.c_str(), address);
	getewayAddr_.ip = address;
	getewayAddr_.port = htons(iport);
	if(endpoint_.connect(getewayAddr_.port, getewayAddr_.ip) == -1)
	{
		DEBUG_MSG(boost::format("Client::login: connect server is error(%s)!\n") % kbe_strerror());
		return false;
	}
	
	endpoint_.setnonblocking(false);

	// 请求登录网关
	Mercury::Bundle bundle3;
	bundle3.newMessage(BaseappInterface::loginGateway);
	bundle3 << name_;
	bundle3 << password_;
	bundle3.send(endpoint_);

	// 服务器返回 Client::onCreatedProxies:服务器端已经创建了一个与客户端关联的代理Entity
	packet.clear(false);
	len = endpoint_.recv(packet.data(), 65535);
	packet.wpos(len);

	uint64 uuid;
	ENTITY_ID eid;
	std::string entityType;
	packet >> msgID;

	Mercury::FixedMessages::MSGInfo* msgInfo =
				Mercury::FixedMessages::getSingleton().isFixed("Client::onLoginGatewayFailed");

	if(msgID == msgInfo->msgid) // 登录失败
	{
		packet >> failedcode;
		DEBUG_MSG(boost::format("登录网关失败:msgID=%u, err=%u\n") % msgID % failedcode);
		return false;
	}
	else
	{
		packet >> msgLength;
		packet >> uuid;
		packet >> eid;
		packet >> entityType;
		DEBUG_MSG(boost::format("Client::onCreatedProxies: size(%1%) : msgID=%2%, uuid:%3%, eid=%4%, entityType=%5%.\n") %
			len % msgID % uuid % eid % entityType.c_str());
	}

	*/
	return true;
}


//-------------------------------------------------------------------------------------
bool Client::process()
{
	return false;
}

//-------------------------------------------------------------------------------------
}
