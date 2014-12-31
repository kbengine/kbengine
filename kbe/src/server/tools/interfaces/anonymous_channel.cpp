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
#include "network/bundle.h"
#include "network/tcp_packet.h"
#include "interfaces.h"
#include "anonymous_channel.h"
#include "orders.h"

#include "dbmgr/dbmgr_interface.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
AnonymousChannel::AnonymousChannel()
{
	initListen();
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
		KBEngine::sleep(1000);
		initListen();
		return false;
	}

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

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

		Network::EndPoint* pEndpoint = listen.accept(&port, &addr);

		if(pEndpoint == NULL)
		{
			ERROR_MSG(fmt::format("AnonymousChannel::process: accept is error:{}.\n", kbe_strerror()));
			continue;
		}

		INFO_MSG(fmt::format("AnonymousChannel::process: accept({}).\n", pEndpoint->c_str()));

		Network::TCPPacket packet;
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
			ERROR_MSG(fmt::format("AnonymousChannel::process: recv is error({}).\n", KBEngine::kbe_strerror()));
			pEndpoint->close();
			delete pEndpoint;
			continue;
		}

		std::string retstr = "HTTP/1.1 200 OK\r\nServer: unknown\r\nCache-Control: private\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 8\r\n\r\nretocde:1";
		pEndpoint->send(retstr.data(), retstr.size());
		pEndpoint->close();
		delete pEndpoint;

		packet.wpos(len);

		std::string getDatas;
		getDatas.assign((const char *)(packet.data() + packet.rpos()), packet.length());
		
		std::string::size_type fi1 = getDatas.find("&chargeID=");
		std::string::size_type fi2 = getDatas.find("&=");

		std::string orderid;
		if(fi1 != std::string::npos && fi2 != std::string::npos)
		{
			int ilen = strlen("&chargeID=");
			orderid.assign(getDatas.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
		}
		
		if(orderid.size() > 0)
		{
			BACK_ORDERS_DATA orderdata;
			orderdata.data = getDatas;
			backOrdersDatas_[orderid] = orderdata;
			DEBUG_MSG(fmt::format("AnonymousChannel::process: getDatas={}\nfi1={}\nfi2={}\n", 
				getDatas, fi1, fi2));
		}
		else
		{
			ERROR_MSG(fmt::format("AnonymousChannel::process: not found orderid!\ngetDatas={}\nfi1={}\nfi2={}\n", 
				getDatas, fi1, fi2));
		}	

	}

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState AnonymousChannel::presentMainThread()
{
	KBEUnordered_map<std::string, BACK_ORDERS_DATA>::iterator iter = backOrdersDatas_.begin();
	Interfaces::getSingleton().lockthread();

	SERVER_ERROR_CODE retcode = SERVER_ERR_OP_FAILED;
	Interfaces::ORDERS& orders = Interfaces::getSingleton().orders();
	Interfaces::ORDERS::iterator oiter = orders.begin();
	
	for(; oiter != orders.end(); )
	{
		if(oiter->second->timeout < timestamp())
		{
			INFO_MSG(fmt::format("AnonymousChannel::presentMainThread: order({}) timeout!\n", oiter->second->ordersID));


			Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

			(*(*bundle)).newMessage(DbmgrInterface::onChargeCB);
			(*(*bundle)) << oiter->second->baseappID << oiter->second->ordersID << oiter->second->dbid;
			std::string backdata = "timeout";
			(*(*bundle)).appendBlob(backdata);
			(*(*bundle)) << oiter->second->cbid;
			(*(*bundle)) << retcode;

			Network::Channel* pChannel = Interfaces::getSingleton().networkInterface().findChannel(oiter->second->address);

			if(pChannel)
			{
				(*(*bundle)).send(Interfaces::getSingleton().networkInterface(), pChannel);
			}
			else
			{
				ERROR_MSG(fmt::format("AnonymousChannel::presentMainThread: not found channel. orders={}\n", 
					oiter->second->ordersID));
			}

			oiter = orders.erase(oiter);
		}
		else
		{
			++oiter;
		}
	}
	
	for(; iter != backOrdersDatas_.end(); ++iter)
	{
		Interfaces::ORDERS::iterator orderiter = orders.find(iter->first);
		COMPONENT_ID baseappID = 0;
		std::string ordersID = iter->first;
		DBID dbid = 0;
		CALLBACK_ID cbid = 0;
		retcode = SERVER_ERR_OP_FAILED;
		
		if(orderiter == orders.end())
		{
			WARNING_MSG(fmt::format("AnonymousChannel::presentMainThread: orders={} not found!\n", 
			iter->first));

			// continue;
		}
		else
		{
			baseappID = orderiter->second->baseappID;
			dbid = orderiter->second->dbid;
			cbid = orderiter->second->cbid;
		}

		std::string::size_type fi = iter->second.data.find("cstate=");
		std::string::size_type fi1 = iter->second.data.find("&chargeID=");

		if(fi != std::string::npos && fi1 != std::string::npos)
		{
			std::string s;
			int ilen = strlen("cstate=");
			s.assign(iter->second.data.c_str() + fi + ilen, fi1 - (fi + ilen));
			
			if(atoi(s.c_str()) > 0)
				retcode = SERVER_SUCCESS;
		}

		INFO_MSG(fmt::format("AnonymousChannel::presentMainThread: orders={}, dbid={}, retcode={}\n", 
			ordersID, dbid, retcode));
		
		if(orderiter != orders.end())
		{
			Network::Channel* pChannel = Interfaces::getSingleton().networkInterface().findChannel(orderiter->second->address);
			if(pChannel)
			{
				Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

				(*(*bundle)).newMessage(DbmgrInterface::onChargeCB);
				(*(*bundle)) << baseappID << ordersID << dbid;
				(*(*bundle)).appendBlob(iter->second.data);
				(*(*bundle)) << cbid;
				(*(*bundle)) << retcode;
				(*(*bundle)).send(Interfaces::getSingleton().networkInterface(), pChannel);
			}
			else
			{
				ERROR_MSG(fmt::format("AnonymousChannel::presentMainThread: not found channel. orders={}\n", 
					ordersID));
			}
		}
		else
		{
			const Network::NetworkInterface::ChannelMap& channels = Interfaces::getSingleton().networkInterface().channels();
			if(channels.size() > 0)
			{
				Network::NetworkInterface::ChannelMap::const_iterator channeliter = channels.begin();
				for(; channeliter != channels.end(); ++channeliter)
				{
					Network::Channel* pChannel = channeliter->second;
					if(pChannel)
					{
						Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

						(*(*bundle)).newMessage(DbmgrInterface::onChargeCB);
						(*(*bundle)) << baseappID << ordersID << dbid;
						(*(*bundle)).appendBlob(iter->second.data);
						(*(*bundle)) << cbid;
						(*(*bundle)) << retcode;
						(*(*bundle)).send(Interfaces::getSingleton().networkInterface(), pChannel);
					}
					else
					{
						ERROR_MSG(fmt::format("AnonymousChannel::presentMainThread: not found channel. orders={}\n", 
							ordersID));
					}
				}
			}
			else
			{
				ERROR_MSG(fmt::format("AnonymousChannel::presentMainThread: not found channel(channels is NULL). orders={}\n", 
					ordersID));
			}
		}
		
		if(orderiter != orders.end())
			orders.erase(orderiter);
	}

	backOrdersDatas_.clear();

	Interfaces::getSingleton().unlockthread();
	return thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD; 
}

//-------------------------------------------------------------------------------------
void AnonymousChannel::initListen()
{
	if (!listen.good())
	{
		listen.socket(SOCK_STREAM);

		if (!listen.good())
		{
			return;
		}

		if (listen.bind(htons(g_kbeSrvConfig.interfacesThirdpartyServiceCBPort()), 
			Interfaces::getSingleton().networkInterface().extaddr().ip) == -1)
		{
			ERROR_MSG(fmt::format("AnonymousChannel::bind({}): \n",
				 kbe_strerror()));

			listen.close();
			return;
		}

		if(listen.listen() == -1)
		{
			ERROR_MSG(fmt::format("AnonymousChannel::listeningSocket({}): \n",
				 kbe_strerror()));

			listen.close();
			return;
		}

		listen.setnonblocking(true);

		INFO_MSG(fmt::format("AnonymousChannel::bind: {}:{}\n",
			inet_ntoa((struct in_addr&)Interfaces::getSingleton().networkInterface().extaddr().ip), 
			g_kbeSrvConfig.interfacesThirdpartyServiceCBPort()));
	}
}

//-------------------------------------------------------------------------------------
}

