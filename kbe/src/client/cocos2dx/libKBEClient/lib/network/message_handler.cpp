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


#include "message_handler.hpp"
#include "network/channel.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"
#include "network/fixed_messages.hpp"
#include "helper/watcher.hpp"

namespace KBEngine { 
namespace Mercury
{
Mercury::MessageHandlers* MessageHandlers::pMainMessageHandlers = 0;
std::vector<MessageHandlers*>* g_pMessageHandlers;

static Mercury::FixedMessages* g_fm;

//-------------------------------------------------------------------------------------
MessageHandlers::MessageHandlers():
msgHandlers_(),
msgID_(1),
exposedMessages_()
{
	g_fm = Mercury::FixedMessages::getSingletonPtr();
	if(g_fm == NULL)
		g_fm = new Mercury::FixedMessages;

	Mercury::FixedMessages::getSingleton().loadConfig("server/fixed_mercury_messages.xml");
	messageHandlers().push_back(this);
}

//-------------------------------------------------------------------------------------
MessageHandlers::~MessageHandlers()
{
	MessageHandlerMap::iterator iter = msgHandlers_.begin();
	for(; iter != msgHandlers_.end(); iter++)
	{
		if(iter->second)
			delete iter->second;
	};
}

//-------------------------------------------------------------------------------------
MessageHandler::MessageHandler():
pArgs(NULL),
send_size(0),
send_count(0),
recv_size(0),
recv_count(0)
{
}

//-------------------------------------------------------------------------------------
MessageHandler::~MessageHandler()
{
	SAFE_RELEASE(pArgs);
}

//-------------------------------------------------------------------------------------
const char* MessageHandler::c_str()
{
	static char buf[MAX_BUF];
	kbe_snprintf(buf, MAX_BUF, "id:%u, len:%d", msgID, msgLen);
	return buf;
}

//-------------------------------------------------------------------------------------
bool MessageHandlers::initializeWatcher()
{
	std::vector< std::string >::iterator siter = exposedMessages_.begin();
	for(; siter != exposedMessages_.end(); siter++)
	{
		MessageHandlerMap::iterator iter = msgHandlers_.begin();
		for(; iter != msgHandlers_.end(); iter++)
		{
			if((*siter) == iter->second->name)
			{
				iter->second->exposed = true;
			}
		}
	}

	MessageHandlerMap::iterator iter = msgHandlers_.begin();
	for(; iter != msgHandlers_.end(); iter++)
	{
		char buf[MAX_BUF];
		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/id", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second->msgID);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/len", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second->msgLen);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/sentSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::sendsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/sentCount", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::sendcount);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/sentAvgSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::sendavgsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/recvSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::recvsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/recvCount", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::recvsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/recvAvgSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::recvavgsize);
	}

	return true;
}

//-------------------------------------------------------------------------------------
MessageHandler* MessageHandlers::add(std::string ihName, MessageArgs* args, 
	int32 msgLen, MessageHandler* msgHandler)
{
	if(msgID_ == 1)
	{
		//printf("\n------------------------------------------------------------------\n");
		//printf("KBEMessage_handlers begin:\n");
	}
	
	//bool isfixedMsg = false;

	FixedMessages::MSGInfo* msgInfo = FixedMessages::getSingleton().isFixed(ihName.c_str());

	if(msgInfo == NULL)
	{
		while(true)
		{
			if(FixedMessages::getSingleton().isFixed(msgID_))
			{
				msgID_++;
				//isfixedMsg = true;
			}
			else
				break;
		};

		msgHandler->msgID = msgID_++;
	}
	else
	{
		msgHandler->msgID = msgInfo->msgid;
	}
	
	msgHandler->name = ihName;					
	msgHandler->pArgs = args;
	msgHandler->msgLen = msgLen;	
	msgHandler->exposed = false;

	msgHandler->onInstall();
	msgHandlers_[msgHandler->msgID] = msgHandler;
	
	if(msgLen == MERCURY_VARIABLE_MESSAGE)
	{
		//printf("\tMessageHandlers::add(%d): name=%s, msgID=%d, size=Variable.\n", 
		//	(int32)msgHandlers_.size(), ihName.c_str(), msgHandler->msgID);
	}
	else
	{
		if(msgLen == 0)
		{
			msgHandler->msgLen = args->dataSize();

			if(msgHandler->type() == MERCURY_MESSAGE_TYPE_ENTITY)
			{
				msgHandler->msgLen += sizeof(ENTITY_ID);
			}
		}
		
		//printf("\tMessageHandlers::add(%d): name=%s, msgID=%d, size=Fixed(%d).\n", 
		//		(int32)msgHandlers_.size(), ihName.c_str(), msgHandler->msgID, msgHandler->msgLen);
	}

	//if(isfixedMsg)
	//	printf("\t\t!!!message is fixed.!!!\n");

	return msgHandlers_[msgHandler->msgID];
}

//-------------------------------------------------------------------------------------
MessageHandler* MessageHandlers::find(MessageID msgID)
{
	MessageHandlerMap::iterator iter = msgHandlers_.find(msgID);
	if(iter != msgHandlers_.end())
	{
		return iter->second;
	};
	
	return NULL;
}

//-------------------------------------------------------------------------------------
std::vector<MessageHandlers*>& MessageHandlers::messageHandlers()
{
	if(g_pMessageHandlers == NULL)
		g_pMessageHandlers = new std::vector<MessageHandlers*>;

	return *g_pMessageHandlers;
}

//-------------------------------------------------------------------------------------
void MessageHandlers::finalise(void)
{
	SAFE_RELEASE(g_fm);
	SAFE_RELEASE(g_pMessageHandlers);
}

//-------------------------------------------------------------------------------------
bool MessageHandlers::pushExposedMessage(std::string msgname)
{
	exposedMessages_.push_back(msgname);
	return true;
}

//-------------------------------------------------------------------------------------
} 
}
