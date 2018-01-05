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

INLINE int8 PropertyDescription::getDetailLevel(void) const
{ 
	return detailLevel_; 
}

INLINE bool PropertyDescription::isPersistent(void) const
{ 
	return isPersistent_; 
};

INLINE DataType* PropertyDescription::getDataType(void) const
{ 
	return dataType_; 
};

INLINE uint32 PropertyDescription::getFlags(void) const
{ 
	return flags_; 
};

INLINE const char* PropertyDescription::getName(void) const
{ 
	return name_.c_str(); 
};

INLINE const char* PropertyDescription::getDataTypeName(void) const
{ 
	return dataTypeName_.c_str(); 
}

INLINE const char* PropertyDescription::getDefaultValStr(void) const
{ 
	return defaultValStr_.c_str(); 
}

INLINE ENTITY_PROPERTY_UID PropertyDescription::getUType(void) const
{ 
	return utype_; 
}

INLINE void PropertyDescription::setIdentifier(bool isIdentifier)
{ 
	isIdentifier_ = isIdentifier; 
}

INLINE void PropertyDescription::setDatabaseLength(uint32 databaseLength)
{ 
	databaseLength_ = databaseLength; 
}

INLINE uint32 PropertyDescription::getDatabaseLength() const 
{ 
	return databaseLength_; 
}

INLINE int16 PropertyDescription::aliasID() const 
{ 
	return aliasID_; 
}

INLINE uint8 PropertyDescription::aliasIDAsUint8() const 
{ 
	return (uint8)aliasID_; 
}

INLINE void PropertyDescription::aliasID(int16 v)
{ 
	aliasID_ = v; 
}

INLINE const char* PropertyDescription::indexType(void) const
{
	return indexType_.c_str();
}

INLINE bool PropertyDescription::hasCell(void) const
{ 
	return (flags_ & ENTITY_CELL_DATA_FLAGS) > 0; 
}

INLINE bool PropertyDescription::hasBase(void) const
{ 
	return (flags_ & ENTITY_BASE_DATA_FLAGS) > 0; 
}

INLINE bool PropertyDescription::hasClient(void) const
{ 
	return (flags_ & ENTITY_CLIENT_DATA_FLAGS) > 0; 
}
	
}
