// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine{

//-------------------------------------------------------------------------------------
INLINE bool Entity::isWitnessed(void) const
{ 
	return witnesses_count_ > 0; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::hasWitness(void) const
{ 
	return pWitness_ != NULL &&  clientEntityCall_ != NULL; 
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
INLINE EntityCall* Entity::baseEntityCall() const
{ 
	return baseEntityCall_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::baseEntityCall(EntityCall* entityCall)
{ 
	baseEntityCall_ = entityCall; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientEntityCall(EntityCall* entityCall)
{
	clientEntityCall_ = entityCall; 
}

//-------------------------------------------------------------------------------------
INLINE EntityCall* Entity::clientEntityCall() const
{ 
	return clientEntityCall_; 
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
INLINE EntityCall* Entity::controlledBy() const
{
	return controlledBy_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::controlledBy(EntityCall* baseEntityCall)
{
	if (controlledBy_)
		Py_DECREF(controlledBy_);

	controlledBy_ = baseEntityCall;

	if (controlledBy_)
		Py_INCREF(controlledBy_);
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setDirty(uint32* digest)
{
	if (digest)
	{
		memcpy((void*)&persistentDigest_[0], (void*)digest, sizeof(persistentDigest_));
	}
	else
	{
		persistentDigest_[0] = 0;
		persistentDigest_[1] = 0;
		persistentDigest_[2] = 0;
		persistentDigest_[3] = 0;
		persistentDigest_[4] = 0;
	}
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isDirty() const
{
	return persistentDigest_[0] == 0 && 
		persistentDigest_[1] == 0 && 
		persistentDigest_[2] == 0 && 
		persistentDigest_[3] == 0 && 
		persistentDigest_[4] == 0;
}

//-------------------------------------------------------------------------------------
INLINE VolatileInfo* Entity::pCustomVolatileinfo(void)
{
	return pCustomVolatileinfo_;
}

//-------------------------------------------------------------------------------------
}
