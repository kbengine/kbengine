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
#include "network/bundle.hpp"
#include "billingsystem.hpp"
#include "anonymous_channel.hpp"
#include "orders.hpp"

#include "dbmgr/dbmgr_interface.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
AnonymousChannel::AnonymousChannel()
{
	listen.socket(SOCK_STREAM);

	if (!listen.good())
	{
		ERROR_MSG("AnonymousChannel::process: couldn't create a socket\n");
		return;
	}

	if (listen.bind(htons(g_kbeSrvConfig.billingSystemThirdpartyServiceCBPort()), 
		BillingSystem::getSingleton().getNetworkInterface().extaddr().ip) == -1)
	{
		ERROR_MSG(boost::format("AnonymousChannel::bind(%1%): \n") %
			 kbe_strerror());

		listen.close();
		return;
	}

	if(listen.listen() == -1)
	{
		ERROR_MSG(boost::format("AnonymousChannel::listeningSocket(%1%): \n") %
			 kbe_strerror());

		listen.close();
		return;
	}

	listen.setnonblocking(true);

	INFO_MSG(boost::format("AnonymousChannel::bind: %1%:%2%\n") %
		inet_ntoa((struct in_addr&)BillingSystem::getSingleton().getNetworkInterface().extaddr().ip) % 
		g_kbeSrvConfig.billingSystemThirdpartyServiceCBPort());
}

//-------------------------------------------------------------------------------------
AnonymousChannel::~AnonymousChannel()
{
}

//-------------------------------------------------------------------------------------
bool AnonymousChannel::process()
{
	if (!listen.good())
	{
		ERROR_MSG("AnonymousChannel::process: invalid endpoint.\n");
		return false;
	}

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	while(1)
	{
		fd_set	frds, fwds;
		struct timeval tv = { 0, 1000000 }; // 1000ms

		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)listen, &frds);
		FD_SET((int)listen, &fwds);

		int selgot = select(listen+1, &frds, &fwds, NULL, &tv);
		if(selgot <= 0)
		{
			return false;
		}
		
		u_int16_t port;
		u_int32_t addr;

		Mercury::EndPoint* pEndpoint = listen.accept(&port, &addr);

		if(pEndpoint == NULL)
		{
			ERROR_MSG(boost::format("AnonymousChannel::process: accept is error:%1%.\n") % kbe_strerror());
			continue;
		}

		INFO_MSG(boost::format("AnonymousChannel::process: accept(%1%).\n") % pEndpoint->c_str());

		Mercury::TCPPacket packet;
		packet.resize(1024);

		struct timeval tv1 = { 0, 300000 }; // 300ms

		FD_ZERO( &frds );
		FD_SET((int)(*pEndpoint), &frds);
		selgot = select((*pEndpoint)+1, &frds, NULL, NULL, &tv1);
		if(selgot <= 0)
		{
			ERROR_MSG("AnonymousChannel::process: recv timeout.\n");
			pEndpoint->close();
			delete pEndpoint;
			continue;
		}
		
		int len = pEndpoint->recv(packet.data(), 1024);

		if(len <= 0)
		{
			ERROR_MSG(boost::format("AnonymousChannel::process: recv is error(%1%).\n") % KBEngine::kbe_strerror());
			pEndpoint->close();
			delete pEndpoint;
			continue;
		}

		std::string retstr = "HTTP/1.1 200 OK\r\nServer: unknown\r\nCache-Control: private\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 8\r\n\r\nretcode1";
		pEndpoint->send(retstr.data(), retstr.size());
		pEndpoint->close();
		delete pEndpoint;

		packet.wpos(len);

		std::string getDatas;
		getDatas.assign((const char *)(packet.data() + packet.rpos()), packet.opsize());
		
		std::string::size_type fi1 = getDatas.find("&chargeID=");
		std::string::size_type fi2 = getDatas.find("&");

		std::string orderid;
		if(fi1 != std::string::npos && fi2 != std::string::npos)
		{
			int ilen = strlen("&chargeID=");
			orderid.assign(getDatas.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
		}

		backOrdersDatas_[orderid] = getDatas;
	}

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState AnonymousChannel::presentMainThread()
{
	KBEUnordered_map<std::string, std::string>::iterator iter = backOrdersDatas_.begin();
	BillingSystem::getSingleton().lockthread();

	for(; iter != backOrdersDatas_.end(); iter++)
	{
		BillingSystem::ORDERS& orders = BillingSystem::getSingleton().orders();
		
		BillingSystem::ORDERS::iterator orderiter = orders.find(iter->first);
		if(orderiter == orders.end())
			continue;
		
		bool success = false;

		std::string::size_type fi = iter->second.find("state=");
		std::string::size_type fi1 = iter->second.find("&chargeID=");

		if(fi != std::string::npos && fi1 != std::string::npos)
		{
			std::string s;
			int ilen = strlen("state=");
			s.assign(iter->second.c_str() + fi + ilen, fi1 - (fi + ilen));
			success = atoi(s.c_str()) > 0;
		}

		INFO_MSG(boost::format("AnonymousChannel::presentMainThread: orders=%1%, success=%2%\n") % 
			orderiter->second->ordersID % success);

		Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

		(*(*bundle)).newMessage(DbmgrInterface::onChargeCB);
		(*(*bundle)) << orderiter->second->baseappID << orderiter->second->ordersID << orderiter->second->dbid;
		(*(*bundle)).appendBlob(iter->second);
		(*(*bundle)) << orderiter->second->cbid;
		(*(*bundle)) << success;

		Mercury::Channel* pChannel = BillingSystem::getSingleton().getNetworkInterface().findChannel(orderiter->second->address);

		if(pChannel)
		{
			(*(*bundle)).send(BillingSystem::getSingleton().getNetworkInterface(), pChannel);
		}
		else
		{
			ERROR_MSG(boost::format("AnonymousChannel::presentMainThread: not found channel. orders=%1%\n") % 
				orderiter->second->ordersID);
		}
		
		orders.erase(orderiter);
	}

	backOrdersDatas_.clear();

	BillingSystem::getSingleton().unlockthread();
	return thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD; 
}

//-------------------------------------------------------------------------------------
}
