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
#include "helper/eventhistory_stats.hpp"

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
	// ��ȡentityAOI��Χ������entity
	// ����Щentity��client������������ĵ���
	MethodDescription* methodDescription = getDescription();

	Entity* pEntity = Cellapp::getSingleton().findEntity(id_);
	if(pEntity == NULL || /*pEntity->pWitness() == NULL ||*/
		pEntity->isDestroyed() /*|| pEntity->clientMailbox() == NULL*/)
	{
		//WARNING_MSG(fmt::format("EntityRemoteMethod::callClientMethod: not found entity({}).\n", 
		//	mailbox->id()));

		S_Return;
	}
	
	const std::list<ENTITY_ID>& entities = pEntity->witnesses();

	if(otherClients_)
	{
		if(pEntity->witnessesSize() == 0)
			S_Return;
	}
	
	// �ȷ����Լ�
	if(methodDescription->checkArgs(args))
	{
		MemoryStream* mstream = MemoryStream::ObjPool().createObject();
		methodDescription->addToStream(mstream, args);

		if((!otherClients_ && (pEntity->pWitness() || (pEntity->clientMailbox()))))
		{
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			pEntity->clientMailbox()->newMail((*pBundle));

			if(mstream->wpos() > 0)
				(*pBundle).append(mstream->data(), mstream->wpos());

			if(Mercury::g_trace_packet > 0)
			{
				if(Mercury::g_trace_packet_use_logfile)
					DebugHelper::getSingleton().changeLogger("packetlogs");

				DEBUG_MSG(fmt::format("ClientsRemoteEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteMethodCall({}::{})\n", 
					pEntity->scriptName(), methodDescription->getName()));
																									
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

			// ��¼����¼���������������С
			g_publicClientEventHistoryStats.trackEvent(pEntity->scriptName(), 
				methodDescription->getName(), 
				pBundle->currMsgLength(), 
				"::");
		}
		
		// �㲥��������
		std::list<ENTITY_ID>::const_iterator iter = entities.begin();
		for(; iter != entities.end(); iter++)
		{
			Entity* pAoiEntity = Cellapp::getSingleton().findEntity((*iter));
			if(pAoiEntity == NULL || pAoiEntity->pWitness() == NULL || pAoiEntity->isDestroyed())
				continue;
			
			EntityMailbox* mailbox = pAoiEntity->clientMailbox();
			if(mailbox == NULL)
				continue;

			Mercury::Channel* pChannel = mailbox->getChannel();
			if(pChannel == NULL)
				continue;

			if(!pAoiEntity->pWitness()->entityInAOI(pEntity->id()))
				continue;

			Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
			Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
			
			pAoiEntity->pWitness()->addSmartAOIEntityMessageToBundle(pForwardBundle, ClientInterface::onRemoteMethodCall, 
					ClientInterface::onRemoteMethodCallOptimized, pEntity->id());

			if(mstream->wpos() > 0)
				(*pForwardBundle).append(mstream->data(), mstream->wpos());

			if(Mercury::g_trace_packet > 0)
			{
				if(Mercury::g_trace_packet_use_logfile)
					DebugHelper::getSingleton().changeLogger("packetlogs");

				DEBUG_MSG(fmt::format("ClientsRemoteEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteOtherEntityMethodCall({}::{})\n", 
					pAoiEntity->scriptName(), methodDescription->getName()));
																									
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

			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pAoiEntity->id(), (*pSendBundle), (*pForwardBundle));

			//mailbox->postMail((*pBundle));
			pAoiEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCallOptimized, pSendBundle);

			//Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);

			// ��¼����¼���������������С
			g_publicClientEventHistoryStats.trackEvent(pAoiEntity->scriptName(), 
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
