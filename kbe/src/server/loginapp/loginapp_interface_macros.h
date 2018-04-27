// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_LOGINAPP_INTERFACE_MACRO_H
#endif


#ifndef KBE_LOGINAPP_INTERFACE_MACRO_H
#define KBE_LOGINAPP_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Loginapp消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_STREAM
	#undef LOGINAPP_MESSAGE_EXPOSED
#endif


#define LOGINAPP_MESSAGE_EXPOSED(NAME)											\
	NETWORK_MESSAGE_EXPOSED(Loginapp, NAME)										\


#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##LoginappMessagehandler_stream::handle(Network::Channel* pChannel,\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::Loginapp::getSingleton().NAME(pChannel, s);				\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##LoginappMessagehandler_stream::handle(Network::Channel* pChannel,\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_STREAM(NAME)									\
	class NAME##LoginappMessagehandler_stream : public Network::MessageHandler	\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)						\
	LOGINAPP_MESSAGE_HANDLER_STREAM(NAME)										\
	NETWORK_MESSAGE_DECLARE_STREAM(Loginapp, NAME,								\
				NAME##LoginappMessagehandler_stream, MSG_LENGTH)				\
																				\

/**
	Loginapp消息宏，  只有零个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##LoginappMessagehandler0::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Loginapp::getSingleton().NAME(pChannel);					\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##LoginappMessagehandler0::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS0(NAME)									\
	class NAME##LoginappMessagehandler0 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)						\
	LOGINAPP_MESSAGE_HANDLER_ARGS0(NAME)										\
	NETWORK_MESSAGE_DECLARE_ARGS0(Loginapp, NAME,								\
				NAME##LoginappMessagehandler0, MSG_LENGTH)						\
																				\

/**
	Loginapp消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##LoginappMessagehandler1::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Loginapp::getSingleton().NAME(pChannel, ARG_NAME1);		\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##LoginappMessagehandler1::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	class NAME##LoginappMessagehandler1 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
	LOGINAPP_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	NETWORK_MESSAGE_DECLARE_ARGS1(Loginapp, NAME,								\
				NAME##LoginappMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)\
																				\

/**
	Loginapp消息宏，  只有二个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##LoginappMessagehandler2::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Loginapp::getSingleton().NAME(pChannel,					\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##LoginappMessagehandler2::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##LoginappMessagehandler2 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\
	LOGINAPP_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Loginapp, NAME,								\
				NAME##LoginappMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2)				\

/**
	Loginapp消息宏，  只有三个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS3
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##LoginappMessagehandler3::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			KBEngine::Loginapp::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3);														\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##LoginappMessagehandler3::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	class NAME##LoginappMessagehandler3 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	LOGINAPP_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	NETWORK_MESSAGE_DECLARE_ARGS3(Loginapp, NAME,								\
				NAME##LoginappMessagehandler3, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\


/**
	Loginapp消息宏，  只有四个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##LoginappMessagehandler4::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			KBEngine::Loginapp::getSingleton().NAME(pChannel,					\
													ARG_NAME1, ARG_NAME2, 		\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##LoginappMessagehandler4::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##LoginappMessagehandler4 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	LOGINAPP_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Loginapp, NAME,								\
				NAME##LoginappMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Loginapp消息宏，  只有五个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##LoginappMessagehandler5::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			ARG_TYPE5 ARG_NAME5;												\
			s >> ARG_NAME5;														\
			KBEngine::Loginapp::getSingleton().NAME(pChannel,					\
													ARG_NAME1, ARG_NAME2, 		\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##LoginappMessagehandler5::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##LoginappMessagehandler5 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	LOGINAPP_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Loginapp, NAME,								\
				NAME##LoginappMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Loginapp消息宏，  只有六个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##LoginappMessagehandler6::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			ARG_TYPE5 ARG_NAME5;												\
			s >> ARG_NAME5;														\
			ARG_TYPE6 ARG_NAME6;												\
			s >> ARG_NAME6;														\
			KBEngine::Loginapp::getSingleton().NAME(pChannel,					\
													ARG_NAME1, ARG_NAME2, 		\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##LoginappMessagehandler6::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##LoginappMessagehandler6 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	LOGINAPP_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Loginapp, NAME,								\
				NAME##LoginappMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

/**
	Loginapp消息宏，  只有八个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef LOGINAPP_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(LOGINAPP)
#define LOGINAPP_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##LoginappMessagehandler8::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			ARG_TYPE5 ARG_NAME5;												\
			s >> ARG_NAME5;														\
			ARG_TYPE6 ARG_NAME6;												\
			s >> ARG_NAME6;														\
			ARG_TYPE7 ARG_NAME7;												\
			s >> ARG_NAME7;														\
			ARG_TYPE8 ARG_NAME8;												\
			s >> ARG_NAME8;														\
			KBEngine::Loginapp::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##LoginappMessagehandler8::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define LOGINAPP_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##LoginappMessagehandler8 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define LOGINAPP_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	LOGINAPP_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(Loginapp, NAME,								\
				NAME##LoginappMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\

}
#endif
