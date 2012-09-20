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


#include "cellappmgr.hpp"
#include "cellappmgr_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Cellappmgr);

//-------------------------------------------------------------------------------------
Cellappmgr::Cellappmgr(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	gameTimer_(),
	forward_cellapp_messagebuffer_(ninterface, CELLAPP_TYPE)
{
}

//-------------------------------------------------------------------------------------
Cellappmgr::~Cellappmgr()
{
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void Cellappmgr::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Cellappmgr::handleGameTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("CellApp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	g_kbetime++;
	getNetworkInterface().handleChannels(&CellappmgrInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::initializeEnd()
{
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	return true;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::finalise()
{
	gameTimer_.cancel();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Cellappmgr::forwardMessage(Mercury::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_ID sender_componentID, forward_componentID;

	s >> sender_componentID >> forward_componentID;
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(forward_componentID);
	KBE_ASSERT(cinfos != NULL && cinfos->pChannel != NULL);

	Mercury::Bundle bundle;
	bundle.append((char*)s.data() + s.rpos(), s.opsize());
	bundle.send(this->getNetworkInterface(), cinfos->pChannel);
	s.read_skip(s.opsize());
}

//-------------------------------------------------------------------------------------
Mercury::Channel* Cellappmgr::findFreeCellapp(void)
{
	Components::COMPONENTS& components = Components::getSingleton().getComponents(CELLAPP_TYPE);
	if(components.size() == 0)
		return NULL;

	/*
	std::tr1::mt19937 engine;
	std::tr1::uniform_int<int> unif(1, components.size());
	std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_int<int> > generator (engine, unif);
	COMPONENT_MAP::iterator iter = components.begin();
	int index = 0;
	for(int i=0; i<10; i++)
		index = generator();
		*/
	static size_t index = 0;
	if(index >= components.size())
		index = 0;

	Components::COMPONENTS::iterator iter = components.begin();
	DEBUG_MSG("Cellappmgr::findFreeCellapp: index=%d.\n", index);
	std::advance(iter, index++);
	return (*iter).pChannel;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::reqCreateInNewSpace(Mercury::Channel* pChannel, MemoryStream& s) 
{
	std::string entityType;
	ENTITY_ID id;
	uint32 cellDataLength;
	std::string strEntityCellData;
	COMPONENT_ID componentID;

	s >> entityType;
	s >> id;
	s >> componentID;
	s >> cellDataLength;

	if(cellDataLength > 0)
	{
		strEntityCellData.assign((char*)(s.data() + s.rpos()), cellDataLength);
		s.read_skip(cellDataLength);
	}

	static SPACE_ID spaceID = 1;

	ForwardItem* pFI = new ForwardItem();
	pFI->pHandler = NULL;

	pFI->bundle.newMessage(CellappInterface::onCreateInNewSpaceFromBaseapp);
	pFI->bundle << entityType;
	pFI->bundle << id;
	pFI->bundle << spaceID++;
	pFI->bundle << componentID;
	pFI->bundle << cellDataLength;
	
	if(cellDataLength > 0)
		pFI->bundle.append(strEntityCellData.data(), cellDataLength);

	DEBUG_MSG("Cellappmgr::reqCreateInNewSpace: entityType=%s, entityID=%d, componentID=%"PRAppID".\n", entityType.c_str(), id, componentID);

	Mercury::Channel* lpChannel = findFreeCellapp();

	if(lpChannel == NULL)
	{
		WARNING_MSG("Cellappmgr::reqCreateInNewSpace: not found cellapp, message is buffered.\n");
		forward_cellapp_messagebuffer_.push(pFI);
		return;
	}
	else
	{
		pFI->bundle.send(this->getNetworkInterface(), lpChannel);
		SAFE_RELEASE(pFI);
	}
}

//-------------------------------------------------------------------------------------

}
