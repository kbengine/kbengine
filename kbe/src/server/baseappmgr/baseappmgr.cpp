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


#include "baseappmgr.hpp"
#include "baseappmgr_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseappmgr);

//-------------------------------------------------------------------------------------
Baseappmgr::Baseappmgr(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	gameTimer_(),
	forward_baseapp_messagebuffer_(ninterface, BASEAPP_TYPE),
	bestBaseappID_(0),
	baseapps_(),
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
	 //DEBUG_MSG("CellApp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	g_kbetime++;
	updateBestBaseapp();
	threadPool_.onMainThreadTick();
	networkInterface().processAllChannelPackets(&BaseappmgrInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onChannelDeregister(Mercury::Channel * pChannel)
{
	// 如果是app死亡了
	if(pChannel->isInternal())
	{
		Components::ComponentInfos* cinfo = Components::getSingleton().findComponent(pChannel);
		if(cinfo)
		{
			std::map< COMPONENT_ID, Baseapp >::iterator iter = baseapps_.find(cinfo->cid);
			if(iter != baseapps_.end())
			{
				WARNING_MSG(boost::format("Baseappmgr::onChannelDeregister: erase baseapp[%1%], currsize=%2%\n") % 
					cinfo->cid % (baseapps_.size() - 1));

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
		Mercury::Bundle::SmartPoolObjectPtr bundleptr = Mercury::Bundle::createSmartPoolObj();

		(*bundleptr)->newMessage(LoginappInterface::onBaseappInitProgress);
		(*(*bundleptr)) << baseappsInitProgress_;
		(*bundleptr)->send(networkInterface_, cinfo->pChannel);
	}
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
	gameTimer_ = this->mainDispatcher().addTimer(1000000 / 50, this,
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
void Baseappmgr::forwardMessage(Mercury::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_ID sender_componentID, forward_componentID;

	s >> sender_componentID >> forward_componentID;
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(forward_componentID);

	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(boost::format("Baseappmgr::forwardMessage: not found forwardComponent(%1%, at:%2%)!\n") % forward_componentID % cinfos);
		KBE_ASSERT(false && "Baseappmgr::forwardMessage: not found forwardComponent!\n");
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).append((char*)s.data() + s.rpos(), s.opsize());
	(*pBundle).send(this->networkInterface(), cinfos->pChannel);
	s.read_skip(s.opsize());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::updateBaseapp(Mercury::Channel* pChannel, COMPONENT_ID componentID,
							ENTITY_ID numBases, ENTITY_ID numProxices, float load)
{
	Baseapp& baseapp = baseapps_[componentID];
	
	baseapp.load(load);
	baseapp.numProxices(numProxices);
	baseapp.numBases(numBases);
}

//-------------------------------------------------------------------------------------
COMPONENT_ID Baseappmgr::findFreeBaseapp()
{
	std::map< COMPONENT_ID, Baseapp >::iterator iter = baseapps_.begin();
	COMPONENT_ID cid = 0;

	float minload = 1.f;
	for(; iter != baseapps_.end(); iter++)
	{
		if(!iter->second.isDestroyed() &&
			minload > iter->second.load())
		{
			cid = iter->first;
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
void Baseappmgr::reqCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s) 
{
	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);

	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		ForwardItem* pFI = new ForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::onCreateBaseAnywhere);
		(*pBundle).append((char*)s.data() + s.rpos(), s.opsize());
		s.read_skip(s.opsize());

		WARNING_MSG("Baseappmgr::reqCreateBaseAnywhere: not found baseapp, message is buffered.\n");
		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(pFI);
		return;
	}
	
	//DEBUG_MSG("Baseappmgr::reqCreateBaseAnywhere: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onCreateBaseAnywhere);

	(*pBundle).append((char*)s.data() + s.rpos(), s.opsize());
	(*pBundle).send(this->networkInterface(), cinfos->pChannel);
	s.read_skip(s.opsize());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateBaseAnywhereFromDBID(Mercury::Channel* pChannel, MemoryStream& s) 
{
	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);

	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		ForwardItem* pFI = new ForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::createBaseAnywhereFromDBIDOtherBaseapp);
		(*pBundle).append((char*)s.data() + s.rpos(), s.opsize());
		s.read_skip(s.opsize());

		WARNING_MSG("Baseappmgr::reqCreateBaseAnywhereFromDBID: not found baseapp, message is buffered.\n");
		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(pFI);
		return;
	}
	
	//DEBUG_MSG("Baseappmgr::reqCreateBaseAnywhereFromDBID: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::createBaseAnywhereFromDBIDOtherBaseapp);

	(*pBundle).append((char*)s.data() + s.rpos(), s.opsize());
	(*pBundle).send(this->networkInterface(), cinfos->pChannel);
	s.read_skip(s.opsize());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::registerPendingAccountToBaseapp(Mercury::Channel* pChannel, 
												 std::string& loginName, std::string& accountName, 
												 std::string& password, DBID entityDBID, uint32 flags, uint64 deadline,
												 COMPONENT_TYPE componentType)
{
	ENTITY_ID eid = 0;
	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);

	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		ForwardItem* pFI = new ForwardItem();

		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
		(*pBundle) << loginName << accountName << password << eid << entityDBID << flags << deadline << componentType;

		WARNING_MSG("Baseappmgr::registerPendingAccountToBaseapp: not found baseapp, message is buffered.\n");
		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(pFI);
		return;
	}


	DEBUG_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseapp:{0}. allocBaseapp=[{1}].\n",
		accountName, bestBaseappID_));

	sendAllocatedBaseappAddr(pChannel, loginName, accountName, cinfos->pExtAddr->ip, cinfos->pExtAddr->port);
	
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
	(*pBundle) << loginName << accountName << password << eid << entityDBID << flags << deadline << componentType;
	(*pBundle).send(this->networkInterface(), cinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::registerPendingAccountToBaseappAddr(Mercury::Channel* pChannel, COMPONENT_ID componentID,
								std::string& loginName, std::string& accountName, std::string& password, 
								ENTITY_ID entityID, DBID entityDBID, uint32 flags, uint64 deadline,
								COMPONENT_TYPE componentType)
{
	DEBUG_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseappAddr:{0}, componentID={1}, entityID={2}.\n",
		accountName, componentID, entityID));

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(boost::format("Baseappmgr::onPendingAccountGetBaseappAddr: not found baseapp(%1%).\n") % componentID);
		sendAllocatedBaseappAddr(pChannel, loginName, accountName, 0, 0);
		return;
	}

	sendAllocatedBaseappAddr(pChannel, loginName, accountName, cinfos->pExtAddr->ip, cinfos->pExtAddr->port);
	
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
	(*pBundle) << loginName << accountName << password << entityID << entityDBID << flags << deadline << componentType;
	(*pBundle).send(this->networkInterface(), cinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onPendingAccountGetBaseappAddr(Mercury::Channel* pChannel, 
							  std::string& loginName, std::string& accountName, uint32 addr, uint16 port)
{
	//sendAllocatedBaseappAddr(pChannel, loginName, accountName, addr, port);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::sendAllocatedBaseappAddr(Mercury::Channel* pChannel, 
							  std::string& loginName, std::string& accountName, uint32 addr, uint16 port)
{
	Components::COMPONENTS& components = Components::getSingleton().getComponents(LOGINAPP_TYPE);
	size_t componentSize = components.size();
	
	if(componentSize == 0)
	{
		ERROR_MSG("Baseappmgr::sendAllocatedBaseappAddr: not found loginapp.\n");
		return;
	}

	Components::COMPONENTS::iterator iter = components.begin();
	Mercury::Channel* lpChannel = (*iter).pChannel;

	Mercury::Bundle* pBundleToLoginapp = Mercury::Bundle::ObjPool().createObject();
	(*pBundleToLoginapp).newMessage(LoginappInterface::onLoginAccountQueryBaseappAddrFromBaseappmgr);

	LoginappInterface::onLoginAccountQueryBaseappAddrFromBaseappmgrArgs4::staticAddToBundle((*pBundleToLoginapp), loginName, 
		accountName, addr, port);

	(*pBundleToLoginapp).send(this->networkInterface(), lpChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundleToLoginapp);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onBaseappInitProgress(Mercury::Channel* pChannel, COMPONENT_ID cid, float progress)
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
	for(; iter1 != baseapps_.end(); iter1++)
	{
		if((*iter1).second.initProgress() > 1.f)
			completedCount++;
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
	for(; iter != cts.end(); iter++)
	{
		if((*iter).pChannel == NULL)
			continue;

		Mercury::Bundle::SmartPoolObjectPtr bundleptr = Mercury::Bundle::createSmartPoolObj();

		(*bundleptr)->newMessage(LoginappInterface::onBaseappInitProgress);
		(*(*bundleptr)) << baseappsInitProgress_;
		(*bundleptr)->send(networkInterface_, (*iter).pChannel);
	}
}

//-------------------------------------------------------------------------------------

}
