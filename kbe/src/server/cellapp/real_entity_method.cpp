// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "entity.h"
#include "real_entity_method.h"
#include "entitydef/method.h"
#include "network/bundle.h"
#include "helper/eventhistory_stats.h"

#include "cellapp_interface.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(RealEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(RealEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(RealEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(RealEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
RealEntityMethod::RealEntityMethod(PropertyDescription* pComponentPropertyDescription,
	MethodDescription* methodDescription, 
	Entity* ghostEntity):
script::ScriptObject(getScriptType(), false),
pComponentPropertyDescription_(pComponentPropertyDescription),
methodDescription_(methodDescription),
ghostEntityID_(ghostEntity->id()),
realCell_(ghostEntity->realCell()),
scriptName_(ghostEntity->scriptName())
{
}

//-------------------------------------------------------------------------------------
RealEntityMethod::~RealEntityMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* RealEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{
	RealEntityMethod* rmethod = static_cast<RealEntityMethod*>(self);
	return rmethod->callmethod(args, kwds);
}

//-------------------------------------------------------------------------------------
PyObject* RealEntityMethod::callmethod(PyObject* args, PyObject* kwds)
{
	GhostManager* gm = Cellapp::getSingleton().pGhostManager();
	if (!gm)
	{
		S_Return;
	}

	MethodDescription* methodDescription = getDescription();
	if(methodDescription->checkArgs(args))
	{
		MemoryStream* mstream = MemoryStream::createPoolObject(OBJECTPOOL_POINT);

		// 如果是给组件的消息
		if (pComponentPropertyDescription_)
		{
			(*mstream) << pComponentPropertyDescription_->getUType();
		}
		else
		{
			(*mstream) << (ENTITY_PROPERTY_UID)0;
		}

		try
		{
			methodDescription->addToStream(mstream, args);
		}
		catch (MemoryStreamWriteOverflow & err)
		{
			ERROR_MSG(fmt::format("RealEntityMethod::tp_call: {}::{} {}, error={}!\n",
				scriptName_, methodDescription->getName(), ghostEntityID_, err.what()));

			MemoryStream::reclaimPoolObject(mstream);
			S_Return;
		}

		Network::Bundle* pForwardBundle = gm->createSendBundle(realCell_);

		(*pForwardBundle).newMessage(CellappInterface::onRemoteRealMethodCall);
		(*pForwardBundle) << ghostEntityID_;

		if(mstream->wpos() > 0)
			(*pForwardBundle).append(mstream->data(), mstream->wpos());

		if(Network::g_trace_packet > 0)
		{
			if(Network::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger("packetlogs");

			DEBUG_MSG(fmt::format("RealEntityMethod::callmethod: pushUpdateData: CellappInterface::onRemoteRealMethodCall({2}({0})::{1})\n",
				ghostEntityID_, methodDescription->getName(), scriptName_));

			switch(Network::g_trace_packet)
			{
			case 1:	
				mstream->hexlike();
				break;
			case 2:	
				mstream->textlike();
				break;
			default:
				mstream->print_storage();
				break;
			};

			if(Network::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(g_componentType));
		}

		// 记录这个事件产生的数据量大小
		g_publicCellEventHistoryStats.trackEvent(scriptName_, 
			methodDescription->getName(), 
			pForwardBundle->currMsgLength(), 
			"::");

		MemoryStream::reclaimPoolObject(mstream);
		gm->pushMessage(realCell_, pForwardBundle);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------

}
