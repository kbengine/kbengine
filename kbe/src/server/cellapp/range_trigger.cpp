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
#include "entity_range_node.hpp"

#ifndef CODE_INLINE
#include "range_trigger.ipp"
#endif

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
	static_cast<EntityRangeNode*>(pRangeTrigger_->origin())->delWatcherNode(this);
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
	return pRangeTrigger_->origin()->x() + range_xz_; 
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::y()const 
{
	return pRangeTrigger_->origin()->y() + range_y_; 
}

//-------------------------------------------------------------------------------------
float RangeTriggerNode::z()const 
{
	return pRangeTrigger_->origin()->z() + range_xz_; 
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
}

//-------------------------------------------------------------------------------------
RangeTrigger::~RangeTrigger()
{
	uninstall();
}

//-------------------------------------------------------------------------------------
bool RangeTrigger::install()
{
	if(positiveBoundary_ == NULL)
		positiveBoundary_ = new RangeTriggerNode(this, 0, 0);

	if(negativeBoundary_ == NULL)
		negativeBoundary_ = new RangeTriggerNode(this, 0, 0);

	positiveBoundary_->range(0.0f, 0.0f);
	negativeBoundary_->range(0.0f, 0.0f);

	origin_->pRangeList()->insert(positiveBoundary_);
	origin_->pRangeList()->insert(negativeBoundary_);
	
	positiveBoundary_->old_x(FLT_MAX);
	positiveBoundary_->old_y(FLT_MAX);
	positiveBoundary_->old_z(FLT_MAX);

	negativeBoundary_->old_x(-FLT_MAX);
	negativeBoundary_->old_y(-FLT_MAX);
	negativeBoundary_->old_z(-FLT_MAX);

	positiveBoundary_->range(range_xz_, range_y_);
	positiveBoundary_->old_range(range_xz_, range_y_);
	negativeBoundary_->range(-range_xz_, -range_y_);
	negativeBoundary_->old_range(-range_xz_, -range_y_);

	positiveBoundary_->update();
	negativeBoundary_->update();
	return true;
}

//-------------------------------------------------------------------------------------
bool RangeTrigger::uninstall(bool isDestroy)
{
	if(positiveBoundary_ && positiveBoundary_->pRangeList())
		positiveBoundary_->pRangeList()->remove(positiveBoundary_);

	if(negativeBoundary_ && negativeBoundary_->pRangeList())
		negativeBoundary_->pRangeList()->remove(negativeBoundary_);

	if(isDestroy)
	{
		SAFE_RELEASE(positiveBoundary_);
		SAFE_RELEASE(negativeBoundary_);
	}

	return true;
}

//-------------------------------------------------------------------------------------
void RangeTrigger::onNodePassX(RangeTriggerNode* pRangeTriggerNode, RangeNode* pNode, bool isfront)
{
	if(pNode == origin())
		return;

	bool wasInZ = pRangeTriggerNode->wasInZRange(pNode);
	bool isInZ = pRangeTriggerNode->isInZRange(pNode);

	if(wasInZ != isInZ)
		return;

	bool wasIn = false;
	bool isIn = false;

	if(RangeList::hasY)
	{
		bool wasInY = pRangeTriggerNode->wasInYRange(pNode);
		bool isInY = pRangeTriggerNode->isInYRange(pNode);

		if(wasInY != isInY)
			return;

		wasIn = pRangeTriggerNode->wasInXRange(pNode) && wasInY && wasInZ;
		isIn = pRangeTriggerNode->isInXRange(pNode) && isInY && isInZ;
	}
	else
	{
		wasIn = pRangeTriggerNode->wasInXRange(pNode) && wasInZ;
		isIn = pRangeTriggerNode->isInXRange(pNode) && isInZ;
	}

	if(wasIn == isIn)
		return;

	if(isIn)
	{
		this->onEnter(pNode);
	}
	else
	{
		this->onLeave(pNode);
	}
}

//-------------------------------------------------------------------------------------
bool RangeTriggerNode::wasInYRange(RangeNode * pNode)
{
	if(!RangeList::hasY)
		return true;

	float originY = old_y() - old_range_y_;

	volatile float lowerBound = originY - fabs(old_range_y_);
	volatile float upperBound = originY + fabs(old_range_y_);
	return (lowerBound < pNode->old_y()) && (pNode->old_y() <= upperBound);
}

//-------------------------------------------------------------------------------------
void RangeTrigger::onNodePassY(RangeTriggerNode* pRangeTriggerNode, RangeNode* pNode, bool isfront)
{
	if(pNode == origin() || !RangeList::hasY)
		return;

	bool wasInZ = pRangeTriggerNode->wasInZRange(pNode);
	bool isInZ = pRangeTriggerNode->isInZRange(pNode);

	if(wasInZ != isInZ)
		return;

	bool wasInY = pRangeTriggerNode->wasInYRange(pNode);
	bool isInY = pRangeTriggerNode->isInYRange(pNode);

	if(wasInY == isInY)
		return;

	bool wasIn = pRangeTriggerNode->wasInXRange(pNode) && wasInY && wasInZ;
	bool isIn = pRangeTriggerNode->isInXRange(pNode) && isInY && isInZ;

	if(wasIn == isIn)
		return;

	if(isIn)
	{
		this->onEnter(pNode);
	}
	else
	{
		this->onLeave(pNode);
	}
}

//-------------------------------------------------------------------------------------
void RangeTrigger::onNodePassZ(RangeTriggerNode* pRangeTriggerNode, RangeNode* pNode, bool isfront)
{
	if(pNode == origin())
		return;

	if(RangeList::hasY)
	{
		bool wasInZ = pRangeTriggerNode->wasInZRange(pNode);
		bool isInZ = pRangeTriggerNode->isInZRange(pNode);

		if(wasInZ == isInZ)
			return;

		bool wasIn = pRangeTriggerNode->wasInXRange(pNode) && 
			pRangeTriggerNode->wasInYRange(pNode) && 
			wasInZ;

		bool isIn = pRangeTriggerNode->isInXRange(pNode) && 
			pRangeTriggerNode->isInYRange(pNode) && 
			isInZ;

		if(wasIn == isIn)
			return;

		if(isIn)
		{
			this->onEnter(pNode);
		}
		else
		{
			this->onLeave(pNode);
		}
	}
	else
	{
		bool wasInZ = pRangeTriggerNode->wasInZRange(pNode);
		bool isInZ = pRangeTriggerNode->isInZRange(pNode);

		if(wasInZ == isInZ)
			return;

		bool wasIn = pRangeTriggerNode->wasInXRange(pNode) && wasInZ;
		bool isIn = pRangeTriggerNode->isInXRange(pNode) && isInZ;

		if(wasIn == isIn)
			return;

		if(isIn)
		{
			this->onEnter(pNode);
		}
		else
		{
			this->onLeave(pNode);
		}
	}
}

//-------------------------------------------------------------------------------------
}
