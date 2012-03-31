/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SCRIPT_PICKLER_H__
#define __SCRIPT_PICKLER_H__
#include "cstdkbe/cstdkbe.hpp"
#include "scriptobject.hpp"

namespace KBEngine{ namespace script{
class Pickler
{						
public:	
	/** 代理 cPicket.dumps */
	static std::string pickle(PyObject* pyobj);
	static std::string pickle(PyObject* pyobj, int8 protocol);
	/** 代理 cPicket.loads */
	static PyObject* unpickle(const std::string& str);
	/** 初始化pickler */
	static bool initialize(void);
	static void finalise(void);
	
	/** 获取unpickle函数表模块对象 */
	static PyObject* getUnpickleFuncTableModule(void){ return m_pyPickleFuncTableModule_; }
	static PyObject* getUnpickleFunc(const char* funcName){ return PyObject_GetAttrString(m_pyPickleFuncTableModule_, funcName); }
	static void registerUnpickleFunc(PyObject* pyFunc, const char* funcName);
private:
	static PyObject* m_picklerMethod_;						// cPicket.dumps方法指针
	static PyObject* m_unPicklerMethod_;					// cPicket.loads方法指针
	static PyObject* m_pyPickleFuncTableModule_;			// unpickle函数表模块对象 所有自定义类的unpickle函数都需要在此注册
	static bool	isInit;										// 是否已经被初始化
} ;

}
}
#endif