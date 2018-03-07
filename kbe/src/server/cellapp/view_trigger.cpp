/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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
