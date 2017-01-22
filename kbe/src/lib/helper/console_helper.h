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


#ifndef KBE_CONSOLE_HELPER_H
#define KBE_CONSOLE_HELPER_H
#include "network/message_handler.h"
#include "network/channel.h"

#define CONSOLE_COMMANDCB_MSGID 65500
#define CONSOLE_LOG_MSGID 65501
#define CONSOLE_WATCHERCB_MSGID 65502
#define CONSOLE_PROFILECB_MSGID 65503

namespace KBEngine{
namespace ConsoleInterface {
	
	class ConsoleExecCommandCBMessageHandlerArgs1 : public Network::MessageArgs	
	{	
	public:
		std::string strarg;
	public:
		ConsoleExecCommandCBMessageHandlerArgs1():Network::MessageArgs(){}	
		ConsoleExecCommandCBMessageHandlerArgs1(std::string init_strarg):				
		Network::MessageArgs(),
		strarg(init_strarg)				
		{}
		~ConsoleExecCommandCBMessageHandlerArgs1(){}	
		
		static void staticAddToBundle(Network::Bundle& s,			
			std::string init_strarg)			
		{
			s.appendBlob(init_strarg);		
		}
		static void staticAddToStream(MemoryStream& s,			
			std::string init_strarg)			
		{
			s.appendBlob(init_strarg);			
		}
		virtual int32 dataSize(void)				
		{
			return NETWORK_VARIABLE_MESSAGE;			
		}
		virtual void addToStream(MemoryStream& s)
		{
			s.appendBlob(strarg);	
		}
		virtual void createFromStream(MemoryStream& s)				
		{
			s.readBlob(strarg);	
		}
	};	
				
	class ConsoleExecCommandCBMessageHandler : public Network::MessageHandler
	{
	public:
		ConsoleExecCommandCBMessageHandler():
		  Network::MessageHandler()
		{
			onInstall();
		}

		virtual void onInstall()
		{
			// ǿ������Э��ID
			msgID = CONSOLE_COMMANDCB_MSGID;
			msgLen = NETWORK_VARIABLE_MESSAGE;
			name = "console::execPythonCommand";
		}

		virtual int32 msglenMax()
		{ 
			return NETWORK_MESSAGE_MAX_SIZE * 1000; 
		}

		virtual void handle(Network::Channel* pChannel, MemoryStream& s)
		{
		};
	};


	class ConsoleLogMessageHandlerArgsStream : public Network::MessageArgs	
	{				
	public:
		ConsoleLogMessageHandlerArgsStream():Network::MessageArgs(){}				
		~ConsoleLogMessageHandlerArgsStream(){}	
	
		virtual int32 dataSize(void)				
		{
			return NETWORK_VARIABLE_MESSAGE;			
		}		

		virtual MessageArgs::MESSAGE_ARGS_TYPE type(void)		
		{														
			return MESSAGE_ARGS_TYPE_VARIABLE;					
		}												

		virtual int32 msglenMax()
		{ 
			return NETWORK_MESSAGE_MAX_SIZE * 1000; 
		}

		virtual void addToStream(MemoryStream& s)
		{
		}			

		virtual void createFromStream(MemoryStream& s)				
		{	
		}	
	};	
				
	class ConsoleLogMessageHandler : public Network::MessageHandler
	{
	public:
		ConsoleLogMessageHandler():
		  Network::MessageHandler()
		{
			onInstall();
		}

		virtual void onInstall()
		{
			// ǿ������Э��ID
			msgID = CONSOLE_LOG_MSGID;
			msgLen = NETWORK_VARIABLE_MESSAGE;
			name = "console::querylogs";
		}

		virtual void handle(Network::Channel* pChannel, MemoryStream& s)
		{
		};
	};
	
	class ConsoleWatcherCBHandlerMessageArgsStream : public Network::MessageArgs	
	{				
	public:
		ConsoleWatcherCBHandlerMessageArgsStream():Network::MessageArgs(){}				
		~ConsoleWatcherCBHandlerMessageArgsStream(){}	
	
		virtual int32 dataSize(void)				
		{
			return NETWORK_VARIABLE_MESSAGE;			
		}		

		virtual MessageArgs::MESSAGE_ARGS_TYPE type(void)		
		{														
			return MESSAGE_ARGS_TYPE_VARIABLE;					
		}													

		virtual int32 msglenMax()
		{ 
			return NETWORK_MESSAGE_MAX_SIZE * 1000; 
		}

		virtual void addToStream(MemoryStream& s)
		{
		}			

		virtual void createFromStream(MemoryStream& s)				
		{	
		}	
	};	
				
	class ConsoleWatcherCBMessageHandler : public Network::MessageHandler
	{
	public:
		ConsoleWatcherCBMessageHandler():
		  Network::MessageHandler()
		{
			onInstall();
		}

		virtual void onInstall()
		{
			// ǿ������Э��ID
			msgID = CONSOLE_WATCHERCB_MSGID;
			msgLen = NETWORK_VARIABLE_MESSAGE;
			name = "console::queryWatcher";
		}

		virtual void handle(Network::Channel* pChannel, MemoryStream& s)
		{
		};
	};

	class ConsoleProfileHandlerArgsStream : public Network::MessageArgs	
	{				
	public:
		ConsoleProfileHandlerArgsStream():Network::MessageArgs(){}				
		~ConsoleProfileHandlerArgsStream(){}	
	
		virtual int32 dataSize(void)				
		{
			return NETWORK_VARIABLE_MESSAGE;			
		}		

		virtual MessageArgs::MESSAGE_ARGS_TYPE type(void)		
		{														
			return MESSAGE_ARGS_TYPE_VARIABLE;					
		}												

		virtual int32 msglenMax()
		{ 
			return NETWORK_MESSAGE_MAX_SIZE * 1000; 
		}

		virtual void addToStream(MemoryStream& s)
		{
		}			

		virtual void createFromStream(MemoryStream& s)				
		{	
		}	
	};	
				
	class ConsoleProfileHandler : public Network::MessageHandler
	{
	public:
		ConsoleProfileHandler():
		  Network::MessageHandler()
		{
			onInstall();
		}

		virtual void onInstall()
		{
			// ǿ������Э��ID
			msgID = CONSOLE_PROFILECB_MSGID;
			msgLen = NETWORK_VARIABLE_MESSAGE;
			name = "console::profile";
		}

		virtual void handle(Network::Channel* pChannel, MemoryStream& s)
		{
		};
	};

	class ConsoleQueryAppsLoadsHandler : public Network::MessageHandler
	{
	public:
		ConsoleQueryAppsLoadsHandler() :
			Network::MessageHandler()
		{
			onInstall();
		}

		virtual void onInstall()
		{
			// ǿ������Э��ID
			msgID = CONSOLE_PROFILECB_MSGID;
			msgLen = NETWORK_VARIABLE_MESSAGE;
			name = "console::queryAppsLoads";
		}

		virtual void handle(Network::Channel* pChannel, MemoryStream& s)
		{
		};
	};

	class ConsoleQuerySpacesHandler : public Network::MessageHandler
	{
	public:
		ConsoleQuerySpacesHandler() :
			Network::MessageHandler()
		{
			onInstall();
		}

		virtual void onInstall()
		{
			// ǿ������Э��ID
			msgID = CONSOLE_PROFILECB_MSGID;
			msgLen = NETWORK_VARIABLE_MESSAGE;
			name = "console::querySpaces";
		}

		virtual void handle(Network::Channel* pChannel, MemoryStream& s)
		{
		};
	};
}
}

#endif // KBE_CONSOLE_HELPER_H
