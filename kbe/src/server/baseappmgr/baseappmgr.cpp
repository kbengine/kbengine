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


#include "baseappmgr.h"
#include "baseappmgr_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"

#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseappmgr);

//-------------------------------------------------------------------------------------
Baseappmgr::Baseappmgr(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	gameTimer_(),
	forward_baseapp_messagebuffer_(ninterface, BASEAPP_TYPE),
	bestBaseappID_(0),
	baseapps_(),
	pending_logins_(),
	baseappsInitProgress_(0.f)
{
}

//-------------------------------------------------------------------------------------
Baseappmgr::~Baseappmgr()
{
	baseapps_.clear();
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::handleTimeout(TimerHandle handle, void * arg)
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
void Baseappmgr::handleGameTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("Baseappmgr::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	++g_kbetime;
	threadPool_.onMainThreadTick();
	networkInterface().processChannels(&BaseappmgrInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onChannelDeregister(Network::Channel * pChannel)
{
	// 如果是app死亡了
	if(pChannel->isInternal())
	{
		Components::ComponentInfos* cinfo = Components::getSingleton().findComponent(pChannel);
		if(cinfo)
		{
			cinfo->state = COMPONENT_STATE_STOP;
			std::map< COMPONENT_ID, Baseapp >::iterator iter = baseapps_.find(cinfo->cid);
			if(iter != baseapps_.end())
			{
				WARNING_MSG(fmt::format("Baseappmgr::onChannelDeregister: erase baseapp[{}], currsize={}\n", 
					cinfo->cid, (baseapps_.size() - 1)));

				baseapps_.erase(iter);
				updateBestBaseapp();
			}
		}
	}

	ServerApp::onChannelDeregister(pChannel);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onAddComponent(const Components::ComponentInfos* pInfos)
{
	Components::ComponentInfos* cinfo = Components::getSingleton().findComponent(pInfos->cid);

	if(pInfos->componentType == LOGINAPP_TYPE && cinfo->pChannel != NULL)
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

		(*pBundle).newMessage(LoginappInterface::onBaseappInitProgress);
		(*pBundle) << baseappsInitProgress_;
		cinfo->pChannel->send(pBundle);
	}

	ServerApp::onAddComponent(pInfos);
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::initializeEnd()
{
	gameTimer_ = this->dispatcher().addTimer(1000000 / 50, this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	return true;
}

//-------------------------------------------------------------------------------------
void Baseappmgr::finalise()
{
	gameTimer_.cancel();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::forwardMessage(Network::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_ID sender_componentID, forward_componentID;

	s >> sender_componentID >> forward_componentID;
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(forward_componentID);

	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(fmt::format("Baseappmgr::forwardMessage: not found forwardComponent({}, at:{:p})!\n", 
			forward_componentID, (void*)cinfos));

		KBE_ASSERT(false && "Baseappmgr::forwardMessage: not found forwardComponent!\n");
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::componentsReady()
{
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(BASEAPP_TYPE);
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
bool Baseappmgr::componentReady(COMPONENT_ID cid)
{
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
void Baseappmgr::updateBaseapp(Network::Channel* pChannel, COMPONENT_ID componentID,
							ENTITY_ID numBases, ENTITY_ID numProxices, float load, uint32 flags)
{
	Baseapp& baseapp = baseapps_[componentID];
	
	baseapp.load(load);
	baseapp.numProxices(numProxices);
	baseapp.numBases(numBases);
	baseapp.flags(flags);
	
	updateBestBaseapp();
}

//-------------------------------------------------------------------------------------
COMPONENT_ID Baseappmgr::findFreeBaseapp()
{
	std::map< COMPONENT_ID, Baseapp >::iterator iter = baseapps_.begin();
	COMPONENT_ID cid = 0;

	float minload = 1.f;
	ENTITY_ID numEntities = 0x7fffffff;

	for(; iter != baseapps_.end(); ++iter)
	{
		if ((iter->second.flags() & APP_FLAGS_NONE) > 0)
			continue;
		
		if(!iter->second.isDestroyed() &&
			iter->second.initProgress() > 1.f && 
			(iter->second.numEntities() == 0 || 
			minload > iter->second.load() || 
			(minload == iter->second.load() && numEntities > iter->second.numEntities())))
		{
			cid = iter->first;

			numEntities = iter->second.numEntities();
			minload = iter->second.load();
		}
	}

	return cid;
}

//-------------------------------------------------------------------------------------
void Baseappmgr::updateBestBaseapp()
{
	bestBaseappID_ = findFreeBaseapp();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateBaseAnywhere(Network::Channel* pChannel, MemoryStream& s) 
{
	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(pChannel);

	// 此时肯定是在运行状态中，但有可能在等待创建space
	// 所以初始化进度没有完成, 在只有一个baseapp的情况下如果这
	// 里不进行设置将是一个相互等待的状态
	if(cinfos)
		cinfos->state = COMPONENT_STATE_RUN;

	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		ForwardItem* pFI = new ForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::onCreateBaseAnywhere);
		(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
		s.done();

		WARNING_MSG("Baseappmgr::reqCreateBaseAnywhere: not found baseapp, message is buffered.\n");
		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(pFI);
		return;
	}
	
	//DEBUG_MSG("Baseappmgr::reqCreateBaseAnywhere: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onCreateBaseAnywhere);

	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateBaseAnywhereFromDBID(Network::Channel* pChannel, MemoryStream& s) 
{
	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(pChannel);

	// 此时肯定是在运行状态中，但有可能在等待创建space
	// 所以初始化进度没有完成, 在只有一个baseapp的情况下如果这
	// 里不进行设置将是一个相互等待的状态
	if(cinfos)
		cinfos->state = COMPONENT_STATE_RUN;

	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		ForwardItem* pFI = new ForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::createBaseAnywhereFromDBIDOtherBaseapp);
		(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
		s.done();

		WARNING_MSG("Baseappmgr::reqCreateBaseAnywhereFromDBID: not found baseapp, message is buffered.\n");
		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(pFI);
		return;
	}
	
	//DEBUG_MSG("Baseappmgr::reqCreateBaseAnywhereFromDBID: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::createBaseAnywhereFromDBIDOtherBaseapp);

	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::registerPendingAccountToBaseapp(Network::Channel* pChannel, MemoryStream& s)
{
	std::string loginName;
	std::string accountName;
	std::string password;
	std::string datas;
	DBID entityDBID;
	uint32 flags;
	uint64 deadline;
	COMPONENT_TYPE componentType;

	s >> loginName >> accountName >> password >> entityDBID >> flags >> deadline >> componentType;
	s.readBlob(datas);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(pChannel);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("Baseappmgr::registerPendingAccountToBaseapp: not found loginapp!\n");
		return;
	}

	pending_logins_[loginName] = cinfos->cid;

	ENTITY_ID eid = 0;
	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);

	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		ForwardItem* pFI = new ForwardItem();

		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
		(*pBundle) << loginName << accountName << password << eid << entityDBID << flags << deadline << componentType;
		pBundle->appendBlob(datas);

		WARNING_MSG("Baseappmgr::registerPendingAccountToBaseapp: not found baseapp, message is buffered.\n");
		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(pFI);
		return;
	}


	DEBUG_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseapp:{0}. allocBaseapp=[{1}].\n",
		accountName, bestBaseappID_));
	
	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
	(*pBundle) << loginName << accountName << password << eid << entityDBID << flags << deadline << componentType;
	pBundle->appendBlob(datas);
	cinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::registerPendingAccountToBaseappAddr(Network::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_ID componentID;
	std::string loginName;
	std::string accountName;
	std::string password;
	std::string datas;
	ENTITY_ID entityID;
	DBID entityDBID;
	uint32 flags;
	uint64 deadline;
	COMPONENT_TYPE componentType;

	s >> componentID >> loginName >> accountName >> password >> entityID >> entityDBID >> flags >> deadline >> componentType;
	s.readBlob(datas);

	DEBUG_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseappAddr:{0}, componentID={1}, entityID={2}.\n",
		accountName, componentID, entityID));

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(pChannel);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("Baseappmgr::registerPendingAccountToBaseapp: not found loginapp!\n");
		return;
	}

	pending_logins_[loginName] = cinfos->cid;

	cinfos = Components::getSingleton().findComponent(componentID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseappAddr: not found baseapp({}).\n", componentID));
		sendAllocatedBaseappAddr(pChannel, loginName, accountName, "", 0);
		return;
	}
	
	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
	(*pBundle) << loginName << accountName << password << entityID << entityDBID << flags << deadline << componentType;
	pBundle->appendBlob(datas);
	cinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onPendingAccountGetBaseappAddr(Network::Channel* pChannel, 
							  std::string& loginName, std::string& accountName, std::string& addr, uint16 port)
{
	sendAllocatedBaseappAddr(pChannel, loginName, accountName, addr, port);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::sendAllocatedBaseappAddr(Network::Channel* pChannel, 
							  std::string& loginName, std::string& accountName, const std::string& addr, uint16 port)
{
	KBEUnordered_map< std::string, COMPONENT_ID >::iterator iter = pending_logins_.find(loginName);
	if(iter == pending_logins_.end())
	{
		ERROR_MSG("Baseappmgr::sendAllocatedBaseappAddr: not found loginapp, pending_logins is error!\n");
		return;
	}
	
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(iter->second);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("Baseappmgr::sendAllocatedBaseappAddr: not found loginapp!\n");
		return;
	}

	Network::Bundle* pBundleToLoginapp = Network::Bundle::ObjPool().createObject();
	(*pBundleToLoginapp).newMessage(LoginappInterface::onLoginAccountQueryBaseappAddrFromBaseappmgr);

	LoginappInterface::onLoginAccountQueryBaseappAddrFromBaseappmgrArgs4::staticAddToBundle((*pBundleToLoginapp), loginName, 
		accountName, addr, port);

	cinfos->pChannel->send(pBundleToLoginapp);
	pending_logins_.erase(iter);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onBaseappInitProgress(Network::Channel* pChannel, COMPONENT_ID cid, float progress)
{
	if(progress > 1.f)
	{
		INFO_MSG(fmt::format("Baseappmgr::onBaseappInitProgress: cid={0}, progress={1}.\n",
			cid , (progress > 1.f ? 1.f : progress)));
	}

	KBE_ASSERT(baseapps_.find(cid) != baseapps_.end());

	baseapps_[cid].initProgress(progress);

	size_t completedCount = 0;

	std::map< COMPONENT_ID, Baseapp >::iterator iter1 = baseapps_.begin();
	for(; iter1 != baseapps_.end(); ++iter1)
	{
		if((*iter1).second.initProgress() > 1.f)
		{
			Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cid);
			if(cinfos)
				cinfos->state = COMPONENT_STATE_RUN;

			completedCount++;
		}
	}

	if(completedCount >= baseapps_.size())
	{
		baseappsInitProgress_ = 100.f;
		INFO_MSG("Baseappmgr::onBaseappInitProgress: all completed!\n");
	}
	else
	{
		baseappsInitProgress_ = float(completedCount) / float(baseapps_.size());
	}

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(LOGINAPP_TYPE);

	Components::COMPONENTS::iterator iter = cts.begin();
	for(; iter != cts.end(); ++iter)
	{
		if((*iter).pChannel == NULL)
			continue;

		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

		(*pBundle).newMessage(LoginappInterface::onBaseappInitProgress);
		(*pBundle) << baseappsInitProgress_;
		(*iter).pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------

}
