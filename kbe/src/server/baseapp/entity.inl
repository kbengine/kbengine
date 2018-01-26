/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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


namespace KBEngine { 

//-------------------------------------------------------------------------------------
INLINE EntityCall* Entity::cellEntityCall(void) const
{
	return cellEntityCall_;
}

//-------------------------------------------------------------------------------------
INLINE void Entity::cellEntityCall(EntityCall* entitycall)
{
	cellEntityCall_ = entitycall;
}

//-------------------------------------------------------------------------------------
INLINE EntityCall* Entity::clientEntityCall() const
{ 
	return clientEntityCall_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::clientEntityCall(EntityCall* entitycall)
{ 
	clientEntityCall_ = entitycall; 
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
