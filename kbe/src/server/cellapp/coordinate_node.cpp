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

#include "coordinate_node.h"
#include "coordinate_system.h"

#ifndef CODE_INLINE
#include "coordinate_node.inl"
#endif

namespace KBEngine{	

//-------------------------------------------------------------------------------------
CoordinateNode::CoordinateNode(CoordinateSystem* pCoordinateSystem):
pPrevX_(NULL),
pNextX_(NULL),
pPrevY_(NULL),
pNextY_(NULL),
pPrevZ_(NULL),
pNextZ_(NULL),
pCoordinateSystem_(pCoordinateSystem),
x_(-FLT_MAX),
y_(-FLT_MAX),
z_(-FLT_MAX),
old_xx_(-FLT_MAX),
old_yy_(-FLT_MAX),
old_zz_(-FLT_MAX),
#ifdef _DEBUG
descr_(),
#endif
flags_(COORDINATE_NODE_FLAG_UNKNOWN)
{
}

//-------------------------------------------------------------------------------------
CoordinateNode::~CoordinateNode()
{
}

//-------------------------------------------------------------------------------------
void CoordinateNode::update()
{
	if(pCoordinateSystem_)
		pCoordinateSystem_->update(this);
}

//-------------------------------------------------------------------------------------
void CoordinateNode::c_str()
{
	DEBUG_MSG(fmt::format("CoordinateNode::c_str(): {:p} curr({}, {}, {}), old({}, {}, {}) pPreX={:p} pNextX={:p} pPreZ={:p} pNextZ={:p} descr={}\n", 
		(void*)this, x(), y(), z(),
		old_xx_, old_yy_, old_zz_,
		(void*)pPrevX_, (void*)pNextX_, (void*)pPrevZ_, (void*)pNextZ_, descr()));
}

//-------------------------------------------------------------------------------------
void CoordinateNode::debugX()
{
	c_str();

	if(pNextX_)
	{
		this->pNextX_->debugX();

		if(this->pNextX_->x() < x())
		{
			ERROR_MSG(fmt::format("CoordinateNode::debugX():: {:p} > {:p}\n", (void*)this, (void*)pNextX_));
		}
	}
}

//-------------------------------------------------------------------------------------
void CoordinateNode::debugY()
{
	c_str();

	if(pNextY_)
	{
		this->pNextY_->debugY();

		if(this->pNextY_->y() < y())
		{
			ERROR_MSG(fmt::format("CoordinateNode::debugY():: {:p} > {:p}\n", (void*)this, (void*)pNextY_));
		}
	}
}

//-------------------------------------------------------------------------------------
void CoordinateNode::debugZ()
{
	c_str();

	if(pNextZ_)
	{
		this->pNextZ_->debugZ();

		if(this->pNextZ_->z() < z())
		{
			ERROR_MSG(fmt::format("CoordinateNode::debugZ():: {:p} > {:p}\n", (void*)this, (void*)pNextZ_));
		}
	}
}

//-------------------------------------------------------------------------------------
void CoordinateNode::onNodePassX(CoordinateNode* pNode, bool isfront)
{
}

//-------------------------------------------------------------------------------------
void CoordinateNode::onNodePassY(CoordinateNode* pNode, bool isfront)
{
}

//-------------------------------------------------------------------------------------
void CoordinateNode::onNodePassZ(CoordinateNode* pNode, bool isfront)
{
}

//-------------------------------------------------------------------------------------
void CoordinateNode::onRemove()
{
	old_xx(x_);
	old_yy(y_);
	old_zz(z_);

	x_ = -FLT_MAX;
}

//-------------------------------------------------------------------------------------
}
