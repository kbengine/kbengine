/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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
INLINE std::map<ENTITY_ID, Entity*>& Entity::getViewEntities(void)
{ 
	return viewEntities_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setClientMailbox(EntityMailbox* mailbox)
{
	clientMailbox_ = mailbox; 
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
INLINE bool Entity::isReal(void)const
{ 
	return isReal_; 
}

//-------------------------------------------------------------------------------------
}
