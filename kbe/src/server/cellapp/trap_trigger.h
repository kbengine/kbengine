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

#ifndef KBE_TRAP_TRIGGER_H
#define KBE_TRAP_TRIGGER_H

#include "range_trigger.h"

namespace KBEngine{

class Entity;
class ProximityController;

class TrapTrigger : public RangeTrigger
{
public:
	TrapTrigger(CoordinateNode* origin, ProximityController* pProximityController, float xz = 0.0f, float y = 0.0f);
	virtual ~TrapTrigger();


	
	/**
		某个节点进入或者离开了rangeTrigger
	*/
	virtual void onEnter(CoordinateNode * pNode);
	virtual void onLeave(CoordinateNode * pNode);

protected:
	ProximityController* pProximityController_;
};

}

#ifdef CODE_INLINE
#include "trap_trigger.inl"
#endif
#endif
