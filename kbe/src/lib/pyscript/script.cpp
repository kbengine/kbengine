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
#include "pyprofile.hpp"
#include "copy.hpp"
#include "pystruct.hpp"
#include "py_gc.hpp"
#include "install_py_dlls.hpp"
#include "resmgr/resmgr.hpp"
#include "thread/concurrency.hpp"

#ifndef CODE_INLINE
#include "script.ipp"
#endif

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

PyObject * PyTuple_FromStringVector(const std::vector< std::string > & v)
{
	int sz = v.size();
	PyObject * t = PyTuple_New( sz );
	for (int i = 0; i < sz; i++)
	{
		PyTuple_SetItem( t, i, PyUnicode_FromString( v[i].c_str() ) );
	}
	return t;
}

//-------------------------------------------------------------------------------------
Script::Script():
module_(NULL),
extraModule_(NULL),
pyStdouterr_(NULL),
pyStdouterrHook_(NULL)
{
}

//-------------------------------------------------------------------------------------
Script::~Script()
{
}

//-------------------------------------------------------------------------------------
int Script::run_simpleString(const char* command, std::string* retBufferPtr)
{
	if(command == NULL)
	{
		ERROR_MSG("Script::Run_SimpleString: command is NULL!\n");
		return 0;
	}

	if(retBufferPtr != NULL)
	{
		if(!pyStdouterrHook_->install()){												
			ERROR_MSG("Script::Run_SimpleString: pyStdouterrHook_->install() is failed!\n");
			SCRIPT_ERROR_CHECK();
			return -1;
		}
			
		pyStdouterrHook_->setHookBuffer(retBufferPtr);
		//PyRun_SimpleString(command);

		PyObject *m, *d, *v;
		m = PyImport_AddModule("__main__");
		if (m == NULL)
		{
			SCRIPT_ERROR_CHECK();
			pyStdouterrHook_->uninstall();
			return -1;
		}

		d = PyModule_GetDict(m);

		v = PyRun_String(command, Py_single_input, d, d);
		if (v == NULL) 
		{
			PyErr_Print();
			pyStdouterrHook_->uninstall();
			return -1;
		}

		Py_DECREF(v);
		SCRIPT_ERROR_CHECK();
		
		pyStdouterrHook_->uninstall();
		
		return 0;
	}

	PyRun_SimpleString(command);

	SCRIPT_ERROR_CHECK();
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

	Py_SetPythonHome(const_cast<wchar_t*>(pythonHomeDir));								// 先设置python的环境变量

#if KBE_PLATFORM != PLATFORM_WIN32
	std::wstring fs = L";";
	std::wstring rs = L":";
	size_t pos = 0; 

	while(true)
	{ 
		pos = pyPaths.find(fs, pos);
		if (pos == std::wstring::npos) break;
		pyPaths.replace(pos, fs.length(), rs);
	}  

	char* tmpchar = strutil::wchar2char(const_cast<wchar_t*>(pyPaths.c_str()));
	DEBUG_MSG(fmt::format("Script::install(): paths={}.\n", tmpchar));
	free(tmpchar);
	
#endif
	// Initialise python
	// Py_VerboseFlag = 2;
	Py_FrozenFlag = 1;

	// Warn if tab and spaces are mixed in indentation.
	// Py_TabcheckFlag = 1;
	Py_NoSiteFlag = 1;
	Py_IgnoreEnvironmentFlag = 1;

	Py_SetPath(pyPaths.c_str());
	Py_Initialize();                      											// python解释器的初始化  
    if (!Py_IsInitialized())
    {
    	ERROR_MSG("Script::install(): Py_Initialize is failed!\n");
        return false;
    } 

	PyObject *m = PyImport_AddModule("__main__");

	module_ = PyImport_AddModule(moduleName);										// 添加一个脚本基础模块
	if (module_ == NULL)
		return false;
	
	const char* componentName = COMPONENT_NAME_EX(componentType);
	if (PyModule_AddStringConstant(module_, "component", componentName))
	{
		ERROR_MSG(fmt::format("Script::install(): Unable to set KBEngine.component to {}\n",
			componentName));
		return false;
	}
	
	// 注册产生uuid方法到py
	APPEND_SCRIPT_MODULE_METHOD(module_,		genUUID64,			__py_genUUID64,					METH_VARARGS,			0);

	if(!install_py_dlls())
	{
		ERROR_MSG("Script::install(): install_py_dlls() is failed!\n");
		return false;
	}

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
	
	PyGC::initialize();
	Pickler::initialize();
	PyProfile::initialize(this);
	PyStruct::initialize();
	Copy::initialize();
	SCRIPT_ERROR_CHECK();

	math::installModule("Math");
	INFO_MSG(fmt::format("Script::install(): is successfully, Python=({})!\n", Py_GetVersion()));
	return installExtraModule("KBExtra");
}

//-------------------------------------------------------------------------------------
bool Script::uninstall()
{
	math::uninstallModule();
	Pickler::finalise();
	PyProfile::finalise();
	PyStruct::finalise();
	Copy::finalise();
	SCRIPT_ERROR_CHECK();

	if(pyStdouterr_)
	{
		if(pyStdouterr_->isInstall() && !pyStdouterr_->uninstall())	{
			ERROR_MSG("Script::uninstall(): pyStdouterr_->uninstall() is failed!\n");
		}
		else
			Py_DECREF(pyStdouterr_);
	}
	
	if(pyStdouterrHook_)
	{
		if(pyStdouterrHook_->isInstall() && !pyStdouterrHook_->uninstall()){
			ERROR_MSG("Script::uninstall(): pyStdouterrHook_->uninstall() is failed!\n");
		}
		else
			Py_DECREF(pyStdouterrHook_);
	}

	ScriptStdOutErr::uninstallScript();	
	ScriptStdOutErrHook::uninstallScript();

	if(!uninstall_py_dlls())
	{
		ERROR_MSG("Script::uninstall(): uninstall_py_dlls() is failed!\n");
		return false;
	}

#ifndef KBE_SINGLE_THREADED
	if (s_pOurInitTimeModules != NULL)
	{
		Py_DECREF(s_pOurInitTimeModules);
		s_pOurInitTimeModules = NULL;
	}
#endif

	PyGC::initialize();
	Py_Finalize();																// 卸载python解释器
	INFO_MSG("Script::uninstall(): is successfully!\n");
	return true;	
}

