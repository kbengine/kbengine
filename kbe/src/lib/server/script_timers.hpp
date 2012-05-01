/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/


#ifndef __SCRIPT_TIMERS_HPP__
#define __SCRIPT_TIMERS_HPP__
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"

namespace KBEngine
{

class MemoryStream;
class EntityApp;

class ScriptTimers
{
public:
	static void initialize(EntityApp & app);
	static void finalise(EntityApp & app);

	ScriptID addTimer(float initialOffset, float repeatOffset, int userArg,
			TimerHandler * pHandler);
	bool delTimer(ScriptID timerID);

	void releaseTimer(TimerHandle handle);

	void cancelAll();

	void writeToStream(MemoryStream & stream ) const;
	void readFromStream(MemoryStream & stream,
			uint32 numTimers, TimerHandler * pHandler);

	ScriptID getIDForHandle(TimerHandle handle) const;

	bool isEmpty() const	{ return map_.empty(); }
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

	void writeToStream( ScriptTimers * pTimers, MemoryStream & stream );
	void readFromStream( ScriptTimers ** ppTimers, MemoryStream & stream,
			TimerHandler * pHandler );

	ScriptID getIDForHandle( ScriptTimers * pTimers,
			TimerHandle handle );
}

}
#endif 
