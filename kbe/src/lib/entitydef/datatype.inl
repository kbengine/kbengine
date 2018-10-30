// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 

INLINE DATATYPE_UID DataType::id() const 
{ 
	return id_; 
}

INLINE void DataType::aliasName(std::string aliasName)
{ 
	aliasName_ = aliasName; 
}

INLINE const char* DataType::aliasName(void) const
{ 
	return aliasName_.c_str(); 
}

}
