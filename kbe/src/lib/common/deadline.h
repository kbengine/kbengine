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
