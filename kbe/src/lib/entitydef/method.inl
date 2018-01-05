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

INLINE const char* MethodDescription::getName(void) const
{ 
	return name_.c_str(); 
};

INLINE ENTITY_METHOD_UID MethodDescription::getUType(void) const
{ 
	return utype_; 
}

INLINE void MethodDescription::setUType(ENTITY_METHOD_UID muid)
{ 
	utype_ = muid; 
}

INLINE bool MethodDescription::isExposed(void) const
{ 
	return isExposed_; 
}
	
INLINE std::vector<DataType*>& MethodDescription::getArgTypes(void)
{ 
	return argTypes_; 
}

INLINE void MethodDescription::currCallerID(ENTITY_ID eid)
{ 
	currCallerID_ = eid; 
}

INLINE int16 MethodDescription::aliasID() const 
{ 
	return aliasID_; 
}

INLINE uint8 MethodDescription::aliasIDAsUint8() const 
{ 
	return (uint8)aliasID_; 
}

INLINE void MethodDescription::aliasID(int16 v)
{ 
	aliasID_ = v; 
}

INLINE COMPONENT_ID MethodDescription::domain() const
{ 
	return methodDomain_; 
}

INLINE bool MethodDescription::isClient() const
{ 
	return !isCell() && !isBase(); 
}

INLINE bool MethodDescription::isCell() const
{ 
	return methodDomain_ == CELLAPP_TYPE; 
}

INLINE bool MethodDescription::isBase() const
{ 
	return methodDomain_ == BASEAPP_TYPE; 
}

}
