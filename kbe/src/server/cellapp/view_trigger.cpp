// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "witness.h"
#include "entity.h"
#include "view_trigger.h"
#include "entity_coordinate_node.h"

#ifndef CODE_INLINE
#include "view_trigger.inl"
#endif

namespace KBEngine{	


//-------------------------------------------------------------------------------------
ViewTrigger::ViewTrigger(CoordinateNode* origin, float xz, float y):
RangeTrigger(origin, xz, y),
pWitness_(static_cast<EntityCoordinateNode*>(origin)->pEntity()->pWitness())
{
}

//-------------------------------------------------------------------------------------
ViewTrigger::~ViewTrigger()
{
}

//-------------------------------------------------------------------------------------
void ViewTrigger::onEnter(CoordinateNode * pNode)
{
	if((pNode->flags() & COORDINATE_NODE_FLAG_ENTITY) <= 0)
		return;

	EntityCoordinateNode* pEntityCoordinateNode = static_cast<EntityCoordinateNode*>(pNode);
	Entity* pEntity = pEntityCoordinateNode->pEntity();
	if(!pEntity->pScriptModule()->hasClient())
		return;

	pWitness_->onEnterView(this, pEntity);
}

//-------------------------------------------------------------------------------------
void ViewTrigger::onLeave(CoordinateNode * pNode)
{
	if((pNode->flags() & COORDINATE_NODE_FLAG_ENTITY) <= 0)
		return;

	EntityCoordinateNode* pEntityCoordinateNode = static_cast<EntityCoordinateNode*>(pNode);
	Entity* pEntity = pEntityCoordinateNode->pEntity();
	if(!pEntity->pScriptModule()->hasClient())
		return;

	pWitness_->onLeaveView(this, pEntity);
}

//-------------------------------------------------------------------------------------
}
