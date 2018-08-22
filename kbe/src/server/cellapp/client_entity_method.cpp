// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "witness.h"
#include "cellapp.h"
#include "entitydef/method.h"
#include "client_entity_method.h"
#include "network/bundle.h"
#include "helper/eventhistory_stats.h"
#include "network/network_stats.h"

#include "client_lib/client_interface.h"
#include "../../server/baseapp/baseapp_interface.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
ClientEntityMethod::ClientEntityMethod(PropertyDescription* pComponentPropertyDescription,
	const ScriptDefModule* pScriptModule, MethodDescription* methodDescription,
		ENTITY_ID srcEntityID, ENTITY_ID clientEntityID):
script::ScriptObject(getScriptType(), false),
pComponentPropertyDescription_(pComponentPropertyDescription),
pScriptModule_(pScriptModule),
methodDescription_(methodDescription),
srcEntityID_(srcEntityID),
clientEntityID_(clientEntityID)
{
}

//-------------------------------------------------------------------------------------
ClientEntityMethod::~ClientEntityMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* ClientEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{
	ClientEntityMethod* rmethod = static_cast<ClientEntityMethod*>(self);
	return rmethod->callmethod(args, kwds);	
}		

//-------------------------------------------------------------------------------------
PyObject* ClientEntityMethod::callmethod(PyObject* args, PyObject* kwds)
{
	Entity* srcEntity = Cellapp::getSingleton().findEntity(srcEntityID_);

	if(srcEntity == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Entity::clientEntity(%s): srcEntityID(%d) not found!\n",
			methodDescription_->getName(), srcEntityID_);		
		PyErr_PrintEx(0);
		return 0;
	}

	if(srcEntity->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "Entity::clientEntity(%s): srcEntityID(%d) is destroyed!\n",
			methodDescription_->getName(), srcEntityID_);
		PyErr_PrintEx(0);
		return 0;
	}

	if(!srcEntity->isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity(%s): not is real entity, srcEntityID(%d).\n",
			srcEntity->scriptName(), methodDescription_->getName(), srcEntity->id());		
		PyErr_PrintEx(0);
		return 0;
	}
	
	if(srcEntity->pWitness() == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity(%s): no client, srcEntityID(%d).\n",
			srcEntity->scriptName(), methodDescription_->getName(), srcEntity->id());		
		PyErr_PrintEx(0);
		return 0;
	}

	Network::Channel* pChannel = srcEntity->pWitness()->pChannel();
	if(!pChannel)
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity(%s): no client, srcEntityID(%d).\n",
			srcEntity->scriptName(), methodDescription_->getName(), srcEntity->id());		
		PyErr_PrintEx(0);
		return 0;
	}
			
	EntityRef* pEntityRef = srcEntity->pWitness()->getViewEntityRef(clientEntityID_);
	Entity* e = (pEntityRef && ((pEntityRef->flags() & (ENTITYREF_FLAG_ENTER_CLIENT_PENDING | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING)) <= 0))
		? pEntityRef->pEntity() : NULL;

	if(e == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity(%s): not found entity(%d), srcEntityID(%d).\n",
			srcEntity->scriptName(), methodDescription_->getName(), clientEntityID_, srcEntity->id());	

		PyErr_PrintEx(0);

		return 0;
	}

	MethodDescription* methodDescription = getDescription();
	if(methodDescription->checkArgs(args))
	{
		MemoryStream* mstream = MemoryStream::createPoolObject(OBJECTPOOL_POINT);

		// 如果是广播给组件的消息
		if (pComponentPropertyDescription_)
		{
			if (pScriptModule_->usePropertyDescrAlias())
				(*mstream) << pComponentPropertyDescription_->aliasIDAsUint8();
			else
				(*mstream) << pComponentPropertyDescription_->getUType();
		}
		else
		{
			if (pScriptModule_->usePropertyDescrAlias())
				(*mstream) << (uint8)0;
			else
				(*mstream) << (ENTITY_PROPERTY_UID)0;
		}

		try
		{
			methodDescription->addToStream(mstream, args);
		}
		catch (MemoryStreamWriteOverflow & err)
		{
			PyErr_Format(PyExc_AssertionError, "%s::clientEntity(%s): srcEntityID(%d), error=%s!\n",
				srcEntity->scriptName(), methodDescription_->getName(), srcEntity->id(), err.what().c_str());
			PyErr_PrintEx(0);

			MemoryStream::reclaimPoolObject(mstream);
			S_Return;
		}

		Network::Bundle* pSendBundle = pChannel->createSendBundle();
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_BEGIN(srcEntity->id(), (*pSendBundle));

		int ialiasID = -1;
		const Network::MessageHandler& msgHandler = 
				srcEntity->pWitness()->getViewEntityMessageHandler(ClientInterface::onRemoteMethodCall, 
				ClientInterface::onRemoteMethodCallOptimized, clientEntityID_, ialiasID);

		ENTITY_MESSAGE_FORWARD_CLIENT_BEGIN(pSendBundle, msgHandler, viewEntityMessage);

		if(ialiasID != -1)
		{
			KBE_ASSERT(msgHandler.msgID == ClientInterface::onRemoteMethodCallOptimized.msgID);
			(*pSendBundle)  << (uint8)ialiasID;
		}
		else
		{
			KBE_ASSERT(msgHandler.msgID == ClientInterface::onRemoteMethodCall.msgID);
			(*pSendBundle)  << clientEntityID_;
		}
			
		if(mstream->wpos() > 0)
			(*pSendBundle).append(mstream->data(), (int)mstream->wpos());

		if(Network::g_trace_packet > 0)
		{
			if(Network::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger("packetlogs");

			DEBUG_MSG(fmt::format("ClientEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteOtherEntityMethodCall({}::{})\n",
				srcEntity->scriptName(), methodDescription->getName()));

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

		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, msgHandler, viewEntityMessage);

		// 记录这个事件产生的数据量大小
		g_publicClientEventHistoryStats.trackEvent(srcEntity->scriptName(), 
			(std::string(e->scriptName()) + "." + methodDescription->getName()), 
			pSendBundle->currMsgLength(), 
			"::");
		
		srcEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCallOptimized, pSendBundle);

		MemoryStream::reclaimPoolObject(mstream);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------

}
