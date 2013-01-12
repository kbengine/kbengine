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

#ifndef __KBENGINE_SCRIPT_H__
#define __KBENGINE_SCRIPT_H__
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "scriptobject.hpp"
#include "scriptstdouterr.hpp"
#include "scriptstdouterrhook.hpp"

namespace KBEngine{ namespace script{

/** 脚本系统路径 */
#ifdef _LP64
#define SCRIPT_PATH												\
					L"../../res/scripts;"						\
					L"../../res/scripts/common;"				\
					L"../../res/scripts/common/lib-dynload64;"	\
					L"../../res/scripts/common/Lib"
#else
#define SCRIPT_PATH												\
					L"../../res/scripts;"						\
					L"../../res/scripts/common;"				\
					L"../../res/scripts/common/lib-dynload;"	\
					L"../../res/scripts/common/Lib"
#endif

// 脚本编译后的后缀名和缓存文件夹
#define SCRIPT_BIN_TAG "cpython-32"
#define SCRIPT_BIN_CACHEDIR "__pycache__"

class Script
{						
public:	
	Script();
	virtual ~Script();
	
	/** 
		安装和卸载脚本模块 
	*/
	virtual bool install(const wchar_t* pythonHomeDir, std::wstring pyPaths, 
		const char* moduleName, COMPONENT_TYPE componentType);

	virtual bool uninstall(void);
	
	bool installExtraModule(const char* moduleName);

	/** 
		添加一个扩展接口到引擎扩展模块 
	*/
	bool registerExtraMethod(const char* attrName, PyMethodDef* pyFunc);

	/** 
		添加一个扩展属性到引擎扩展模块 
	*/
	bool registerExtraObject(const char* attrName, PyObject* pyObj);

	/** 
		获取脚本基础模块 
	*/
	INLINE PyObject* getModule(void)const;

	/** 
		获取脚本扩展模块 
	*/
	INLINE PyObject* getExtraModule(void)const;

	int run_simpleString(const char* command, std::string* retBufferPtr);
	INLINE int run_simpleString(std::string command, std::string* retBufferPtr);

	int registerToModule(const char* attrName, PyObject* pyObj);
	int unregisterToModule(const char* attrName);

	void initThread( bool plusOwnInterpreter = false );
	void finiThread( bool plusOwnInterpreter = false );

	static void acquireLock();
	static void releaseLock();

	INLINE ScriptStdOutErrHook* pyStdouterrHook()const;
protected:
	PyObject* 					module_;
	PyObject*					extraModule_;		// 扩展脚本模块

	ScriptStdOutErr*			pyStdouterr_;
	ScriptStdOutErrHook*		pyStdouterrHook_;	// 提供telnet 执行脚本回显用
} ;

}
}

#ifdef CODE_INLINE
#include "script.ipp"
#endif

#endif
