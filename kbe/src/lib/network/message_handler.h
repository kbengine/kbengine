/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#ifndef KBE_MESSAGE_HANDLER_H
#define KBE_MESSAGE_HANDLER_H

#include "common/memorystream.h"
#include "common/smartpointer.h"
#include "helper/debug_helper.h"
#include "network/common.h"

namespace KBEngine{

class KBE_MD5;

namespace Network
{

class Channel;
class MessageHandlers;

/** һ����Ϣ�Ĳ��������� */
class MessageArgs
{
public:
	enum MESSAGE_ARGS_TYPE
	{
		MESSAGE_ARGS_TYPE_VARIABLE = -1,		// �ɱ��������
		MESSAGE_ARGS_TYPE_FIXED = 0				// �̶���������
	};

	MessageArgs():strArgsTypes(){};
	virtual ~MessageArgs(){};
	virtual void createFromStream(MemoryStream& s) = 0;
	virtual void addToStream(MemoryStream& s) = 0;
	virtual int32 dataSize(void) = 0;
	virtual MessageArgs::MESSAGE_ARGS_TYPE type(void){ return MESSAGE_ARGS_TYPE_FIXED; }

	std::vector<std::string> strArgsTypes;
};

struct ExposedMessageInfo
{
	std::string name;
	Network::MessageID id;
	int16 msgLen; // ������Ϣ���ᳬ��1500
	int8 argsType;
	std::vector<uint8> argsTypes;
};

class MessageHandler
{
public:
	MessageHandler();
	virtual ~MessageHandler();

	std::string name;
	MessageID msgID;
	MessageArgs* pArgs;
	int32 msgLen;					// �������Ϊ-1��Ϊ�ǹ̶�������Ϣ
	bool exposed;
	MessageHandlers* pMessageHandlers;

	// stats
	volatile mutable uint32 send_size;
	volatile mutable uint32 send_count;
	volatile mutable uint32 recv_size;
	volatile mutable uint32 recv_count;

	uint32 sendsize() const  { return send_size; }
	uint32 sendcount() const  { return send_count; }
	uint32 sendavgsize() const  { return (send_count <= 0) ? 0 : send_size / send_count; }

	uint32 recvsize() const  { return recv_size; }
	uint32 recvcount() const  { return recv_count; }
	uint32 recvavgsize() const  { return (recv_count <= 0) ? 0 : recv_size / recv_count; }

	/**
		Ĭ�Ϸ������Ϊ�����Ϣ
	*/
	virtual NETWORK_MESSAGE_TYPE type() const
	{ 
		return NETWORK_MESSAGE_TYPE_COMPONENT; 
	}

	virtual int32 msglenMax(){ return NETWORK_MESSAGE_MAX_SIZE; }

	const char* c_str();

	/**
		�����handler�����ǰ�װ��MessageHandlers�󱻵���
	*/
	virtual void onInstall(){}

	virtual void handle(Channel* pChannel, MemoryStream& s)
	{
		pArgs->createFromStream(s);
		
		// �������������յĽӿ�
	};
};

class MessageHandlers
{
public:
	static Network::MessageHandlers* pMainMessageHandlers;
	typedef std::map<MessageID, MessageHandler*> MessageHandlerMap;
	MessageHandlers();
	~MessageHandlers();
	
	MessageHandler* add(std::string ihName, MessageArgs* args, int32 msgLen, 
						MessageHandler* msgHandler);
	
	bool pushExposedMessage(std::string msgname);

	MessageHandler* find(MessageID msgID);
	
	MessageID lastMsgID() {return msgID_ - 1;}

	bool initializeWatcher();
	
	static void finalise(void);
	static std::vector<MessageHandlers*>& messageHandlers();

	const MessageHandlerMap& msgHandlers(){ return msgHandlers_; }

	static std::string getDigestStr();

private:
	MessageHandlerMap msgHandlers_;
	MessageID msgID_;

	std::vector< std::string > exposedMessages_;
};

}
}
#endif 
