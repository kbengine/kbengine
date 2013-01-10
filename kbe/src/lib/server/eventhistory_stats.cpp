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


#include "eventhistory_stats.hpp"
#include "profile_handler.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
EventHistoryStats::EventHistoryStats(std::string name):
stats_(),
name_(name)
{
}

//-------------------------------------------------------------------------------------
EventHistoryStats::~EventHistoryStats()
{
}

//-------------------------------------------------------------------------------------
void EventHistoryStats::add(const std::string& type, const std::string& name, uint32 size)
{
	std::string fullname = type + "." + name;
	
	STATS::iterator iter = stats_.find(fullname);
	if(iter == stats_.end())
	{
		stats_[name].name = fullname;
		stats_[name].size += size;
		stats_[name].count++;
		EventProfileHandler::triggerEvent(*this, stats_[name]);
		return;
	}

	iter->second.size += size;
	iter->second.count++;
	
	EventProfileHandler::triggerEvent(*this, iter->second);
}

//-------------------------------------------------------------------------------------
}
