// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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

INLINE MethodDescription::EXPOSED_TYPE MethodDescription::isExposed(void) const
{
	return exposedType_;
}
	
INLINE std::vector<DataType*>& MethodDescription::getArgTypes(void)
{ 
	return argTypes_; 
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
