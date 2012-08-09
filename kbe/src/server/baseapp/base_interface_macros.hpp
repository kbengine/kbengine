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


#if defined(DEFINE_IN_INTERFACE)
	#undef __BASE_INTERFACE_MACRO_H__
#endif


#ifndef __BASE_INTERFACE_MACRO_H__
#define __BASE_INTERFACE_MACRO_H__

// common include	
#include "network/interface_defs.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	BASE消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASE_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define BASE_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##BaseMessagehandler_stream::handle(Mercury::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Base* e =													\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			if(e)																\
			{																	\
				e->NAME(pChannel, s);											\
			}																	\
			else																\
			{																	\
				ERROR_MSG("Messagehandler::handle: can't found entityID:%d.\n",	\
					eid);														\
			}																	\
	}																			\

#else
#define BASE_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##BaseMessagehandler_stream::handle(Mercury::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define BASE_MESSAGE_HANDLER_STREAM(NAME)										\
	class NAME##BaseMessagehandler_stream : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASE_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)							\
	BASE_MESSAGE_HANDLER_STREAM(NAME)											\
	NETWORK_MESSAGE_DECLARE_STREAM(Base, NAME,									\
				NAME##BaseMessagehandler_stream, MSG_LENGTH)					\
																				\

/**
	Base消息宏，  只有零个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASE_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define BASE_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##BaseMessagehandler0::handle(Mercury::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Base* e =													\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			if(e)																\
			{																	\
				e->NAME(pChannel);												\
			}																	\
			else																\
			{																	\
				ERROR_MSG("Messagehandler::handle: can't found entityID:%d.\n",	\
					eid);														\
			}																	\
	}																			\

#else
#define BASE_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##BaseMessagehandler0::handle(Mercury::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
		
#endif
#else
#define BASE_MESSAGE_HANDLER_ARGS0(NAME)										\
	class NAME##BaseMessagehandler0 : public Mercury::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASE_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)							\
	BASE_MESSAGE_HANDLER_ARGS0(NAME)											\
	NETWORK_MESSAGE_DECLARE_ARGS0(Base, NAME,									\
				NAME##BaseMessagehandler0, MSG_LENGTH)							\
																				\

/**
	Base消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASE_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define BASE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##BaseMessagehandler1::handle(Mercury::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Base* e =													\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			if(e)																\
			{																	\
				e->NAME(pChannel, ARG_NAME1);									\
			}																	\
			else																\
			{																	\
				ERROR_MSG("Messagehandler::handle: can't found entityID:%d.\n",	\
					eid);														\
			}																	\
	}																			\

#else
#define BASE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##BaseMessagehandler1::handle(Mercury::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
		
#endif
#else
#define BASE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	class NAME##BaseMessagehandler1 : public Mercury::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASE_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)		\
	BASE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)						\
	NETWORK_MESSAGE_DECLARE_ARGS1(Base, NAME,									\
				NAME##BaseMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\


}
#endif
