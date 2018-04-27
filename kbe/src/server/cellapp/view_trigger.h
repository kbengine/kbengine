// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_View_TRIGGER_H
#define KBE_View_TRIGGER_H

#include "range_trigger.h"

namespace KBEngine{

class Witness;

class ViewTrigger : public RangeTrigger
{
public:
	ViewTrigger(CoordinateNode* origin, float xz = 0.0f, float y = 0.0f);
	virtual ~ViewTrigger();
	
	/**
		某个节点进入或者离开了rangeTrigger
	*/
	virtual void onEnter(CoordinateNode * pNode);
	virtual void onLeave(CoordinateNode * pNode);

	INLINE Witness* pWitness() const;

protected:
	Witness* pWitness_;
};

}

#ifdef CODE_INLINE
#include "view_trigger.inl"
#endif
#endif
