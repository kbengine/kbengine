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
	forward_cellapp_messagebuffer_(ninterface, CELLAPP_TYPE),
	cellapps_()
{
}

//-------------------------------------------------------------------------------------
Cellappmgr::~Cellappmgr()
{
	cellapps_.clear();
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
void Cellappmgr::onChannelDeregister(Mercury::Channel * pChannel)
{
	// 如果是app死亡了
	if(pChannel->isInternal())
	{
		Components::ComponentInfos* cinfo = Components::getSingleton().findComponent(pChannel);
		if(cinfo)
		{
			std::map< COMPONENT_ID, Cellapp >::iterator iter = cellapps_.find(cinfo->cid);
			if(iter != cellapps_.end())
			{
				WARNING_MSG(fmt::format("Cellappmgr::onChannelDeregister: erase cellapp[{0}], currsize={1}\n",
					cinfo->cid, (cellapps_.size() - 1)));

				cellapps_.erase(iter);
				updateBestCellapp();
			}
		}
	}

	ServerApp::onChannelDeregister(pChannel);
}

//-------------------------------------------------------------------------------------
void Cellappmgr::updateBestCellapp()
{
	bestCellappID_ = findFreeCellapp();
}

//-------------------------------------------------------------------------------------
void Cellappmgr::handleGameTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("CellApp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	g_kbetime++;
	threadPool_.onMainThreadTick();
	networkInterface().processAllChannelPackets(&CellappmgrInterface::messageHandlers);
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
	gameTimer_ = this->mainDispatcher().addTimer(1000000 / 50, this,
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
	bundle.send(this->networkInterface(), cinfos->pChannel);
	s.opfini();
}

//-------------------------------------------------------------------------------------
COMPONENT_ID Cellappmgr::findFreeCellapp(void)
{
	Components::COMPONENTS& components = Components::getSingleton().getComponents(CELLAPP_TYPE);
	if(components.size() == 0)
		return 0;

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
	DEBUG_MSG(fmt::format("Cellappmgr::findFreeCellapp: index={0}.\n", index));
	std::advance(iter, index++);
	return (*iter).cid;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::reqCreateInNewSpace(Mercury::Channel* pChannel, MemoryStream& s) 
{
	std::string entityType;
	ENTITY_ID id;
	COMPONENT_ID componentID;

	s >> entityType;
	s >> id;
	s >> componentID;

	static SPACE_ID spaceID = 1;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	ForwardItem* pFI = new ForwardItem();
	pFI->pHandler = NULL;
	
	pFI->pBundle = pBundle;
	(*pBundle).newMessage(CellappInterface::onCreateInNewSpaceFromBaseapp);
	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << spaceID++;
	(*pBundle) << componentID;

	(*pBundle).append(&s);
	s.opfini();

	DEBUG_MSG(fmt::format("Cellappmgr::reqCreateInNewSpace: entityType={0}, entityID={1}, componentID={2}.\n",
		entityType, id, componentID));

	updateBestCellapp();

	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(CELLAPP_TYPE, bestCellappID_);

	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		WARNING_MSG("Cellappmgr::reqCreateInNewSpace: not found cellapp, message is buffered.\n");
		forward_cellapp_messagebuffer_.push(pFI);
		return;
	}
	else
	{
		(*pBundle).send(this->networkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		SAFE_RELEASE(pFI);
	}
}

//-------------------------------------------------------------------------------------
void Cellappmgr::reqRestoreSpaceInCell(Mercury::Channel* pChannel, MemoryStream& s) 
{
	std::string entityType;
	ENTITY_ID id;
	COMPONENT_ID componentID;
	SPACE_ID spaceID;

	s >> entityType;
	s >> id;
	s >> componentID;
	s >> spaceID;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	ForwardItem* pFI = new ForwardItem();
	pFI->pHandler = NULL;
	
	pFI->pBundle = pBundle;
	(*pBundle).newMessage(CellappInterface::onRestoreSpaceInCellFromBaseapp);
	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << spaceID;
	(*pBundle) << componentID;

	(*pBundle).append(&s);
	s.opfini();

	DEBUG_MSG(fmt::format("Cellappmgr::reqRestoreSpaceInCell: entityType={0}, entityID={1}, componentID={2}, spaceID={3}.\n",
		entityType, id, componentID, spaceID));

	updateBestCellapp();

	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(CELLAPP_TYPE, bestCellappID_);

	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		WARNING_MSG("Cellappmgr::reqRestoreSpaceInCell: not found cellapp, message is buffered.\n");
		forward_cellapp_messagebuffer_.push(pFI);
		return;
	}
	else
	{
		(*pBundle).send(this->networkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		SAFE_RELEASE(pFI);
	}
}

//-------------------------------------------------------------------------------------
void Cellappmgr::updateCellapp(Mercury::Channel* pChannel, COMPONENT_ID componentID, float load)
{
	// updateBestCellapp();
}

//-------------------------------------------------------------------------------------

}
