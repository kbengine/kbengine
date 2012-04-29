
namespace KBEngine{

//-------------------------------------------------------------------------------------
INLINE std::map<ENTITY_ID, Entity*>& Entity::getViewEntities(void)
{ 
	return viewEntities_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setClientMailbox(EntityMailbox* mailbox)
{
	clientMailbox_ = mailbox; 
	if(clientMailbox_!= NULL) 
		onGetWitness(); 
	else 
		onLoseWitness(); 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isWitnessed(void)const
{ 
	return isWitnessed_; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::hasWitness(void)const
{ 
	return hasWitness_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setTopSpeedY(float speed)
{ 
	topSpeedY_ = speed; 
}

//-------------------------------------------------------------------------------------
INLINE float Entity::getTopSpeedY()const
{ 
	return topSpeedY_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setTopSpeed(float speed)
{ 
	topSpeed_ = speed; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isDestroyed()
{ 
	return isDestroyed_; 
}

//-------------------------------------------------------------------------------------
INLINE float Entity::getAoiRadius(void)const
{ 
	return aoiRadius_; 
}

//-------------------------------------------------------------------------------------
INLINE float Entity::getAoiHystArea(void)const
{ 
	return aoiHysteresisArea_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setDirection(Direction3D& dir)
{ 
	direction_ = dir; 
}

//-------------------------------------------------------------------------------------
INLINE Direction3D& Entity::getDirection()
{ 
	return direction_; 
}

//-------------------------------------------------------------------------------------
INLINE Position3D& Entity::getPosition()
{
	return position_; 
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Entity::getBaseMailbox()const
{ 
	return baseMailbox_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setBaseMailbox(EntityMailbox* mailbox)
{ 
	baseMailbox_ = mailbox; 
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Entity::getClientMailbox()const
{ 
	return clientMailbox_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setSpaceID(int id)
{ 
	spaceID_ = id; 
}

//-------------------------------------------------------------------------------------
INLINE uint32 Entity::getSpaceID()const
{ 
	return spaceID_; 
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_ID Entity::getID()const
{ 
	return id_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setID(const int& id)
{ 
	id_ = id; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::pChannel(Mercury::Channel* pchannel)
{ 
	pChannel_ = pchannel; 
}

//-------------------------------------------------------------------------------------
INLINE Mercury::Channel* Entity::pChannel(void)const 
{ 
	return pChannel_; 
}

//-------------------------------------------------------------------------------------
INLINE const char* Entity::getScriptModuleName(void)const
{ 
	return scriptModule_->getScriptType()->tp_name; 
}	

//-------------------------------------------------------------------------------------
INLINE ScriptModule* Entity::getScriptModule(void)const
{ 
	return scriptModule_; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isReal(void)const
{ 
	return isReal_; 
}

//-------------------------------------------------------------------------------------
}
