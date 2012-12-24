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

		pEndpoint->close();
		delete pEndpoint;

		packet.wpos(len);

		std::string getDatas;
		getDatas.assign((const char *)(packet.data() + packet.rpos()), packet.opsize());
	}

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState AnonymousChannel::presentMainThread()
{
	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	/*
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
		ERROR_MSG(boost::format("AnonymousChannel::process: not found channel. commitName=%1%\n") % commitName);
	}
	*/
	return thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD; 
}

//-------------------------------------------------------------------------------------
}
