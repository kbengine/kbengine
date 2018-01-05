/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_LOGWATCHER_H
#define KBE_LOGWATCHER_H
	
// common include	

//#define NDEBUG
#include "common/common.h"
#include "network/address.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine
{
class MemoryStream;
struct LOG_ITEM;

struct FilterOptions
{
	int32 uid;
	uint32 logtypes;
	uint8 componentBitmap[COMPONENT_END_TYPE];
	COMPONENT_ORDER globalOrder;
	COMPONENT_ORDER groupOrder;
	std::string keyStr;
	std::string date;
};

class LogWatcher
{
public:
	enum STATES
	{
		STATE_AUTO = 0,
		STATE_FINDING = 1,
	};

	LogWatcher();
	~LogWatcher();

	bool createFromStream(MemoryStream * s);
	bool updateSetting(MemoryStream * s);

	void reset();
	void addr(const Network::Address& address) { addr_ = address; }
	
	void onMessage(LOG_ITEM* pLogItem);

	STATES state() const{ return state_; }

protected:
	bool validDate_(const std::string& log);
	bool containKeyworlds_(const std::string& log);

protected:
	Network::Address addr_;
	FilterOptions filterOptions_;
	STATES state_;
};

}

#endif // KBE_LOGWATCHER_H
