// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
