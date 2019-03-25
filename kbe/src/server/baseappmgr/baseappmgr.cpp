// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "baseappmgr.h"
#include "baseappmgr_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "helper/console_helper.h"

#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseappmgr);


class AppForwardItem : public ForwardItem
{
public:
	virtual bool isOK()
	{
		// 必须存在一个准备好的进程
		Components::COMPONENTS& cts = Components::getSingleton().getComponents(BASEAPP_TYPE);
		Components::COMPONENTS::iterator ctiter = cts.begin();
		for (; ctiter != cts.end(); ++ctiter)
		{
			if (Baseappmgr::getSingleton().componentReady((*ctiter).cid))
			{
				std::map< COMPONENT_ID, Baseapp >& baseapps = Baseappmgr::getSingleton().baseapps();
				std::map< COMPONENT_ID, Baseapp >::iterator baseapps_iter = baseapps.find((*ctiter).cid);
				if (baseapps_iter == baseapps.end())
					continue;

				if ((baseapps_iter->second.flags() & APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING) > 0)
					continue;

				if (baseapps_iter->second.isDestroyed())
					continue;

				if (baseapps_iter->second.initProgress() < 1.f)
					continue;

				return true;
			}
		}

		return false;
	}
};

//-------------------------------------------------------------------------------------
Baseappmgr::Baseappmgr(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	gameTimer_(),
	forward_anywhere_baseapp_messagebuffer_(ninterface, BASEAPP_TYPE),
	forward_baseapp_messagebuffer_(ninterface),
	bestBaseappID_(0),
	baseapps_(),
	pending_logins_(),
	baseappsInitProgress_(0.f)
{
	KBEngine::Network::MessageHandlers::pMainMessageHandlers = &BaseappmgrInterface::messageHandlers;
}

//-------------------------------------------------------------------------------------
Baseappmgr::~Baseappmgr()
{
	baseapps_.clear();
}

//-------------------------------------------------------------------------------------
std::map< COMPONENT_ID, Baseapp >& Baseappmgr::baseapps()
{
	return baseapps_;
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
	//DEBUG_MSG(fmt::format("Baseappmgr::handleGameTick[{}]:{}\n", t, g_kbetime));
	
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
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

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
	forward_anywhere_baseapp_messagebuffer_.clear();
	forward_baseapp_messagebuffer_.clear();

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

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
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
uint32 Baseappmgr::numLoadBalancingApp()
{
	uint32 num = 0;
	std::map< COMPONENT_ID, Baseapp >::iterator iter = baseapps_.begin();

	for (; iter != baseapps_.end(); ++iter)
	{
		if ((iter->second.flags() & APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING) > 0)
			continue;

		++num;
	}

	return num;
}

//-------------------------------------------------------------------------------------
void Baseappmgr::updateBaseapp(Network::Channel* pChannel, COMPONENT_ID componentID,
							ENTITY_ID numEntitys, ENTITY_ID numProxices, float load, uint32 flags)
{
	Baseapp& baseapp = baseapps_[componentID];
	
	baseapp.load(load);
	baseapp.numProxices(numProxices);
	baseapp.numEntitys(numEntitys);
	baseapp.flags(flags);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentID);
	if (cinfos)
		cinfos->appFlags = flags;
	
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
void Baseappmgr::updateBestBaseapp()
{
	bestBaseappID_ = findFreeBaseapp();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateEntityAnywhere(Network::Channel* pChannel, MemoryStream& s) 
{
	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(pChannel);

	// 此时肯定是在运行状态中，但有可能在等待创建space
	// 所以初始化进度没有完成, 在只有一个baseapp的情况下如果这
	// 里不进行设置将是一个相互等待的状态
	if(cinfos)
		cinfos->state = COMPONENT_STATE_RUN;

	updateBestBaseapp();

	if (bestBaseappID_ == 0 && numLoadBalancingApp() == 0)
	{
		ERROR_MSG(fmt::format("Baseappmgr::reqCreateEntityAnywhere: Unable to allocate baseapp for load balancing! baseappSize={}.\n",
			baseapps_.size()));
	}

	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);
	if (cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		ForwardItem* pFI = new AppForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::onCreateEntityAnywhere);
		(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
		s.done();

		int runstate = -1;
		if (cinfos)
			runstate = (int)cinfos->state;

		WARNING_MSG(fmt::format("Baseappmgr::reqCreateEntityAnywhere: not found baseapp({}, runstate={}, pChannel={}), message is buffered.\n",
			bestBaseappID_, runstate, (cinfos && cinfos->pChannel ? cinfos->pChannel->c_str() : "NULL")));

		pFI->pHandler = NULL;
		forward_anywhere_baseapp_messagebuffer_.push(pFI);
		return;
	}
	
	//DEBUG_MSG("Baseappmgr::reqCreateEntityAnywhere: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::onCreateEntityAnywhere);

	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();

	// 预先将实体数量增加
	std::map< COMPONENT_ID, Baseapp >::iterator baseapps_iter = baseapps_.find(bestBaseappID_);
	if (baseapps_iter != baseapps_.end())
	{
		baseapps_iter->second.incNumEntities();
	}
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateEntityRemotely(Network::Channel* pChannel, MemoryStream& s)
{
	Components::ComponentInfos* cinfos =
		Components::getSingleton().findComponent(pChannel);

	// 此时肯定是在运行状态中，但有可能在等待创建space
	// 所以初始化进度没有完成, 在只有一个baseapp的情况下如果这
	// 里不进行设置将是一个相互等待的状态
	if (cinfos)
		cinfos->state = COMPONENT_STATE_RUN;

	COMPONENT_ID createToComponentID = 0;
	s >> createToComponentID;

	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, createToComponentID);
	if (cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		ForwardItem* pFI = new AppForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::onCreateEntityRemotely);
		(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
		s.done();

		int runstate = -1;
		if (cinfos)
			runstate = (int)cinfos->state;

		WARNING_MSG(fmt::format("Baseappmgr::reqCreateEntityRemotely: not found baseapp({}, runstate={}, pChannel={}), message is buffered.\n",
			createToComponentID, runstate, (cinfos && cinfos->pChannel ? cinfos->pChannel->c_str() : "NULL")));

		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(createToComponentID, pFI);
		return;
	}

	//DEBUG_MSG("Baseappmgr::reqCreateEntityRemotely: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::onCreateEntityRemotely);

	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();

	// 预先将实体数量增加
	std::map< COMPONENT_ID, Baseapp >::iterator baseapps_iter = baseapps_.find(createToComponentID);
	if (baseapps_iter != baseapps_.end())
	{
		baseapps_iter->second.incNumEntities();
	}
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateEntityAnywhereFromDBIDQueryBestBaseappID(Network::Channel* pChannel, MemoryStream& s)
{
	Components::ComponentInfos* cinfos =
		Components::getSingleton().findComponent(pChannel);

	// 此时肯定是在运行状态中，但有可能在等待创建space
	// 所以初始化进度没有完成, 在只有一个baseapp的情况下如果这
	// 里不进行设置将是一个相互等待的状态
	if (cinfos)
		cinfos->state = COMPONENT_STATE_RUN;

	updateBestBaseapp();

	if (bestBaseappID_ == 0 && numLoadBalancingApp() == 0)
	{
		ERROR_MSG(fmt::format("Baseappmgr::reqCreateEntityAnywhereFromDBIDQueryBestBaseappID: Unable to allocate baseapp for load balancing! baseappSize={}.\n",
			baseapps_.size()));
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::onGetCreateEntityAnywhereFromDBIDBestBaseappID);

	(*pBundle) << bestBaseappID_;
	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateEntityAnywhereFromDBID(Network::Channel* pChannel, MemoryStream& s) 
{
	Components::ComponentInfos* cinfos = 
		Components::getSingleton().findComponent(pChannel);

	// 此时肯定是在运行状态中，但有可能在等待创建space
	// 所以初始化进度没有完成, 在只有一个baseapp的情况下如果这
	// 里不进行设置将是一个相互等待的状态
	if(cinfos)
		cinfos->state = COMPONENT_STATE_RUN;

	COMPONENT_ID targetComponentID = 0;
	s >> targetComponentID;

	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, targetComponentID);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		ForwardItem* pFI = new AppForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::createEntityAnywhereFromDBIDOtherBaseapp);
		(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
		s.done();

		int runstate = -1;
		if (cinfos)
			runstate = (int)cinfos->state;

		WARNING_MSG(fmt::format("Baseappmgr::reqCreateEntityAnywhereFromDBID: not found baseapp({}, runstate={}, pChannel={}), message is buffered.\n",
			targetComponentID, runstate, (cinfos && cinfos->pChannel ? cinfos->pChannel->c_str() : "NULL")));

		pFI->pHandler = NULL;
		forward_anywhere_baseapp_messagebuffer_.push(pFI);
		return;
	}
	
	//DEBUG_MSG("Baseappmgr::reqCreateEntityAnywhereFromDBID: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::createEntityAnywhereFromDBIDOtherBaseapp);

	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();

	// 预先将实体数量增加
	std::map< COMPONENT_ID, Baseapp >::iterator baseapps_iter = baseapps_.find(targetComponentID);
	if (baseapps_iter != baseapps_.end())
	{
		baseapps_iter->second.incNumEntities();
	}
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateEntityRemotelyFromDBID(Network::Channel* pChannel, MemoryStream& s)
{
	Components::ComponentInfos* cinfos =
		Components::getSingleton().findComponent(pChannel);

	// 此时肯定是在运行状态中，但有可能在等待创建space
	// 所以初始化进度没有完成, 在只有一个baseapp的情况下如果这
	// 里不进行设置将是一个相互等待的状态
	if (cinfos)
		cinfos->state = COMPONENT_STATE_RUN;

	COMPONENT_ID targetComponentID = 0;
	s >> targetComponentID;

	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, targetComponentID);
	if (cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		ForwardItem* pFI = new AppForwardItem();
		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::createEntityRemotelyFromDBIDOtherBaseapp);
		(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
		s.done();

		int runstate = -1;
		if (cinfos)
			runstate = (int)cinfos->state;

		WARNING_MSG(fmt::format("Baseappmgr::reqCreateEntityRemotelyFromDBID: not found baseapp({}, runstate={}, pChannel={}), message is buffered.\n", 
			targetComponentID, runstate, (cinfos && cinfos->pChannel ? cinfos->pChannel->c_str() : "NULL")));

		pFI->pHandler = NULL;
		forward_baseapp_messagebuffer_.push(targetComponentID, pFI);
		return;
	}

	//DEBUG_MSG("Baseappmgr::reqCreateEntityRemotelyFromDBID: %s opsize=%d, selBaseappIdx=%d.\n", 
	//	pChannel->c_str(), s.opsize(), currentBaseappIndex);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::createEntityRemotelyFromDBIDOtherBaseapp);

	(*pBundle).append((char*)s.data() + s.rpos(), (int)s.length());
	cinfos->pChannel->send(pBundle);
	s.done();

	// 预先将实体数量增加
	std::map< COMPONENT_ID, Baseapp >::iterator baseapps_iter = baseapps_.find(targetComponentID);
	if (baseapps_iter != baseapps_.end())
	{
		baseapps_iter->second.incNumEntities();
	}
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
	int clientType;
	bool forceInternalLogin;
	bool needCheckPassword;

	s >> loginName >> accountName >> password >> needCheckPassword >> entityDBID >> flags >> deadline >> clientType >> forceInternalLogin;
	s.readBlob(datas);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(pChannel);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("Baseappmgr::registerPendingAccountToBaseapp: not found loginapp!\n");
		return;
	}

	if (pending_logins_.find(loginName) != pending_logins_.end())
	{
		ERROR_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseapp: Already registered! accountName={}.\n",
			loginName));

		return;
	}

	pending_logins_[loginName] = cinfos->cid;

	updateBestBaseapp();

	if (bestBaseappID_ == 0 && numLoadBalancingApp() == 0)
	{
		ERROR_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseapp: Unable to allocate baseapp for load balancing! baseappSize={}, accountName={}.\n",
			baseapps_.size(), loginName));
	}

	ENTITY_ID eid = 0;
	cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, bestBaseappID_);

	if (cinfos == NULL || cinfos->pChannel == NULL || cinfos->state != COMPONENT_STATE_RUN)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		ForwardItem* pFI = new AppForwardItem();

		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
		(*pBundle) << loginName << accountName << password << needCheckPassword << eid << entityDBID << flags << deadline << clientType << forceInternalLogin;
		pBundle->appendBlob(datas);

		int runstate = -1;
		if (cinfos)
			runstate = (int)cinfos->state;

		WARNING_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseapp: not found baseapp({}, runstate={}, pChannel={}), message is buffered.\n",
			bestBaseappID_, runstate, (cinfos && cinfos->pChannel ? cinfos->pChannel->c_str() : "NULL")));

		pFI->pHandler = NULL;
		forward_anywhere_baseapp_messagebuffer_.push(pFI);
		return;
	}

	std::map< COMPONENT_ID, Baseapp >::iterator baseapps_iter = baseapps_.find(bestBaseappID_);

	DEBUG_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseapp:{}. allocBaseapp={}, numEntities={}.\n",
		accountName, bestBaseappID_, (bestBaseappID_ > 0 ? baseapps_iter->second.numEntities() : 0)));
	
	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
	(*pBundle) << loginName << accountName << password << needCheckPassword << eid << entityDBID << flags << deadline << clientType << forceInternalLogin;
	pBundle->appendBlob(datas);
	cinfos->pChannel->send(pBundle);

	// 预先将实体数量增加
	if (baseapps_iter != baseapps_.end())
	{
		baseapps_iter->second.incNumProxices();
	}
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
	int clientType;
	bool forceInternalLogin;
	bool needCheckPassword;

	s >> componentID >> loginName >> accountName >> password >> needCheckPassword >> entityID >> entityDBID >> flags >> deadline >> clientType >> forceInternalLogin;
	s.readBlob(datas);

	DEBUG_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseappAddr:{0}, componentID={1}, entityID={2}.\n",
		accountName, componentID, entityID));

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(pChannel);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("Baseappmgr::registerPendingAccountToBaseapp: not found loginapp!\n");
		return;
	}

	if (pending_logins_.find(loginName) != pending_logins_.end())
	{
		ERROR_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseappAddr: Already registered! accountName={}.\n",
			loginName));

		return;
	}

	pending_logins_[loginName] = cinfos->cid;

	cinfos = Components::getSingleton().findComponent(componentID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(fmt::format("Baseappmgr::registerPendingAccountToBaseappAddr: not found baseapp({}).\n", componentID));
		sendAllocatedBaseappAddr(pChannel, loginName, accountName, "", 0, 0);
		return;
	}
	
	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::registerPendingLogin);
	(*pBundle) << loginName << accountName << password << needCheckPassword << entityID << entityDBID << flags << deadline << clientType << forceInternalLogin;
	pBundle->appendBlob(datas);
	cinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onPendingAccountGetBaseappAddr(Network::Channel* pChannel, 
							  std::string& loginName, std::string& accountName, std::string& addr, uint16 tcp_port, uint16 udp_port)
{
	sendAllocatedBaseappAddr(pChannel, loginName, accountName, addr, tcp_port, udp_port);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::sendAllocatedBaseappAddr(Network::Channel* pChannel, 
							  std::string& loginName, std::string& accountName, const std::string& addr, uint16 tcp_port, uint16 udp_port)
{
	KBEUnordered_map< std::string, COMPONENT_ID >::iterator iter = pending_logins_.find(loginName);
	if(iter == pending_logins_.end())
	{
		ERROR_MSG(fmt::format("Baseappmgr::sendAllocatedBaseappAddr: not found accountName({}), pending_logins error!\n", loginName));
		return;
	}
	
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(iter->second);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(fmt::format("Baseappmgr::sendAllocatedBaseappAddr: not found loginapp! accountName={}\n", loginName));
		return;
	}

	Network::Bundle* pBundleToLoginapp = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundleToLoginapp).newMessage(LoginappInterface::onLoginAccountQueryBaseappAddrFromBaseappmgr);

	LoginappInterface::onLoginAccountQueryBaseappAddrFromBaseappmgrArgs5::staticAddToBundle((*pBundleToLoginapp), loginName, 
		accountName, addr, tcp_port, udp_port);

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

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

		(*pBundle).newMessage(LoginappInterface::onBaseappInitProgress);
		(*pBundle) << baseappsInitProgress_;
		(*iter).pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Baseappmgr::queryAppsLoads(Network::Channel* pChannel, MemoryStream& s)
{
	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	ConsoleInterface::ConsoleQueryAppsLoadsHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	//(*pBundle) << g_componentType;

	std::map< COMPONENT_ID, Baseapp >::iterator iter1 = baseapps_.begin();
	for (; iter1 != baseapps_.end(); ++iter1)
	{
		Baseapp& baseappref = iter1->second;
		(*pBundle) << iter1->first;
		(*pBundle) << baseappref.load();
		(*pBundle) << baseappref.numEntitys();
		(*pBundle) << baseappref.numEntities();
		(*pBundle) << baseappref.numProxices();
		(*pBundle) << baseappref.flags();
	}

	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqAccountBindEmailAllocCallbackLoginapp(Network::Channel* pChannel, COMPONENT_ID reqBaseappID, ENTITY_ID entityID, std::string& accountName, std::string& email,
	SERVER_ERROR_CODE failedcode, std::string& code)
{
	INFO_MSG(fmt::format("Baseappmgr::reqAccountBindEmailAllocCallbackLoginapp: {}({}) failedcode={}! reqBaseappID={}\n",
		accountName, entityID, failedcode, reqBaseappID));

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(LOGINAPP_TYPE);

	Components::COMPONENTS::iterator iter = cts.begin();
	for (; iter != cts.end(); ++iter)
	{
		if ((*iter).groupOrderid != 1)
			continue;

		if ((*iter).pChannel == NULL)
			continue;

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

		(*pBundle).newMessage(LoginappInterface::onReqAccountBindEmailAllocCallbackLoginapp);

		LoginappInterface::onReqAccountBindEmailAllocCallbackLoginappArgs6::staticAddToBundle((*pBundle), reqBaseappID,
			entityID, accountName, email, failedcode, code);

		(*iter).pChannel->send(pBundle);
		break;
	}
}

//-------------------------------------------------------------------------------------
void Baseappmgr::onReqAccountBindEmailCBFromLoginapp(Network::Channel* pChannel, COMPONENT_ID reqBaseappID, ENTITY_ID entityID, std::string& accountName, std::string& email,
	SERVER_ERROR_CODE failedcode, std::string& code, std::string& loginappCBHost, uint16 loginappCBPort)
{
	INFO_MSG(fmt::format("Baseappmgr::onReqAccountBindEmailCBFromLoginapp: {}({}) failedcode={}! loginappAddr={}:{}, reqBaseappID={}\n",
		accountName, entityID, failedcode, loginappCBHost, loginappCBPort, reqBaseappID));

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(reqBaseappID);
	if (cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("Baseappmgr::onReqAccountBindEmailCBFromLoginapp: not found baseapp!\n");
		return;
	}

	Network::Bundle* pBundleToBaseapp = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundleToBaseapp).newMessage(BaseappInterface::onReqAccountBindEmailCBFromBaseappmgr);

	BaseappInterface::onReqAccountBindEmailCBFromBaseappmgrArgs7::staticAddToBundle((*pBundleToBaseapp),
		entityID, accountName, email, failedcode, code, loginappCBHost, loginappCBPort);

	cinfos->pChannel->send(pBundleToBaseapp);
}

//-------------------------------------------------------------------------------------

}
