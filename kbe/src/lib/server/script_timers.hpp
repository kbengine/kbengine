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

#ifndef __SCRIPT_TIMERS_HPP__
#define __SCRIPT_TIMERS_HPP__
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"

namespace KBEngine
{

class MemoryStream;
class ServerApp;

class ScriptTimers
{
public:
	ScriptTimers();
	~ScriptTimers();

	static void initialize(ServerApp & app);
	static void finalise(ServerApp & app);

	ScriptID addTimer(float initialOffset, float repeatOffset, int userArg,
			TimerHandler * pHandler);
	bool delTimer(ScriptID timerID);

	void releaseTimer(TimerHandle handle);

	void cancelAll();

	ScriptID getIDForHandle(TimerHandle handle) const;

	bool isEmpty() const	{ return map_.empty(); }

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);
private:
	typedef std::map<ScriptID, TimerHandle> Map;

	ScriptID getNewID();
	Map::const_iterator findTimer(TimerHandle handle) const;
	Map::iterator findTimer(TimerHandle handle);

	Map map_;
};


namespace ScriptTimersUtil
{
	ScriptID addTimer( ScriptTimers ** ppTimers,
		float initialOffset, float repeatOffset, int userArg,
		TimerHandler * pHandler );

	bool delTimer( ScriptTimers * pTimers, ScriptID timerID );
	void releaseTimer( ScriptTimers ** ppTimers, TimerHandle handle );
	void cancelAll( ScriptTimers * pTimers );

	ScriptID getIDForHandle( ScriptTimers * pTimers,
			TimerHandle handle );
}

}
#endif 
