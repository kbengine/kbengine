/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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


#include "script.hpp"
#include "math.hpp"
#include "pickler.hpp"
#include "copy.hpp"
#include "uuid.hpp"
#include "resmgr/resmgr.hpp"
#include "thread/concurrency.hpp"

namespace KBEngine{ 
namespace script{

#ifndef KBE_SINGLE_THREADED
static PyObject * s_pOurInitTimeModules;
static PyThreadState * s_pMainThreadState;
static PyThreadState* s_defaultContext;
#endif

static PyObject* __py_genUUID64(PyObject *self, void *closure)	
{
	return PyLong_FromUnsignedLongLong(genUUID64());
}
			
//-------------------------------------------------------------------------------------
Script::Script():
module_(NULL),
pyStdouterr_(NULL)
{
}

//-------------------------------------------------------------------------------------
Script::~Script()
{
}

//-------------------------------------------------------------------------------------
int Script::run_simpleString(std::string command, std::string* retBufferPtr)
{
	if(retBufferPtr != NULL)
	{
		if(!pyStdouterrHook_->install()){												
			ERROR_MSG("Script::Run_SimpleString::pyStdouterrHook_->install() is failed!\n");
			SCRIPT_ERROR_CHECK();
			return -1;
		}
			
		pyStdouterrHook_->setHookBuffer(retBufferPtr);
		PyRun_SimpleString(command.c_str());
		SCRIPT_ERROR_CHECK();														// 检查是否有错误产生
		
		pyStdouterrHook_->uninstall();
		
		return 0;
	}

	PyRun_SimpleString(command.c_str());
	SCRIPT_ERROR_CHECK();															// 检查是否有错误产生
	return 0;
}

//-------------------------------------------------------------------------------------
bool Script::install(const wchar_t* pythonHomeDir, std::wstring pyPaths, 
	const char* moduleName, COMPONENT_TYPE componentType)
{
	std::wstring pySysPaths = SCRIPT_PATH;
	wchar_t* pwpySysResPath = strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().getPySysResPath().c_str()));
	strutil::kbe_replace(pySysPaths, L"../../res/", pwpySysResPath);
	pyPaths += pySysPaths;
	free(pwpySysResPath);

#if KBE_PLATFORM == PLATFORM_WIN32
	Py_SetPythonHome(const_cast<wchar_t*>(pythonHomeDir));								// 先设置python的环境变量
#else
	std::wstring fs = L";";
	std::wstring rs = L":";
	size_t pos = 0; 

	while(true)
	{ 
		pos = pyPaths.find(fs, pos);
		if (pos == std::wstring::npos) break;
		pyPaths.replace(pos, fs.length(), rs);
	}  

	Py_SetPath(pyPaths.c_str()); 
	char* tmpchar = strutil::wchar2char(const_cast<wchar_t*>(pyPaths.c_str()));
	DEBUG_MSG(boost::format("Script::install: paths=%1%.\n") % tmpchar);
	free(tmpchar);
	
#endif
	// Initialise python
	// Py_VerboseFlag = 2;
	Py_FrozenFlag = 1;

	// Warn if tab and spaces are mixed in indentation.
	// Py_TabcheckFlag = 1;
	Py_NoSiteFlag = 1;
	Py_IgnoreEnvironmentFlag = 1;
	Py_Initialize();                      												// python解释器的初始化  
    if (!Py_IsInitialized())
    {
    	ERROR_MSG("Script::install::Py_Initialize is failed!\n");
        return false;
    } 

#if KBE_PLATFORM == PLATFORM_WIN32
	PySys_SetPath(pyPaths.c_str());
#endif

	PyObject *m = PyImport_AddModule("__main__");

	module_ = PyImport_AddModule(moduleName);										// 添加一个脚本基础模块
	if (module_ == NULL)
		return false;
	
	const char* componentName = COMPONENT_NAME_EX(componentType);
	if (PyModule_AddStringConstant(module_, "component", componentName))
	{
		ERROR_MSG(boost::format("Script::init: Unable to set KBEngine.component to %1%\n") %
			componentName );
		return false;
	}
	
