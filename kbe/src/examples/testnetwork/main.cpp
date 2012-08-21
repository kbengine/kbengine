/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#include "cstdkbe/cstdkbe.hpp"
#include "network/address.hpp"
#include "network/endpoint.hpp"
#include "network/event_poller.hpp"
#include "helper/debug_helper.hpp"
#include "network/event_dispatcher.hpp"
#include "network/interfaces.hpp"
#include "network/tcp_packet.hpp"
#include "network/error_reporter.hpp"
#include "network/bundle.hpp"
#include "network/fixed_messages.hpp"
#include "network/common.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"

#include "log4cxx/logger.h"
#include "log4cxx/propertyconfigurator.h"

using namespace KBEngine;
using namespace KBEngine::Mercury;
Address address;
EndPoint mysocket;
EventDispatcher gdispatcher;

class MyPacketReceiver : public InputNotificationHandler
{
public:
	MyPacketReceiver(EndPoint & mysocket):
	socket_(mysocket),
	pNextPacket_(new TCPPacket())
	{
	}
	
	~MyPacketReceiver()
	{
	}
	
	EventDispatcher& dispatcher(){return gdispatcher;}
private:
	virtual int handleInputNotification(int fd)
	{
		if (this->processSocket(/*expectingPacket:*/true))
		{
			while (this->processSocket(/*expectingPacket:*/false))
			{
				/* pass */;
			}
		}

		return 0;
	}
	
	bool processSocket(bool expectingPacket)
	{
		int len = pNextPacket_->recvFromEndPoint(socket_);
		if (len <= 0)
		{
			return this->checkSocketErrors(len, expectingPacket);
		}

		PacketPtr curPacket = pNextPacket_;
		pNextPacket_ = new TCPPacket();
		Address srcAddr = socket_.getRemoteAddress();
		Reason ret = this->processPacket(srcAddr, curPacket.get());

		if (ret != REASON_SUCCESS)
		{
			this->dispatcher().errorReporter().reportException(ret, srcAddr);
		}
		return true;
	}

	Reason processPacket(const Address & addr, Packet * p)
	{
		return REASON_SUCCESS;
	}
	
	bool checkSocketErrors(int len, bool expectingPacket)
	{
		// is len weird?
		if (len == 0)
		{
			WARNING_MSG("PacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK (1)- %s\n",
				kbe_strerror());

			this->dispatcher().errorReporter().reportException(
					REASON_GENERAL_NETWORK);

			return true;
		}
			// I'm not quite sure what it means if len is 0
			// (0 => 'end of file', but with dgram sockets?)

	#ifdef _WIN32
		DWORD wsaErr = WSAGetLastError();
	#endif //def _WIN32

		// is the buffer empty?
		if (
	#ifdef _WIN32
			wsaErr == WSAEWOULDBLOCK
	#else
			errno == EAGAIN && !expectingPacket
	#endif
			)
		{
			return false;
		}

	#ifdef unix
		// is it telling us there's an error?
		if (errno == EAGAIN ||
			errno == ECONNREFUSED ||
			errno == EHOSTUNREACH)
		{
	#if defined(PLAYSTATION3)
			this->dispatcher().errorReporter().reportException(
					REASON_NO_SUCH_PORT);
			return true;
	#else
			Mercury::Address offender;

			if (socket_.getClosedPort(offender))
			{
				// If we got a NO_SUCH_PORT error and there is an internal
				// channel to this address, mark it as remote failed.  The logic
				// for dropping external channels that get NO_SUCH_PORT
				// exceptions is built into BaseApp::onClientNoSuchPort().
				if (errno == ECONNREFUSED)
				{
				}

				this->dispatcher().errorReporter().reportException(
						REASON_NO_SUCH_PORT, offender);

				return true;
			}
			else
			{
				WARNING_MSG("PacketReceiver::processPendingEvents: "
					"getClosedPort() failed\n");
			}
	#endif
		}
	#else
		if (wsaErr == WSAECONNRESET)
		{
			return true;
		}
	#endif // unix

		// ok, I give up, something's wrong
	#ifdef _WIN32
		WARNING_MSG("PacketReceiver::processPendingEvents: "
					"Throwing REASON_GENERAL_NETWORK - %d\n",
					wsaErr);
	#else
		WARNING_MSG("PacketReceiver::processPendingEvents: "
					"Throwing REASON_GENERAL_NETWORK - %s\n",
				kbe_strerror());
	#endif
		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK);

		return true;
	}
private:
	EndPoint & socket_;
	PacketPtr pNextPacket_;
};

MyPacketReceiver* packetReceiver;

struct AvatarInfos
{
	uint32 dbid;
	std::string name;
};


