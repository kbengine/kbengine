// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
