// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
