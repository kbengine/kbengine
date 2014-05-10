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

#include "witness.hpp"
#include "cellapp.hpp"
#include "entitydef/method.hpp"
#include "clients_remote_entity_method.hpp"
#include "network/bundle.hpp"
#include "server/eventhistory_stats.hpp"

#include "client_lib/client_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientsRemoteEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
ClientsRemoteEntityMethod::ClientsRemoteEntityMethod(MethodDescription* methodDescription,
													 bool otherClients,
													 ENTITY_ID id):
script::ScriptObject(getScriptType(), false),
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
	// 获取entityAOI范围内其他entity
	// 向这些entity的client推送这个方法的调用
	MethodDescription* methodDescription = getDescription();

	Entity* pEntity = Cellapp::getSingleton().findEntity(id_);
	if(pEntity == NULL || /*pEntity->pWitness() == NULL ||*/
		pEntity->isDestroyed() /*|| pEntity->getClientMailbox() == NULL*/)
	{
		//WARNING_MSG(boost::format("EntityRemoteMethod::callClientMethod: not found entity(%1%).\n") % 
		//	mailbox->getID());

		S_Return;
	}
	
	const std::list<ENTITY_ID>& entities = pEntity->witnesses();

	if(otherClients_)
	{
		if(entities.size() == 0)
			S_Return;
	}
	
	// 先发给自己
	if(methodDescription->checkArgs(args))
	{
		MemoryStream* mstream = MemoryStream::ObjPool().createObject();
		methodDescription->addToStream(mstream, args);

		if((!otherClients_ && (pEntity->pWitness() || (pEntity->getClientMailbox()))))
		{
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			pEntity->getClientMailbox()->newMail((*pBundle));

			if(mstream->wpos() > 0)
				(*pBundle).append(mstream->data(), mstream->wpos());

			if(Mercury::g_trace_packet > 0)
			{
				if(Mercury::g_trace_packet_use_logfile)
					DebugHelper::getSingleton().changeLogger("packetlogs");

				DEBUG_MSG(boost::format("ClientsRemoteEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteMethodCall(%1%::%2%)\n") % 
					pEntity->getScriptName() % methodDescription->getName());
																									
				switch(Mercury::g_trace_packet)																	
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

				if(Mercury::g_trace_packet_use_logfile)	
					DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(g_componentType));																				
			}

			//mailbox->postMail((*pBundle));
			pEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCall, pBundle);

			//Mercury::Bundle::ObjPool().reclaimObject(pBundle);

			// 记录这个事件产生的数据量大小
			g_publicClientEventHistoryStats.trackEvent(pEntity->getScriptName(), 
				methodDescription->getName(), 
				pBundle->currMsgLength(), 
				"::");
		}
		
		// 广播给其他人
		std::list<ENTITY_ID>::const_iterator iter = entities.begin();
		for(; iter != entities.end(); iter++)
		{
			Entity* pAoiEntity = Cellapp::getSingleton().findEntity((*iter));
			if(pAoiEntity == NULL || pAoiEntity->pWitness() == NULL || pAoiEntity->isDestroyed())
				continue;
			
			EntityMailbox* mailbox = pAoiEntity->getClientMailbox();
			if(mailbox == NULL)
				continue;

			Mercury::Channel* pChannel = mailbox->getChannel();
			if(pChannel == NULL)
				continue;

			Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
			Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
			
			pAoiEntity->pWitness()->addSmartAOIEntityMessageToBundle(pForwardBundle, ClientInterface::onRemoteMethodCall, 
					ClientInterface::onRemoteMethodCallOptimized, pEntity->getID());

			if(mstream->wpos() > 0)
				(*pForwardBundle).append(mstream->data(), mstream->wpos());

			if(Mercury::g_trace_packet > 0)
			{
				if(Mercury::g_trace_packet_use_logfile)
					DebugHelper::getSingleton().changeLogger("packetlogs");

				DEBUG_MSG(boost::format("ClientsRemoteEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteOtherEntityMethodCall(%1%::%2%)\n") % 
					pAoiEntity->getScriptName() % methodDescription->getName());
																									
				switch(Mercury::g_trace_packet)																	
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

				if(Mercury::g_trace_packet_use_logfile)	
					DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(g_componentType));																				
			}

			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pAoiEntity->getID(), (*pSendBundle), (*pForwardBundle));

			//mailbox->postMail((*pBundle));
			pAoiEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCallOptimized, pSendBundle);

			//Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);

			// 记录这个事件产生的数据量大小
			g_publicClientEventHistoryStats.trackEvent(pAoiEntity->getScriptName(), 
				methodDescription->getName(), 
				pForwardBundle->currMsgLength(), 
				"::");

			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
		}

		MemoryStream::ObjPool().reclaimObject(mstream);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------

}
