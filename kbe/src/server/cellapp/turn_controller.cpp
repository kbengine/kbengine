// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "entity.h"
#include "turn_controller.h"		

namespace KBEngine{	


//-------------------------------------------------------------------------------------
TurnController::TurnController(Entity* pEntity, RotatorHandler* pRotatorHandler, uint32 id) :
Controller(Controller::CONTROLLER_TYPE_ROTATE, pEntity, 0, id),
pRotatorHandler_(pRotatorHandler)
{
}

//-------------------------------------------------------------------------------------
TurnController::~TurnController()
{
	// DEBUG_MSG(fmt::format("TurnController::~TurnController(): {:p}\n", (void*)this);
	if (pRotatorHandler_)
		pRotatorHandler_->pController(KBEShared_ptr<Controller>());

	pRotatorHandler_ = NULL;
}

//-------------------------------------------------------------------------------------
void TurnController::addToStream(KBEngine::MemoryStream& s)
{
	Controller::addToStream(s);
	pRotatorHandler_->addToStream(s);
}

//-------------------------------------------------------------------------------------
void TurnController::createFromStream(KBEngine::MemoryStream& s)
{
	Controller::createFromStream(s);
	KBE_ASSERT(pRotatorHandler_ == NULL);
	
	pRotatorHandler_ = new RotatorHandler();
	pRotatorHandler_->createFromStream(s);
}

//-------------------------------------------------------------------------------------
void TurnController::destroy()
{
	Controller::destroy();

	// 既然自己要销毁了，那么与自己相联的updatable也应该停止了
	if (pRotatorHandler_)
	{
		pRotatorHandler_->pController(KBEShared_ptr<Controller>());
		pRotatorHandler_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
}

