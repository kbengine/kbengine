// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine {
namespace script{

INLINE int Script::run_simpleString(std::string command, std::string* retBufferPtr)
{
	return run_simpleString(command.c_str(), retBufferPtr);
}

INLINE PyObject* Script::getModule(void) const 
{ 
	return module_; 
}

INLINE PyObject* Script::getExtraModule(void) const 
{ 
	return extraModule_; 
}

INLINE PyObject* Script::getSysInitModules(void) const
{
	return sysInitModules_;
}

INLINE ScriptStdOutErr* Script::pyStdouterr() const
{
	return pyStdouterr_;
}

INLINE void Script::pyPrint(const std::string& str)
{
	pyStdouterr_->pyPrint(str);
}

}
}

