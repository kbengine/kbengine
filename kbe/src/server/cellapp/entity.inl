/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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
INLINE bool Entity::isWitnessed(void) const
{ 
	return witnesses_count_ > 0; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::hasWitness(void) const
{ 
	return pWitness_ != NULL &&  clientMailbox_ != NULL; 
}

//-------------------------------------------------------------------------------------
INLINE const std::list<ENTITY_ID>&	Entity::witnesses()
{
	return witnesses_;
}

//-------------------------------------------------------------------------------------
INLINE size_t Entity::witnessesSize() const
{
	return witnesses_count_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::topSpeedY(float speed)
{ 
	topSpeedY_ = speed; 
}

//-------------------------------------------------------------------------------------
INLINE float Entity::topSpeedY() const
{ 
	return topSpeedY_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::topSpeed(float speed)
{ 
	topSpeed_ = speed; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::position(const Position3D& pos)
{ 
	Vector3 movement = pos - position_;

	if(KBEVec3Length(&movement) < 0.0004f)
		return;
		
	position_ = pos; 
	onPositionChanged();
}

//-------------------------------------------------------------------------------------
INLINE void Entity::direction(const Direction3D& dir)
{
	if(almostEqual(direction_.yaw(), dir.yaw()) && almostEqual(direction_.roll(), dir.roll()) && almostEqual(direction_.pitch(), dir.pitch()))
		return;

	direction_ = dir; 
	onDirectionChanged();
}

//-------------------------------------------------------------------------------------
INLINE Direction3D& Entity::direction()
{ 
	return direction_; 
}

//-------------------------------------------------------------------------------------
INLINE Position3D& Entity::position()
{
	return position_; 
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Entity::baseMailbox() const
{ 
	return baseMailbox_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::baseMailbox(EntityMailbox* mailbox)
{ 
	baseMailbox_ = mailbox; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientMailbox(EntityMailbox* mailbox)
{
	clientMailbox_ = mailbox; 
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Entity::clientMailbox() const
{ 
	return clientMailbox_; 
}

//-------------------------------------------------------------------------------------
INLINE AllClients* Entity::allClients() const
{ 
	return allClients_; 
}

//-------------------------------------------------------------------------------------
INLINE AllClients* Entity::otherClients() const
{ 
	return otherClients_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::allClients(AllClients* clients)
{
	allClients_ = clients;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::otherClients(AllClients* clients)
{
	otherClients_ = clients;
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isReal(void) const
{ 
	return realCell_ == 0; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::hasGhost(void) const
{ 
	return ghostCell_ > 0; 
}

//-------------------------------------------------------------------------------------
INLINE COMPONENT_ID Entity::realCell(void) const
{ 
	return realCell_; 
}

//-------------------------------------------------------------------------------------
INLINE COMPONENT_ID Entity::ghostCell(void) const
{ 
	return ghostCell_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::realCell(COMPONENT_ID cellID)
{ 
	realCell_ = cellID; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::ghostCell(COMPONENT_ID cellID)
{ 
	ghostCell_ = cellID; 
}

//-------------------------------------------------------------------------------------
INLINE SPACE_ENTITIES::size_type Entity::spaceEntityIdx() const
{
	return spaceEntityIdx_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::spaceEntityIdx(SPACE_ENTITIES::size_type idx)
{
	spaceEntityIdx_ = idx;
}

//-------------------------------------------------------------------------------------
INLINE Witness* Entity::pWitness() const
{
	return pWitness_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::pWitness(Witness* w)
{
	pWitness_ = w;
}

//-------------------------------------------------------------------------------------
INLINE Controllers*	Entity::pControllers() const
{
	return pControllers_;
}

//-------------------------------------------------------------------------------------
INLINE EntityCoordinateNode* Entity::pEntityCoordinateNode() const
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
INLINE bool Entity::isOnGround() const
{
	return isOnGround_;
}

//-------------------------------------------------------------------------------------
INLINE GAME_TIME Entity::posChangedTime() const
{
	return posChangedTime_;
}

//-------------------------------------------------------------------------------------
INLINE GAME_TIME Entity::dirChangedTime() const
{
	return dirChangedTime_;
}

//-------------------------------------------------------------------------------------
INLINE int8 Entity::layer() const
{
	return layer_;
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isControlledNotSelfClient() const
{
	return controlledBy_ == NULL || controlledBy_->id() != id();
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Entity::controlledBy() const
{
	return controlledBy_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::controlledBy(EntityMailbox* baseMailbox)
{
	if (controlledBy_)
		Py_DECREF(controlledBy_);

	controlledBy_ = baseMailbox;

	if (controlledBy_)
		Py_INCREF(controlledBy_);
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setDirty(bool dirty)
{
	isDirty_ = dirty;
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isDirty() const
{
	return isDirty_;
}

//-------------------------------------------------------------------------------------
INLINE VolatileInfo* Entity::pCustomVolatileinfo(void)
{
	return pCustomVolatileinfo_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::localPosition(const Position3D& pos)
{
	Vector3 movement = pos - localPosition_;

	if (KBEVec3Length(&movement) < 0.0004f)
		return;

	localPosition_ = pos;

	posChangedTime_ = g_kbetime;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::localDirection(const Direction3D& dir)
{
	if (almostEqual(localDirection_.yaw(), dir.yaw()) && almostEqual(localDirection_.roll(), dir.roll()) && almostEqual(localDirection_.pitch(), dir.pitch()))
		return;

	localDirection_ = dir;

	dirChangedTime_ = g_kbetime;
}

//-------------------------------------------------------------------------------------
INLINE Direction3D& Entity::localDirection()
{
	return localDirection_;
}

//-------------------------------------------------------------------------------------
INLINE Position3D& Entity::localPosition()
{
	return localPosition_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::syncPositionWorldToLocal()
{
	if (parent())
	{
		Position3D posOut;
		parent()->positionWorldToLocal(position(), posOut);
		localPosition(posOut);
	}
	else
	{
		localPosition(position());
	}
}

//-------------------------------------------------------------------------------------
INLINE void Entity::syncDirectionWorldToLocal()
{
	if (parent())
	{
		Direction3D localDir;
		parent()->directionWorldToLocal(direction(), localDir);
		localDirection(localDir);
	}
	else
	{
		localDirection(direction());
	}
}

//-------------------------------------------------------------------------------------
INLINE void Entity::syncPositionLocalToWorld()
{
	if (parent())
	{
		Position3D worldPos;
		parent()->positionLocalToWorld(localPosition(), worldPos);
		position(worldPos);
	}
	else
	{
		position(localPosition());
	}
}

//-------------------------------------------------------------------------------------
INLINE void Entity::syncDirectionLocalToWorld()
{
	if (parent())
	{
		Direction3D worldDir;
		parent()->directionLocalToWorld(localDirection(), worldDir);
		direction(worldDir);
	}
	else
	{
		direction(localDirection());
	}
}

//-------------------------------------------------------------------------------------
INLINE Entity* Entity::parent(void)
{
	return pParent_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::removeChild(Entity* ent)
{
	removeChild(ent->id());
}

//-------------------------------------------------------------------------------------
INLINE void Entity::removeChild(ENTITY_ID eid)
{
	children_.erase(eid);
}

//-------------------------------------------------------------------------------------
INLINE void Entity::addChild(Entity* ent)
{
	KBE_ASSERT(ent);
	KBE_ASSERT(children_.find(ent->id()) == children_.end());

	children_[ent->id()] = ent;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::updateChildrenPosition()
{
	CHILD_ENTITIES::iterator iter = children_.begin();
	for (; iter != children_.end(); ++iter)
	{
		Entity* child = iter->second;
		Position3D newPos;
		positionLocalToWorld(child->localPosition(), newPos);
		child->position(newPos);

		// 嵌套父子关系处理
		//child->updateChildrenPosition();
	}
}

//-------------------------------------------------------------------------------------
INLINE void Entity::updateChildrenPositionAndDirection()
{
	CHILD_ENTITIES::iterator iter = children_.begin();
	for (; iter != children_.end(); ++iter)
	{
		Entity* child = iter->second;
		Position3D newPos;
		positionLocalToWorld(child->localPosition(), newPos);
		child->position(newPos);

		Direction3D newDir;
		directionLocalToWorld(child->localDirection(), newDir);
		child->direction(newDir);

		// 嵌套父子关系处理
		//child->updateChildrenPositionAndDirection();
	}
}

//-------------------------------------------------------------------------------------
INLINE void Entity::callAllClientsMethod(const Network::MessageHandler& msgHandler, const MemoryStream* mstream)
{
	callSelfClientMethod(msgHandler, mstream);
	callOtherClientsMethod(msgHandler, mstream);
}

//-------------------------------------------------------------------------------------
}
