#include "script.hpp"
#include "math.hpp"
#include "pickler.hpp"
#include "thread/concurrency.hpp"

namespace KBEngine{ 
namespace script{

#ifndef KBE_SINGLE_THREADED
static PyObject * s_pOurInitTimeModules;
static PyThreadState * s_pMainThreadState;
static PyThreadState* s_defaultContext;
#endif

//-------------------------------------------------------------------------------------
Script::Script():
m_module_(NULL),
m_pyStdouterr_(NULL)
{
}

//-------------------------------------------------------------------------------------
Script::~Script()
{
}

//-------------------------------------------------------------------------------------
int Script::Run_SimpleString(std::string command, std::string* retBufferPtr)
{
	if(retBufferPtr != NULL)
	{
		if(!m_pyStdouterrHook_->install()){												
			ERROR_MSG("Script::Run_SimpleString::m_pyStdouterrHook_->install() is failed!\n");
			SCRIPT_ERROR_CHECK();
			return -1;
		}
			
		m_pyStdouterrHook_->setHookBuffer(retBufferPtr);
		PyRun_SimpleString(command.c_str());
		SCRIPT_ERROR_CHECK();														// 检查是否有错误产生
		m_pyStdouterrHook_->uninstall();
		return 0;
	}

	PyRun_SimpleString(command.c_str());
	SCRIPT_ERROR_CHECK();															// 检查是否有错误产生
	return 0;
}

//-------------------------------------------------------------------------------------
bool Script::install(wchar_t* pythonHomeDir, std::wstring pyPaths, const char* moduleName, COMPONENT_TYPE componentType)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	Py_SetPythonHome(pythonHomeDir);												// 先设置python的环境变量
#endif
	// Initialise python
	// Py_VerboseFlag = 2;
	Py_FrozenFlag = 1;

	// Warn if tab and spaces are mixed in indentation.
	// Py_TabcheckFlag = 1;
	Py_NoSiteFlag = 1;
	Py_IgnoreEnvironmentFlag = 1;
	Py_Initialize();                      											// python解释器的初始化  
    if (!Py_IsInitialized())
    {
    	ERROR_MSG("Script::install::Py_Initialize is failed!\n");
        return false;
    } 

	pyPaths += SCRIPT_PATH;

	
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
#endif

	PySys_SetPath(pyPaths.c_str());
	PyObject *m = PyImport_AddModule("__main__");

	m_module_ = PyImport_AddModule(moduleName);										// 添加一个脚本基础模块
	if (m_module_ == NULL)
		return false;
	
	const char* componentName = COMPONENT_NAME[componentType];
	PyObject* pStr = PyUnicode_FromString(componentName);
	if (PyObject_SetAttrString(m_module_, "component", pStr) == -1)
	{
		ERROR_MSG( "Script::init: Unable to set KBEngine.component to %s\n",
			componentName );
	}
	Py_XDECREF(pStr);

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
	PyObject_SetAttrString(m, moduleName, m_module_);								// 将模块对象加入main

	m_pyStdouterr_ = new ScriptStdOutErr();											// 重定向python输出
	m_pyStdouterrHook_ = new ScriptStdOutErrHook();
	
	if(!m_pyStdouterr_->install()){													// 安装py重定向脚本模块
		ERROR_MSG("Script::install::m_pyStdouterr_->install() is failed!\n");
		SCRIPT_ERROR_CHECK();
		return false;
	}
	
	Pickler::initialize();
	math::installModule("Math");
	INFO_MSG("Script::install is successfully!\n");
	return true;
}

//-------------------------------------------------------------------------------------
bool Script::uninstall()
{
	math::uninstallModule();
	Pickler::finalise();
	SCRIPT_ERROR_CHECK();															// 检查是否有错误产生

	if(m_pyStdouterr_->isInstall() && !m_pyStdouterr_->uninstall())					// 卸载py重定向脚本模块
		ERROR_MSG("Script::uninstall::m_pyStdouterr_->uninstall() is failed!\n");
	else
		Py_DECREF(m_pyStdouterr_);

	if(m_pyStdouterrHook_->isInstall() && !m_pyStdouterrHook_->uninstall())
		ERROR_MSG("Script::uninstall::m_pyStdouterrHook_->uninstall() is failed!\n");
	else
		Py_DECREF(m_pyStdouterrHook_);

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
	return PyObject_SetAttrString(m_module_, attrName, pyObj);
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
