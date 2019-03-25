// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 

//-------------------------------------------------------------------------------------
INLINE EntityCall* Entity::cellEntityCall(void) const
{
	return cellEntityCall_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::cellEntityCall(EntityCall* entityCall)
{
	cellEntityCall_ = entityCall;
}

//-------------------------------------------------------------------------------------
INLINE EntityCall* Entity::clientEntityCall() const
{ 
	return clientEntityCall_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientEntityCall(EntityCall* entityCall)
{ 
	clientEntityCall_ = entityCall; 
}

//-------------------------------------------------------------------------------------
INLINE PyObject* Entity::getCellData(void) const
{ 
	return cellDataDict_; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::creatingCell(void) const
{ 
	return creatingCell_; 
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::hasDB() const 
{
	return hasDB_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::hasDB(bool has) 
{
	hasDB_ = has; 
}

//-------------------------------------------------------------------------------------
INLINE DBID Entity::dbid() const
{
	return DBID_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::dbid(uint16 dbInterfaceIndex, DBID id)
{
	DBID_ = id;
	dbInterfaceIndex_ = dbInterfaceIndex;

	if(DBID_ > 0)
		hasDB_ = true;
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::isCreatedSpace()
{
	return createdSpace_;
}

//-------------------------------------------------------------------------------------
INLINE bool Entity::inRestore()
{
	return inRestore_;
}

//-------------------------------------------------------------------------------------
INLINE int8 Entity::shouldAutoArchive() const
{
	return shouldAutoArchive_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::shouldAutoArchive(int8 v)
{
	shouldAutoArchive_ = v;
}

//-------------------------------------------------------------------------------------
INLINE int8 Entity::shouldAutoBackup() const
{
	return shouldAutoBackup_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::shouldAutoBackup(int8 v)
{
	shouldAutoBackup_ = v;
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
INLINE uint16 Entity::dbInterfaceIndex() const
{
	return dbInterfaceIndex_;
}

//-------------------------------------------------------------------------------------
INLINE BaseMessagesForwardClientHandler* Entity::pBufferedSendToClientMessages()
{
	return pBufferedSendToClientMessages_;
}

//-------------------------------------------------------------------------------------
}
