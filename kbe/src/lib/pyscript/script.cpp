/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "script.h"
#include "math.h"
#include "pickler.h"
#include "pyprofile.h"
#include "copy.h"
#include "pystruct.h"
#include "py_gc.h"
#include "install_py_dlls.h"
#include "resmgr/resmgr.h"
#include "thread/concurrency.h"

#ifndef CODE_INLINE
#include "script.inl"
#endif

namespace KBEngine{ 

KBE_SINGLETON_INIT(script::Script);
namespace script{

//-------------------------------------------------------------------------------------
static PyObject* __py_genUUID64(PyObject *self, void *closure)	
{
	static int8 check = -1;

	if(check < 0)
	{
		if(g_componentGlobalOrder <= 0 || g_componentGlobalOrder > 65535)
		{
			WARNING_MSG(fmt::format("globalOrder({}) is not in the range of 0~65535, genUUID64 is not safe, "
				"in the multi process may be repeated.\n", g_componentGlobalOrder));
		}

		check = 1;
	}

	return PyLong_FromUnsignedLongLong(genUUID64());
}

//-------------------------------------------------------------------------------------
PyObject * PyTuple_FromStringVector(const std::vector< std::string > & v)
{
	int sz = v.size();
	PyObject * t = PyTuple_New( sz );
	for (int i = 0; i < sz; ++i)
	{
		PyTuple_SetItem( t, i, PyUnicode_FromString( v[i].c_str() ) );
	}

	return t;
}

//-------------------------------------------------------------------------------------
Script::Script():
module_(NULL),
extraModule_(NULL),
pyStdouterr_(NULL)
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

	ScriptStdOutErrHook* pStdouterrHook = new ScriptStdOutErrHook();

	if(retBufferPtr != NULL)
	{
		DebugHelper::getSingleton().resetScriptMsgType();
		if(!pStdouterrHook->install()){												
			ERROR_MSG("Script::Run_SimpleString: pyStdouterrHook_->install() is failed!\n");
			SCRIPT_ERROR_CHECK();
			delete pStdouterrHook;
			return -1;
		}
			
		pStdouterrHook->setHookBuffer(retBufferPtr);
		//PyRun_SimpleString(command);

		PyObject *m, *d, *v;
		m = PyImport_AddModule("__main__");
		if (m == NULL)
		{
			SCRIPT_ERROR_CHECK();
			pStdouterrHook->uninstall();
			delete pStdouterrHook;
			return -1;
		}

		d = PyModule_GetDict(m);

		v = PyRun_String(command, Py_single_input, d, d);
		if (v == NULL) 
		{
			PyErr_Print();
			pStdouterrHook->uninstall();
			delete pStdouterrHook;
			return -1;
		}

		Py_DECREF(v);
		SCRIPT_ERROR_CHECK();
		
		pStdouterrHook->uninstall();
		delete pStdouterrHook;
		return 0;
	}

	PyRun_SimpleString(command);

	SCRIPT_ERROR_CHECK();
	delete pStdouterrHook;
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

	// ������python�Ļ�������
	Py_SetPythonHome(const_cast<wchar_t*>(pythonHomeDir));								

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

	// python�������ĳ�ʼ��  
	Py_Initialize();
    if (!Py_IsInitialized())
    {
    	ERROR_MSG("Script::install(): Py_Initialize is failed!\n");
        return false;
    } 

	PyObject *m = PyImport_AddModule("__main__");

	// ���һ���ű�����ģ��
	module_ = PyImport_AddModule(moduleName);
	if (module_ == NULL)
		return false;
	
	const char* componentName = COMPONENT_NAME_EX(componentType);
	if (PyModule_AddStringConstant(module_, "component", componentName))
	{
		ERROR_MSG(fmt::format("Script::install(): Unable to set KBEngine.component to {}\n",
			componentName));
		return false;
	}
	
	// ע�����uuid������py
	APPEND_SCRIPT_MODULE_METHOD(module_,		genUUID64,			__py_genUUID64,					METH_VARARGS,			0);

	if(!install_py_dlls())
	{
		ERROR_MSG("Script::install(): install_py_dlls() is failed!\n");
		return false;
	}

