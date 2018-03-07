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
INLINE bool EntityCallAbstract::isClient() const
{
	return type_ == ENTITYCALL_TYPE_CLIENT || type_ == ENTITYCALL_TYPE_CLIENT_VIA_CELL
		|| type_ == ENTITYCALL_TYPE_CLIENT_VIA_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityCallAbstract::isCell() const
{
	return type_ == ENTITYCALL_TYPE_CELL || type_ == ENTITYCALL_TYPE_CELL_VIA_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityCallAbstract::isCellReal() const
{
	return type_ == ENTITYCALL_TYPE_CELL;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityCallAbstract::isBase() const
{
	return type_ == ENTITYCALL_TYPE_BASE || type_ == ENTITYCALL_TYPE_BASE_VIA_CELL;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityCallAbstract::isBaseReal() const
{
	return type_ == ENTITYCALL_TYPE_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityCallAbstract::isCellViaBase() const
{
	return type_ == ENTITYCALL_TYPE_CELL_VIA_BASE;
}

//-------------------------------------------------------------------------------------
INLINE bool EntityCallAbstract::isBaseViaCell() const
{
	return type_ == ENTITYCALL_TYPE_BASE_VIA_CELL;
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_ID EntityCallAbstract::id() const { 
	return id_; 
}

//-------------------------------------------------------------------------------------
INLINE void EntityCallAbstract::id(int v) { 
	id_ = v; 
}

//-------------------------------------------------------------------------------------
INLINE COMPONENT_ID EntityCallAbstract::componentID(void) const { 
	return componentID_; 
}

//-------------------------------------------------------------------------------------
INLINE void EntityCallAbstract::componentID(COMPONENT_ID cid)
{
	componentID_ = cid;
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_SCRIPT_UID EntityCallAbstract::utype(void) const { 
	return utype_; 
}

//-------------------------------------------------------------------------------------
INLINE ENTITYCALL_TYPE EntityCallAbstract::type(void) const {
	return type_; 
}
	
}
