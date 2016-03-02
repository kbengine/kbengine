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
INLINE float Witness::aoiRadius() const
{ 
	return aoiRadius_; 
}

//-------------------------------------------------------------------------------------
INLINE float Witness::aoiHysteresisArea() const
{ 
	return aoiHysteresisArea_; 
}

//-------------------------------------------------------------------------------------
INLINE EntityRef* Witness::getAOIEntityRef(ENTITY_ID entityID)
{
	EntityRef::AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), 
		findif_vector_entityref_exist_by_entityid_handler(entityID));

	if(iter != aoiEntities_.end())
	{
		return (*iter);
	}
	
	return NULL;
}

//-------------------------------------------------------------------------------------
INLINE bool Witness::entityInAOI(ENTITY_ID entityID)
{
	EntityRef* pEntityRef = getAOIEntityRef(entityID);

	if(pEntityRef == NULL || pEntityRef->pEntity() == NULL || pEntityRef->flags() == ENTITYREF_FLAG_UNKONWN || 
		(pEntityRef->flags() & (ENTITYREF_FLAG_ENTER_CLIENT_PENDING | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING)) > 0)
		return false;
		
	return true;
}

//-------------------------------------------------------------------------------------
INLINE AOITrigger* Witness::pAOITrigger()
{
	return pAOITrigger_;
}

//-------------------------------------------------------------------------------------
INLINE AOITrigger* Witness::pAOIHysteresisAreaTrigger()
{
	return pAOIHysteresisAreaTrigger_;
}

//-------------------------------------------------------------------------------------
INLINE EntityRef::AOI_ENTITIES& Witness::aoiEntities()
{ 
	return aoiEntities_; 
}

//-------------------------------------------------------------------------------------
}
