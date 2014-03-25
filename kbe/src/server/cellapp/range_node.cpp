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
x_(-FLT_MAX),
y_(-FLT_MAX),
z_(-FLT_MAX),
old_xx_(-FLT_MAX),
old_yy_(-FLT_MAX),
old_zz_(-FLT_MAX),
#ifdef _DEBUG
descr_(),
#endif
flags_(RANGENODE_FLAG_UNKNOWN)
{
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
		old_xx_ % old_yy_ % old_zz_ %
		pPrevX_ % pNextX_ % pPrevZ_ % pNextZ_ % descr());
}

//-------------------------------------------------------------------------------------
void RangeNode::debugX()
{
	c_str();

	if(pNextX_)
	{
		this->pNextX_->debugX();

		if(this->pNextX_->x() < x())
		{
			ERROR_MSG(boost::format("RangeNode::debugX():: %1% > %2%\n") % this % pNextX_);
		}
	}
}

//-------------------------------------------------------------------------------------
void RangeNode::debugY()
{
	c_str();

	if(pNextY_)
	{
		this->pNextY_->debugY();

		if(this->pNextY_->y() < y())
		{
			ERROR_MSG(boost::format("RangeNode::debugY():: %1% > %2%\n") % this % pNextY_);
		}
	}
}

//-------------------------------------------------------------------------------------
void RangeNode::debugZ()
{
	c_str();

	if(pNextZ_)
	{
		this->pNextZ_->debugZ();

		if(this->pNextZ_->z() < z())
		{
			ERROR_MSG(boost::format("RangeNode::debugZ():: %1% > %2%\n") % this % pNextZ_);
		}
	}
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
