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


#include "profile_handler.hpp"
#include "network/network_interface.hpp"
#include "network/event_dispatcher.hpp"
#include "network/address.hpp"
#include "network/bundle.hpp"
#include "pyscript/pyprofile.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/console_helper.hpp"
#include "helper/profile.hpp"
#include "server/serverconfig.hpp"

namespace KBEngine { 

KBEUnordered_map<std::string, KBEShared_ptr< ProfileHandler > > ProfileHandler::profiles;

//-------------------------------------------------------------------------------------
ProfileHandler::ProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
	networkInterface_(networkInterface),
	reportLimitTimerHandle_(),
	name_(name),
	addr_(addr)
{
	profiles[name].reset(this);

	reportLimitTimerHandle_ = networkInterface_.dispatcher().addTimer(
							timinglen * 1000000, this);
}

//-------------------------------------------------------------------------------------
ProfileHandler::~ProfileHandler()
{
	reportLimitTimerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
void ProfileHandler::handleTimeout(TimerHandle handle, void * arg)
{
	KBE_ASSERT(handle == reportLimitTimerHandle_);
	timeout();

	profiles.erase(name_);
}

//-------------------------------------------------------------------------------------
PyProfileHandler::PyProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
	script::PyProfile::start(name_);
}

//-------------------------------------------------------------------------------------
PyProfileHandler::~PyProfileHandler()
{
	if(name_ != "kbengine" || !g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
		script::PyProfile::remove(name_);
}

//-------------------------------------------------------------------------------------
void PyProfileHandler::timeout()
{
	if(name_ != "kbengine" || !g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
		script::PyProfile::stop(name_);

	MemoryStream s;
	script::PyProfile::addToStream(name_, &s);

	if(name_ == "kbengine" && g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
		script::PyProfile::start(name_);
	
	Mercury::Channel* pChannel = networkInterface_.findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(boost::format("PyProfileHandler::timeout: not found %1% addr(%2%)\n") % 
			name_ % addr_.c_str());
		return;
	}

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*(*bundle)).newMessage(msgHandler);

	int8 type = 0;
	(*(*bundle)) << type;
	(*(*bundle)).append(&s);
	(*(*bundle)).send(networkInterface_, pChannel);
}

//-------------------------------------------------------------------------------------
CProfileHandler::CProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
	networkInterface_.dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
CProfileHandler::~CProfileHandler()
{
	networkInterface_.dispatcher().cancelFrequentTask(this);
}

//-------------------------------------------------------------------------------------
void CProfileHandler::timeout()
{
	MemoryStream s;

	ArraySize size = profileVals_.size();
	s << size - 1;

	CProfileHandler::PROFILEVALS::iterator iter = profileVals_.begin();
	for(; iter != profileVals_.end(); iter++)
	{
		if(iter->first == "RunningTime")
		{
			continue;
		}

		uint32 count = iter->second.count;

		float lastTime = (float)stampsToSeconds(iter->second.diff_lastTime);
		float sumTime = (float)stampsToSeconds(iter->second.diff_sumTime);
		float lastIntTime = (float)stampsToSeconds(iter->second.diff_lastIntTime);
		float sumIntTime = (float)stampsToSeconds(iter->second.diff_sumIntTime);

		if(lastTime < 0.000f)
			lastTime = 0.0f;

		if(sumTime < 0.000f)
			sumTime = 0.0f;

		if(lastIntTime < 0.000f)
			lastIntTime = 0.0f;

		if(sumIntTime < 0.000f)
			sumIntTime = 0.0f;

		s << iter->first << count << lastTime << sumTime << lastIntTime << sumIntTime;
	}

	Mercury::Channel* pChannel = networkInterface_.findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(boost::format("CProfileHandler::timeout: not found %1% addr(%2%)\n") % 
			name_ % addr_.c_str());
		return;
	}

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*(*bundle)).newMessage(msgHandler);

	int8 type = 1;
	(*(*bundle)) << type;
	(*(*bundle)).append(&s);
	(*(*bundle)).send(networkInterface_, pChannel);
}

//-------------------------------------------------------------------------------------
bool CProfileHandler::process()
{
	// 这里每个tick都检查一遍， 防止中途有新加入的profileVal没被收集到
	ProfileGroup& defaultGroup = ProfileGroup::defaultGroup();
	ProfileGroup::PROFILEVALS::const_iterator iter = defaultGroup.profiles().begin();

	for(; iter != defaultGroup.profiles().end(); iter++)
	{
		std::string name = (*iter)->name();
	
		if(name != "RunningTime")
		{
			KBE_ASSERT(!(*iter)->running());
		}

		CProfileHandler::PROFILEVALS::iterator iter1 = profileVals_.find(name);
		
		// 如果已经初始化过则忽略
		if(iter1 != profileVals_.end())
		{
			CProfileHandler::ProfileVal& profileVal = iter1->second;

			profileVal.diff_count = (*iter)->count() - profileVal.count;
			profileVal.diff_lastTime = (*iter)->lastTime();
			profileVal.diff_sumTime = (*iter)->sumTime() - profileVal.sumTime;
			profileVal.diff_lastIntTime = (*iter)->lastIntTime();
			profileVal.diff_sumIntTime = (*iter)->sumIntTime() - profileVal.sumIntTime;
			continue;
		}

		profileVals_[name].name = name;
		profileVals_[name].count = (*iter)->count();
		profileVals_[name].lastTime = (*iter)->lastTime();
		profileVals_[name].sumTime = (*iter)->sumTime();
		profileVals_[name].lastIntTime = (*iter)->lastIntTime();
		profileVals_[name].sumIntTime = (*iter)->sumIntTime();
	}

	return true;
}

//-------------------------------------------------------------------------------------
EventProfileHandler::EventProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
}

//-------------------------------------------------------------------------------------
EventProfileHandler::~EventProfileHandler()
{
}

//-------------------------------------------------------------------------------------
void EventProfileHandler::timeout()
{
}

//-------------------------------------------------------------------------------------

}
