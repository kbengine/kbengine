// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_CELLAPPMG_INTERFACE_MACRO_H
#endif


#ifndef KBE_CELLAPPMG_INTERFACE_MACRO_H
#define KBE_CELLAPPMG_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Cellappmgr消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##CellappmgrMessagehandler_stream::handle(Network::Channel* pChannel,\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel, s);				\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##CellappmgrMessagehandler_stream::handle(Network::Channel* pChannel,\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	class NAME##CellappmgrMessagehandler_stream : public Network::MessageHandler\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)						\
	CELLAPPMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	NETWORK_MESSAGE_DECLARE_STREAM(Cellappmgr, NAME,							\
				NAME##CellappmgrMessagehandler_stream, MSG_LENGTH)				\
																				\

/**
	Cellappmgr消息宏，  只有零个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##CellappmgrMessagehandler0::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel);				\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##CellappmgrMessagehandler0::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS0(NAME)									\
	class NAME##CellappmgrMessagehandler0 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)						\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS0(NAME)										\
	NETWORK_MESSAGE_DECLARE_ARGS0(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler0, MSG_LENGTH)					\
																				\


/**
	Cellappmgr消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##CellappmgrMessagehandler1::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel, ARG_NAME1);		\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##CellappmgrMessagehandler1::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	class NAME##CellappmgrMessagehandler1 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	NETWORK_MESSAGE_DECLARE_ARGS1(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\


/**
	Cellappmgr消息宏，  只有二个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##CellappmgrMessagehandler2::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##CellappmgrMessagehandler2::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##CellappmgrMessagehandler2 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2)				\


/**
	Cellappmgr消息宏，  只有三个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS3
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##CellappmgrMessagehandler3::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3);														\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##CellappmgrMessagehandler3::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	class NAME##CellappmgrMessagehandler3 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	NETWORK_MESSAGE_DECLARE_ARGS3(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler3, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\



/**
	Cellappmgr消息宏，  只有四个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##CellappmgrMessagehandler4::handle(Network::Channel* pChannel,	\
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
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##CellappmgrMessagehandler4::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##CellappmgrMessagehandler4 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Cellappmgr消息宏，  只有五个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##CellappmgrMessagehandler5::handle(Network::Channel* pChannel,	\
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
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##CellappmgrMessagehandler5::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##CellappmgrMessagehandler5 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Cellappmgr消息宏，  只有六个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##CellappmgrMessagehandler6::handle(Network::Channel* pChannel,	\
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
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##CellappmgrMessagehandler6::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##CellappmgrMessagehandler6 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

/**
	Cellappmgr消息宏，  只有八个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##CellappmgrMessagehandler8::handle(Network::Channel* pChannel,		\
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
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##CellappmgrMessagehandler8::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##CellappmgrMessagehandler8 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\


/**
	Cellappmgr消息宏，  只有九个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS9
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##CellappmgrMessagehandler9::handle(Network::Channel* pChannel,	\
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
			ARG_TYPE9 ARG_NAME9;												\
			s >> ARG_NAME9;														\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9);		\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##CellappmgrMessagehandler9::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	class NAME##CellappmgrMessagehandler9 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS9(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	NETWORK_MESSAGE_DECLARE_ARGS9(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler9, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\


/**
	Cellappmgr消息宏，  只有十个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS10
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##CellappmgrMessagehandler10::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE9 ARG_NAME9;												\
			s >> ARG_NAME9;														\
			ARG_TYPE10 ARG_NAME10;												\
			s >> ARG_NAME10;													\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10);							\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##CellappmgrMessagehandler10::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	class NAME##CellappmgrMessagehandler10 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS10(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	NETWORK_MESSAGE_DECLARE_ARGS10(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler10, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\


/**
	Cellappmgr消息宏，  只有十一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CELLAPPMGR_MESSAGE_HANDLER_ARGS11
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CELLAPPMGR)
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	void NAME##CellappmgrMessagehandler11::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE9 ARG_NAME9;												\
			s >> ARG_NAME9;														\
			ARG_TYPE10 ARG_NAME10;												\
			s >> ARG_NAME10;													\
			ARG_TYPE11 ARG_NAME11;												\
			s >> ARG_NAME11;													\
			KBEngine::Cellappmgr::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11);				\
	}																			\

#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	void NAME##CellappmgrMessagehandler11::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define CELLAPPMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	class NAME##CellappmgrMessagehandler11 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CELLAPPMGR_MESSAGE_DECLARE_ARGS11(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	CELLAPPMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	NETWORK_MESSAGE_DECLARE_ARGS11(Cellappmgr, NAME,								\
				NAME##CellappmgrMessagehandler11, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\


}
#endif
