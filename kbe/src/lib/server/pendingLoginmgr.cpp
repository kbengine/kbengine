// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "pendingLoginmgr.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "helper/profile.h"

namespace KBEngine { 

#define OP_TIME_OUT_MAX 120 * stampsPerSecondD()

//-------------------------------------------------------------------------------------
PendingLoginMgr::PendingLoginMgr(Network::NetworkInterface & networkInterface) :
	Task(),
	networkInterface_(networkInterface),
	start_(false)
{
	// dispatcher().addTask(this);
}

//-------------------------------------------------------------------------------------
PendingLoginMgr::~PendingLoginMgr()
{
	//dispatcher().cancelTask(this);

	PTINFO_MAP::iterator iter = pPLMap_.begin();
	for(; iter != pPLMap_.end(); ++iter)
	{
		delete iter->second;
	}

	pPLMap_.clear();
}

//-------------------------------------------------------------------------------------
Network::EventDispatcher & PendingLoginMgr::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
bool PendingLoginMgr::add(PLInfos* infos)
{
	PTINFO_MAP::iterator iter = pPLMap_.find(infos->accountName);
	if(iter != pPLMap_.end())
	{
		return false;
	}

	if(!start_)
	{
		//dispatcher().addTask(this);
		start_ = true;
	}
	
	pPLMap_[infos->accountName] = infos;
	infos->lastProcessTime = timestamp();

	DEBUG_MSG(fmt::format("PendingLoginMgr::add: {}, size={}.\n", infos->accountName, pPLMap_.size()));
	return true;
}

//-------------------------------------------------------------------------------------
PendingLoginMgr::PLInfos* PendingLoginMgr::remove(std::string& accountName)
{
	PTINFO_MAP::iterator iter = pPLMap_.find(accountName);
	if(iter != pPLMap_.end())
	{
		PLInfos* infos = iter->second;
		pPLMap_.erase(iter);
		DEBUG_MSG(fmt::format("PendingLoginMgr::remove: {}, size={}.\n", accountName, pPLMap_.size()));
		return infos;
	}
	
	//ERROR_MSG("PendingLoginMgr::remove::not found accountName[%s].\n", accountName);
	return NULL;
}

//-------------------------------------------------------------------------------------
PendingLoginMgr::PLInfos* PendingLoginMgr::find(std::string& accountName)
{
	PTINFO_MAP::iterator iter = pPLMap_.find(accountName);
	if(iter != pPLMap_.end())
	{
		PLInfos* infos = iter->second;
		return infos;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void PendingLoginMgr::removeNextTick(std::string& accountName)
{
	PTINFO_MAP::iterator iter = pPLMap_.find(accountName);
	if (iter != pPLMap_.end())
	{
		PLInfos* infos = iter->second;

		// 下一tick处理时就超时了
		TimeStamp curr = timestamp();
		infos->lastProcessTime = curr - OP_TIME_OUT_MAX - 1;
	}
}

//-------------------------------------------------------------------------------------
bool PendingLoginMgr::process()
{
	AUTO_SCOPED_PROFILE("PendingMgr_process");

	if(pPLMap_.size() <= 0)
	{
		start_ = false;
		return false;
	}
	
	PTINFO_MAP::iterator iter = pPLMap_.begin();
	while (iter != pPLMap_.end())
	{
		PLInfos* infos = iter->second;
		
		TimeStamp curr = timestamp();
		if(curr - infos->lastProcessTime >= OP_TIME_OUT_MAX)
		{
			iter = pPLMap_.erase(iter);
			DEBUG_MSG(fmt::format("PendingLoginMgr::process: [{1}] is timeout, currsize={0}.\n", 
				pPLMap_.size(), infos->accountName));

			SAFE_RELEASE(infos);
		}
		else
		{
			++iter;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
}
