#include "message_handler.hpp"

#include "network/channel.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
MessageHandlers::MessageHandlers():
msgHandlers_(),
msgID_(1)
{
}

//-------------------------------------------------------------------------------------
MessageHandlers::~MessageHandlers()
{
	MessageHandlerMap::iterator iter = msgHandlers_.begin();
	for(; iter != msgHandlers_.end(); iter++)
	{
		delete iter->second;
	};
}

//-------------------------------------------------------------------------------------
MessageHandler* MessageHandlers::add(std::string ihName, MessageArgs* args, MessageHandler* msgHandler)
{
	if(msgID_ == 1)
		printf("message_handlers begin:\n");
	
	msgHandler->msgID = msgID_++;
	msgHandler->name = ihName;					
	msgHandler->pArgs = args;			
	msgHandlers_[msgHandler->msgID] = msgHandler;
	printf("\tMessageHandlers::add: name=%s, msgID=%d.\n", ihName.c_str(), msgHandler->msgID);
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
