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


#include "script_timers.hpp"
#include "server/serverapp.hpp"
#include "pyscript/script.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "cstdkbe/smartpointer.hpp"

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
		WARNING_MSG(boost::format("ScriptTimers::addTimer: Negative timer offset (%1%)\n") %
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
			time, interval, pHandler, (void *)(intptr_t)userData );
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

//-------------------------------------------------------------------------------------

class ScriptTimerHandler : public TimerHandler
{
public:
	ScriptTimerHandler( PyObject * pObject ) : pObject_( pObject ) 
	{
	}

private:
	virtual void handleTimeout( TimerHandle handle, void * pUser )
	{
		int id = ScriptTimersUtil::getIDForHandle( g_pTimers, handle );

		PyObject * pObject = pObject_.get();

		Py_INCREF( pObject );
		
		PyObject * pResult =
			PyObject_CallFunction( pObject, const_cast<char*>("ik"), id, uintptr( pUser ) );

		Py_XDECREF( pResult );
		Py_DECREF( pObject );
		SCRIPT_ERROR_CHECK();
	}

	virtual void onRelease( TimerHandle handle, void * /*pUser*/ )
	{
		ScriptTimersUtil::releaseTimer( &g_pTimers, handle );
		delete this;
	}

	SmartPointer<PyObject> pObject_;
};

//-------------------------------------------------------------------------------------
PyObject * py_addTimer( PyObject * args )
{
	PyObject * pObject;
	float initialOffset;
	float repeatOffset = 0.f;
	int userArg = 0;

	if (!PyArg_ParseTuple( args, "Of|fi:addTimer",
				&pObject, &initialOffset, &repeatOffset, &userArg ))
	{
		return NULL;
	}

	Py_INCREF( pObject );

	if (!PyCallable_Check( pObject ))
	{
		// For backward compatibility
		PyObject * pOnTimer = PyObject_GetAttrString( pObject, "onTimer" );

		Py_DECREF( pObject );

		if (pOnTimer == NULL)
		{
			PyErr_SetString( PyExc_TypeError,
					"Callback function is not callable" );

			SCRIPT_ERROR_CHECK();
			return NULL;
		}

		pObject = pOnTimer;
	}

	TimerHandler * pHandler = new ScriptTimerHandler( pObject );

	Py_DECREF( pObject );

	int id = ScriptTimersUtil::addTimer( &g_pTimers,
			initialOffset, repeatOffset,
			userArg, pHandler );

	if (id == 0)
	{
		PyErr_SetString( PyExc_ValueError, "Unable to add timer" );
		SCRIPT_ERROR_CHECK();
		delete pHandler;

		return NULL;
	}

	SCRIPT_ERROR_CHECK();
	return PyLong_FromLong( id );
}

//-------------------------------------------------------------------------------------
PyObject * py_delTimer( PyObject * args )
{
	int timerID;

	if (!PyArg_ParseTuple( args, "i", &timerID ))
	{
		return NULL;
	}

	if (!ScriptTimersUtil::delTimer( g_pTimers, timerID ))
	{
		ERROR_MSG(boost::format("KBEngine.delTimer: Unable to cancel timer %1%\n") %
				timerID );
	}

	S_Return;
}

}



