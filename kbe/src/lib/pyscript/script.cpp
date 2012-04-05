#include "script.hpp"
#include "math.hpp"
#include "pickler.hpp"
namespace KBEngine{ 
namespace script{

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
bool Script::install(wchar_t* pythonHomeDir, std::wstring pyPaths, const char* moduleName)
{
	Py_SetPythonHome(pythonHomeDir);												// 先设置python的环境变量
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

}
}
