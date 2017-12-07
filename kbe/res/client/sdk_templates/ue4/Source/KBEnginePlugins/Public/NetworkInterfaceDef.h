// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "KBECommon.h"

#ifdef KBENGINEAPP
#include "KBEngine.h"
#include "Message.h"
#endif


#ifdef NETWORK_MESSAGE_HANDLER_STREAM
	#undef NETWORK_MESSAGE_HANDLER_STREAM
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_STREAM(NAME, MSG_LENGTH)												\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), NETWORK_VARIABLE_MESSAGE);													\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		KBEngineApp::getSingleton().NAME(s);															\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_STREAM(NAME, MSG_LENGTH)												\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS0
#undef NETWORK_MESSAGE_HANDLER_ARGS0
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS0(NAME, MSG_LENGTH)													\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), NETWORK_FIXED_MESSAGE);													\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		KBEngineApp::getSingleton().NAME();																\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS0(NAME, MSG_LENGTH)													\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS1
#undef NETWORK_MESSAGE_HANDLER_ARGS1
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		s >> ARG_NAME1;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1);													\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS2
#undef NETWORK_MESSAGE_HANDLER_ARGS2
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2);											\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS3
#undef NETWORK_MESSAGE_HANDLER_ARGS3
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3);								\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS4
#undef NETWORK_MESSAGE_HANDLER_ARGS4
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		ARG_TYPE4 ARG_NAME4;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		s >> ARG_NAME4;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4);					\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS5
#undef NETWORK_MESSAGE_HANDLER_ARGS5
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		ARG_TYPE4 ARG_NAME4;																			\
		ARG_TYPE5 ARG_NAME5;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		s >> ARG_NAME4;																					\
		s >> ARG_NAME5;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4, ARG_NAME5);		\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS6
#undef NETWORK_MESSAGE_HANDLER_ARGS6
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		ARG_TYPE4 ARG_NAME4;																			\
		ARG_TYPE5 ARG_NAME5;																			\
		ARG_TYPE6 ARG_NAME6;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		s >> ARG_NAME4;																					\
		s >> ARG_NAME5;																					\
		s >> ARG_NAME6;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4, ARG_NAME5,			\
										ARG_NAME6);														\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS7
#undef NETWORK_MESSAGE_HANDLER_ARGS7
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS7(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		ARG_TYPE4 ARG_NAME4;																			\
		ARG_TYPE5 ARG_NAME5;																			\
		ARG_TYPE6 ARG_NAME6;																			\
		ARG_TYPE7 ARG_NAME7;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		s >> ARG_NAME4;																					\
		s >> ARG_NAME5;																					\
		s >> ARG_NAME6;																					\
		s >> ARG_NAME7;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4, ARG_NAME5,			\
										ARG_NAME6, ARG_NAME7);											\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS7(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS8
#undef NETWORK_MESSAGE_HANDLER_ARGS8
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7							\
														, ARG_TYPE8, ARG_NAME8)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		ARG_TYPE4 ARG_NAME4;																			\
		ARG_TYPE5 ARG_NAME5;																			\
		ARG_TYPE6 ARG_NAME6;																			\
		ARG_TYPE7 ARG_NAME7;																			\
		ARG_TYPE8 ARG_NAME8;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		s >> ARG_NAME4;																					\
		s >> ARG_NAME5;																					\
		s >> ARG_NAME6;																					\
		s >> ARG_NAME7;																					\
		s >> ARG_NAME8;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4, ARG_NAME5,			\
										ARG_NAME6, ARG_NAME7, ARG_NAME8);								\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7							\
														, ARG_TYPE8, ARG_NAME8)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS9
