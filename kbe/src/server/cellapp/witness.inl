// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
