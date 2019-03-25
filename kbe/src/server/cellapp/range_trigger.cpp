// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "range_trigger.h"
#include "coordinate_system.h"
#include "entity_coordinate_node.h"
#include "range_trigger_node.h"

#ifndef CODE_INLINE
#include "range_trigger.inl"
#endif

namespace KBEngine{	

//-------------------------------------------------------------------------------------
RangeTrigger::RangeTrigger(CoordinateNode* origin, float xz, float y):
range_xz_(fabs(xz)),
range_y_(fabs(y)),
origin_(origin),
positiveBoundary_(NULL),
negativeBoundary_(NULL),
removing_(false)
{
}

//-------------------------------------------------------------------------------------
RangeTrigger::~RangeTrigger()
{
	uninstall();
}

//-------------------------------------------------------------------------------------
bool RangeTrigger::reinstall(CoordinateNode* pCoordinateNode)
{
	uninstall();
	origin_ = pCoordinateNode;
	return install();
}

//-------------------------------------------------------------------------------------
bool RangeTrigger::install()
{
	if(positiveBoundary_ == NULL)
		positiveBoundary_ = new RangeTriggerNode(this, 0, 0, true);
	else
		positiveBoundary_->range(0.0f, 0.0f);

	if(negativeBoundary_ == NULL)
		negativeBoundary_ = new RangeTriggerNode(this, 0, 0, false);
	else
		negativeBoundary_->range(0.0f, 0.0f);

	positiveBoundary_->addFlags(COORDINATE_NODE_FLAG_INSTALLING);
	negativeBoundary_->addFlags(COORDINATE_NODE_FLAG_INSTALLING);

	origin_->pCoordinateSystem()->insert(positiveBoundary_);
	origin_->pCoordinateSystem()->insert(negativeBoundary_);
	
	/*
	注意：此处必须是先安装negativeBoundary_再安装positiveBoundary_，如果调换顺序则会导致View的BUG，例如：在一个实体enterView触发时销毁了进入View的实体
	此时实体销毁时并未触发离开View事件，而未触发View事件导致其他实体的View列表中引用的该销毁的实体是一个无效指针。

	原因如下：
	由于总是优先安装在positiveBoundary_，而边界在安装过程中导致另一个实体进入View了， 然后他在这个过程中可能销毁了， 而另一个边界negativeBoundary_还没有安装， 
	而节点删除时会设置节点的xx为-FLT_MAX，让其向negativeBoundary_方向离开，所以positiveBoundary_不能检查到这个边界也就不会触发View离开事件。
	*/
	negativeBoundary_->old_xx(-FLT_MAX);
	negativeBoundary_->old_yy(-FLT_MAX);
	negativeBoundary_->old_zz(-FLT_MAX);
	negativeBoundary_->range(-range_xz_, -range_y_);
	negativeBoundary_->old_range(-range_xz_, -range_y_);
	negativeBoundary_->update();

	// update可能导致实体销毁间接导致自己被重置，此时应该返回安装失败
	if (!negativeBoundary_)
		return false;

	negativeBoundary_->removeFlags(COORDINATE_NODE_FLAG_INSTALLING);

	positiveBoundary_->old_xx(FLT_MAX);
	positiveBoundary_->old_yy(FLT_MAX);
	positiveBoundary_->old_zz(FLT_MAX);

	positiveBoundary_->range(range_xz_, range_y_);
	positiveBoundary_->old_range(range_xz_, range_y_);
	positiveBoundary_->update();

	if (positiveBoundary_)
	{
		positiveBoundary_->removeFlags(COORDINATE_NODE_FLAG_INSTALLING);
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool RangeTrigger::uninstall()
{
	if (removing_)
		return false;

	removing_ = true;
	if(positiveBoundary_ && positiveBoundary_->pCoordinateSystem())
	{
		positiveBoundary_->pCoordinateSystem()->remove(positiveBoundary_);
		positiveBoundary_->onTriggerUninstall();
	}

	if(negativeBoundary_ && negativeBoundary_->pCoordinateSystem())
	{
		negativeBoundary_->pCoordinateSystem()->remove(negativeBoundary_);
		negativeBoundary_->onTriggerUninstall();
	}
	
	// 此处不必release node， 节点的释放统一交给CoordinateSystem
	positiveBoundary_ = NULL;
	negativeBoundary_ = NULL;
	removing_ = false;
	return true;
}

//-------------------------------------------------------------------------------------
void RangeTrigger::onNodePassX(RangeTriggerNode* pRangeTriggerNode, CoordinateNode* pNode, bool isfront)
{
	if(pNode == origin())
		return;

	bool wasInZ = pRangeTriggerNode->wasInZRange(pNode);
	bool isInZ = pRangeTriggerNode->isInZRange(pNode);

	// 如果Z轴情况有变化，则Z轴再判断，优先级为zyx，这样才可以保证只有一次enter或者leave
	if(wasInZ != isInZ)
		return;

	bool wasIn = false;
	bool isIn = false;

	// 必须同时检查其他轴， 如果节点x轴在范围内，理论上其他轴也在范围内
	if(CoordinateSystem::hasY)
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

	// 如果情况没有发生变化则忽略
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
void RangeTrigger::onNodePassY(RangeTriggerNode* pRangeTriggerNode, CoordinateNode* pNode, bool isfront)
{
	if(pNode == origin() || !CoordinateSystem::hasY)
		return;

	bool wasInZ = pRangeTriggerNode->wasInZRange(pNode);
	bool isInZ = pRangeTriggerNode->isInZRange(pNode);

	// 如果Z轴情况有变化，则Z轴再判断，优先级为zyx，这样才可以保证只有一次enter或者leave
	if(wasInZ != isInZ)
		return;

	bool wasInY = pRangeTriggerNode->wasInYRange(pNode);
	bool isInY = pRangeTriggerNode->isInYRange(pNode);

	if(wasInY == isInY)
		return;

	// 必须同时检查其他轴， 如果节点x轴在范围内，理论上其他轴也在范围内
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
void RangeTrigger::onNodePassZ(RangeTriggerNode* pRangeTriggerNode, CoordinateNode* pNode, bool isfront)
{
	if(pNode == origin())
		return;

	if(CoordinateSystem::hasY)
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
void RangeTrigger::update(float xz, float y)
{
	float old_range_xz_ = range_xz_;
	float old_range_y_ = range_y_;

	range(xz, y);

	if (positiveBoundary_)
	{
		positiveBoundary_->range(range_xz_, range_y_);
		positiveBoundary_->old_range(old_range_xz_, old_range_y_);
		positiveBoundary_->update();
	}

	if (negativeBoundary_)
	{
		negativeBoundary_->range(-range_xz_, -range_y_);
		negativeBoundary_->old_range(-old_range_xz_, -old_range_y_);
		negativeBoundary_->update();
	}
}

//-------------------------------------------------------------------------------------
}
