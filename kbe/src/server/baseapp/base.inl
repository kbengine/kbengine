/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
INLINE EntityMailbox* Base::cellMailbox(void) const
{
	return cellMailbox_;
}

//-------------------------------------------------------------------------------------
INLINE void Base::cellMailbox(EntityMailbox* mailbox)
{
	cellMailbox_ = mailbox;
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Base::clientMailbox() const
{ 
	return clientMailbox_; 
}

//-------------------------------------------------------------------------------------
INLINE void Base::clientMailbox(EntityMailbox* mailbox)
{ 
	clientMailbox_ = mailbox; 
}

//-------------------------------------------------------------------------------------
INLINE PyObject* Base::getCellData(void) const
{ 
	return cellDataDict_; 
}

//-------------------------------------------------------------------------------------
INLINE bool Base::creatingCell(void) const
{ 
	return creatingCell_; 
}

//-------------------------------------------------------------------------------------
INLINE bool Base::hasDB() const 
{
	return hasDB_; 
}

//-------------------------------------------------------------------------------------
INLINE void Base::hasDB(bool has) 
{
	hasDB_ = has; 
}

//-------------------------------------------------------------------------------------
INLINE DBID Base::dbid() const
{
	return DBID_;
}

//-------------------------------------------------------------------------------------
INLINE void Base::dbid(uint16 dbInterfaceIndex, DBID id)
{
	DBID_ = id;
	dbInterfaceIndex_ = dbInterfaceIndex;

	if(DBID_ > 0)
		hasDB_ = true;
}

//-------------------------------------------------------------------------------------
INLINE bool Base::isCreatedSpace()
{
	return createdSpace_;
}

//-------------------------------------------------------------------------------------
INLINE bool Base::inRestore()
{
	return inRestore_;
}

//-------------------------------------------------------------------------------------
INLINE int8 Base::shouldAutoArchive() const
{
	return shouldAutoArchive_;
}

//-------------------------------------------------------------------------------------
INLINE void Base::shouldAutoArchive(int8 v)
{
	shouldAutoArchive_ = v;
}

//-------------------------------------------------------------------------------------
INLINE int8 Base::shouldAutoBackup() const
{
	return shouldAutoBackup_;
}

//-------------------------------------------------------------------------------------
INLINE void Base::shouldAutoBackup(int8 v)
{
	shouldAutoBackup_ = v;
}

//-------------------------------------------------------------------------------------
INLINE void Base::setDirty(bool dirty)
{
	isDirty_ = dirty;
}

//-------------------------------------------------------------------------------------
INLINE bool Base::isDirty() const
{
	return isDirty_;
}

INLINE uint16 Base::dbInterfaceIndex() const
{
	
	return dbInterfaceIndex_;
}

//-------------------------------------------------------------------------------------
}
