// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "entity.h"
#include "navigate_handler.h"	
#include "navigation/navigation.h"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
NavigateHandler::NavigateHandler(KBEShared_ptr<Controller>& pController, const Position3D& destPos, 
											 float velocity, float distance, bool faceMovement, 
											 float maxMoveDistance, VECTOR_POS3D_PTR paths_ptr,
											PyObject* userarg):
MoveToPointHandler(pController, pController->pEntity()->layer(), pController->pEntity()->position(), velocity, distance, faceMovement, false, userarg),
destPosIdx_(0),
paths_(paths_ptr),
maxMoveDistance_(maxMoveDistance)
{
	destPos_ = (*paths_)[destPosIdx_++];
	
	updatableName = "NavigateHandler";
}

//-------------------------------------------------------------------------------------
NavigateHandler::NavigateHandler():
MoveToPointHandler(),
destPosIdx_(0),
paths_(),
maxMoveDistance_(0.f)
{
	updatableName = "NavigateHandler";
}

//-------------------------------------------------------------------------------------
NavigateHandler::~NavigateHandler()
{
}

//-------------------------------------------------------------------------------------
void NavigateHandler::addToStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::addToStream(s);
	s << maxMoveDistance_;
}

//-------------------------------------------------------------------------------------
void NavigateHandler::createFromStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::createFromStream(s);
	s >> maxMoveDistance_;
}

//-------------------------------------------------------------------------------------
bool NavigateHandler::requestMoveOver(const Position3D& oldPos)
{
	if(destPosIdx_ == ((int)paths_->size()))
		return MoveToPointHandler::requestMoveOver(oldPos);
	else
		destPos_ = (*paths_)[destPosIdx_++];

	return false;
}

//-------------------------------------------------------------------------------------
}

