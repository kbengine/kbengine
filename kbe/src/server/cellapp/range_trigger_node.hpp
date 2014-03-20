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

#ifndef __KBE_RANGE_TRIGGER_NODE_HPP__
#define __KBE_RANGE_TRIGGER_NODE_HPP__

#include "range_node.hpp"
#include "math/math.hpp"

namespace KBEngine{

class RangeTrigger;

class RangeTriggerNode : public RangeNode
{
public:
	RangeTriggerNode(RangeTrigger* pRangeTrigger, float xz, float y);
	virtual ~RangeTriggerNode();

	INLINE void range(float xz, float y);
	INLINE void old_range(float xz, float y);
	INLINE float range_xz()const;
	INLINE float range_y()const;

	INLINE RangeTrigger* pRangeTrigger()const;
	INLINE void pRangeTrigger(RangeTrigger* pRangeTrigger);

	/**
		x && z�ɲ�ͬ��Ӧ��ʵ��(�Ӳ�ͬ����ȡ)
	*/
	virtual float x()const;
	virtual float y()const;
	virtual float z()const;

	INLINE bool isInXRange(RangeNode * pNode);
	INLINE bool isInYRange(RangeNode * pNode);
	INLINE bool isInZRange(RangeNode * pNode);

	INLINE bool wasInXRange(RangeNode * pNode);
	bool wasInYRange(RangeNode * pNode);
	INLINE bool wasInZRange(RangeNode * pNode);

	virtual void resetOld(){ 
		RangeNode::resetOld();
		old_range_xz_ = range_xz_;
		old_range_y_ = range_y_;
	}

	/**
		���ڵ�ɾ��
	*/
	virtual void onParentRemove(RangeNode* pParentNode);

	/**
		ĳ���ڵ�䶯�����˱��ڵ�
		@isfront: ��ǰ�ƶ���������ƶ�
	*/
	virtual void onNodePassX(RangeNode* pNode, bool isfront);
	virtual void onNodePassY(RangeNode* pNode, bool isfront);
	virtual void onNodePassZ(RangeNode* pNode, bool isfront);
protected:
	float range_xz_, range_y_, old_range_xz_, old_range_y_;
	RangeTrigger* pRangeTrigger_;
};

}

#endif