	// 注册产生uuid方法到py
	APPEND_SCRIPT_MODULE_METHOD(module_,		genUUID64,			__py_genUUID64,					METH_VARARGS,			0);
	
#ifndef KBE_SINGLE_THREADED
	s_pOurInitTimeModules = PyDict_Copy( PySys_GetObject( "modules" ) );
	s_pMainThreadState = PyThreadState_Get();
	s_defaultContext = s_pMainThreadState;
	PyEval_InitThreads();

	KBEConcurrency::setMainThreadIdleFunctions(
		&Script::releaseLock, &Script::acquireLock );
#endif

	ScriptStdOutErr::installScript(NULL);											// 安装py重定向模块
	ScriptStdOutErrHook::installScript(NULL);

	static struct PyModuleDef moduleDesc =   
	{  
			 PyModuleDef_HEAD_INIT,  
			 moduleName,  
			 "This module is created by KBEngine!",  
			 -1,  
			 NULL  
	};  

	PyModule_Create(&moduleDesc);													// 初始化基础模块
	PyObject_SetAttrString(m, moduleName, module_);									// 将模块对象加入main

	pyStdouterr_ = new ScriptStdOutErr();											// 重定向python输出
	pyStdouterrHook_ = new ScriptStdOutErrHook();
	
	if(!pyStdouterr_->install()){													// 安装py重定向脚本模块
		ERROR_MSG("Script::install::pyStdouterr_->install() is failed!\n");
		SCRIPT_ERROR_CHECK();
		return false;
	}
	
	Pickler::initialize();
	Copy::initialize();
	Uuid::initialize();

	math::installModule("Math");
	INFO_MSG("Script::install is successfully!\n");
	return true;
}

//-------------------------------------------------------------------------------------
bool Script::uninstall()
{
	math::uninstallModule();
	Pickler::finalise();
	Copy::finalise();
	Uuid::finalise();
	SCRIPT_ERROR_CHECK();															// 检查是否有错误产生

	if(pyStdouterr_->isInstall() && !pyStdouterr_->uninstall())	{					// 卸载py重定向脚本模块
		ERROR_MSG("Script::uninstall::pyStdouterr_->uninstall() is failed!\n");
	}
	else
		Py_DECREF(pyStdouterr_);

	if(pyStdouterrHook_->isInstall() && !pyStdouterrHook_->uninstall()){
		ERROR_MSG("Script::uninstall::pyStdouterrHook_->uninstall() is failed!\n");
	}
	else
		Py_DECREF(pyStdouterrHook_);

	ScriptStdOutErr::uninstallScript();	
	ScriptStdOutErrHook::uninstallScript();

#ifndef KBE_SINGLE_THREADED
	if (s_pOurInitTimeModules != NULL)
	{
		Py_DECREF(s_pOurInitTimeModules);
		s_pOurInitTimeModules = NULL;
	}
#endif

	Py_Finalize();																	// 卸载python解释器
	INFO_MSG("Script::uninstall is successfully!\n");
	return true;	
}

//-------------------------------------------------------------------------------------
int Script::registerToModule(const char* attrName, PyObject* pyObj)
{
	return PyObject_SetAttrString(module_, attrName, pyObj);
}

//-------------------------------------------------------------------------------------
int Script::unregisterToModule(const char* attrName)
{
	return PyObject_DelAttrString(module_, attrName);
}

//-------------------------------------------------------------------------------------
void Script::acquireLock()
{
#ifndef KBE_SINGLE_THREADED
	if (s_defaultContext == NULL) return;

	//KBE_ASSERT( PyThreadState_Get() != s_defaultContext );
	// can't do assert above because PyThreadState_Get can't (since 2.4)
	// be called when the thread state is null - it generates a fatal
	// error. NULL is what we expect it to be as set by releaseLock anyway...
	// there doesn't appear to be a good way to assert this here. Oh well.
	PyEval_RestoreThread( s_defaultContext );
#endif
}

//-------------------------------------------------------------------------------------
void Script::releaseLock()
{
#ifndef KBE_SINGLE_THREADED
	if (s_defaultContext == NULL) return;

	PyThreadState * oldState = PyEval_SaveThread();
	KBE_ASSERT(oldState == s_defaultContext && "releaseLock: default context is incorrect");
#endif
}

//-------------------------------------------------------------------------------------

}
}