//-------------------------------------------------------------------------------------
bool Script::installExtraModule(const char* moduleName)
{
	PyObject *m = PyImport_AddModule("__main__");
	extraModule_ = PyImport_AddModule(moduleName);								// 添加一个脚本扩展模块
	if (extraModule_ == NULL)
		return false;
	
	PyObject *module_ = PyImport_AddModule(moduleName);							// 初始化扩展模块
	if (module_ == NULL)
		return false;

	PyObject_SetAttrString(m, moduleName, extraModule_);						// 将扩展模块对象加入main

	INFO_MSG(fmt::format("Script::install(): {} is successfully!\n", moduleName));
	return true;
}

//-------------------------------------------------------------------------------------
bool Script::registerExtraMethod(const char* attrName, PyMethodDef* pyFunc)
{
	PyObject* obj = PyCFunction_New(pyFunc, NULL);
	bool ret = PyModule_AddObject(extraModule_, attrName, obj) != -1;
	return ret;
}

//-------------------------------------------------------------------------------------
bool Script::registerExtraObject(const char* attrName, PyObject* pyObj)
{
	return PyObject_SetAttrString(extraModule_, attrName, pyObj) != -1;
}

//-------------------------------------------------------------------------------------
int Script::registerToModule(const char* attrName, PyObject* pyObj)
{
	return PyObject_SetAttrString(module_, attrName, pyObj);
}

//-------------------------------------------------------------------------------------
int Script::unregisterToModule(const char* attrName)
{
	if(module_ == NULL || attrName == NULL)
		return 0;

	return PyObject_DelAttrString(module_, attrName);
}

//-------------------------------------------------------------------------------------
PyThreadState* Script::createInterpreter()
{
	PyThreadState* 	pCurInterpreter = PyThreadState_Get();
	PyObject * 		pCurPath = PySys_GetObject( "path" );

	PyThreadState* pNewInterpreter = Py_NewInterpreter();
	if (pNewInterpreter)
	{
		PySys_SetObject( "path", pCurPath );
#ifndef KBE_SINGLE_THREADED
		PyDict_Merge( PySys_GetObject( "modules" ), s_pOurInitTimeModules, 0 );
#endif

		PyThreadState* pSwapped = PyThreadState_Swap( pCurInterpreter );
		if( pSwapped != pNewInterpreter )
		{
			KBE_EXIT( "error creating new python interpreter" );
		}
	}

	return pNewInterpreter;
}

//-------------------------------------------------------------------------------------
void Script::destroyInterpreter( PyThreadState* pInterpreter )
{
	if( pInterpreter == PyThreadState_Get() )
	{
		KBE_EXIT( "trying to destroy current interpreter" );
	}

	PyInterpreterState_Clear( pInterpreter->interp );
	PyInterpreterState_Delete( pInterpreter->interp );
}

//-------------------------------------------------------------------------------------
PyThreadState* Script::swapInterpreter( PyThreadState* pInterpreter )
{
#ifndef KBE_SINGLE_THREADED
	s_defaultContext = pInterpreter;
#endif
	return PyThreadState_Swap( pInterpreter );
}

//-------------------------------------------------------------------------------------
#ifndef KBE_SINGLE_THREADED
void Script::initThread( bool plusOwnInterpreter )
{
	if( s_defaultContext != NULL )
	{
		KBE_EXIT( "trying to initialise scripting when already initialised" );
	}

	PyEval_AcquireLock();

	PyThreadState * newTState = NULL;

	if (plusOwnInterpreter)
	{
		newTState = Py_NewInterpreter();

		PyObject * pMainPyPath = PyDict_GetItemString(
			s_pMainThreadState->interp->sysdict, "path" );
		PySys_SetObject( "path", pMainPyPath );

		PyDict_Merge( PySys_GetObject( "modules" ), s_pOurInitTimeModules, 0);
	}
	else
	{
		newTState = PyThreadState_New( s_pMainThreadState->interp );
	}

	if( newTState == NULL )
	{
		KBE_EXIT( "failed to create a new thread object" );
	}

	PyEval_ReleaseLock();

	s_defaultContext = newTState;
	Script::acquireLock();
}

//-------------------------------------------------------------------------------------
void Script::finiThread( bool plusOwnInterpreter )
{
	if( s_defaultContext != PyThreadState_Get() )
	{
		KBE_EXIT( "trying to finalise script thread when not in default context" );
	}

	if (plusOwnInterpreter)
	{
		{
			PyInterpreterState_Clear( s_defaultContext->interp );
			PyThreadState_Swap( NULL );
			PyInterpreterState_Delete( s_defaultContext->interp );
		}

		PyEval_ReleaseLock();
	}
	else
	{
		PyThreadState_Clear( s_defaultContext );
		PyThreadState_DeleteCurrent();								// releases GIL
	}

	s_defaultContext = NULL;
}

#endif

//-------------------------------------------------------------------------------------
void Script::acquireLock()
{
#ifndef KBE_SINGLE_THREADED
	if (s_defaultContext == NULL) return;

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
