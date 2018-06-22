// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "entity.h"
#include "moveto_entity_handler.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
MoveToEntityHandler::MoveToEntityHandler(KBEShared_ptr<Controller>& pController, ENTITY_ID pTargetID, float velocity, float range, bool faceMovement, 
		bool moveVertically, PyObject* userarg, const Position3D& offsetPos):
MoveToPointHandler(pController, pController->pEntity()->layer(), pController->pEntity()->position(), velocity, range, faceMovement, moveVertically, userarg),
pTargetID_(pTargetID), 
offsetPos_(offsetPos)
{
	updatableName = "MoveToEntityHandler";
}

//-------------------------------------------------------------------------------------
MoveToEntityHandler::MoveToEntityHandler():
MoveToPointHandler(),
pTargetID_(0)
{
	updatableName = "MoveToEntityHandler";
}

//-------------------------------------------------------------------------------------
MoveToEntityHandler::~MoveToEntityHandler()
{
}

//-------------------------------------------------------------------------------------
void MoveToEntityHandler::addToStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::addToStream(s);
	s << pTargetID_;
}

//-------------------------------------------------------------------------------------
void MoveToEntityHandler::createFromStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::createFromStream(s);
	s >> pTargetID_;
}

//-------------------------------------------------------------------------------------
const Position3D& MoveToEntityHandler::destPos()
{
	Entity* pEntity = Cellapp::getSingleton().findEntity(pTargetID_);
	destPos_ = pEntity->position() + offsetPos_;
	return destPos_;
}

//-------------------------------------------------------------------------------------
bool MoveToEntityHandler::update()
{
	if (isDestroyed_)
	{
		delete this;
		return false;
	}

	Entity* pEntity = Cellapp::getSingleton().findEntity(pTargetID_);
	if(pEntity == NULL)
	{
		if(pController_ && pController_->pEntity())
			pController_->pEntity()->onMoveFailure(pController_->id(), pyuserarg_);
		
		if(pController_)
			pController_->destroy();
		
		pController_.reset();
	}

	return MoveToPointHandler::update();
}

//-------------------------------------------------------------------------------------
}

