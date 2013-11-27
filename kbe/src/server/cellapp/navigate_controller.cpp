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

#include "cellapp.hpp"
#include "entity.hpp"
#include "navigate_controller.hpp"	
#include "navigation/navmeshex.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
NavigateController::NavigateController(Entity* pEntity, const Position3D& destPos, 
											 float velocity, float range, bool faceMovement, 
											bool moveVertically, PyObject* userarg, uint32 id):
Controller(pEntity, 0, id),
destPosIdx_(0),
velocity_(velocity),
faceMovement_(faceMovement),
moveVertically_(moveVertically),
pyuserarg_(userarg),
range_(range),
destroyed_(false),
paths_(),
pNavMeshHandle_(NULL)
{
	Cellapp::getSingleton().addUpdatable(this);

	if(pNavMeshHandle_ == NULL)
	{
		Space* pSpace = Spaces::findSpace(pEntity_->getSpaceID());
		if(pSpace == NULL)
		{
			ERROR_MSG(boost::format("NavigateController::NavigateController(): not found space(%1%), entityID(%2%)!\n") % 
				pEntity_->getID() % pEntity_->getSpaceID());

			destroyed_ = true;
		}
		else
		{
			pNavMeshHandle_ = pSpace->pNavMeshHandle();
			Position3D currpos = pEntity_->getPosition();
			pNavMeshHandle_->findStraightPath(currpos, destPos, paths_);

			if(paths_.size() == 0)
				destroyed_ = true;
		}
	}
}

//-------------------------------------------------------------------------------------
NavigateController::~NavigateController()
{
	if(pyuserarg_ != NULL)
	{
		Py_DECREF(pyuserarg_);
	}
}

//-------------------------------------------------------------------------------------
bool NavigateController::update()
{
	if(destroyed_)
	{
		destroy();
		return false;
	}

	const Position3D& dstPos = paths_[destPosIdx_];
	Position3D currpos = pEntity_->getPosition();
	Direction3D direction = pEntity_->getDirection();

	Vector3 movement = dstPos - currpos;
	if (!moveVertically_) movement.y = 0.f;
	
	bool ret = true;

	if(KBEVec3Length(&movement) < velocity_ + range_)
	{
		float y = currpos.y;
		currpos = dstPos;

		if(range_ > 0.0f)
		{
			// 单位化向量
			KBEVec3Normalize(&movement, &movement); 
			movement *= range_;
			currpos -= movement;
		}

		if (!moveVertically_)
			currpos.y = y;

		if(destPosIdx_ == paths_.size() - 1)
			ret = false;
		else
			destPosIdx_++;
	}
	else
	{
		// 单位化向量
		KBEVec3Normalize(&movement, &movement); 

		// 移动位置
		movement *= velocity_;
		currpos += movement;
	}
	
	// 是否需要改变面向
	if (faceMovement_ && (movement.x != 0.f || movement.z != 0.f))
		direction.yaw = movement.yaw();
	
	// 设置entity的新位置和面向
	pEntity_->setPositionAndDirection(currpos, direction);

	pEntity_->isOnGround(true);

	// 通知脚本
	pEntity_->onMove(id(), pyuserarg_);

	// 如果达到目的地则返回true
	if(!ret)
	{
		pEntity_->onMoveOver(id(), pyuserarg_);
		destroy();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
}