void init_network(void)
{
	Mercury::g_trace_packet = 3;

	mysocket.close();
	mysocket.socket(SOCK_DGRAM);
	mysocket.setbroadcast(true);
	if (!mysocket.good())
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: couldn't create a socket\n");
		return;
	}
	
	srand(getSystemTime());
	std::string accountname = "kebiao";
	char ttt1[256];
	memset(ttt1, 0, 256);
	int nnn = rand() % 65535;
	sprintf(ttt1, "%d", nnn);
	accountname += ttt1;

	while(1)
	{
		mysocket.close();
		mysocket.socket(SOCK_STREAM);
		if (!mysocket.good())
		{
			ERROR_MSG("NetworkInterface::recreateListeningSocket: couldn't create a socket\n");
			return;
		}
		
		packetReceiver = new MyPacketReceiver(mysocket);
		gdispatcher.registerFileDescriptor(mysocket, packetReceiver);

		printf("请输入服务器端口号:\n>>>");
		static int port = 0;
		if(port == 0)
			std::cin >> port;
		
		// 连接游戏登陆进程
		printf("连接游戏登陆进程\n");
		u_int32_t address;
		std::string ip = "192.168.4.44";
		mysocket.convertAddress(ip.c_str(), address );
		if(mysocket.connect(htons(port), address) == -1)
		{
			ERROR_MSG("NetworkInterface::recreateListeningSocket: connect server is error(%s)!\n", kbe_strerror());
			port = 0;
			continue;
		}
		
		// 请求创建账号
		printf("请求创建账号\n");
	//	mysocket.setnodelay(false);
		mysocket.setnonblocking(false);
		MessageID msgID = 0;
		MessageLength msgLength = 0;
		Mercury::Bundle bundle1;
		bundle1.newMessage(LoginappInterface::reqCreateAccount);

		std::string avatarname = "kebiao";
		char ttt[256];
		memset(ttt, 0, 256);
		sprintf(ttt, "%d", nnn++);
		avatarname += ttt;
		bundle1 << accountname;
		bundle1 << "123456";
		bundle1.send(mysocket);
		//::sleep(300);

		// 创建账号成功 failedcode == 0
		TCPPacket packet1;
		packet1.resize(65535);
		int len = mysocket.recv(packet1.data(), 65535);
		packet1.wpos(len);
		uint16 failedcode = 0;
		packet1 >> msgID;
		packet1 >> failedcode;
		printf("Client::onCreateAccountResult: 创建账号%s size(%d) failedcode=%u.\n", 
			failedcode == 0 ? "成功" : "失败",len, failedcode);

		// 提交账号密码请求登录
		printf("提交账号密码请求登录\n");
		Mercury::Bundle bundle2;
		bundle2.newMessage(LoginappInterface::login);
		int8 tclient = 1;
		bundle2 << tclient;
		bundle2 << "phone";
		bundle2 << accountname;
		bundle2 << "123456";
		bundle2.send(mysocket);


		// 获取返回的网关ip地址
		TCPPacket packet2;
		packet2.resize(65535);
		
		len = -1;
		len = mysocket.recv(packet2.data(), 65535);
		packet2.wpos(len);
		packet2.print_storage();
		uint16 iport;
		packet2 >> msgID;
		packet2 >> msgLength;
		packet2 >> ip;
		packet2 >> iport;
		printf("Client::onLoginSuccessfully: 获取返回的网关ip地址 size(%d) msgID=%u, ip:%s, port=%u.\n", len, msgID, ip.c_str(), iport);

		// 连接网关
		printf("连接网关\n");
		mysocket.close();
		mysocket.socket(SOCK_STREAM);
		mysocket.convertAddress(ip.c_str(), address );
		if(mysocket.connect(htons(iport), address) == -1)
		{
			printf("NetworkInterface::recreateListeningSocket: connect server is error(%s)!\n", kbe_strerror());
			port = 0;
			continue;
		}
		
		mysocket.setnonblocking(false);

		// 请求登录网关
		Mercury::Bundle bundle3;
		bundle3.newMessage(BaseappInterface::loginGateway);
		bundle3 << accountname;
		bundle3 << "123456";
		bundle3.send(mysocket);
		//::sleep(300);

		// 服务器返回 Client::onCreatedProxies:服务器端已经创建了一个与客户端关联的代理Entity
		TCPPacket packet33;
		packet33.resize(65535);
		len = mysocket.recv(packet33.data(), 65535);
		packet33.wpos(len);

		uint64 uuid;
		ENTITY_ID eid;
		std::string entityType;
		MessageLength msgLen;
		packet33 >> msgID;

		if(msgID == 505) // 登录失败
		{
			packet33 >> failedcode;
			printf("登录网关失败:msgID=%u, err=%u\n", msgID, failedcode);
			continue;
		}
		else
		{
			packet33 >> msgLen;
			packet33 >> uuid;
			packet33 >> eid;
			packet33 >> entityType;
			printf("Client::onCreatedProxies: size(%d) : msgID=%u, uuid:%"PRIu64", eid=%d, entityType=%s.\n", 
				len, msgID, uuid, eid, entityType.c_str());
		}
		
		printf("向服务器请求查询角色列表\n");
		// 向服务器请求查询角色列表
		Mercury::Bundle bundle44;
		bundle44.newMessage(BaseappInterface::onRemoteMethodCall);
		uint16 methodID = 10001;
		bundle44 << eid;
		bundle44 << methodID;
		bundle44.send(mysocket);
		//::sleep(300);

		// 开始接收列表
		TCPPacket packet444;
		packet444.resize(65535);
		len = mysocket.recv(packet444.data(), 65535);
		packet444.wpos(len);
		packet444 >> msgID;
		packet444 >> msgLen;
		packet444 >> eid;
		packet444 >> methodID;
		uint32 size;

		std::vector<AvatarInfos> vargs;

		packet444 >> size;
		for(uint32 i=0; i<size; i++)
		{
			AvatarInfos ainfo;
			packet444 >> ainfo.dbid;
			packet444 >> ainfo.name;
			vargs.push_back(ainfo);
			printf("接收角色列表:dbid=%u,name=%s\n", ainfo.dbid, ainfo.name.c_str());
		}

		printf("向服务器请求创建角色:%s\n", avatarname.c_str());

		// 向服务器请求创建角色
		Mercury::Bundle bundle55;
		bundle55.newMessage(BaseappInterface::onRemoteMethodCall);
		methodID = 10002;
		bundle55 << eid;
		bundle55 << methodID;
		bundle55 << avatarname;
		bundle55.send(mysocket);
		//::sleep(3000);

		// 开始接收创建结果
		TCPPacket packet555;
		packet555.resize(65535);
		len = mysocket.recv(packet555.data(), 65535);
		packet555.wpos(len);
		packet555 >> msgID;
		packet555 >> msgLen;
		packet555 >> eid;
		packet555 >> methodID;
		// 错误码
		uint8 errorcode = 0;
		packet555 >> errorcode;
		AvatarInfos retainfo;
		packet555 >> retainfo.dbid;
		packet555 >> retainfo.name;
		printf("创建角色结果:错误码:%u,dbid=%u,name=%s\n", errorcode, retainfo.dbid, retainfo.name.c_str());

		printf("向服务器请求选择某个角色进行游戏\n");
		// 向服务器请求选择某个角色进行游戏
		Mercury::Bundle bundle66;
		bundle66.newMessage(BaseappInterface::onRemoteMethodCall);
		methodID = 10004;
		bundle66 << eid;
		bundle66 << methodID;
		bundle66 << retainfo.dbid;
		bundle66.send(mysocket);
		//::sleep(3000);

		// 服务器端告知可销毁客户端账号entity了
		TCPPacket packet77;
		packet77.resize(65535);
		len = mysocket.recv(packet77.data(), 65535);
		packet77.wpos(len);
		//packet77 >> msgID;
		//packet77 >> eid;
		//printf("Client::onEntityDestroyed: 服务器端告知可销毁客户端账号entity了 size(%d) : msgID=%u, eid=%d.\n", 
		//	len, msgID, eid);
		 
		//::sleep(3000);

		// 服务器返回 Client::onCreatedProxies:服务器端已经创建了一个与客户端关联的代理player
		//TCPPacket packet88;
		//len = mysocket.recv(packet88.data(), 65535);
		packet77 >> msgID;
		packet77 >> msgLen;
		packet77 >> uuid;
		packet77 >> eid;
		packet77 >> entityType;
		printf("Client::onCreatedProxies: size(%d) : msgID=%u, uuid:%"PRIu64", eid=%d, entityType=%s.\n", 
			len, msgID, uuid, eid, entityType.c_str());

		//::sleep(5000);
	};
}

int main(int argc, char* argv[])
{
	Mercury::FixedMessages::getSingleton().loadConfig("../../res/server/fixed_mercury_messages.xml");
	DebugHelper::initHelper(UNKNOWN_COMPONENT_TYPE);
    INFO_MSG("你好，log4cxx---%d!---%s", 1, __FUNCTION__);
	//LOG4CXX_INFO("Attempted to " << " in MemoryStream (pos:" << 111 <<  "size: " << 222 << ").\n");
	init_network();
	gdispatcher.processUntilBreak();
	getchar();
	return 0; 
}
