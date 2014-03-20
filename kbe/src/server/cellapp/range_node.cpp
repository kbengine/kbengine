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

#include "range_node.hpp"
#include "range_list.hpp"

#ifndef CODE_INLINE
#include "range_node.ipp"
#endif

namespace KBEngine{	

//-------------------------------------------------------------------------------------
RangeNode::RangeNode(RangeList* pRangeList):
pPrevX_(NULL),
pNextX_(NULL),
pPrevY_(NULL),
pNextY_(NULL),
pPrevZ_(NULL),
pNextZ_(NULL),
pRangeList_(pRangeList),
x_(0.f),
y_(0.f),
z_(0.f),
old_x_(0.f),
old_y_(0.f),
old_z_(0.f),
#ifdef _DEBUG
descr_(),
#endif
flags_(RANGENODE_FLAG_UNKNOWN)
{
	old_x_ = x();
	old_y_ = y();
	old_z_ = z();
}

//-------------------------------------------------------------------------------------
RangeNode::~RangeNode()
{
}

//-------------------------------------------------------------------------------------
void RangeNode::update()
{
	if(pRangeList_)
		pRangeList_->update(this);
}

//-------------------------------------------------------------------------------------
void RangeNode::c_str()
{
	DEBUG_MSG(boost::format("RangeNode::c_str(): %1% curr(%2%, %3%, %4%), old(%5%, %6%, %7%) pPreX=%8% pNextX=%9% pPreZ=%10% pNextZ=%11% descr=%12%\n") % 
		this % x() % y() % z() %
		old_x_ % old_y_ % old_z_ %
		pPrevX_ % pNextX_ % pPrevZ_ % pNextZ_ % descr());
}

//-------------------------------------------------------------------------------------
void RangeNode::debugX()
{
	c_str();

	if(pNextX_)
		this->pNextX_->debugX();
}

//-------------------------------------------------------------------------------------
void RangeNode::debugY()
{
	c_str();

	if(pNextY_)
		this->pNextY_->debugY();
}

//-------------------------------------------------------------------------------------
void RangeNode::debugZ()
{
	c_str();

	if(pNextZ_)
		this->pNextZ_->debugZ();
}

//-------------------------------------------------------------------------------------
void RangeNode::onNodePassX(RangeNode* pNode, bool isfront)
{
}

//-------------------------------------------------------------------------------------
void RangeNode::onNodePassY(RangeNode* pNode, bool isfront)
{
}

//-------------------------------------------------------------------------------------
void RangeNode::onNodePassZ(RangeNode* pNode, bool isfront)
{
}

//-------------------------------------------------------------------------------------
void RangeNode::onRemove()
{
	x_ = -FLT_MAX;
}

//-------------------------------------------------------------------------------------
}
