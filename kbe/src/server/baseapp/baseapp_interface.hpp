/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#if defined(DEFINE_IN_INTERFACE)
	#undef __BASEAPP_INTERFACE_H__
#endif


#ifndef __BASEAPP_INTERFACE_H__
#define __BASEAPP_INTERFACE_H__

// common include	
#if defined(BASEAPP)
#include "entity.hpp"
#include "baseapp.hpp"
#endif
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPP)
#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##EntityMessagehandler::handle(KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Entity* e =												\
					KBEngine::BaseApp::getSingleton().findEntity(eid);			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			e->NAME(ARG_NAME1);													\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##EntityMessagehandler::handle(KBEngine::MemoryStream& s)			\
	{																			\
	}																			\
		
#endif
#else
#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	class NAME##EntityMessagehandler : public Mercury::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_ARGS1(NAME, ARG_TYPE1, ARG_NAME)					\
	ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME)						\
	NETWORK_MESSAGE_DECLARE_ARGS1(Entity, NAME,									\
				NAME##EntityMessagehandler, ARG_TYPE1, ARG_NAME)				\
																				\


NETWORK_INTERFACE_DECLARE_BEGIN(BaseAppInterface)
	
	ENTITY_MESSAGE_DECLARE_ARGS1(test,
								std::string, name
	)
	
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
