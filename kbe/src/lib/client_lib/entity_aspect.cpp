// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "entity_aspect.h"
#include "helper/debug_helper.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
EntityAspect::EntityAspect(const EntityAspect& entityAspect)
{
	aspectID_ = entityAspect.aspectID_;
	modelScale_ = entityAspect.modelScale_;
	modelres_ = entityAspect.modelres_;
}

//-------------------------------------------------------------------------------------
EntityAspect::EntityAspect(ENTITY_ID aspectID):
aspectID_(aspectID),
modelres_(),
modelScale_(1.0f)
{
}

//-------------------------------------------------------------------------------------
EntityAspect::EntityAspect():
aspectID_(0),
modelres_(),
modelScale_(1.0f)
{
}


//-------------------------------------------------------------------------------------
EntityAspect::~EntityAspect()
{
}

//-------------------------------------------------------------------------------------

}
