// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
