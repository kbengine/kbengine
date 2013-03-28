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

#include "range_trigger.hpp"
#include "range_list.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
RangeTriggerNode::RangeTriggerNode(RangeTrigger* pRangeTrigger, float xz, float y):
RangeNode(NULL),
range_xz_(xz),
range_y_(y),
pRangeTrigger_(pRangeTrigger)
{
	flags_ = RANGENODE_FLAG_HIDE;
}

//-------------------------------------------------------------------------------------
RangeTriggerNode::~RangeTriggerNode()
{
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::x()const 
{ 
	return pRangeTrigger_->origin()->x() + range_xz_; 
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::y()const 
{ 
	return pRangeTrigger_->origin()->x() + range_y_; 
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::z()const 
{ 
	return pRangeTrigger_->origin()->x() + range_xz_; 
}

//-------------------------------------------------------------------------------------
void RangeTriggerNode::onNodePassX(RangeNode* pNode, bool isfront)
{
	pRangeTrigger_->onNodePassX(this, pNode, isfront);
}

//-------------------------------------------------------------------------------------
void RangeTriggerNode::onNodePassY(RangeNode* pNode, bool isfront)
{
	pRangeTrigger_->onNodePassY(this, pNode, isfront);
}

//-------------------------------------------------------------------------------------
void RangeTriggerNode::onNodePassZ(RangeNode* pNode, bool isfront)
{
	pRangeTrigger_->onNodePassZ(this, pNode, isfront);
}

//-------------------------------------------------------------------------------------
RangeTrigger::RangeTrigger(RangeNode* origin, float xz, float y):
range_xz_(fabs(xz)),
range_y_(fabs(y)),
origin_(origin),
positiveBoundary_(NULL),
negativeBoundary_(NULL)
{
	bool ret = initBoundary();
	KBE_ASSERT(ret);
}

//-------------------------------------------------------------------------------------
RangeTrigger::~RangeTrigger()
{
	if(positiveBoundary_)origin_->pRangeList()->remove(positiveBoundary_);
	if(negativeBoundary_)origin_->pRangeList()->remove(negativeBoundary_);

	SAFE_RELEASE(positiveBoundary_);
	SAFE_RELEASE(negativeBoundary_);
}

//-------------------------------------------------------------------------------------
bool RangeTrigger::initBoundary()
{
	positiveBoundary_ = new RangeTriggerNode(this, range_xz_, range_y_);
	negativeBoundary_ = new RangeTriggerNode(this, -range_xz_, -range_y_);
	origin_->pRangeList()->insert(positiveBoundary_);
	origin_->pRangeList()->insert(negativeBoundary_);
	return true;
}

//-------------------------------------------------------------------------------------
bool RangeTrigger::inRange(RangeNode * pNode)
{
	return false;
}

//-------------------------------------------------------------------------------------
void RangeTrigger::onNodePassX(RangeTriggerNode* pRangeTriggerNode, RangeNode* pNode, bool isfront)
{
	if(pNode == origin())
		return;

	// 先判断是否在y或者z中, 
}

//-------------------------------------------------------------------------------------
void RangeTrigger::onNodePassY(RangeTriggerNode* pRangeTriggerNode, RangeNode* pNode, bool isfront)
{
	if(pNode == origin())
		return;
}

//-------------------------------------------------------------------------------------
void RangeTrigger::onNodePassZ(RangeTriggerNode* pRangeTriggerNode, RangeNode* pNode, bool isfront)
{
	if(pNode == origin())
		return;
}

//-------------------------------------------------------------------------------------
}
