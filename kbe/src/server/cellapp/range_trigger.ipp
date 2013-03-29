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


namespace KBEngine{

//-------------------------------------------------------------------------------------
INLINE void RangeTrigger::range(float xz, float y)
{
	range_xz_ = fabs(xz);
	range_y_ = fabs(y);
}

//-------------------------------------------------------------------------------------
INLINE float RangeTrigger::range_xz()const
{
	return range_xz_;
}

//-------------------------------------------------------------------------------------
INLINE float RangeTrigger::range_y()const
{
	return range_y_;
}

//-------------------------------------------------------------------------------------
INLINE RangeNode* RangeTrigger::origin()const
{
	return origin_;
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTrigger::isInXRange(RangeNode * pNode)
{
	float originX = origin_->x();

	volatile float lowerBound = originX - range_xz_;
	volatile float upperBound = originX + range_xz_;
	return (lowerBound < pNode->x()) && (pNode->x() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTrigger::isInYRange(RangeNode * pNode)
{
	float originY = origin_->y();

	volatile float lowerBound = originY - range_y_;
	volatile float upperBound = originY + range_y_;
	return (lowerBound < pNode->y()) && (pNode->y() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTrigger::isInZRange(RangeNode * pNode)
{
	float originZ = origin_->z();

	volatile float lowerBound = originZ - range_xz_;
	volatile float upperBound = originZ + range_xz_;
	return (lowerBound < pNode->z()) && (pNode->z() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTrigger::wasInXRange(RangeNode * pNode)
{
	float originX = origin_->old_x();

	volatile float lowerBound = originX - range_xz_;
	volatile float upperBound = originX + range_xz_;
	return (lowerBound < pNode->old_x()) && (pNode->old_x() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTrigger::wasInYRange(RangeNode * pNode)
{
	float originY = origin_->old_y();

	volatile float lowerBound = originY - range_y_;
	volatile float upperBound = originY + range_y_;
	return (lowerBound < pNode->old_y()) && (pNode->old_y() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTrigger::wasInZRange(RangeNode * pNode)
{
	float originZ = origin_->old_z();

	volatile float lowerBound = originZ - range_xz_;
	volatile float upperBound = originZ + range_xz_;
	return (lowerBound < pNode->old_z()) && (pNode->old_z() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE void RangeTriggerNode::range(float xz, float y)
{
	range_xz_ = fabs(xz);
	range_y_ = fabs(y);
}

//-------------------------------------------------------------------------------------
INLINE float RangeTriggerNode::range_xz()const
{
	return range_xz_;
}

//-------------------------------------------------------------------------------------
INLINE float RangeTriggerNode::range_y()const
{
	return range_y_;
}

//-------------------------------------------------------------------------------------
INLINE RangeTrigger* RangeTriggerNode::pRangeTrigger()const
{
	return pRangeTrigger_;
}

//-------------------------------------------------------------------------------------
}
