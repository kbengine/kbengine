/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef KBE_RANGE_TRIGGER_NODE_H
#define KBE_RANGE_TRIGGER_NODE_H

#include "coordinate_node.h"
#include "math/math.h"

namespace KBEngine{

class RangeTrigger;

class RangeTriggerNode : public CoordinateNode
{
public:
	RangeTriggerNode(RangeTrigger* pRangeTrigger, float xz, float y);
	virtual ~RangeTriggerNode();

	INLINE void range(float xz, float y);
	INLINE void old_range(float xz, float y);
	INLINE float range_xz() const;
	INLINE float range_y() const;

	INLINE RangeTrigger* pRangeTrigger() const;
	INLINE void pRangeTrigger(RangeTrigger* pRangeTrigger);

	/**
		(��չ����)
		x && z�ɲ�ͬ��Ӧ��ʵ��(�Ӳ�ͬ����ȡ)
	*/
	virtual float xx() const;
	virtual float yy() const;
	virtual float zz() const;

	INLINE bool isInXRange(CoordinateNode * pNode);
	INLINE bool isInYRange(CoordinateNode * pNode);
	INLINE bool isInZRange(CoordinateNode * pNode);

	INLINE bool wasInXRange(CoordinateNode * pNode);
	bool wasInYRange(CoordinateNode * pNode);
	INLINE bool wasInZRange(CoordinateNode * pNode);

	virtual void resetOld(){ 
		CoordinateNode::resetOld();
		old_range_xz_ = range_xz_;
		old_range_y_ = range_y_;
	}

	/**
		���ڵ�ɾ��
	*/
	virtual void onParentRemove(CoordinateNode* pParentNode);

	/**
		ĳ���ڵ�䶯�����˱��ڵ�
		@isfront: ��ǰ�ƶ���������ƶ�
	*/
	virtual void onNodePassX(CoordinateNode* pNode, bool isfront);
	virtual void onNodePassY(CoordinateNode* pNode, bool isfront);
	virtual void onNodePassZ(CoordinateNode* pNode, bool isfront);

protected:
	float range_xz_, range_y_, old_range_xz_, old_range_y_;
	RangeTrigger* pRangeTrigger_;
};
}

#ifdef CODE_INLINE
#include "range_trigger_node.inl"
#endif
#endif
