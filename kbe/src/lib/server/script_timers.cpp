#include "script_timers.hpp"
#include "server/entity_app.hpp"


namespace KBEngine
{

EntityApp * g_pApp = NULL;

//-------------------------------------------------------------------------------------
void ScriptTimers::initialize(EntityApp & app)
{
	KBE_ASSERT(g_pApp == NULL);
	g_pApp = &app;
}

//-------------------------------------------------------------------------------------
void ScriptTimers::finalise(EntityApp & app)
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
		WARNING_MSG( "ScriptTimers::addTimer: Negative timer offset (%f)\n",
				initialOffset );

		initialOffset = 0.f;
	}

	KBE_ASSERT( g_pApp );

	int hertz = g_kbeSrvConfig.gameUpdateHertz();
	int initialTicks = GameTime( g_pApp->time() +
			initialOffset * hertz );
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
			pHandler, (void *)userArg );

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
void ScriptTimers::writeToStream(MemoryStream & stream) const
{
	KBE_ASSERT( g_pApp );

	stream << uint32( map_.size() );

	Map::const_iterator iter = map_.begin();

	while (iter != map_.end())
	{
		Timers::TimeStamp time;
		Timers::TimeStamp interval;
		void *	pUser;

		KBE_VERIFY( g_pApp->timers().getTimerInfo(
						iter->second,
						time, interval, pUser ) );

		stream << iter->first << time << interval << int32( uintptr(pUser) );

		++iter;
	}
}

//-------------------------------------------------------------------------------------
void ScriptTimers::readFromStream(MemoryStream & stream, uint32 numTimers,
		TimerHandler * pHandler)
{
	for (uint32 i = 0; i < numTimers; ++i)
	{
		ScriptID timerID;
		Timers::TimeStamp time;
		Timers::TimeStamp interval;
		int32 userData;

		stream >> timerID >> time >> interval >> userData;

		map_[ timerID ] = g_pApp->timers().add(
			time, interval, pHandler, (void *)userData );
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
namespace ScriptTimersUtil
{
//-------------------------------------------------------------------------------------
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
void writeToStream(ScriptTimers * pTimers, MemoryStream & stream)
{
	if (pTimers)
	{
		pTimers->writeToStream( stream );
	}
	else
	{
		stream << uint32( 0 );
	}
}

//-------------------------------------------------------------------------------------
void readFromStream( ScriptTimers ** ppTimers, MemoryStream & stream,
		TimerHandler * pHandler )
{
	ScriptTimers *& rpTimers = *ppTimers;
	KBE_ASSERT( rpTimers == NULL );

	uint32 numTimers;
	stream >> numTimers;

	if (numTimers != 0)
	{
		rpTimers = new ScriptTimers;
		rpTimers->readFromStream(stream, numTimers, pHandler);
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