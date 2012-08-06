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


#include "pendingLoginmgr.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

namespace KBEngine { 
KBE_SINGLETON_INIT(PendingLoginMgr);

#define OP_TIME_OUT_MAX 15 * stampsPerSecondD()

//-------------------------------------------------------------------------------------
PendingLoginMgr::PendingLoginMgr(Mercury::NetworkInterface & networkInterface) :
	Task(),
	networkInterface_(networkInterface),
	start_(false)
{
	// dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
PendingLoginMgr::~PendingLoginMgr()
{
	//dispatcher().cancelFrequentTask(this);
}

//-------------------------------------------------------------------------------------
Mercury::EventDispatcher & PendingLoginMgr::dispatcher()
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
		//dispatcher().addFrequentTask(this);
		start_ = true;
	}
	
	pPLMap_[infos->accountName] = infos;
	infos->lastProcessTime = timestamp();

	DEBUG_MSG("PendingLoginMgr::add: size=%d, time=%"PRIu64".\n", pPLMap_.size(), infos->lastProcessTime);
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
bool PendingLoginMgr::process()
{
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
			DEBUG_MSG("PendingLoginMgr::process: size=%d, remove=%s. (%"PRIu64", %"PRIu64")\n", 
				pPLMap_.size(), infos->accountName.c_str(), curr, infos->lastProcessTime);

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
