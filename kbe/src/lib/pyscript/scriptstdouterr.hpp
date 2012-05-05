/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
/*
	kbengine-脚本系统::重定向python输入输出：
		重写sys.stdout stderr
		
		使用例子:
			将本模块实例覆盖 python.sys.stdout and stderr 就行了， 
			使用完还原stdout and stderr
*/
#ifndef __SCRIPTSTDOUTERR_H__
#define __SCRIPTSTDOUTERR_H__
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "scriptobject.hpp"

namespace KBEngine{ namespace script{
class ScriptStdOutErr: public ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ScriptStdOutErr, ScriptObject)							
public:	
	ScriptStdOutErr();
	virtual ~ScriptStdOutErr();

	/** python执行写操作 */
	static PyObject* write(PyObject* self, PyObject *args);	
	static PyObject* flush(PyObject* self, PyObject *args);

	/** 安装和卸载这个模块 */
	bool install(void);
	bool uninstall(void);
	bool isInstall(void)const{ return isInstall_; }

	virtual void onPrint(const char* msg);
protected:
	bool softspace_;
	PyObject* sysModule_;
	PyObject* prevStderr_;
	PyObject* prevStdout_;
	bool isInstall_;
} ;

}
}
#endif