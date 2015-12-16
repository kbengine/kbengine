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
INLINE bool EntityMailboxAbstract::isClient() const
{
	return type_ == MAILBOX_TYPE_CLIENT || type_ == MAILBOX_TYPE_CLIENT_VIA_CELL
		|| type_ == MAILBOX_TYPE_CLIENT_VIA_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityMailboxAbstract::isCell() const
{
	return type_ == MAILBOX_TYPE_CELL || type_ == MAILBOX_TYPE_CELL_VIA_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityMailboxAbstract::isCellReal() const
{
	return type_ == MAILBOX_TYPE_CELL;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityMailboxAbstract::isBase() const
{
	return type_ == MAILBOX_TYPE_BASE || type_ == MAILBOX_TYPE_BASE_VIA_CELL;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityMailboxAbstract::isBaseReal() const
{
	return type_ == MAILBOX_TYPE_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityMailboxAbstract::isCellViaBase() const
{
	return type_ == MAILBOX_TYPE_CELL_VIA_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityMailboxAbstract::isBaseViaCell() const
{
	return type_ == MAILBOX_TYPE_BASE_VIA_CELL;
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_ID EntityMailboxAbstract::id() const{ 
	return id_; 
}

//-------------------------------------------------------------------------------------
INLINE void EntityMailboxAbstract::id(int v){ 
	id_ = v; 
}

//-------------------------------------------------------------------------------------
INLINE COMPONENT_ID EntityMailboxAbstract::componentID(void) const{ 
	return componentID_; 
}

//-------------------------------------------------------------------------------------
void EntityMailboxAbstract::componentID(COMPONENT_ID cid)
{
	componentID_ = cid;
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_SCRIPT_UID EntityMailboxAbstract::utype(void) const{ 
	return utype_; 
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_MAILBOX_TYPE EntityMailboxAbstract::type(void) const{
	return type_; 
}
	
}
