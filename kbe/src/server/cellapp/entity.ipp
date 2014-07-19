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
INLINE void Entity::setClientMailbox(EntityMailbox* mailbox)
{
	clientMailbox_ = mailbox; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isWitnessed(void)const
{ 
	return witnesses_.size() > 0; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::hasWitness(void)const
{ 
	return pWitness_ != NULL &&  clientMailbox_ != NULL; 
}

INLINE const std::list<ENTITY_ID>&	Entity::witnesses()
{
	return witnesses_;
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
INLINE void Entity::setPosition(const Position3D& pos)
{ 
	Vector3 movement = pos - position_;

	if(KBEVec3Length(&movement) < 0.0004f)
		return;
		
	position_ = pos; 
	onPositionChanged();
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setDirection(const Direction3D& dir)
{
	if(almostEqual(direction_.yaw(), dir.yaw()) && almostEqual(direction_.roll(), dir.roll()) && almostEqual(direction_.pitch(), dir.pitch()))
		return;

	direction_ = dir; 
	onDirectionChanged();
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
INLINE AllClients* Entity::getAllClients()const
{ 
	return allClients_; 
}

//-------------------------------------------------------------------------------------
INLINE AllClients* Entity::getOtherClients()const
{ 
	return otherClients_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setAllClients(AllClients* clients)
{
	allClients_ = clients;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setOtherClients(AllClients* clients)
{
	otherClients_ = clients;
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isReal(void)const
{ 
	return isReal_; 
}

//-------------------------------------------------------------------------------------
INLINE SPACE_ENTITIES::size_type Entity::spaceEntityIdx()const
{
	return spaceEntityIdx_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::spaceEntityIdx(SPACE_ENTITIES::size_type idx)
{
	spaceEntityIdx_ = idx;
}

//-------------------------------------------------------------------------------------
INLINE Witness* Entity::pWitness()const
{
	return pWitness_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::pWitness(Witness* w)
{
	pWitness_ = w;
}

//-------------------------------------------------------------------------------------
INLINE EntityCoordinateNode* Entity::pEntityCoordinateNode()const
{
	return pEntityCoordinateNode_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::pEntityCoordinateNode(EntityCoordinateNode* pNode)
{
	pEntityCoordinateNode_ = pNode;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::isOnGround(bool v)
{
	isOnGround_ = v;
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isOnGround()const
{
	return isOnGround_;
}

//-------------------------------------------------------------------------------------
INLINE int8 Entity::shouldAutoBackup()const
{
	return shouldAutoBackup_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::shouldAutoBackup(int8 v)
{
	shouldAutoBackup_ = v;
}

//-------------------------------------------------------------------------------------
INLINE GAME_TIME Entity::posChangedTime()const
{
	return posChangedTime_;
}

//-------------------------------------------------------------------------------------
INLINE GAME_TIME Entity::dirChangedTime()const
{
	return dirChangedTime_;
}

//-------------------------------------------------------------------------------------
INLINE int8 Entity::layer()const
{
	return layer_;
}

//-------------------------------------------------------------------------------------
}
