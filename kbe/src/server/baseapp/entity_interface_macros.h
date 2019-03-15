// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_BASE_ENTITY_INTERFACE_MACRO_H
#endif


#ifndef KBE_BASE_ENTITY_INTERFACE_MACRO_H
#define KBE_BASE_ENTITY_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	BASE��Ϣ�꣬  ����Ϊ���� ��Ҫ�Լ��⿪
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define ENTITY_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##EntityMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
																				\
			if(pChannel->isExternal() && pChannel->proxyID() != eid)			\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: Illegal access to entityID:{}! proxyID={}\n",\
					eid, pChannel->proxyID()));									\
																				\
				pChannel->condemn(#NAME"handler::handle: Illegal access");		\
																				\
				return;															\
			}																	\
																				\
			KBEngine::Entity* e =												\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			if(e)																\
			{																	\
				e->NAME(pChannel, s);											\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: not found entityID:{}.\n",			\
					eid));														\
			}																	\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler_stream::type() const\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##EntityMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler_stream::type() const\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\
		
#endif
#else
#define ENTITY_MESSAGE_HANDLER_STREAM(NAME)										\
	class NAME##EntityMessagehandler_stream : public Network::MessageHandler	\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
		virtual Network::NETWORK_MESSAGE_TYPE type() const;						\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)							\
	ENTITY_MESSAGE_HANDLER_STREAM(NAME)											\
	NETWORK_MESSAGE_DECLARE_STREAM(Entity, NAME,								\
				NAME##EntityMessagehandler_stream, MSG_LENGTH)					\
																				\

/**
	Entity��Ϣ�꣬  ֻ�������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define ENTITY_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##EntityMessagehandler0::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
																				\
			if(pChannel->isExternal() && pChannel->proxyID() != eid)			\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: Illegal access to entityID:{}! proxyID={}\n",\
					eid, pChannel->proxyID()));									\
																				\
				pChannel->condemn(#NAME"handler::handle: Illegal access");		\
																				\
				return;															\
			}																	\
																				\
			KBEngine::Entity* e =												\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			if(e)																\
			{																	\
				e->NAME(pChannel);												\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: not found entityID:{}.\n",			\
					eid));														\
			}																	\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler0::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##EntityMessagehandler0::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler0::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\
		
#endif
#else
#define ENTITY_MESSAGE_HANDLER_ARGS0(NAME)										\
	class NAME##EntityMessagehandler0 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
		virtual Network::NETWORK_MESSAGE_TYPE type() const;						\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)							\
	ENTITY_MESSAGE_HANDLER_ARGS0(NAME)											\
	NETWORK_MESSAGE_DECLARE_ARGS0(Entity, NAME,									\
				NAME##EntityMessagehandler0, MSG_LENGTH)						\
																				\

/**
	Entity��Ϣ�꣬  ֻ��һ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##EntityMessagehandler1::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
																				\
			if(pChannel->isExternal() && pChannel->proxyID() != eid)			\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: Illegal access to entityID:{}! proxyID={}\n",\
					eid, pChannel->proxyID()));									\
																				\
				pChannel->condemn(#NAME"handler::handle: Illegal access");		\
																				\
				return;															\
			}																	\
																				\
			KBEngine::Entity* e =												\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			if(e)																\
			{																	\
				e->NAME(pChannel, ARG_NAME1);									\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: not found entityID:{}.\n",			\
					eid));														\
			}																	\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler1::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##EntityMessagehandler1::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler1::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\
		
#endif
#else
#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	class NAME##EntityMessagehandler1 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
		virtual Network::NETWORK_MESSAGE_TYPE type() const;						\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
	ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	NETWORK_MESSAGE_DECLARE_ARGS1(Entity, NAME,									\
				NAME##EntityMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\

/**
	Entity��Ϣ�꣬  ֻ�ж�����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define ENTITY_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2)					\
	void NAME##EntityMessagehandler2::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
																				\
			if(pChannel->isExternal() && pChannel->proxyID() != eid)			\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: Illegal access to entityID:{}! proxyID={}\n",\
					eid, pChannel->proxyID()));									\
																				\
				pChannel->condemn(#NAME"handler::handle: Illegal access");		\
																				\
				return;															\
			}																	\
																				\
			KBEngine::Entity* e =												\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			ARG_TYPE1 ARG_NAME1;												\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME1 >> ARG_NAME2;										\
			if(e)																\
			{																	\
				e->NAME(pChannel, ARG_NAME1, ARG_NAME2);						\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: not found entityID:{}.\n",			\
					eid));														\
			}																	\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler2::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2)					\
	void NAME##EntityMessagehandler2::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler2::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\
		
#endif
#else
#define ENTITY_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2)					\
	class NAME##EntityMessagehandler2 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
		virtual Network::NETWORK_MESSAGE_TYPE type() const;						\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
										ARG_TYPE2, ARG_NAME2)					\
	ENTITY_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
										ARG_TYPE2, ARG_NAME2)					\
	NETWORK_MESSAGE_DECLARE_ARGS2(Entity, NAME,									\
				NAME##EntityMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
										ARG_TYPE2, ARG_NAME2)					\
																				\



/**
	Entity��Ϣ�꣬  ֻ��������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_ARGS3
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define ENTITY_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3)					\
	void NAME##EntityMessagehandler3::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
																				\
			if(pChannel->isExternal() && pChannel->proxyID() != eid)			\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: Illegal access to entityID:{}! proxyID={}\n",\
					eid, pChannel->proxyID()));									\
																				\
				pChannel->condemn(#NAME"handler::handle: Illegal access");		\
																				\
				return;															\
			}																	\
																				\
			KBEngine::Entity* e =												\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			ARG_TYPE1 ARG_NAME1;												\
			ARG_TYPE2 ARG_NAME2;												\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME1 >> ARG_NAME2 >> ARG_NAME3;							\
			if(e)																\
			{																	\
				e->NAME(pChannel, ARG_NAME1, ARG_NAME2, ARG_NAME3);				\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: not found entityID:{}.\n",			\
					eid));														\
			}																	\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler3::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3)					\
	void NAME##EntityMessagehandler3::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler3::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\
		
#endif
#else
#define ENTITY_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3)					\
	class NAME##EntityMessagehandler3 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
		virtual Network::NETWORK_MESSAGE_TYPE type() const;						\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3)					\
	ENTITY_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,					\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3)					\
	NETWORK_MESSAGE_DECLARE_ARGS3(Entity, NAME,									\
				NAME##EntityMessagehandler3, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3)					\
																				\

/**
	Entity��Ϣ�꣬  ֻ���ĸ���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define ENTITY_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3,					\
										ARG_TYPE4, ARG_NAME4)					\
	void NAME##EntityMessagehandler4::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
																				\
			if(pChannel->isExternal() && pChannel->proxyID() != eid)			\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: Illegal access to entityID:{}! proxyID={}\n",\
					eid, pChannel->proxyID()));									\
																				\
				pChannel->condemn(#NAME"handler::handle: Illegal access");		\
																				\
				return;															\
			}																	\
																				\
			KBEngine::Entity* e =												\
					KBEngine::Baseapp::getSingleton().findEntity(eid);			\
			ARG_TYPE1 ARG_NAME1;												\
			ARG_TYPE2 ARG_NAME2;												\
			ARG_TYPE3 ARG_NAME3;												\
			ARG_TYPE3 ARG_NAME4;												\
			s >> ARG_NAME1 >> ARG_NAME2 >> ARG_NAME3 >> ARG_NAME4;				\
			if(e)																\
			{																	\
				e->NAME(pChannel, ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4);	\
			}																	\
			else																\
			{																	\
				ERROR_MSG(fmt::format(											\
					#NAME"handler::handle: not found entityID:{}.\n",			\
					eid));														\
			}																	\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler4::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3,					\
										ARG_TYPE4, ARG_NAME4)					\
	void NAME##EntityMessagehandler4::handle(Network::Channel* pChannel,		\
											KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
	Network::NETWORK_MESSAGE_TYPE NAME##EntityMessagehandler4::type() const		\
	{																			\
		return Network::NETWORK_MESSAGE_TYPE_ENTITY;							\
	}																			\
		
#endif
#else
#define ENTITY_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3,					\
										ARG_TYPE4, ARG_NAME4)					\
	class NAME##EntityMessagehandler4 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
		virtual Network::NETWORK_MESSAGE_TYPE type() const;						\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3,					\
										ARG_TYPE4, ARG_NAME4)					\
	ENTITY_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3,					\
										ARG_TYPE4, ARG_NAME4)					\
	NETWORK_MESSAGE_DECLARE_ARGS4(Entity, NAME,									\
				NAME##EntityMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
										ARG_TYPE2, ARG_NAME2,					\
										ARG_TYPE3, ARG_NAME3,					\
										ARG_TYPE4, ARG_NAME4)					\
																				\


}

#endif // KBE_BASE_ENTITY_INTERFACE_MACRO_H
