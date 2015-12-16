/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#include "network_stats.h"
#include "helper/watcher.h"
#include "network/message_handler.h"

namespace KBEngine { 

KBE_SINGLETON_INIT(Network::NetworkStats);

namespace Network
{

NetworkStats g_networkStats;

//-------------------------------------------------------------------------------------
NetworkStats::NetworkStats():
stats_(),
handlers_()
{
}

//-------------------------------------------------------------------------------------
NetworkStats::~NetworkStats()
{
}

//-------------------------------------------------------------------------------------
void NetworkStats::addHandler(NetworkStatsHandler* pHandler)
{
	handlers_.push_back(pHandler);
}

//-------------------------------------------------------------------------------------
void NetworkStats::removeHandler(NetworkStatsHandler* pHandler)
{
	std::vector<NetworkStatsHandler*>::iterator iter = handlers_.begin();
	for(; iter != handlers_.end(); ++iter)
	{
		if((*iter) == pHandler)
		{
			handlers_.erase(iter);
			break;
		}
	}
}

//-------------------------------------------------------------------------------------
void NetworkStats::trackMessage(S_OP op, const MessageHandler& msgHandler, uint32 size)
{
	MessageHandler* pMsgHandler = const_cast<MessageHandler*>(&msgHandler);

	if(op == SEND)
	{
		pMsgHandler->send_size += size;
		pMsgHandler->send_count++;
	}
	else
	{
		pMsgHandler->recv_size += size;
		pMsgHandler->recv_count++;
	}

	std::vector<NetworkStatsHandler*>::iterator iter = handlers_.begin();
	for(; iter != handlers_.end(); ++iter)
	{
		if(op == SEND)
			(*iter)->onSendMessage(msgHandler, size);
		else
			(*iter)->onRecvMessage(msgHandler, size);
	}
}

//-------------------------------------------------------------------------------------
}
}
