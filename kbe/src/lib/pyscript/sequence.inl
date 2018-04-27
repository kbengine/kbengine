// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine {
namespace script{

INLINE int Sequence::length(void) const
{ 
	return (int)values_.size();
}

INLINE std::vector<PyObject*>& Sequence::getValues(void)
{ 
	return values_; 
}
	
}
}