#undef NETWORK_MESSAGE_HANDLER_ARGS9
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS9(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7							\
														, ARG_TYPE8, ARG_NAME8							\
														, ARG_TYPE9, ARG_NAME9)							\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		ARG_TYPE4 ARG_NAME4;																			\
		ARG_TYPE5 ARG_NAME5;																			\
		ARG_TYPE6 ARG_NAME6;																			\
		ARG_TYPE7 ARG_NAME7;																			\
		ARG_TYPE8 ARG_NAME8;																			\
		ARG_TYPE9 ARG_NAME9;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		s >> ARG_NAME4;																					\
		s >> ARG_NAME5;																					\
		s >> ARG_NAME6;																					\
		s >> ARG_NAME7;																					\
		s >> ARG_NAME8;																					\
		s >> ARG_NAME9;																					\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4, ARG_NAME5,			\
										ARG_NAME6, ARG_NAME7, ARG_NAME8, ARG_NAME9);					\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS9(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7							\
														, ARG_TYPE8, ARG_NAME8							\
														, ARG_TYPE9, ARG_NAME9)							\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif



#ifdef NETWORK_MESSAGE_HANDLER_ARGS10
#undef NETWORK_MESSAGE_HANDLER_ARGS10
#endif


#ifdef KBENGINEAPP

#define NETWORK_MESSAGE_HANDLER_ARGS10(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7							\
														, ARG_TYPE8, ARG_NAME8							\
														, ARG_TYPE9, ARG_NAME9							\
														, ARG_TYPE10, ARG_NAME10)						\
	const Message* pMessage##NAME = g_Messages.add(new NAME##KBENetworkMessagehandler(), 0,				\
				TEXT(#NAME), MSG_LENGTH);																\
	void NAME##KBENetworkMessagehandler::handle(MemoryStream& s)										\
	{																									\
		ARG_TYPE1 ARG_NAME1;																			\
		ARG_TYPE2 ARG_NAME2;																			\
		ARG_TYPE3 ARG_NAME3;																			\
		ARG_TYPE4 ARG_NAME4;																			\
		ARG_TYPE5 ARG_NAME5;																			\
		ARG_TYPE6 ARG_NAME6;																			\
		ARG_TYPE7 ARG_NAME7;																			\
		ARG_TYPE8 ARG_NAME8;																			\
		ARG_TYPE9 ARG_NAME9;																			\
		ARG_TYPE10 ARG_NAME10;																			\
		s >> ARG_NAME1;																					\
		s >> ARG_NAME2;																					\
		s >> ARG_NAME3;																					\
		s >> ARG_NAME4;																					\
		s >> ARG_NAME5;																					\
		s >> ARG_NAME6;																					\
		s >> ARG_NAME7;																					\
		s >> ARG_NAME8;																					\
		s >> ARG_NAME9;																					\
		s >> ARG_NAME10;																				\
		KBEngineApp::getSingleton().NAME(ARG_NAME1, ARG_NAME2, ARG_NAME3, ARG_NAME4, ARG_NAME5,			\
										ARG_NAME6, ARG_NAME7, ARG_NAME8, ARG_NAME9, ARG_NAME10);		\
	}																									\

#else
#define NETWORK_MESSAGE_HANDLER_ARGS10(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1							\
														, ARG_TYPE2, ARG_NAME2							\
														, ARG_TYPE3, ARG_NAME3							\
														, ARG_TYPE4, ARG_NAME4							\
														, ARG_TYPE5, ARG_NAME5							\
														, ARG_TYPE6, ARG_NAME6							\
														, ARG_TYPE7, ARG_NAME7							\
														, ARG_TYPE8, ARG_NAME8							\
														, ARG_TYPE9, ARG_NAME9							\
														, ARG_TYPE10, ARG_NAME10)						\
	class KBENGINEPLUGINS_API NAME##KBENetworkMessagehandler : public Message							\
	{																									\
	public:																								\
		virtual void handle(MemoryStream& s);															\
	};																									\

#endif

#ifdef NETWORK_INTERFACE_DECLARE_BEGIN
#undef NETWORK_INTERFACE_DECLARE_BEGIN
#endif

#ifdef KBENGINEAPP
#define NETWORK_INTERFACE_DECLARE_BEGIN(INAME)	Messages g_Messages;	
#else
#define NETWORK_INTERFACE_DECLARE_BEGIN(INAME)	extern Messages g_Messages;	
#endif

#define NETWORK_INTERFACE_DECLARE_END() 










