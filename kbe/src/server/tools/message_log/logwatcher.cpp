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


#include "logwatcher.hpp"
#include "messagelog.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/console_helper.hpp"

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
	for(uint8 i =0; i<COMPONENT_END_TYPE; i++)
	{
		componentBitmap_[i] = 0;
	}
	
	logtypes_ = 0;
	appOrder_ = 0;
}

//-------------------------------------------------------------------------------------
bool LogWatcher::loadFromStream(MemoryStream * s)
{
	reset();
	
	(*s) >> logtypes_;
	(*s) >> appOrder_;
	int8 count = 0;
	(*s) >> count;
	
	for(int8 i=0; i<count; i++)
	{
		COMPONENT_TYPE type;
		(*s) >> type;
		if(VALID_COMPONENT(type))
			componentBitmap_[type] = 1;
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
void LogWatcher::onMessage(uint32 logtype, COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER componentOrder, 
	int64 tm, GAME_TIME kbetime, const std::string& str, const std::stringstream& sstr)
{
	if(!VALID_COMPONENT(componentType) || componentBitmap_[componentType] == 0)
		return;

	if((logtypes_ & logtype) <= 0)
		return;

	if(appOrder_ > 0 && appOrder_ != componentOrder)
		return;


	Mercury::Channel* pChannel = Messagelog::getSingleton().networkInterface().findChannel(addr_);

	if(pChannel == NULL)
		return;

	Mercury::Bundle bundle;
	ConsoleInterface::ConsoleLogMessageHandler msgHandler;
	bundle.newMessage(msgHandler);
	bundle << sstr.str().c_str();
	bundle.send(Messagelog::getSingleton().networkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------

}
