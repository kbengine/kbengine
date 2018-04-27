// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "script_timers.h"
#include "server/serverapp.h"
#include "pyscript/pickler.h"
#include "pyscript/script.h"
#include "pyscript/pyobject_pointer.h"
#include "common/smartpointer.h"

namespace KBEngine
{

ServerApp * g_pApp = NULL;
ScriptTimers * g_pTimers = NULL;

//-------------------------------------------------------------------------------------
ScriptTimers::ScriptTimers()
{
}

//-------------------------------------------------------------------------------------
ScriptTimers::~ScriptTimers()
{
	// DEBUG_MSG("ScriptTimers::~ScriptTimers: timers_size(%d).\n", map_.size());
	cancelAll();
}

//-------------------------------------------------------------------------------------
void ScriptTimers::initialize(ServerApp & app)
{
	KBE_ASSERT(g_pApp == NULL);
	g_pApp = &app;
}

//-------------------------------------------------------------------------------------
void ScriptTimers::finalise(ServerApp & app)
{
	KBE_ASSERT(g_pApp == &app);
	g_pApp = NULL;
}

//-------------------------------------------------------------------------------------
ScriptID ScriptTimers::addTimer( float initialOffset,
		float repeatOffset, int userArg, TimerHandler * pHandler )
{
	if (initialOffset < 0.f)
	{
		WARNING_MSG(fmt::format("ScriptTimers::addTimer: Negative timer offset ({})\n",
				initialOffset));

		initialOffset = 0.f;
	}

	KBE_ASSERT( g_pApp );

	int hertz = g_kbeSrvConfig.gameUpdateHertz();
	int initialTicks = GameTime( g_pApp->time() + initialOffset * hertz );
	int repeatTicks = 0;

	if (repeatOffset > 0.f)
	{
		repeatTicks = GameTime( repeatOffset * hertz );
		if (repeatTicks < 1)
		{
			repeatTicks = 1;
		}
	}

	TimerHandle timerHandle = g_pApp->timers().add(
			initialTicks, repeatTicks,
			pHandler, (void *)(intptr_t)userArg );

	if (timerHandle.isSet())
	{
		int id = this->getNewID();

		map_[ id ] = timerHandle;

		return id;
	}

	return 0;
}

//-------------------------------------------------------------------------------------
ScriptID ScriptTimers::getNewID()
{
	ScriptID id = 1;

	while (map_.find( id ) != map_.end())
	{
		++id;
	}

	return id;
}

//-------------------------------------------------------------------------------------
bool ScriptTimers::delTimer(ScriptID timerID)
{
	Map::iterator iter = map_.find( timerID );

	if (iter != map_.end())
	{
		TimerHandle handle = iter->second;
		handle.cancel();
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
void ScriptTimers::releaseTimer( TimerHandle handle )
{
	int numErased = 0;

	Map::iterator iter = map_.begin();

	while (iter != map_.end())
	{
		KBE_ASSERT( iter->second.isSet() );

		if (handle == iter->second)
		{
			map_.erase( iter++ );
			++numErased;
		}
		else
		{
			iter++;
		}
	}

	KBE_ASSERT( numErased == 1 );
}

//-------------------------------------------------------------------------------------
void ScriptTimers::cancelAll()
{
	Map::size_type size = map_.size();

	for (Map::size_type i = 0; i < size; ++i)
	{
		KBE_ASSERT( i + map_.size() == size );
		TimerHandle handle = map_.begin()->second;
		handle.cancel();
	}
}

//-------------------------------------------------------------------------------------
ScriptID ScriptTimers::getIDForHandle(TimerHandle handle) const
{
	Map::const_iterator iter = this->findTimer( handle );

	return (iter != map_.end()) ? iter->first : 0;
}

//-------------------------------------------------------------------------------------
ScriptTimers::Map::const_iterator ScriptTimers::findTimer(TimerHandle handle) const
{
	Map::const_iterator iter = map_.begin();

	while (iter != map_.end())
	{
		if (iter->second == handle)
		{
			return iter;
		}

		++iter;
	}

	return iter;
}

//-------------------------------------------------------------------------------------
void ScriptTimers::directAddTimer(ScriptID tid, TimerHandle handle)
{
	map_[tid] = handle;
}

//-------------------------------------------------------------------------------------
namespace ScriptTimersUtil
{

ScriptID addTimer( ScriptTimers ** ppTimers,
								float initialOffset, float repeatOffset, int userArg,
								TimerHandler * pHandler )
{
	ScriptTimers *& rpTimers = *ppTimers;

	if (rpTimers == NULL)
	{
		rpTimers = new ScriptTimers;
	}

	return rpTimers->addTimer( initialOffset, repeatOffset,
			userArg, pHandler );
}

//-------------------------------------------------------------------------------------
bool delTimer(ScriptTimers * pTimers, ScriptID timerID)
{
	return pTimers && pTimers->delTimer( timerID );
}

//-------------------------------------------------------------------------------------
void cancelAll( ScriptTimers * pTimers )
{
	if (pTimers)
	{
		pTimers->cancelAll();
	}
}

//-------------------------------------------------------------------------------------
void releaseTimer( ScriptTimers ** ppTimers, TimerHandle handle )
{
	ScriptTimers *& rpTimers = *ppTimers;
	KBE_ASSERT( rpTimers );

	rpTimers->releaseTimer( handle );

	if (rpTimers->isEmpty())
	{
		delete rpTimers;
		rpTimers = NULL;
	}
}

//-------------------------------------------------------------------------------------
ScriptID getIDForHandle( ScriptTimers * pTimers,
		TimerHandle handle )
{
	if (pTimers == NULL)
	{
		return 0;
	}

	return pTimers->getIDForHandle( handle );
}

//-------------------------------------------------------------------------------------

}

}



