/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "cellappmgr.h"
#include "cellappmgr_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Cellappmgr);

class AppForwardItem : public ForwardItem
{
public:
	virtual bool isOK()
	{
		// 必须存在一个准备好的进程
		Components::COMPONENTS& cts = Components::getSingleton().getComponents(CELLAPP_TYPE);
		Components::COMPONENTS::iterator ctiter = cts.begin();
		for (; ctiter != cts.end(); ++ctiter)
		{
			if (Cellappmgr::getSingleton().componentReady((*ctiter).cid))
			{
				std::map< COMPONENT_ID, Cellapp >& cellapps = Cellappmgr::getSingleton().cellapps();
				std::map< COMPONENT_ID, Cellapp >::iterator cellapp_iter = cellapps.find((*ctiter).cid);
				if (cellapp_iter == cellapps.end())
					continue;

				if ((cellapp_iter->second.flags() & APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING) > 0)
					continue;

				if (cellapp_iter->second.isDestroyed())
					continue;

				if (cellapp_iter->second.initProgress() < 1.f)
					continue;

				return true;
			}
		}

		return false;
	}
};

//-------------------------------------------------------------------------------------
Cellappmgr::Cellappmgr(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	gameTimer_(),
	forward_anywhere_cellapp_messagebuffer_(ninterface, CELLAPP_TYPE),
	forward_cellapp_messagebuffer_(ninterface),
	cellapps_(),
	cellapp_cids_()
{
}

//-------------------------------------------------------------------------------------
Cellappmgr::~Cellappmgr()
{
	cellapps_.clear();
	cellapp_cids_.clear();
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
std::map< COMPONENT_ID, Cellapp >& Cellappmgr::cellapps()
{
	return cellapps_;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::removeCellapp(COMPONENT_ID cid)
{
	std::map< COMPONENT_ID, Cellapp >::iterator iter = cellapps_.find(cid);
	if (iter != cellapps_.end())
	{
		WARNING_MSG(fmt::format("Cellappmgr::removeCellapp: erase cellapp[{0}], currsize={1}\n",
			cid, (cellapps_.size() - 1)));

		cellapps_.erase(iter);
		
		std::vector<COMPONENT_ID>::iterator viter = cellapp_cids_.begin();
		for (; viter != cellapp_cids_.end(); ++viter)
		{
			if ((*viter) == cid)
			{
				cellapp_cids_.erase(viter);
				break;
			}
		}

		updateBestCellapp();
	}
}

//-------------------------------------------------------------------------------------
Cellapp& Cellappmgr::getCellapp(COMPONENT_ID cid)
{
	std::map< COMPONENT_ID, Cellapp >::iterator iter = cellapps_.find(cid);
	if (iter != cellapps_.end())
	{
		return iter->second;
	}

	// 添加了一个新的cellapp
	Cellapp& cellapp = cellapps_[cid];

	INFO_MSG(fmt::format("Cellappmgr::getCellapp: added new cellapp({0}).\n",
		cid));

	cellapp_cids_.push_back(cid);
	return cellapp;
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
void Cellappmgr::onChannelDeregister(Network::Channel * pChannel)
{
	// 如果是app死亡了
	if(pChannel->isInternal())
	{
		Components::ComponentInfos* cinfo = Components::getSingleton().findComponent(pChannel);
		if(cinfo)
		{
			cinfo->state = COMPONENT_STATE_STOP;
			removeCellapp(cinfo->cid);
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
	 //DEBUG_MSG("Cellappmgr::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	++g_kbetime;
	threadPool_.onMainThreadTick();
	networkInterface().processChannels(&CellappmgrInterface::messageHandlers);
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
	gameTimer_ = this->dispatcher().addTimer(1000000 / 50, this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	return true;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::finalise()
{
	gameTimer_.cancel();
	forward_anywhere_cellapp_messagebuffer_.clear();
	forward_cellapp_messagebuffer_.clear();
	
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Cellappmgr::forwardMessage(Network::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_ID sender_componentID, forward_componentID;

	s >> sender_componentID >> forward_componentID;
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(forward_componentID);
	KBE_ASSERT(cinfos != NULL && cinfos->pChannel != NULL);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();
}

//-------------------------------------------------------------------------------------
COMPONENT_ID Cellappmgr::findFreeCellapp(void)
{
	std::map< COMPONENT_ID, Cellapp >::iterator iter = cellapps_.begin();
	COMPONENT_ID cid = 0;

	float minload = 1.f;
	ENTITY_ID numEntities = 0x7fffffff;

	for(; iter != cellapps_.end(); ++iter)
	{
		if ((iter->second.flags() & APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING) > 0)
			continue;
		
		// 首先进程必须活着且初始化完毕
		if(!iter->second.isDestroyed() && iter->second.initProgress() > 1.f)
		{
			// 如果没有任何实体则无条件分配
			if(iter->second.numEntities() == 0)
				return iter->first;

			// 比较并记录负载最小的进程最终被分配
			if(minload > iter->second.load() || 
				(minload == iter->second.load() && numEntities > iter->second.numEntities()))
			{
				cid = iter->first;

				numEntities = iter->second.numEntities();
				minload = iter->second.load();
			}
		}
	}

	return cid;
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::componentsReady()
{
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(CELLAPP_TYPE);
	Components::COMPONENTS::iterator ctiter = cts.begin();
	for(; ctiter != cts.end(); ++ctiter)
	{
		if((*ctiter).pChannel == NULL)
			return false;

		if((*ctiter).state != COMPONENT_STATE_RUN)
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::componentReady(COMPONENT_ID cid)
{
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(CELLAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
uint32 Cellappmgr::numLoadBalancingApp()
{
	uint32 num = 0;
	std::map< COMPONENT_ID, Cellapp >::iterator iter = cellapps_.begin();

	for (; iter != cellapps_.end(); ++iter)
	{
		if ((iter->second.flags() & APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING) > 0)
			continue;

		++num;
	}

	return num;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::reqCreateInNewSpace(Network::Channel* pChannel, MemoryStream& s) 
{
	std::string entityType;
	ENTITY_ID id;
	COMPONENT_ID componentID;
	bool hasClient;

	// 如果cellappIndex为0，则代表不强制指定cellapp
	// 非0的情况下，选择的cellapp可以用1,2,3,4来代替
	// 假如预期有4个cellapp， 假如不够4个， 只有3个， 那么4代表1
	uint32 cellappIndex = 0;

	s >> entityType;
	s >> id;
	s >> cellappIndex;
	s >> componentID;
	s >> hasClient;

	static SPACE_ID spaceID = 1;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(CellappInterface::onCreateInNewSpaceFromBaseapp);
	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << spaceID++;
	(*pBundle) << componentID;
	(*pBundle) << hasClient;

	(*pBundle).append(&s);
	s.done();

	uint32 cellappSize = cellapp_cids_.size();

	if (cellappSize > 0)
	{
		updateBestCellapp();

		// 选择特定的cellapp创建space
		if (cellappIndex > 0)
		{
			uint32 index = (cellappIndex - 1) % cellappSize;
			bestCellappID_ = cellapp_cids_[index];
		}
		else if (bestCellappID_ == 0 && numLoadBalancingApp() == 0)
		{
			ERROR_MSG(fmt::format("Cellappmgr::reqCreateInNewSpace: Unable to allocate cellapp for load balancing! entityType={}, entityID={}, componentID={}, cellappSize={}.\n",
				entityType, id, componentID, cellappSize));
		}
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(CELLAPP_TYPE, bestCellappID_);
	if (cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		WARNING_MSG("Cellappmgr::reqCreateInNewSpace: not found cellapp, message is buffered.\n");

		ForwardItem* pFI = new AppForwardItem();
		pFI->pHandler = NULL;
		pFI->pBundle = pBundle;

		if (cellappIndex == 0 || bestCellappID_ == 0)
			forward_anywhere_cellapp_messagebuffer_.push(pFI);
		else
			forward_cellapp_messagebuffer_.push(bestCellappID_, pFI);

		return;
	}
	else
	{
		cinfos->pChannel->send(pBundle);
	}

	std::map< COMPONENT_ID, Cellapp >::iterator cellapp_iter = cellapps_.find(bestCellappID_);
	DEBUG_MSG(fmt::format("Cellappmgr::reqCreateInNewSpace: entityType={}, entityID={}, componentID={}, cellapp(cid={}, load={}, numEntities={}).\n",
		entityType, id, componentID, bestCellappID_, cellapp_iter->second.load(), cellapp_iter->second.numEntities()));

	// 预先将实体数量增加
	if (cellapp_iter != cellapps_.end())
	{
		cellapp_iter->second.incNumEntities();
	}
}

//-------------------------------------------------------------------------------------
void Cellappmgr::reqRestoreSpaceInCell(Network::Channel* pChannel, MemoryStream& s) 
{
	std::string entityType;
	ENTITY_ID id;
	COMPONENT_ID componentID;
	SPACE_ID spaceID;
	bool hasClient;

	s >> entityType;
	s >> id;
	s >> componentID;
	s >> spaceID;
	s >> hasClient;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(CellappInterface::onRestoreSpaceInCellFromBaseapp);
	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << spaceID;
	(*pBundle) << componentID;
	(*pBundle) << hasClient;

	(*pBundle).append(&s);
	s.done();

	DEBUG_MSG(fmt::format("Cellappmgr::reqRestoreSpaceInCell: entityType={0}, entityID={1}, componentID={2}, spaceID={3}.\n",
		entityType, id, componentID, spaceID));

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(CELLAPP_TYPE, bestCellappID_);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		WARNING_MSG("Cellappmgr::reqRestoreSpaceInCell: not found cellapp, message is buffered.\n");
		ForwardItem* pFI = new AppForwardItem();
		pFI->pHandler = NULL;
		pFI->pBundle = pBundle;
		forward_anywhere_cellapp_messagebuffer_.push(pFI);
		return;
	}
	else
	{
		cinfos->pChannel->send(pBundle);
	}

	// 预先将实体数量增加
	std::map< COMPONENT_ID, Cellapp >::iterator cellapp_iter = cellapps_.find(bestCellappID_);
	if (cellapp_iter != cellapps_.end())
	{
		cellapp_iter->second.incNumEntities();
	}
}

//-------------------------------------------------------------------------------------
void Cellappmgr::updateCellapp(Network::Channel* pChannel, COMPONENT_ID componentID, 
	ENTITY_ID numEntities, float load, uint32 flags)
{
	Cellapp& cellapp = getCellapp(componentID);
	
	cellapp.load(load);
	cellapp.numEntities(numEntities);
	cellapp.flags(flags);
	
	updateBestCellapp();
}

//-------------------------------------------------------------------------------------
void Cellappmgr::onCellappInitProgress(Network::Channel* pChannel, COMPONENT_ID cid, float progress)
{
	if(progress > 1.f)
	{
		INFO_MSG(fmt::format("Cellappmgr::onCellappInitProgress: cid={0}, progress={1}.\n",
			cid , (progress > 1.f ? 1.f : progress)));

		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cid);
		if(cinfos)
			cinfos->state = COMPONENT_STATE_RUN;
	}

	KBE_ASSERT(cellapps_.find(cid) != cellapps_.end());

	getCellapp(cid).initProgress(progress);
}

//-------------------------------------------------------------------------------------

}
