// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "profile_handler.h"
#include "network/network_interface.h"
#include "network/event_dispatcher.h"
#include "network/address.h"
#include "network/network_stats.h"
#include "network/bundle.h"
#include "network/message_handler.h"
#include "common/memorystream.h"
#include "helper/console_helper.h"
#include "helper/profile.h"

namespace KBEngine { 

KBEUnordered_map<std::string, KBEShared_ptr< ProfileHandler > > ProfileHandler::profiles;

//-------------------------------------------------------------------------------------
ProfileHandler::ProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Network::Address& addr) :
	networkInterface_(networkInterface),
	reportLimitTimerHandle_(),
	name_(name),
	addr_(addr),
	timinglen_(timinglen)
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
CProfileHandler::CProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
std::string name, const Network::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
	networkInterface_.dispatcher().addTask(this);

	ProfileGroup& defaultGroup = ProfileGroup::defaultGroup();
	ProfileGroup::PROFILEVALS::const_iterator iter = defaultGroup.profiles().begin();

	for(; iter != defaultGroup.profiles().end(); ++iter)
	{
		std::string name = (*iter)->name();
	
		if(name == "RunningTime")
		{
			continue;
		}

		profileVals_[name].name = name;
		profileVals_[name].count = (*iter)->count();
		profileVals_[name].lastTime = (*iter)->lastTime();
		profileVals_[name].sumTime = (*iter)->sumTime();
		profileVals_[name].lastIntTime = (*iter)->lastIntTime();
		profileVals_[name].sumIntTime = (*iter)->sumIntTime();
	}
}

//-------------------------------------------------------------------------------------
CProfileHandler::~CProfileHandler()
{
	networkInterface_.dispatcher().cancelTask(this);
}

