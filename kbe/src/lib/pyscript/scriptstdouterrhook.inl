// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine {
namespace script{

INLINE void ScriptStdOutErrHook::setHookBuffer(std::string* buffer){ 
	buffer_ = buffer; 
	wbuffer_ = L""; 
};

INLINE void ScriptStdOutErrHook::setPrint(bool v)
{
	isPrint_ = v;
}

}
}

