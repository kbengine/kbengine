/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef __MESSAGE_HANDLER_HPP__
#define __MESSAGE_HANDLER_HPP__

#include "cstdkbe/memorystream.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"

namespace KBEngine{
namespace Mercury
{

/** 一个消息的参数抽象类 */
class MessageArgs
{
public:
	MessageArgs(){};
	virtual ~MessageArgs(){};
	virtual void createFromStream(MemoryStream& s) = 0;
	virtual void addToStream(MemoryStream& s)	= 0;
};

class MessageHandler
{
public:
	MessageHandler(){};
	~MessageHandler(){delete pArgs;};

	std::string name;
	MessageID msgID;
	MessageArgs* pArgs;
	int32 msgLen;					// 如果长度为-1则为非固定长度消息
	
	virtual void handle(MemoryStream& s)
	{
		pArgs->createFromStream(s);
		
		// 将参数传给最终的接口
	};
};

class MessageHandlers
{
public:
	static Mercury::MessageHandlers* pMainMessageHandlers;
	typedef std::map<MessageID, MessageHandler*> MessageHandlerMap;
	MessageHandlers();
	~MessageHandlers();
	
	MessageHandler* add(std::string ihName, MessageArgs* args, int32 msgLen, 
						MessageHandler* msgHandler);
	
	MessageHandler* find(MessageID msgID);
	
	MessageID lastMsgID() {return msgID_ - 1;}
private:
	MessageHandlerMap msgHandlers_;
	MessageID msgID_;
};

}
}
#endif 
