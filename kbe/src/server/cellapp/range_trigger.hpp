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

#ifndef __KBE_RANGE_TRIGGER_HPP__
#define __KBE_RANGE_TRIGGER_HPP__

#include "range_node.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"	

namespace KBEngine{

class RangeTrigger;
class RangeTriggerNode : public RangeNode
{
public:
	RangeTriggerNode(RangeTrigger* pRangeTrigger, float xz, float y);
	virtual ~RangeTriggerNode();

	INLINE void range(float xz, float y);
	INLINE float range_xz()const;
	INLINE float range_y()const;

	INLINE RangeTrigger* pRangeTrigger()const;

	/**
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float x()const;
	virtual float y()const;
	virtual float z()const;

	/**
		某个节点变动经过了本节点
		@isfront: 向前移动还是向后移动
	*/
	virtual void onNodePassX(RangeNode* pNode, bool isfront);
	virtual void onNodePassY(RangeNode* pNode, bool isfront);
	virtual void onNodePassZ(RangeNode* pNode, bool isfront);
protected:
	float range_xz_, range_y_;
	RangeTrigger* pRangeTrigger_;
};



class RangeTrigger
{
public:
	RangeTrigger(RangeNode* origin, float xz, float y);
	virtual ~RangeTrigger();

	bool initBoundary();

	INLINE void range(float xz, float y);
	INLINE float range_xz()const;
	INLINE float range_y()const;

	INLINE RangeNode* origin()const;

	/**
		某个节点进入或者离开了rangeTrigger
	*/
	virtual void onEnter(RangeNode * pNode) = 0;
	virtual void onLeave(RangeNode * pNode) = 0;
	
	bool inRange(RangeNode * pNode);
protected:
	float range_xz_, range_y_;

	RangeNode* origin_;

	RangeTriggerNode* positiveBoundary_;
	RangeTriggerNode* negativeBoundary_;
};

}

#ifdef CODE_INLINE
#include "range_trigger.ipp"
#endif
#endif
