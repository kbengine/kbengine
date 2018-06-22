// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine{
namespace client
{

//-------------------------------------------------------------------------------------
INLINE void Entity::direction(const Direction3D& dir)
{ 
	direction_ = dir; 
	onDirectionChanged();
}

//-------------------------------------------------------------------------------------
INLINE void Entity::position(const Position3D& pos)
{ 
	position_ = pos; 
	onPositionChanged();
}

//-------------------------------------------------------------------------------------
INLINE void Entity::serverPosition(const Position3D& pos)
{ 
	serverPosition_ = pos; 
}

//-------------------------------------------------------------------------------------
INLINE Position3D& Entity::clientPos()
{
	return clientPos_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientPos(const Position3D& pos)
{
	clientPos_ = pos;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientPos(float x, float y, float z)
{ 
	clientPos_ = Position3D(x, y, z);
}

//-------------------------------------------------------------------------------------
INLINE Direction3D& Entity::clientDir()
{
	return clientDir_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientDir(const Direction3D& dir)
{
	clientDir_ = dir;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientDir(float roll, float pitch, float yaw)
{
	clientDir_ = Direction3D(roll, pitch, yaw);
}

//-------------------------------------------------------------------------------------
INLINE void Entity::moveSpeed(float speed)
{
	velocity_ = speed; 
	onMoveSpeedChanged();
}

//-------------------------------------------------------------------------------------
INLINE float Entity::moveSpeed() const
{
	return velocity_;
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
INLINE Position3D& Entity::serverPosition()
{
	return serverPosition_; 
}

//-------------------------------------------------------------------------------------
INLINE ClientObjectBase* Entity::pClientApp() const
{
	return pClientApp_;
}

//-------------------------------------------------------------------------------------
INLINE EntityCall* Entity::baseEntityCall() const
{ 
	return baseEntityCall_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::cellEntityCall(EntityCall* entityCall)
{ 
	cellEntityCall_ = entityCall; 
}

//-------------------------------------------------------------------------------------
INLINE EntityCall* Entity::cellEntityCall() const
{ 
	return cellEntityCall_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::baseEntityCall(EntityCall* entityCall)
{ 
	baseEntityCall_ = entityCall; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isInited()
{
	return inited_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::isInited(bool status)
{
	inited_ = status;
}

//-------------------------------------------------------------------------------------
}
}

