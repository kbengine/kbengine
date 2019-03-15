// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "witness.h"
#include "cellapp.h"
#include "entitydef/method.h"
#include "clients_remote_entity_method.h"
#include "network/bundle.h"
#include "network/network_stats.h"
#include "helper/eventhistory_stats.h"

#include "client_lib/client_interface.h"
#include "../../server/baseapp/baseapp_interface.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientsRemoteEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
ClientsRemoteEntityMethod::ClientsRemoteEntityMethod(PropertyDescription* pComponentPropertyDescription, 
													const ScriptDefModule* pScriptModule,
													MethodDescription* methodDescription,
													 bool otherClients,
													 ENTITY_ID id):
script::ScriptObject(getScriptType(), false),
pComponentPropertyDescription_(pComponentPropertyDescription),
pScriptModule_(pScriptModule),
methodDescription_(methodDescription),
otherClients_(otherClients),
id_(id)
{
}

//-------------------------------------------------------------------------------------
ClientsRemoteEntityMethod::~ClientsRemoteEntityMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* ClientsRemoteEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{
	ClientsRemoteEntityMethod* rmethod = static_cast<ClientsRemoteEntityMethod*>(self);
	return rmethod->callmethod(args, kwds);	
}		

//-------------------------------------------------------------------------------------
PyObject* ClientsRemoteEntityMethod::callmethod(PyObject* args, PyObject* kwds)
{
	// 获取entityView范围内其他entity
	// 向这些entity的client推送这个方法的调用
	MethodDescription* methodDescription = getDescription();

	Entity* pEntity = Cellapp::getSingleton().findEntity(id_);
	if(pEntity == NULL || /*pEntity->pWitness() == NULL ||*/
		pEntity->isDestroyed() /*|| pEntity->clientEntityCall() == NULL*/)
	{
		//WARNING_MSG(fmt::format("EntityRemoteMethod::callClientMethod: not found entity({}).\n", 
		//	entityCall->id()));

		S_Return;
	}
	
	const std::list<ENTITY_ID>& entities = pEntity->witnesses();

	if(otherClients_)
	{
		if(pEntity->witnessesSize() == 0)
			S_Return;
	}
	
	// 先发给自己
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
			ERROR_MSG(fmt::format("ClientsRemoteEntityMethod::callmethod: {}::{} {}, error={}!\n",
				pEntity->scriptName(), methodDescription->getName(), pEntity->id(), err.what()));

			MemoryStream::reclaimPoolObject(mstream);
			S_Return;
		}

		if((!otherClients_ && (pEntity->pWitness() && (pEntity->clientEntityCall()))))
		{
			Network::Bundle* pSendBundle = NULL;
			Network::Channel* pChannel = pEntity->clientEntityCall()->getChannel();

			if (!pChannel)
				pSendBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			else
				pSendBundle = pChannel->createSendBundle();

			pEntity->clientEntityCall()->newCall_((*pSendBundle));

			if(mstream->wpos() > 0)
				(*pSendBundle).append(mstream->data(), (int)mstream->wpos());

			if(Network::g_trace_packet > 0)
			{
				if(Network::g_trace_packet_use_logfile)
					DebugHelper::getSingleton().changeLogger("packetlogs");

				DEBUG_MSG(fmt::format("ClientsRemoteEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteMethodCall({}::{})\n", 
					pEntity->scriptName(), methodDescription->getName()));

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
			g_publicClientEventHistoryStats.trackEvent(pEntity->scriptName(),
				methodDescription->getName(),
				pSendBundle->currMsgLength(),
				"::");

			//entityCall->sendCall((*pBundle));
			pEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCall, pSendBundle);
		}

		// 广播给其他人
		std::list<ENTITY_ID>::const_iterator iter = entities.begin();
		for(; iter != entities.end(); ++iter)
		{
			Entity* pViewEntity = Cellapp::getSingleton().findEntity((*iter));
			if(pViewEntity == NULL || pViewEntity->pWitness() == NULL || pViewEntity->isDestroyed())
				continue;
			
			EntityCall* entityCall = pViewEntity->clientEntityCall();
			if(entityCall == NULL)
				continue;

			Network::Channel* pChannel = entityCall->getChannel();
			if(pChannel == NULL)
				continue;

			// 这个可能性是存在的，例如数据来源于createWitnessFromStream()
			// 又如自己的entity还未在目标客户端上创建
			if (!pViewEntity->pWitness()->entityInView(pEntity->id()))
				continue;
			
			Network::Bundle* pSendBundle = pChannel->createSendBundle();
			NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_BEGIN(pViewEntity->id(), (*pSendBundle));
			
			int ialiasID = -1;
			const Network::MessageHandler& msgHandler = 
			pViewEntity->pWitness()->getViewEntityMessageHandler(ClientInterface::onRemoteMethodCall, 
					ClientInterface::onRemoteMethodCallOptimized, pEntity->id(), ialiasID);

			ENTITY_MESSAGE_FORWARD_CLIENT_BEGIN(pSendBundle, msgHandler, viewEntityMessage);

			if(ialiasID != -1)
			{
				KBE_ASSERT(msgHandler.msgID == ClientInterface::onRemoteMethodCallOptimized.msgID);
				(*pSendBundle)  << (uint8)ialiasID;
			}
			else
			{
				KBE_ASSERT(msgHandler.msgID == ClientInterface::onRemoteMethodCall.msgID);
				(*pSendBundle)  << pEntity->id();
			}

			if(mstream->wpos() > 0)
				(*pSendBundle).append(mstream->data(), (int)mstream->wpos());

			if(Network::g_trace_packet > 0)
			{
				if(Network::g_trace_packet_use_logfile)
					DebugHelper::getSingleton().changeLogger("packetlogs");

				DEBUG_MSG(fmt::format("ClientsRemoteEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteOtherEntityMethodCall({}::{})\n", 
					pViewEntity->scriptName(), methodDescription->getName()));

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
			g_publicClientEventHistoryStats.trackEvent(pViewEntity->scriptName(), 
				methodDescription->getName(), 
				pSendBundle->currMsgLength(), 
				"::");

			pViewEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCallOptimized, pSendBundle);
		}

		MemoryStream::reclaimPoolObject(mstream);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------

}
