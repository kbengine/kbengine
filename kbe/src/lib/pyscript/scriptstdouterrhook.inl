// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine {
namespace script{

INLINE void ScriptStdOutErrHook::setHookBuffer(std::string* buffer){ 
	pBuffer_ = buffer; 
	buffer_ = ""; 
};

INLINE void ScriptStdOutErrHook::setPrint(bool v)
{
	isPrint_ = v;
}

}
}

