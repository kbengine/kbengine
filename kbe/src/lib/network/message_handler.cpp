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

namespace KBEngine { 
namespace Mercury
{
Mercury::MessageHandlers* MessageHandlers::pMainMessageHandlers = 0;
static Mercury::FixedMessages* g_fm;

//-------------------------------------------------------------------------------------
MessageHandlers::MessageHandlers():
msgHandlers_(),
msgID_(1)
{
	g_fm = Mercury::FixedMessages::getSingletonPtr();
	if(g_fm == NULL)
	{
		g_fm = new Mercury::FixedMessages;
	}

	Mercury::FixedMessages::getSingleton().loadConfig("../../res/server/fixed_mercury_messages.xml");
}

//-------------------------------------------------------------------------------------
MessageHandlers::~MessageHandlers()
{
	SAFE_RELEASE(g_fm);

	MessageHandlerMap::iterator iter = msgHandlers_.begin();
	for(; iter != msgHandlers_.end(); iter++)
	{
		if(iter->second)
			delete iter->second;
	};
}

//-------------------------------------------------------------------------------------
MessageHandler* MessageHandlers::add(std::string ihName, MessageArgs* args, 
	int32 msgLen, MessageHandler* msgHandler)
{
	if(msgID_ == 1)
	{
		printf("\n------------------------------------------------------------------\n");
		printf("KBEMessage_handlers begin:\n");
	}
	
	bool isfixedMsg = false;

	FixedMessages::MSGInfo* msgInfo = FixedMessages::getSingleton().isFixed(ihName.c_str());

	if(msgInfo == NULL)
	{
		while(true)
		{
			if(FixedMessages::getSingleton().isFixed(msgID_))
			{
				msgID_++;
				isfixedMsg = true;
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
	msgHandlers_[msgHandler->msgID] = msgHandler;
	
	if(msgLen == MERCURY_VARIABLE_MESSAGE)
	{
		printf("\tMessageHandlers::add(%d): name=%s, msgID=%d, size=Variable.\n", 
			(int32)msgHandlers_.size(), ihName.c_str(), msgHandler->msgID);
	}
	else
	{
		if(msgLen == 0)
			msgHandler->msgLen = args->msgsize();
		
		printf("\tMessageHandlers::add(%d): name=%s, msgID=%d, size=Fixed(%d).\n", 
				(int32)msgHandlers_.size(), ihName.c_str(), msgHandler->msgID, msgHandler->msgLen);
	}

	if(isfixedMsg)
		printf("\t\t!!!message is fixed.!!!\n");
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
} 
}
