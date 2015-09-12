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

#include "cellapp.h"
#include "entity.h"
#include "navigate_handler.h"	
#include "navigation/navigation.h"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
NavigateHandler::NavigateHandler(Controller* pController, const Position3D& destPos, 
											 float velocity, float distance, bool faceMovement, 
											 float maxMoveDistance, float maxDistance, float girth,
											PyObject* userarg):
MoveToPointHandler(pController, pController->pEntity()->layer(), pController->pEntity()->position(), velocity, distance, faceMovement, true, userarg),
destPosIdx_(0),
paths_(),
pNavHandle_(NULL),
maxMoveDistance_(maxMoveDistance),
maxDistance_(maxDistance)
{
	Entity* pEntity = pController->pEntity();
	if(pNavHandle_ == NULL)
	{
		Space* pSpace = Spaces::findSpace(pEntity->spaceID());
		if(pSpace == NULL)
		{
			ERROR_MSG(fmt::format("NavigateHandler::NavigateHandler(): not found space({}), entityID({})!\n",
				pEntity->spaceID(), pEntity->id()));

			pController_ = NULL;
		}
		else
		{
			pNavHandle_ = pSpace->pNavHandle();

			if(pNavHandle_)
			{
				const Position3D& currpos = pEntity->position();
				pNavHandle_->findStraightPath(pController->pEntity()->layer(), currpos, destPos, paths_);

				if(paths_.size() == 0)
				{
					pController_ = NULL;
				}
				else
				{
					bool canMove = false;
					std::vector<Position3D>::iterator iter = paths_.begin();
					for(; iter != paths_.end(); iter++)
					{
						destPos_ = paths_[destPosIdx_++];

						Vector3 movement = destPos_ - currpos;
						if(KBEVec3Length(&movement) > 0.00001f)
						{
							canMove = true;
							break;
						}
					}

					if(!canMove)
					{
						pController_ = NULL;
					}
				}
			}
			else
			{
				pController_ = NULL;

				WARNING_MSG(fmt::format("NavigateHandler::NavigateHandler(): space({}), entityID({}), not found navhandle!\n",
					pEntity->spaceID(), pEntity->id()));
			}
		}
	}
}

//-------------------------------------------------------------------------------------
NavigateHandler::NavigateHandler():
MoveToPointHandler(),
destPosIdx_(0),
paths_(),
pNavHandle_(NULL),
maxMoveDistance_(0.f),
maxDistance_(0.f)
{
}

//-------------------------------------------------------------------------------------
NavigateHandler::~NavigateHandler()
{
}

//-------------------------------------------------------------------------------------
void NavigateHandler::addToStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::addToStream(s);
	s << maxMoveDistance_ << maxDistance_;
}

//-------------------------------------------------------------------------------------
void NavigateHandler::createFromStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::createFromStream(s);
	s >> maxMoveDistance_ >> maxDistance_;
}

//-------------------------------------------------------------------------------------
bool NavigateHandler::requestMoveOver(const Position3D& oldPos)
{
	if(destPosIdx_ == ((int)paths_.size()))
		return MoveToPointHandler::requestMoveOver(oldPos);
	else
		destPos_ = paths_[destPosIdx_++];

	return false;
}

//-------------------------------------------------------------------------------------
}

