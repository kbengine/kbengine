/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "logwatcher.h"
#include "logger.h"
#include "common/memorystream.h"
#include "helper/console_helper.h"

namespace KBEngine{
	
//-------------------------------------------------------------------------------------
LogWatcher::LogWatcher()
{
	reset();
}

//-------------------------------------------------------------------------------------
LogWatcher::~LogWatcher()
{
}

//-------------------------------------------------------------------------------------
void LogWatcher::reset()
{
	for(uint8 i =0; i<COMPONENT_END_TYPE; ++i)
	{
		filterOptions_.componentBitmap[i] = 0;
	}
	
	filterOptions_.uid = 0;
	filterOptions_.logtypes = 0;
	filterOptions_.globalOrder = 0;
	filterOptions_.groupOrder = 0;
	filterOptions_.keyStr = "";
	filterOptions_.date = "";

	state_ = STATE_AUTO;
}

//-------------------------------------------------------------------------------------
bool LogWatcher::createFromStream(MemoryStream * s)
{
	bool ret = updateSetting(s);

	bool isfind = false;
	(*s) >> isfind;

	if(isfind)
		state_ = STATE_FINDING;
	else
		state_ = STATE_AUTO;

	return ret;
}

//-------------------------------------------------------------------------------------
bool LogWatcher::updateSetting(MemoryStream * s)
{
	reset();
	
	(*s) >> filterOptions_.uid;
	(*s) >> filterOptions_.logtypes;
	(*s) >> filterOptions_.globalOrder;
	(*s) >> filterOptions_.groupOrder;
	(*s) >> filterOptions_.date;
	(*s) >> filterOptions_.keyStr;

	int8 count = 0;
	(*s) >> count;
	
	for(int8 i=0; i<count; ++i)
	{
		COMPONENT_TYPE type;
		(*s) >> type;

		if(VALID_COMPONENT(type))
			filterOptions_.componentBitmap[type] = 1;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void LogWatcher::onMessage(LOG_ITEM* pLogItem)
{
	if(!VALID_COMPONENT(pLogItem->componentType) || filterOptions_.componentBitmap[pLogItem->componentType] == 0)
		return;

	if(filterOptions_.uid != pLogItem->uid)
		return;

	if((filterOptions_.logtypes & pLogItem->logtype) <= 0)
		return;

	if(filterOptions_.globalOrder > 0 && filterOptions_.globalOrder != pLogItem->componentGlobalOrder)
		return;

	if(filterOptions_.groupOrder > 0 && filterOptions_.groupOrder != pLogItem->componentGroupOrder)
		return;

	Network::Channel* pChannel = Logger::getSingleton().networkInterface().findChannel(addr_);

	if(pChannel == NULL)
		return;

	if(!validDate_(pLogItem->logstream.str()) || !containKeyworlds_(pLogItem->logstream.str()))
		return;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	ConsoleInterface::ConsoleLogMessageHandler msgHandler;
	(*pBundle).newMessage(msgHandler);
	(*pBundle) << pLogItem->logstream.str().c_str();
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
bool LogWatcher::validDate_(const std::string& log)
{
	if(filterOptions_.date.size() == 0)
		return true;

	if(log.find(filterOptions_.date.c_str()) != std::string::npos)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------
bool LogWatcher::containKeyworlds_(const std::string& log)
{
	if(filterOptions_.keyStr.size() == 0)
		return true;

	if(log.find(filterOptions_.keyStr.c_str()) != std::string::npos)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------

}
