// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_PROXY_INTERFACE_MACRO_H
#endif


#ifndef KBE_PROXY_INTERFACE_MACRO_H
#define KBE_PROXY_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	PROXY消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef PROXY_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define PROXY_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##ProxyMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Proxy* e =												\
					static_cast<KBEngine::Proxy*>(								\
				KBEngine::Baseapp::getSingleton().findEntity(eid));				\
			if(e)																\
			{																	\
				e->NAME(pChannel, s);											\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					"Messagehandler::handle: can't found entityID:{}.\n",		\
					eid));														\
			}																	\
	}																			\

#else
#define PROXY_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##ProxyMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define PROXY_MESSAGE_HANDLER_STREAM(NAME)										\
	class NAME##ProxyMessagehandler_stream : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define PROXY_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)							\
	PROXY_MESSAGE_HANDLER_STREAM(NAME)											\
	NETWORK_MESSAGE_DECLARE_STREAM(Proxy, NAME,									\
				NAME##ProxyMessagehandler_stream, MSG_LENGTH)					\
																				\

/**
	Proxy消息宏，  只有零个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef PROXY_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define PROXY_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##ProxyMessagehandler0::handle(Network::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Proxy* e =												\
					static_cast<KBEngine::Proxy*>(								\
				KBEngine::Baseapp::getSingleton().findEntity(eid));				\
			if(e)																\
			{																	\
				e->NAME(pChannel);												\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					"Messagehandler::handle: can't found entityID:{}.\n",		\
					eid));														\
			}																	\
	}																			\

#else
#define PROXY_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##ProxyMessagehandler0::handle(Network::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
		
#endif
#else
#define PROXY_MESSAGE_HANDLER_ARGS0(NAME)										\
	class NAME##ProxyMessagehandler0 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define PROXY_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)							\
	PROXY_MESSAGE_HANDLER_ARGS0(NAME)											\
	NETWORK_MESSAGE_DECLARE_ARGS0(Proxy, NAME,									\
				NAME##ProxyMessagehandler0, MSG_LENGTH)							\
																				\
	
/**
	Proxy消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef PROXY_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define PROXY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##ProxyMessagehandler1::handle(Network::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Proxy* e =												\
					static_cast<KBEngine::Proxy*>(								\
				KBEngine::Baseapp::getSingleton().findEntity(eid));				\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			if(e)																\
			{																	\
				e->NAME(pChannel, ARG_NAME1);									\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					"Messagehandler::handle: can't found entityID:{}.\n",		\
					eid));														\
			}																	\
	}																			\

#else
#define PROXY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##ProxyMessagehandler1::handle(Network::Channel* pChannel,			\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
		
#endif
#else
#define PROXY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	class NAME##ProxyMessagehandler1 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define PROXY_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)		\
	PROXY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)						\
	NETWORK_MESSAGE_DECLARE_ARGS1(Proxy, NAME,									\
				NAME##ProxyMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\


}
#endif
