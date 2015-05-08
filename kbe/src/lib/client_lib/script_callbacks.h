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

#ifndef KBE_SCRIPT_CALLBACKS_H
#define KBE_SCRIPT_CALLBACKS_H
#include "common/common.h"
#include "common/timer.h"
#include "pyscript/scriptobject.h"

namespace KBEngine
{

class ScriptCallbacks
{
public:
	ScriptCallbacks(Timers& timers);
	~ScriptCallbacks();

	ScriptID addCallback(float initialOffset, float repeatOffset,
			TimerHandler * pHandler);
	bool delCallback(ScriptID timerID);

	void releaseCallback(TimerHandle handle);

	void cancelAll();

	ScriptID getIDForHandle(TimerHandle handle) const;

	bool isEmpty() const	{ return map_.empty(); }

private:
	typedef std::map<ScriptID, TimerHandle> Map;

	ScriptID getNewID();
	Map::const_iterator findCallback(TimerHandle handle) const;
	Map::iterator findCallback(TimerHandle handle);

	Map map_;
	
	Timers& timers_;
};

class ScriptCallbackHandler : public TimerHandler
{
public:
	ScriptCallbackHandler(ScriptCallbacks& scriptCallbacks, PyObject * pObject ) : scriptCallbacks_(scriptCallbacks), pObject_( pObject ) 
	{
	}

	~ScriptCallbackHandler()
	{
		if(pObject_)
		{
			Py_DECREF( pObject_ );
			pObject_ = NULL;
		}
	}

protected:
	virtual void handleTimeout( TimerHandle handle, void * pUser );

	virtual void onRelease( TimerHandle handle, void * /*pUser*/ );

	ScriptCallbacks& scriptCallbacks_;
	PyObject* pObject_;
};

}
#endif 