	// ��װpy�ض���ģ��
	ScriptStdOut::installScript(NULL);
	ScriptStdErr::installScript(NULL);

	/*
	static struct PyModuleDef moduleDesc =
	{  
			 PyModuleDef_HEAD_INIT,  
			 moduleName,  
			 "This module is created by KBEngine!", 
			 -1,  
			 NULL  
	};  

	// ��ʼ������ģ��
	PyModule_Create(&moduleDesc);
	*/

	// ��ģ��������main
	PyObject_SetAttrString(m, moduleName, module_);	
	PyObject_SetAttrString(module_, "__doc__", PyUnicode_FromString("This module is created by KBEngine!"));

	// �ض���python���
	pyStdouterr_ = new ScriptStdOutErr();
	
	// ��װpy�ض���ű�ģ��
	if(!pyStdouterr_->install()){
		ERROR_MSG("Script::install::pyStdouterr_->install() is failed!\n");
		delete pyStdouterr_;
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
		
		delete pyStdouterr_;
	}

	ScriptStdOut::uninstallScript();
	ScriptStdErr::uninstallScript();

	if(!uninstall_py_dlls())
	{
		ERROR_MSG("Script::uninstall(): uninstall_py_dlls() is failed!\n");
		return false;
	}

	PyGC::initialize();

	// ж��python������
	Py_Finalize();

	INFO_MSG("Script::uninstall(): is successfully!\n");
	return true;	
}

//-------------------------------------------------------------------------------------
bool Script::installExtraModule(const char* moduleName)
{
	PyObject *m = PyImport_AddModule("__main__");

	// ���һ���ű���չģ��
	extraModule_ = PyImport_AddModule(moduleName);
	if (extraModule_ == NULL)
		return false;
	
	// ��ʼ����չģ��
	PyObject *module = PyImport_AddModule(moduleName);
	if (module == NULL)
		return false;

	// ����չģ��������main
	PyObject_SetAttrString(m, moduleName, extraModule_);

	INFO_MSG(fmt::format("Script::install(): {} is successfully!\n", moduleName));
	return true;
}

//-------------------------------------------------------------------------------------
bool Script::registerExtraMethod(const char* attrName, PyMethodDef* pyFunc)
{
	return PyModule_AddObject(extraModule_, attrName, PyCFunction_New(pyFunc, NULL)) != -1;
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
void Script::setenv(const std::string& name, const std::string& value)
{
	PyObject* osModule = PyImport_ImportModule("os");

	if(osModule)
	{
		PyObject* py_environ = NULL;
		PyObject* py_name = NULL;
		PyObject* py_value = NULL;

		PyObject* supports_bytes_environ = PyObject_GetAttrString(osModule, "supports_bytes_environ");
		if(Py_True == supports_bytes_environ)
		{
			py_environ = PyObject_GetAttrString(osModule, "environb");
			py_name = PyBytes_FromString(name.c_str());
			py_value = PyBytes_FromString(value.c_str());
		}
		else
		{
			py_environ = PyObject_GetAttrString(osModule, "environ");
			py_name = PyUnicode_FromString(name.c_str());
			py_value = PyUnicode_FromString(value.c_str());
		}

		Py_DECREF(supports_bytes_environ);
		Py_DECREF(osModule);

		if (!py_environ)
		{
			ERROR_MSG("Script::setenv: get os.environ error!\n");
			PyErr_PrintEx(0);
			Py_DECREF(py_value);
			Py_DECREF(py_name);
			return;
		}

		PyObject* environData = PyObject_GetAttrString(py_environ, "_data");
		if (!environData)
		{
			ERROR_MSG("Script::setenv: os.environ._data not exist!\n");
			PyErr_PrintEx(0);
			Py_DECREF(py_value);
			Py_DECREF(py_name);
			Py_DECREF(py_environ);
			return;
		}

		int ret = PyDict_SetItem(environData, py_name, py_value);
		
		Py_DECREF(environData);
		Py_DECREF(py_environ);
		Py_DECREF(py_value);
		Py_DECREF(py_name);
		
		if(ret == -1)
		{
			ERROR_MSG("Script::setenv: get os.environ error!\n");
			PyErr_PrintEx(0);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------

}
}
