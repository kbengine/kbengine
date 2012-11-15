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
Client::Client(const Mercury::Address& addr):
s_(0),
addr_(addr),
endpoint_(),
name_(),
password_(),
getewayAddr_()
{
}

//-------------------------------------------------------------------------------------
Client::~Client()
{
}

//-------------------------------------------------------------------------------------
bool Client::send(Mercury::Bundle& bundle)
{
	Mercury::Channel* pChannel = Bots::getSingleton().getNetworkInterface().findChannel(addr_);
	
	if(pChannel){
		bundle.send(Bots::getSingleton().getNetworkInterface(), pChannel);
	}
	else{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Client::initNetwork()
{
	endpoint_.socket(SOCK_STREAM);
	if (!endpoint_.good())
	{
		ERROR_MSG("Client::initNetwork: couldn't create a socket\n");
		return false;
	}
	
	ENGINE_COMPONENT_INFO& infos = g_kbeSrvConfig.getBots();
	u_int32_t address;

	endpoint_.convertAddress(infos.login_ip, address);
	if(endpoint_.connect(htons(infos.login_port), address) == -1)
	{
		ERROR_MSG("Client::initNetwork: connect server is error(%s)!\n", kbe_strerror());
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Client::login()
{
	endpoint_.setnonblocking(false);

	Mercury::MessageID msgID = 0;
	Mercury::MessageLength msgLength = 0;

	char tmp[256];
	sprintf(tmp, "%"PRIu64, KBEngine::genUUID64());
	name_ = tmp;
	password_ = "123456";

	Mercury::Bundle bundle;

	// 创建账号
	bundle.newMessage(LoginappInterface::reqCreateAccount);
	bundle << name_;
	bundle << password_;
	bundle.send(endpoint_);

	// 接收创建结果
	// 创建账号成功 failedcode == 0
	Mercury::TCPPacket packet;
	packet.resize(65535);
	int len = endpoint_.recv(packet.data(), 65535);

	packet.wpos(len);
	uint16 failedcode = 0;
	packet >> msgID;
	packet >> failedcode;
	DEBUG_MSG("Client::onCreateAccountResult: 创建账号[%s]%s size(%d) failedcode=%u.\n", 
		name_.c_str(), failedcode == 0 ? "成功" : "失败", len, failedcode);
	
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
	DEBUG_MSG("Client::onLoginSuccessfully: 获取返回的网关ip地址 size(%d) msgID=%u, ip:%s, port=%u.\n", 
		len, msgID, ip.c_str(), iport);

	// 连接网关
	endpoint_.close();
	endpoint_.socket(SOCK_STREAM);
	u_int32_t address;
	endpoint_.convertAddress(ip.c_str(), address);
	getewayAddr_.ip = address;
	getewayAddr_.port = htons(iport);
	if(endpoint_.connect(getewayAddr_.port, getewayAddr_.ip) == -1)
	{
		DEBUG_MSG("Client::login: connect server is error(%s)!\n", kbe_strerror());
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
		DEBUG_MSG("登录网关失败:msgID=%u, err=%u\n", msgID, failedcode);
		return false;
	}
	else
	{
		packet >> msgLength;
		packet >> uuid;
		packet >> eid;
		packet >> entityType;
		DEBUG_MSG("Client::onCreatedProxies: size(%d) : msgID=%u, uuid:%"PRIu64", eid=%d, entityType=%s.\n", 
			len, msgID, uuid, eid, entityType.c_str());
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Client::messageloop()
{
	endpoint_.setnonblocking(true);

	fd_set fds;
	//Mercury::MessageID msgID = 0;
	//Mercury::MessageLength msgLength = 0;

	while (true)
	{
		FD_ZERO( &fds );
		FD_SET((int)endpoint_, &fds);
		int selgot = select(endpoint_+1, &fds, NULL, NULL, NULL);

		if (selgot == 0)
		{
			DEBUG_MSG("Client::process: is failed!\n");

			break;
		}
		else if (selgot == -1)
		{
			ERROR_MSG("Client::process: select error. %s.\n",
					kbe_strerror());

			break;
		}
		else
		{
			sockaddr_in	sin;

			Mercury::TCPPacket packet;
			int len = endpoint_.recvfrom(packet.data(), PACKET_MAX_SIZE_TCP, sin);
			if (len == -1)
			{
				ERROR_MSG("Client::process: recvfrom error. %s.\n",
						kbe_strerror());

				break;
			}
			
			DEBUG_MSG("Client::process: from %s, datalen=%d.\n", inet_ntoa((struct in_addr&)sin.sin_addr.s_addr), len);
			break;

		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Client::process()
{
	if(!initNetwork())
		return false;
	
	messageloop();

	return false;
}

//-------------------------------------------------------------------------------------
}
