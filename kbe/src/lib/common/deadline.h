// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_DEADLINE_FORMAT_H
#define KBE_DEADLINE_FORMAT_H
#include "common/common.h"
#include <math.h>

namespace KBEngine
{

/**
 *	ÆÚÏÞ
 */
class Deadline
{
public:
	Deadline(uint32 usecs):
	days(0),
	hours(0),
	minutes(0),
	seconds(0),
	secs(usecs)
	{
		if(usecs > 0)
		{
	            days = usecs / (3600 * 24);
	            int m = usecs % (3600 * 24);
	            hours = m / 3600;
	            m = m % 3600;
	            minutes = m / 60;
	            seconds = m % 60;
		}
	}
	
	virtual ~Deadline() {
	}

	std::string print()
	{
		return fmt::format("{}days/{}:{}:{}", days, hours, minutes, seconds);
	}
	
	uint32 days, hours, minutes, seconds;
	uint32 secs;
};


}

#endif // KBE_DEADLINE_FORMAT_H