//-------------------------------------------------------------------------------------
void CProfileHandler::timeout()
{
	MemoryStream s;
	
	s << timinglen_;

	ArraySize size = (ArraySize)profileVals_.size();
	s << size - 1;

	CProfileHandler::PROFILEVALS::iterator iter = profileVals_.begin();
	for(; iter != profileVals_.end(); ++iter)
	{
		if(iter->first == "RunningTime")
		{
			continue;
		}

		uint32 count = iter->second.diff_count;

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

	sendStream(&s);
}

//-------------------------------------------------------------------------------------
void CProfileHandler::sendStream(MemoryStream* s)
{
	Network::Channel* pChannel = networkInterface_.findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(fmt::format("CProfileHandler::sendStream: not found {} addr({})\n",
			name_, addr_.c_str()));
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	int8 type = 1;
	(*pBundle) << type;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
bool CProfileHandler::process()
{
	// 这里每个tick都检查一遍， 防止中途有新加入的profileVal没被收集到
	ProfileGroup& defaultGroup = ProfileGroup::defaultGroup();
	ProfileGroup::PROFILEVALS::const_iterator iter = defaultGroup.profiles().begin();

	for(; iter != defaultGroup.profiles().end(); ++iter)
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
std::vector<EventProfileHandler*> EventProfileHandler::eventProfileHandlers_;
	
EventProfileHandler::EventProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Network::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr),
profileMaps_(),
removeHandle_(-1)
{
	eventProfileHandlers_.push_back(this);
	removeHandle_ = (int)(eventProfileHandlers_.size() - 1);
}

//-------------------------------------------------------------------------------------
EventProfileHandler::~EventProfileHandler()
{
	KBE_ASSERT(eventProfileHandlers_[removeHandle_] == this);
	EventProfileHandler* pBack = eventProfileHandlers_.back();
	eventProfileHandlers_[removeHandle_] = pBack;

	if(eventProfileHandlers_.size() > 0)
		eventProfileHandlers_.resize(eventProfileHandlers_.size() - 1);
}

//-------------------------------------------------------------------------------------
void EventProfileHandler::timeout()
{
	MemoryStream s;

	s << timinglen_;

	ArraySize size = (ArraySize)profileMaps_.size();
	s << size;

	EventProfileHandler::PROFILEVALMAP::iterator iter = profileMaps_.begin();
	for(; iter != profileMaps_.end(); ++iter)
	{
		std::string type_name = iter->first;
		PROFILEVALS& vals = iter->second;
		
		s << type_name;

		size = (ArraySize)vals.size();
		s << size;

		EventProfileHandler::PROFILEVALS::iterator iter1 = vals.begin();
		for(; iter1 != vals.end(); ++iter1)
		{
			ProfileVal& val = iter1->second;

			s << val.name;
			s << val.count;
			s << val.size;
		}
	}

	sendStream(&s);
}

//-------------------------------------------------------------------------------------
void EventProfileHandler::sendStream(MemoryStream* s)
{
	Network::Channel* pChannel = networkInterface_.findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(fmt::format("EventProfileHandler::sendStream: not found {} addr({})\n",
			name_, addr_.c_str()));
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	int8 type = 2;
	(*pBundle) << type;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void EventProfileHandler::onTriggerEvent(const EventHistoryStats& eventHistory, const EventHistoryStats::Stats& stats, 
										 uint32 size)
{
	EventProfileHandler::PROFILEVALMAP::iterator iter = profileMaps_.find(eventHistory.name());

	ProfileVal* pval = NULL;
	if(iter == profileMaps_.end())
	{
		PROFILEVALS& vals = profileMaps_[eventHistory.name()];
		pval = &vals[stats.name];
		pval->name = stats.name;
	}	
	else
	{
		PROFILEVALS& vals = iter->second;

		EventProfileHandler::PROFILEVALS::iterator iter1 = vals.find(stats.name);
		if(iter1 == vals.end())
		{
			pval = &vals[stats.name];
			pval->name = stats.name;
		}
		else
		{
			pval = &iter1->second;
		}
	}

	pval->count++;
	pval->size += size;
}

//-------------------------------------------------------------------------------------
void EventProfileHandler::triggerEvent(const EventHistoryStats& eventHistory, const EventHistoryStats::Stats& stats, 
									  uint32 size)
{
	std::vector<EventProfileHandler*>::iterator iter = eventProfileHandlers_.begin();
	for(; iter != eventProfileHandlers_.end(); ++iter)
	{
		(*iter)->onTriggerEvent(eventHistory, stats, size);
	}
}

//-------------------------------------------------------------------------------------
NetworkProfileHandler::NetworkProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Network::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr),
profileVals_()
{
	Network::NetworkStats::getSingleton().addHandler(this);
}

//-------------------------------------------------------------------------------------
NetworkProfileHandler::~NetworkProfileHandler()
{
	Network::NetworkStats::getSingleton().removeHandler(this);
}

//-------------------------------------------------------------------------------------
void NetworkProfileHandler::timeout()
{
	MemoryStream s;

	s << timinglen_;

	ArraySize size = (ArraySize)profileVals_.size();
	s << size;

	NetworkProfileHandler::PROFILEVALS::iterator iter = profileVals_.begin();
	for(; iter != profileVals_.end(); ++iter)
	{
		NetworkProfileHandler::ProfileVal& profileVal = iter->second;

		s << profileVal.name;

		s << profileVal.send_count << profileVal.send_size << profileVal.send_avgsize << 
			profileVal.total_send_size << profileVal.total_send_count;

		s << profileVal.recv_count << profileVal.recv_size << profileVal.recv_avgsize << 
			profileVal.total_recv_size << profileVal.total_recv_count;
	}

	sendStream(&s);
}

//-------------------------------------------------------------------------------------
void NetworkProfileHandler::sendStream(MemoryStream* s)
{
	Network::Channel* pChannel = networkInterface_.findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(fmt::format("NetworkProfileHandler::sendStream: not found {} addr({})\n",
			name_, addr_.c_str()));
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	int8 type = 3;
	(*pBundle) << type;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void NetworkProfileHandler::onSendMessage(const Network::MessageHandler& msgHandler, int size)
{
	NetworkProfileHandler::PROFILEVALS::iterator iter1 = profileVals_.find(msgHandler.name);
	
	NetworkProfileHandler::ProfileVal* pProfileVal = NULL;
	if(iter1 == profileVals_.end())
	{
		NetworkProfileHandler::ProfileVal& profileVal = profileVals_[msgHandler.name];
		pProfileVal = &profileVal;
		profileVal.name = msgHandler.name;
	}
	else
	{
		pProfileVal = &iter1->second;
	}

	pProfileVal->send_size += size;
	pProfileVal->send_count++;
	
	pProfileVal->send_avgsize = msgHandler.sendavgsize();
	pProfileVal->total_send_size = msgHandler.sendsize();
	pProfileVal->total_send_count = msgHandler.sendcount();
}

//-------------------------------------------------------------------------------------
void NetworkProfileHandler::onRecvMessage(const Network::MessageHandler& msgHandler, int size)
{
	NetworkProfileHandler::PROFILEVALS::iterator iter1 = profileVals_.find(msgHandler.name);
	
	NetworkProfileHandler::ProfileVal* pProfileVal = NULL;
	if(iter1 == profileVals_.end())
	{
		NetworkProfileHandler::ProfileVal& profileVal = profileVals_[msgHandler.name];
		pProfileVal = &profileVal;
		profileVal.name = msgHandler.name;
	}
	else
	{
		pProfileVal = &iter1->second;
	}

	pProfileVal->recv_size += size;
	pProfileVal->recv_count++;
	
	pProfileVal->recv_avgsize = msgHandler.recvavgsize();
	pProfileVal->total_recv_size = msgHandler.recvsize();
	pProfileVal->total_recv_count = msgHandler.recvcount();
}

//-------------------------------------------------------------------------------------

}
