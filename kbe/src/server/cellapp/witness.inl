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


namespace KBEngine{

//-------------------------------------------------------------------------------------
INLINE Entity* Witness::pEntity()
{
	return pEntity_;
}

//-------------------------------------------------------------------------------------
INLINE void Witness::pEntity(Entity* pEntity)
{
	pEntity_ = pEntity;
}

//-------------------------------------------------------------------------------------
INLINE float Witness::viewRadius() const
{ 
	return viewRadius_; 
}

//-------------------------------------------------------------------------------------
INLINE float Witness::viewHysteresisArea() const
{ 
	return viewHysteresisArea_; 
}

//-------------------------------------------------------------------------------------
INLINE EntityRef* Witness::getViewEntityRef(ENTITY_ID entityID)
{
	VIEW_ENTITIES_MAP::iterator iter = viewEntities_map_.find(entityID);
	if(iter != viewEntities_map_.end())
	{
		return iter->second;
	}
	
	return NULL;
}

//-------------------------------------------------------------------------------------
INLINE bool Witness::entityInView(ENTITY_ID entityID)
{
	EntityRef* pEntityRef = getViewEntityRef(entityID);

	if(pEntityRef == NULL || pEntityRef->pEntity() == NULL || pEntityRef->flags() == ENTITYREF_FLAG_UNKONWN || 
		(pEntityRef->flags() & (ENTITYREF_FLAG_ENTER_CLIENT_PENDING | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING)) > 0)
		return false;
		
	return true;
}

//-------------------------------------------------------------------------------------
INLINE ViewTrigger* Witness::pViewTrigger()
{
	return pViewTrigger_;
}

//-------------------------------------------------------------------------------------
INLINE ViewTrigger* Witness::pViewHysteresisAreaTrigger()
{
	return pViewHysteresisAreaTrigger_;
}

//-------------------------------------------------------------------------------------
INLINE Witness::VIEW_ENTITIES_MAP& Witness::viewEntitiesMap()
{ 
	return viewEntities_map_; 
}

//-------------------------------------------------------------------------------------
INLINE Witness::VIEW_ENTITIES& Witness::viewEntities()
{
	return viewEntities_;
}

//-------------------------------------------------------------------------------------
}
