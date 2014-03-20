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

#include "range_list.hpp"
#include "range_trigger.hpp"
#include "range_trigger_node.hpp"
#include "entity_range_node.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
RangeTriggerNode::RangeTriggerNode(RangeTrigger* pRangeTrigger, float xz, float y):
RangeNode(NULL),
range_xz_(xz),
range_y_(y),
old_range_xz_(range_xz_),
old_range_y_(range_y_),
pRangeTrigger_(pRangeTrigger)
{
	flags(RANGENODE_FLAG_HIDE);

#ifdef _DEBUG
	descr((boost::format("RangeTriggerNode(origin=%1%->%2%)") % pRangeTrigger_->origin() % pRangeTrigger_->origin()->descr()).str());
#endif

	static_cast<EntityRangeNode*>(pRangeTrigger_->origin())->addWatcherNode(this);
}

//-------------------------------------------------------------------------------------
RangeTriggerNode::~RangeTriggerNode()
{
	if(pRangeTrigger_)
		static_cast<EntityRangeNode*>(pRangeTrigger_->origin())->delWatcherNode(this);
}

//-------------------------------------------------------------------------------------
bool RangeTriggerNode::isInXRange(RangeNode * pNode)
{
	float originX = pRangeTrigger_->origin()->x();

	volatile float lowerBound = originX - fabs(range_xz_);
	volatile float upperBound = originX + fabs(range_xz_);
	return (lowerBound < pNode->x()) && (pNode->x() < upperBound);
}

//-------------------------------------------------------------------------------------
bool RangeTriggerNode::isInYRange(RangeNode * pNode)
{
	float originY = pRangeTrigger_->origin()->y();

	volatile float lowerBound = originY - fabs(range_y_);
	volatile float upperBound = originY + fabs(range_y_);
	return (lowerBound < pNode->y()) && (pNode->y() < upperBound);
}

//-------------------------------------------------------------------------------------
bool RangeTriggerNode::isInZRange(RangeNode * pNode)
{
	float originZ = pRangeTrigger_->origin()->z();

	volatile float lowerBound = originZ - fabs(range_xz_);
	volatile float upperBound = originZ + fabs(range_xz_);
	return (lowerBound < pNode->z()) && (pNode->z() < upperBound);
}

//-------------------------------------------------------------------------------------
bool RangeTriggerNode::wasInXRange(RangeNode * pNode)
{
	float originX = old_x() - old_range_xz_;

	volatile float lowerBound = originX - fabs(old_range_xz_);
	volatile float upperBound = originX + fabs(old_range_xz_);
	return (lowerBound < pNode->old_x()) && (pNode->old_x() < upperBound);
}

//-------------------------------------------------------------------------------------
bool RangeTriggerNode::wasInZRange(RangeNode * pNode)
{
	float originZ = old_z() - old_range_xz_;

	volatile float lowerBound = originZ - fabs(old_range_xz_);
	volatile float upperBound = originZ + fabs(old_range_xz_);
	return (lowerBound < pNode->old_z()) && (pNode->old_z() < upperBound);
}

//-------------------------------------------------------------------------------------
void RangeTriggerNode::onParentRemove(RangeNode* pParentNode)
{
	if((flags() & RANGENODE_FLAG_REMOVE) <= 0)
		pParentNode->pRangeList()->remove(this);
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::x()const 
{
	if(pRangeTrigger_== NULL)
		return old_x_ + range_xz_; 

	return pRangeTrigger_->origin()->x() + range_xz_; 
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::y()const 
{
	if(pRangeTrigger_== NULL)
		return old_y_ + range_y_; 

	return pRangeTrigger_->origin()->y() + range_y_; 
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::z()const 
{
	if(pRangeTrigger_== NULL)
		return old_z_ + range_xz_; 

	return pRangeTrigger_->origin()->z() + range_xz_; 
}

//-------------------------------------------------------------------------------------
void RangeTriggerNode::onNodePassX(RangeNode* pNode, bool isfront)
{
	if(pRangeTrigger_)
		pRangeTrigger_->onNodePassX(this, pNode, isfront);
}

//-------------------------------------------------------------------------------------
void RangeTriggerNode::onNodePassY(RangeNode* pNode, bool isfront)
{
	if(pRangeTrigger_)
		pRangeTrigger_->onNodePassY(this, pNode, isfront);
}

//-------------------------------------------------------------------------------------
void RangeTriggerNode::onNodePassZ(RangeNode* pNode, bool isfront)
{
	if(pRangeTrigger_)
		pRangeTrigger_->onNodePassZ(this, pNode, isfront);
}

//-------------------------------------------------------------------------------------
}
