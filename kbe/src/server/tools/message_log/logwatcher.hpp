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

#ifndef KBE_LOGWATCHER_HPP
#define KBE_LOGWATCHER_HPP
	
// common include	

//#define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "network/address.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{
class MemoryStream;

class LogWatcher
{
public:
	LogWatcher();
	~LogWatcher();

	bool loadFromStream(MemoryStream * s);
	
	void reset();
	void addr(const Network::Address& address) { addr_ = address; }
	
	void onMessage(uint32 logtype, COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER componentOrder, 
		int64 tm, GAME_TIME kbetime, const std::string& str, const std::stringstream& sstr);
protected:
	uint32 logtypes_;
	uint8 componentBitmap_[COMPONENT_END_TYPE];
	COMPONENT_ORDER appOrder_;
	Network::Address addr_;
};

}

#endif // KBE_LOGWATCHER_HPP
