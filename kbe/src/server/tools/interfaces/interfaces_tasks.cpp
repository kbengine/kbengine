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

#include "orders.h"
#include "interfaces.h"
#include "interfaces_tasks.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "server/serverconfig.h"
#include "openssl/md5.h"

#include "baseapp/baseapp_interface.h"
#include "cellapp/cellapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "loginapp/loginapp_interface.h"
#include "dbmgr/dbmgr_interface.h"

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
InterfacesTask::InterfacesTask()
{
}

//-------------------------------------------------------------------------------------
InterfacesTask::~InterfacesTask()
{
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState InterfacesTask::presentMainThread()
{
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
CreateAccountTask::CreateAccountTask():
InterfacesTask()
{
}

//-------------------------------------------------------------------------------------
CreateAccountTask::~CreateAccountTask()
{
}

//-------------------------------------------------------------------------------------
void CreateAccountTask::removeLog()
{
	Interfaces::getSingleton().lockthread();
	Interfaces::getSingleton().reqCreateAccount_requests().erase(commitName);
	Interfaces::getSingleton().unlockthread();
}

//-------------------------------------------------------------------------------------
bool CreateAccountTask::process()
{
	retcode = SERVER_ERR_OP_FAILED;

	if(!enable)
	{
		return false;
	}

	// 如果没有设置第三方服务地址则我们默认为成功
	if(strlen(serviceAddr()) == 0)
	{
		retcode = SERVER_SUCCESS;
		getDatas = postDatas;
		return false;
	}

	Network::EndPoint endpoint;
	endpoint.socket(SOCK_STREAM);

	if (!endpoint.good())
	{
		ERROR_MSG("InterfacesTask::process: couldn't create a socket\n");
		return false;
	}

	if(postDatas.size() == 0)
	{
		ERROR_MSG(fmt::format("InterfacesTask::process: {} postData is NULL.\n", commitName));
		return false;
	}

	u_int32_t addr;
	KBEngine::Network::EndPoint::convertAddress(serviceAddr(), addr);

	if(endpoint.connect(htons(servicePort()), addr) == -1)
	{
		ERROR_MSG(fmt::format("InterfacesTask::process: connect billingserver({}:{}) is error({})!\n", 
			serviceAddr(), servicePort(), kbe_strerror()));

		endpoint.close();
		return false;
	}

	endpoint.setnonblocking(true);
	endpoint.setnodelay(true);

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();
	(*(*bundle)).append(postDatas.data(), postDatas.size());
	(*(*bundle)).send(endpoint);

	Network::TCPPacket packet;
	packet.resize(1024);

	fd_set	frds;
	struct timeval tv = { 0, 5000000 }; // 5000ms

	FD_ZERO( &frds );
	FD_SET((int)endpoint, &frds);
	int selgot = select(endpoint+1, &frds, NULL, NULL, &tv);
	if(selgot <= 0)
	{
		ERROR_MSG(fmt::format("InterfacesTask::process: {} send({}).\n", commitName, postDatas));
		ERROR_MSG(fmt::format("InterfacesTask::process: {} recv is error({}).\n", commitName, KBEngine::kbe_strerror()));
		endpoint.close();
		return false;
	}
	
	int error;

	if(FD_ISSET(endpoint, &frds))
	{
		socklen_t len = sizeof(error);

#if KBE_PLATFORM == PLATFORM_WIN32
		if( getsockopt(endpoint, SOL_SOCKET, SO_ERROR, (char*)&error, &len) != 0)
		{
			ERROR_MSG(fmt::format("InterfacesTask::process: {} send({}).\n", commitName, postDatas));
			ERROR_MSG(fmt::format("InterfacesTask::process: {} recv is error({}).\n", commitName, error));
			endpoint.close();
			return false;
		}
#else
		if( getsockopt(endpoint, SOL_SOCKET, SO_ERROR, &error, &len) >= 0)
		{
			ERROR_MSG(fmt::format("InterfacesTask::process: {} send({}).\n", commitName, postDatas));
			ERROR_MSG(fmt::format("InterfacesTask::process: {} recv is error({}).\n", commitName, error));
			endpoint.close();
			return false;
		}
#endif
	}

	int len = endpoint.recv(packet.data(), 1024);

	if(len <= 0)
	{
		ERROR_MSG(fmt::format("InterfacesTask::process: {} recv is size<= 0.\n===>postdatas={}\n", commitName, postDatas));
		endpoint.close();
		return false;
	}

	packet.wpos(len);

	getDatas.assign((const char *)(packet.data() + packet.rpos()), packet.length());

	try
	{
		std::string::size_type fi = getDatas.find("\r\n\r\n");
		if(fi != std::string::npos)
		{
			fi += 4;
			MemoryStream s;
			s.append(getDatas.data() + fi, getDatas.size() - fi);

			while(s.length() > 0)
			{
				int32 type, len;
				s >> type >> len;
				EndianConvertReverse<int32>(type);
				EndianConvertReverse<int32>(len);
				
				int32 error = 0;

				switch(type)
				{
				case 1:
					s >> error;
					EndianConvertReverse<int32>(error);

					if(error != 0)
					{
						retcode = SERVER_ERR_OP_FAILED;
						endpoint.close();
						
						std::string err;
						if(s.length() >= (sizeof(int32) * 2))
						{
							s >> type >> len;
							
							if(len > 0 && len < 1024)
							{
								char* buf = new char[len + 1];
								memcpy(buf, s.data() + s.rpos(), len);
								buf[len] = 0;
								err = buf;
								delete[] buf;
							}

						}

						DEBUG_MSG(fmt::format("InterfacesTask::process: ({})op is failed! err={}\n<==send({})\n==>recv({}).\n", 
							commitName, err, postDatas, getDatas));
						
						return false;
					}
					else
					{
						retcode = SERVER_SUCCESS;
					}

					break;
				case 2:
					{
						s.read_skip(len);
					}
					break;
				case 3:
					{
						char* buf = new char[len + 1];
						memcpy(buf, s.data() + s.rpos(), len);
						buf[len] = 0;
						accountName = buf;
						delete[] buf;

						s.read_skip(len);
					}
					break;
				default:
					break;
				};
			}
		}
	}
	catch(...)
	{
		retcode = SERVER_ERR_OP_FAILED;
		ERROR_MSG(fmt::format("InterfacesTask::process: {} recv is error.\n===>postdatas={}\n===>recv={}\n", 
			commitName, postDatas, getDatas));
	}

	endpoint.close();
	enable = true;
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState CreateAccountTask::presentMainThread()
{
	if(!enable)
	{
		removeLog();
		DEBUG_MSG(fmt::format("CreateAccountTask::presentMainThread: commitName={} is disable!\n", commitName));
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(DbmgrInterface::onCreateAccountCBFromInterfaces);
	(*(*bundle)) << baseappID << commitName << accountName << password << retcode;

	(*(*bundle)).appendBlob(postDatas);
	(*(*bundle)).appendBlob(getDatas);

	Network::Channel* pChannel = Interfaces::getSingleton().networkInterface().findChannel(address);

	if(pChannel)
	{
		(*(*bundle)).send(Interfaces::getSingleton().networkInterface(), pChannel);
	}
	else
	{
		ERROR_MSG(fmt::format("InterfacesTask::presentMainThread: not found channel. commitName={}\n", commitName));
	}

	removeLog();
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
LoginAccountTask::LoginAccountTask():
CreateAccountTask()
{
}

//-------------------------------------------------------------------------------------
LoginAccountTask::~LoginAccountTask()
{
}

//-------------------------------------------------------------------------------------
void LoginAccountTask::removeLog()
{
	Interfaces::getSingleton().lockthread();
	Interfaces::getSingleton().reqAccountLogin_requests().erase(commitName);
	Interfaces::getSingleton().unlockthread();
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState LoginAccountTask::presentMainThread()
{
	if(!enable)
	{
		removeLog();
		DEBUG_MSG(fmt::format("LoginAccountTask::presentMainThread: commitName={} is disable!\n", commitName));
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();
	
	if(retcode == SERVER_ERR_OP_FAILED)
	{
		if(accountName.size() == 0)
			accountName = commitName;
	}

	(*(*bundle)).newMessage(DbmgrInterface::onLoginAccountCBBFromInterfaces);
	(*(*bundle)) << baseappID << commitName << accountName << password << retcode;

	(*(*bundle)).appendBlob(postDatas);
	(*(*bundle)).appendBlob(getDatas);

	Network::Channel* pChannel = Interfaces::getSingleton().networkInterface().findChannel(address);

	if(pChannel)
	{
		(*(*bundle)).send(Interfaces::getSingleton().networkInterface(), pChannel);
	}
	else
	{
		ERROR_MSG(fmt::format("InterfacesTask::presentMainThread: not found channel. commitName={}\n", commitName));
	}

	removeLog();
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
ChargeTask::ChargeTask():
InterfacesTask(),
pOrders(NULL),
retcode(SERVER_ERR_OP_FAILED)
{
}

//-------------------------------------------------------------------------------------
ChargeTask::~ChargeTask()
{
}

//-------------------------------------------------------------------------------------
bool ChargeTask::process()
{
	if(!Interfaces::getSingleton().hasOrders(orders.ordersID))
	{
		WARNING_MSG(fmt::format("ChargeTask::process: not found ordersID({}), exit threadProcess.\n",
			orders.ordersID));
		return false;
	}
	
	KBE_ASSERT(pOrders != NULL);

	pOrders->state = Orders::STATE_FAILED;
	orders.state = pOrders->state;
	orders.postDatas = pOrders->postDatas;

	// 如果是不需要请求的直接返回成功
	if(pOrders->postDatas.size() == 0)
	{
		retcode = SERVER_SUCCESS;
		return false;
	}

	// 如果没有设置第三方服务地址则我们默认为成功
	if(strlen(serviceAddr()) == 0)
	{
		pOrders->state = Orders::STATE_SUCCESS;
		pOrders->getDatas = pOrders->postDatas;
		orders.state = pOrders->state;
		orders.getDatas = pOrders->getDatas;
		retcode = SERVER_SUCCESS;
		return false;
	}

	Network::EndPoint endpoint;
	endpoint.socket(SOCK_STREAM);

	if (!endpoint.good())
	{
		ERROR_MSG("ChargeTask::process: couldn't create a socket\n");
		pOrders->getDatas = "couldn't create a socket!";
		orders.getDatas = pOrders->getDatas;
		return false;
	}

	if(pOrders->postDatas.size() == 0)
	{
		ERROR_MSG("ChargeTask::process: postData is NULL.\n");
		pOrders->getDatas = "postDatas is error!";
		orders.getDatas = pOrders->getDatas;
		return false;
	}

	u_int32_t addr;
	KBEngine::Network::EndPoint::convertAddress(serviceAddr(), addr);

	if(endpoint.connect(htons(servicePort()), addr) == -1)
	{
		ERROR_MSG(fmt::format("ChargeTask::process: connect billing server is error({})!\n", 
			kbe_strerror()));

		pOrders->getDatas = "connect is error!";
		orders.getDatas = pOrders->getDatas;
		return false;
	}

	endpoint.setnonblocking(true);
	endpoint.setnodelay(true);

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();
	(*(*bundle)).append(pOrders->postDatas.data(), pOrders->postDatas.size());
	(*(*bundle)).send(endpoint);

	Network::TCPPacket packet;
	packet.resize(1024);

	fd_set	frds;
	struct timeval tv = { 0, 500000 }; // 500ms

	FD_ZERO( &frds );
	FD_SET((int)endpoint, &frds);
	int selgot = select(endpoint+1, &frds, NULL, NULL, &tv);
	if(selgot <= 0)
	{
		ERROR_MSG(fmt::format("InterfacesTask::process: recv is error({}).\n", KBEngine::kbe_strerror()));
		pOrders->getDatas = "recv is error!";
		orders.getDatas = pOrders->getDatas;
		return false;
	}
	
	int len = endpoint.recv(packet.data(), 1024);

	if(len <= 0)
	{
		ERROR_MSG(fmt::format("InterfacesTask::process: recv is size<= 0.\n===>postdatas={}\n", pOrders->postDatas));
		pOrders->getDatas = "recv is error!";
		orders.getDatas = pOrders->getDatas;
		return false;
	}

	packet.wpos(len);

	pOrders->getDatas.assign((const char *)(packet.data() + packet.rpos()), packet.length());
	orders.getDatas = pOrders->getDatas;

	std::string::size_type fi = pOrders->getDatas.find("retcode:1");

	if(fi != std::string::npos)
		retcode = SERVER_SUCCESS;

	endpoint.close();

	INFO_MSG(fmt::format("ChargeTask::process: orders={}, commit={}\n==>postdatas={}\n", 
		pOrders->ordersID, retcode, pOrders->getDatas));

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState ChargeTask::presentMainThread()
{
	// 如果成功使用异步接收处理
	if(retcode != SERVER_SUCCESS)
	{
		if(!Interfaces::getSingleton().hasOrders(orders.ordersID))
		{
			WARNING_MSG(fmt::format("ChargeTask::presentMainThread: not found ordersID({}), exit thread!\n",
				orders.ordersID));

			return thread::TPTask::TPTASK_STATE_COMPLETED;
		}
	
		Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

		(*(*bundle)).newMessage(DbmgrInterface::onChargeCB);
		(*(*bundle)) << orders.baseappID << orders.ordersID << orders.dbid;
		(*(*bundle)).appendBlob(orders.getDatas);
		(*(*bundle)) << orders.cbid;
		(*(*bundle)) << retcode;

		Network::Channel* pChannel = Interfaces::getSingleton().networkInterface().findChannel(orders.address);

		if(pChannel)
		{
			WARNING_MSG(fmt::format("ChargeTask::presentMainThread: orders={} commit is failed!\n", 
				pOrders->ordersID));

			(*(*bundle)).send(Interfaces::getSingleton().networkInterface(), pChannel);
		}
		else
		{
			ERROR_MSG(fmt::format("ChargeTask::presentMainThread: not found channel. orders={}\n", 
				orders.ordersID));
		}

		Interfaces::getSingleton().lockthread();
		Interfaces::ORDERS& ordersmap = Interfaces::getSingleton().orders();
		Interfaces::ORDERS::iterator iter = ordersmap.find(orders.ordersID);
		if(iter != ordersmap.end())ordersmap.erase(iter);
		Interfaces::getSingleton().unlockthread();
	}

	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
}
