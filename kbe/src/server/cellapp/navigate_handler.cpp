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
#include "navigate_handler.hpp"	
#include "navigation/navmeshex.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
NavigateHandler::NavigateHandler(Controller* pController, const Position3D& destPos, 
											 float velocity, float range, bool faceMovement, 
											 float maxMoveDistance, float maxDistance, float girth,
											PyObject* userarg):
MoveToPointHandler(pController, pController->pEntity()->getPosition(), velocity, range, faceMovement, true, userarg),
destPosIdx_(0),
paths_(),
pNavMeshHandle_(NULL),
maxMoveDistance_(maxMoveDistance),
maxDistance_(maxDistance),
girth_(girth)
{
	Entity* pEntity = pController->pEntity();
	if(pNavMeshHandle_ == NULL)
	{
		Space* pSpace = Spaces::findSpace(pEntity->getSpaceID());
		if(pSpace == NULL)
		{
			ERROR_MSG(boost::format("NavigateHandler::NavigateHandler(): not found space(%1%), entityID(%2%)!\n") % 
				pEntity->getSpaceID() % pEntity->getID());

			pController_ = NULL;
		}
		else
		{
			pNavMeshHandle_ = pSpace->pNavMeshHandle();

			if(pNavMeshHandle_)
			{
				Position3D currpos = pEntity->getPosition();
				pNavMeshHandle_->findStraightPath(currpos, destPos, paths_);

				if(paths_.size() == 0)
					pController_ = NULL;
				else
					destPos_ = paths_[destPosIdx_++];
			}
			else
			{
				pController_ = NULL;

				WARNING_MSG(boost::format("NavigateHandler::NavigateHandler(): space(%1%), entityID(%2%), not found navmesh!\n") % 
					pEntity->getSpaceID() % pEntity->getID());
			}
		}
	}
}

//-------------------------------------------------------------------------------------
NavigateHandler::~NavigateHandler()
{
}

//-------------------------------------------------------------------------------------
bool NavigateHandler::requestMoveOver()
{
	if(destPosIdx_ == ((int)paths_.size()))
		return MoveToPointHandler::requestMoveOver();
	else
		destPos_ = paths_[destPosIdx_++];

	return false;
}

//-------------------------------------------------------------------------------------
}

