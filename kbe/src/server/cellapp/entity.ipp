
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
}
