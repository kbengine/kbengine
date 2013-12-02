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
											 float maxMoveDistance, float maxDistance, float girth,
											PyObject* userarg, uint32 id):
MoveToPointController(pEntity, pEntity->getPosition(), velocity, range, faceMovement, true, userarg, id),
destPosIdx_(0),
paths_(),
pNavMeshHandle_(NULL),
maxMoveDistance_(maxMoveDistance),
maxDistance_(maxDistance),
girth_(girth)
{
	if(pNavMeshHandle_ == NULL)
	{
		Space* pSpace = Spaces::findSpace(pEntity_->getSpaceID());
		if(pSpace == NULL)
		{
			ERROR_MSG(boost::format("NavigateController::NavigateController(): not found space(%1%), entityID(%2%)!\n") % 
				pEntity_->getSpaceID() % pEntity_->getID());

			destroyed_ = true;
		}
		else
		{
			pNavMeshHandle_ = pSpace->pNavMeshHandle();
			Position3D currpos = pEntity_->getPosition();
			pNavMeshHandle_->findStraightPath(currpos, destPos, paths_);

			if(paths_.size() == 0)
				destroyed_ = true;
			else
				destPos_ = paths_[destPosIdx_++];
		}
	}
}

//-------------------------------------------------------------------------------------
NavigateController::~NavigateController()
{
}

//-------------------------------------------------------------------------------------
bool NavigateController::requestMoveOver()
{
	if(destPosIdx_ == ((int)paths_.size()))
		return MoveToPointController::requestMoveOver();
	else
		destPos_ = paths_[destPosIdx_++];

	return false;
}

//-------------------------------------------------------------------------------------
}

