// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "config.h"
#include "script_callbacks.h"
#include "server/serverapp.h"
#include "server/serverconfig.h"
#include "pyscript/script.h"
#include "pyscript/pyobject_pointer.h"
#include "common/smartpointer.h"

namespace KBEngine
{


//-------------------------------------------------------------------------------------
ScriptCallbacks::ScriptCallbacks(Timers& timers):
timers_(timers)
{
}

//-------------------------------------------------------------------------------------
ScriptCallbacks::~ScriptCallbacks()
{
	// DEBUG_MSG("ScriptCallbacks::~ScriptCallbacks: timers_size(%d).\n", map_.size());
	cancelAll();
}

//-------------------------------------------------------------------------------------
ScriptID ScriptCallbacks::addCallback( float initialOffset, float repeatOffset, TimerHandler * pHandler )
{
	if (initialOffset < 0.f)
	{
		WARNING_MSG(fmt::format("ScriptCallbacks::addTimer: Negative timer offset ({})\n",
				initialOffset));

		initialOffset = 0.f;
	}

	int hertz = 0;
	
	if(g_componentType == BOTS_TYPE)
		hertz = g_kbeSrvConfig.gameUpdateHertz();
	else
		hertz = Config::getSingleton().gameUpdateHertz();

	int initialTicks = GameTime( g_kbetime +
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

	TimerHandle timerHandle = timers_.add(
			initialTicks, repeatTicks,
			pHandler, NULL );

	if (timerHandle.isSet())
	{
		int id = this->getNewID();

		map_[ id ] = timerHandle;

		return id;
	}

	return 0;
}

//-------------------------------------------------------------------------------------
ScriptID ScriptCallbacks::getNewID()
{
	ScriptID id = 1;

	while (map_.find( id ) != map_.end())
	{
		++id;
	}

	return id;
}

//-------------------------------------------------------------------------------------
bool ScriptCallbacks::delCallback(ScriptID timerID)
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
void ScriptCallbacks::releaseCallback( TimerHandle handle )
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
void ScriptCallbacks::cancelAll()
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
ScriptID ScriptCallbacks::getIDForHandle(TimerHandle handle) const
{
	Map::const_iterator iter = this->findCallback( handle );

	return (iter != map_.end()) ? iter->first : 0;
}

//-------------------------------------------------------------------------------------
ScriptCallbacks::Map::const_iterator ScriptCallbacks::findCallback(TimerHandle handle) const
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
void ScriptCallbackHandler::handleTimeout( TimerHandle handle, void * pUser )
{
	AUTO_SCOPED_PROFILE("callCallbacks");

	//int id = scriptCallbacks_.getIDForHandle(handle);

	PyObject * pObject = pObject_;

	Py_INCREF( pObject );
	
	PyObject * pResult =
		PyObject_CallFunction( pObject, const_cast<char*>(""));

	Py_XDECREF( pResult );
	Py_DECREF( pObject );
	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void ScriptCallbackHandler::onRelease( TimerHandle handle, void * /*pUser*/ )
{
	scriptCallbacks_.releaseCallback(handle);
	delete this;
}

//-------------------------------------------------------------------------------------


}



