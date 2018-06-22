// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
weight_(0),
#ifdef _DEBUG
descr_(),
#endif
flags_(COORDINATE_NODE_FLAG_UNKNOWN)
{
}

//-------------------------------------------------------------------------------------
CoordinateNode::~CoordinateNode()
{
	//DEBUG_MSG(fmt::format("CoordinateNode::~CoordinateNode(), addr = {}, desc = {}\n", (void*)this, descr_));
	KBE_ASSERT(pPrevX_ == NULL &&
			   pNextX_ == NULL &&
			   pPrevY_ == NULL &&
			   pNextY_ == NULL &&
			   pPrevZ_ == NULL &&
			   pNextZ_ == NULL &&
			   pCoordinateSystem_ == NULL);
}

//-------------------------------------------------------------------------------------
void CoordinateNode::update()
{
	if(pCoordinateSystem_)
		pCoordinateSystem_->update(this);
}

//-------------------------------------------------------------------------------------
std::string CoordinateNode::c_str()
{
	return fmt::format("CoordinateNode::c_str(): {:p} curr({}, {}, {}), {}, pPreX={:p} pNextX={:p} pPreZ={:p} pNextZ={:p} flags={} descr={}\n",
		(void*)this, x(), y(), z(),
		fmt::format("xxyyzz({}, {}, {}), old_xxyyzz({}, {}, {})",
		xx(), yy(), zz(),
		old_xx(), old_yy(), old_zz()),
		(void*)pPrevX_, (void*)pNextX_, (void*)pPrevZ_, (void*)pNextZ_, flags_, descr());
}

//-------------------------------------------------------------------------------------
void CoordinateNode::debugX()
{
	DEBUG_MSG(c_str());

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
	DEBUG_MSG(c_str());

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
	DEBUG_MSG(c_str());

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
	//y_ = -FLT_MAX;
	//z_ = -FLT_MAX;
}

//-------------------------------------------------------------------------------------
}
