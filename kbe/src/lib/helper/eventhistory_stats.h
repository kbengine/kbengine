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

#ifndef KBE_EVENT_HISTORY_STATS_H
#define KBE_EVENT_HISTORY_STATS_H

#include "common/common.h"

namespace KBEngine { 

/*
	¼ÇÂ¼event_historyÁ÷Á¿
*/
class EventHistoryStats
{
public:

	struct Stats
	{
		Stats()
		{
			name = "";
			size = 0;
			count = 0;
		}

		std::string name;
		uint32 size;
		uint32 count;
	};

	typedef KBEUnordered_map<std::string, Stats> STATS;

	EventHistoryStats(std::string name);
	~EventHistoryStats();

	void trackEvent(const std::string& type, const std::string& name, uint32 size, const char* flags = ".");

	EventHistoryStats::STATS& stats(){ return stats_; }

	const char* name() const { return name_.c_str(); }
private:
	STATS stats_;

	std::string name_;
};

}

#endif // KBE_EVENT_HISTORY_STATS_H
