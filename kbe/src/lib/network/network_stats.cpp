// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
