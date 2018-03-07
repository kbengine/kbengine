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

#ifndef KBE_RANGE_TRIGGER_H
#define KBE_RANGE_TRIGGER_H

#include "coordinate_node.h"
#include "helper/debug_helper.h"
#include "common/common.h"	

namespace KBEngine{

class RangeTriggerNode;
class RangeTrigger
{
public:
	RangeTrigger(CoordinateNode* origin, float xz, float y);
	virtual ~RangeTrigger();

	bool install();
	bool uninstall();
	bool reinstall(CoordinateNode* pCoordinateNode);
	INLINE bool isInstalled() const;

	INLINE void range(float xz, float y);
	INLINE float range_xz() const;
	INLINE float range_y() const;

	INLINE CoordinateNode* origin() const;
	INLINE void origin(CoordinateNode* pCoordinateNode);

	/**
		更新范围数据
	*/
	virtual void update(float xz, float y);

	/**
		某个节点进入或者离开了rangeTrigger
	*/
	virtual void onEnter(CoordinateNode * pNode) = 0;
	virtual void onLeave(CoordinateNode * pNode) = 0;

	/**
		某个节点变动经过了本节点
		@isfront: 向前移动还是向后移动
	*/
	virtual void onNodePassX(RangeTriggerNode* pRangeTriggerNode, CoordinateNode* pNode, bool isfront);
	virtual void onNodePassY(RangeTriggerNode* pRangeTriggerNode, CoordinateNode* pNode, bool isfront);
	virtual void onNodePassZ(RangeTriggerNode* pRangeTriggerNode, CoordinateNode* pNode, bool isfront);

protected:
	float range_xz_, range_y_;

	CoordinateNode* origin_;

	RangeTriggerNode* positiveBoundary_;
	RangeTriggerNode* negativeBoundary_;

	bool removing_;
};

}

#ifdef CODE_INLINE
#include "range_trigger.inl"
#endif
#endif
