/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_SCRIPT_PICKLER_H
#define KBE_SCRIPT_PICKLER_H

#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class Pickler
{						
public:	
	/** 
		代理 cPicket.dumps 
	*/
	static std::string pickle(PyObject* pyobj);
	static std::string pickle(PyObject* pyobj, int8 protocol);

	/** 
		代理 cPicket.loads 
	*/
	static PyObject* unpickle(const std::string& str);

	/** 
		初始化pickler 
	*/
	static bool initialize(void);
	static void finalise(void);
	
	/** 
		获取unpickle函数表模块对象 
	*/
	static PyObject* getUnpickleFuncTableModule(void){ return pyPickleFuncTableModule_; }
	static PyObject* getUnpickleFunc(const char* funcName);

	static void registerUnpickleFunc(PyObject* pyFunc, const char* funcName);

private:
	static PyObject* picklerMethod_;						// cPicket.dumps方法指针
	static PyObject* unPicklerMethod_;						// cPicket.loads方法指针

	static PyObject* pyPickleFuncTableModule_;				// unpickle函数表模块对象 所有自定义类的unpickle函数都需要在此注册

	static bool	isInit;										// 是否已经被初始化
} ;

}
}

#endif // KBE_SCRIPT_PICKLER_H
