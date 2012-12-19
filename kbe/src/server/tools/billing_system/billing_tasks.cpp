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

#include "billingsystem.hpp"
#include "billing_tasks.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "openssl/md5.h"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"
#include "dbmgr/dbmgr_interface.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
BillingTask::BillingTask()
{
}

//-------------------------------------------------------------------------------------
BillingTask::~BillingTask()
{
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState BillingTask::presentMainThread()
{
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
CreateAccountTask::CreateAccountTask():
BillingTask()
{
}

//-------------------------------------------------------------------------------------
CreateAccountTask::~CreateAccountTask()
{
}

//-------------------------------------------------------------------------------------
bool CreateAccountTask::process()
{
	// 如果没有设置第三方服务地址则我们默认为成功
	if(strlen(g_kbeSrvConfig.billingSystemThirdpartyServiceAddr()) == 0)
	{
		success = true;
		getDatas = postDatas;
		return false;
	}

	Mercury::EndPoint endpoint;
	endpoint.socket(SOCK_STREAM);

	if (!endpoint.good())
	{
		ERROR_MSG("BillingTask::process: couldn't create a socket\n");
		return false;
	}

	if(postDatas.size() == 0)
	{
		ERROR_MSG(boost::format("BillingTask::process: %1% postData is NULL.\n") % commitName);
		return false;
	}

	endpoint.setnonblocking(true);

	u_int32_t addr;
	KBEngine::Mercury::EndPoint::convertAddress(g_kbeSrvConfig.billingSystemThirdpartyServiceAddr(), addr);

	int trycount = 0;

	while(true)
	{
		fd_set	frds, fwds;
		struct timeval tv = { 0, 100000 }; // 100ms

		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)endpoint, &frds);
		FD_SET((int)endpoint, &fwds);

		if(endpoint.connect(htons(g_kbeSrvConfig.billingSystemThirdpartyServicePort()), addr) == -1)
		{
			int selgot = select(endpoint+1, &frds, &fwds, NULL, &tv);
			if(selgot > 0)
			{
				break;
			}

			trycount++;
			if(trycount > 3)
			{
				ERROR_MSG(boost::format("BillingTask::process: connect billing server is error(%1%)!\n") % 
					kbe_strerror());

				endpoint.close();
				return false;
			}
		}
	}

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();
	(*(*bundle)).append(postDatas.data(), postDatas.size());
	(*(*bundle)).send(endpoint);

	Mercury::TCPPacket packet;
	packet.resize(1024);

	fd_set	frds;
	struct timeval tv = { 0, 300000 }; // 300ms

	FD_ZERO( &frds );
	FD_SET((int)endpoint, &frds);
	int selgot = select(endpoint+1, &frds, NULL, NULL, &tv);
	if(selgot <= 0)
	{
		ERROR_MSG(boost::format("BillingTask::process: %1% recv is error(%2%).\n") % commitName % KBEngine::kbe_strerror());
		endpoint.close();
		return false;
	}

	int len = endpoint.recv(packet.data(), 1024);

	if(len <= 0)
	{
		ERROR_MSG(boost::format("BillingTask::process: %1% recv is size<= 0.\n") % commitName);
		endpoint.close();
		return false;
	}

	packet.wpos(len);

	getDatas.assign((const char *)(packet.data() + packet.rpos()), packet.opsize());

	try
	{
		std::string::size_type fi = getDatas.find("\r\n\r\n");
		if(fi > 0)
		{
			fi += 4;
			MemoryStream s;
			s.append(getDatas.data() + fi, getDatas.size() - fi);

			while(s.opsize() > 0)
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
						success = false;
						endpoint.close();
						return false;
					}
					else
					{
						success = true;
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
		success = false;
		ERROR_MSG(boost::format("BillingTask::process: %1% recv is error.\n") % commitName);
	}

	endpoint.close();
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState CreateAccountTask::presentMainThread()
{
	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(DbmgrInterface::onCreateAccountCBFromBilling);
	(*(*bundle)) << baseappID << commitName << accountName << password << success;

	(*(*bundle)).appendBlob(getDatas);

	Mercury::Channel* pChannel = BillingSystem::getSingleton().getNetworkInterface().findChannel(address);

	if(pChannel)
	{
		(*(*bundle)).send(BillingSystem::getSingleton().getNetworkInterface(), pChannel);
	}
	else
	{
		ERROR_MSG(boost::format("BillingTask::process: not found channel. commitName=%1%\n") % commitName);
	}

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
thread::TPTask::TPTaskState LoginAccountTask::presentMainThread()
{
	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();
	
	if(success)
	{
		if(accountName.size() == 0)
			accountName = commitName;
	}

	(*(*bundle)).newMessage(DbmgrInterface::onLoginAccountCBBFromBilling);
	(*(*bundle)) << baseappID << commitName << accountName << password << success;

	(*(*bundle)).appendBlob(getDatas);

	Mercury::Channel* pChannel = BillingSystem::getSingleton().getNetworkInterface().findChannel(address);

	if(pChannel)
	{
		(*(*bundle)).send(BillingSystem::getSingleton().getNetworkInterface(), pChannel);
	}
	else
	{
		ERROR_MSG(boost::format("BillingTask::process: not found channel. commitName=%1%\n") % commitName);
	}

	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
}
