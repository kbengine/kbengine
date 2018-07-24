// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "controllers.h"	
#include "cellapp.h"	
#include "entity.h"	
#include "helper/profile.h"	
#include "common/memorystream.h"	

#include "proximity_controller.h"	
#include "moveto_point_handler.h"	
#include "moveto_entity_handler.h"	
#include "navigate_handler.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Controllers::Controllers(ENTITY_ID entityID):
lastid_(0),
entityID_(entityID)
{
}

//-------------------------------------------------------------------------------------
Controllers::~Controllers()
{
	clear();
}

//-------------------------------------------------------------------------------------
void Controllers::clear()
{
	objects_.clear();
	lastid_ = 0;
}

//-------------------------------------------------------------------------------------
bool Controllers::add(KBEShared_ptr<Controller> pController)
{
	uint32 id = pController->id();
	if(id == 0)
	{
		id = freeID();
	}
	else
	{
		// 刷新id计数器
		if(lastid_ < id)
			lastid_ = id;
	}

	objects_[id] = pController;
	pController->id(id);
	pController->pControllers(this);

	if(objects_.size() > 32)
	{
		WARNING_MSG(fmt::format("Controllers::add: size = {}.\n", objects_.size()));
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Controllers::remove(KBEShared_ptr<Controller> pController)
{
	return remove(pController->id());
}

//-------------------------------------------------------------------------------------
bool Controllers::remove(uint32 id)
{
	CONTROLLERS_MAP::iterator iter = objects_.find(id);
	if (iter == objects_.end())
		return true;

	// 做个引用，防止在Controller析构中导致某些情况下在erase未结束的情况下又进入这里执行erase而产生问题
	KBEShared_ptr< Controller > pController = iter->second;
	objects_.erase(iter);
	return pController != NULL;
}

//-------------------------------------------------------------------------------------
void Controllers::addToStream(KBEngine::MemoryStream& s)
{
	uint32 size = (uint32)objects_.size();
	s << lastid_ << size;

	CONTROLLERS_MAP::iterator iter = objects_.begin();
	for(; iter != objects_.end(); ++iter)
	{
		uint8 itype = (uint8)iter->second->type();
		s << itype;
		iter->second->addToStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Controllers::createFromStream(KBEngine::MemoryStream& s)
{
	uint32 size = 0;
	s >> lastid_ >> size;

	Entity* pEntity = Cellapp::getSingleton().findEntity(entityID_);
	KBE_ASSERT(pEntity);

	for(uint32 i=0; i<size; ++i)
	{
		uint8 itype;
		s >> itype;

		Controller::ControllerType type = (Controller::ControllerType)itype;
		
		KBEShared_ptr<Controller> pController;

		switch(type)
		{
		case Controller::CONTROLLER_TYPE_PROXIMITY:
			pController = KBEShared_ptr<Controller>(new ProximityController(pEntity));
			break;
		case Controller::CONTROLLER_TYPE_ROTATE:
		case Controller::CONTROLLER_TYPE_MOVE:
		default:
			KBE_ASSERT(false);
			break;
		};
		
		if(pController == NULL)
			continue;
		
		pController->type(type);
		pController->createFromStream(s);

		add(pController);
	}
}

//-------------------------------------------------------------------------------------
}
