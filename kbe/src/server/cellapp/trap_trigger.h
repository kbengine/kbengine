// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
