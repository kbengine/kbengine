// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "controller.h"	
#include "controllers.h"	
#include "common/memorystream.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Controller::Controller(Controller::ControllerType type, Entity* pEntity, int32 userarg, uint32 id):
id_(id),
pEntity_(pEntity),
userarg_(userarg),
pControllers_(0),
type_(type)
{
}

//-------------------------------------------------------------------------------------
Controller::Controller(Entity* pEntity):
id_(0),
pEntity_(pEntity),
userarg_(0),
pControllers_(0),
type_(CONTROLLER_TYPE_NORMAL)
{
}

//-------------------------------------------------------------------------------------
Controller::~Controller()
{
	id_ = 0;
	pControllers_ = NULL;
}

//-------------------------------------------------------------------------------------
void Controller::destroy()
{
	if(pControllers_ && !pControllers_->remove(this->id_))
	{
		ERROR_MSG(fmt::format("Controller::destroy(): not found {}.\n",
			id_));
	}

	pControllers_ = NULL;
}

//-------------------------------------------------------------------------------------
void Controller::addToStream(KBEngine::MemoryStream& s)
{
	s << id_ << userarg_;
}

//-------------------------------------------------------------------------------------
void Controller::createFromStream(KBEngine::MemoryStream& s)
{
	s >> id_ >> userarg_;
}

//-------------------------------------------------------------------------------------
}
