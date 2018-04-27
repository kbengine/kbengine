// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPTSTDOUT_H
#define KBE_SCRIPTSTDOUT_H

#include "helper/debug_helper.h"
#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class ScriptStdOutErr;
class ScriptStdOut: public ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ScriptStdOut, ScriptObject)
public:	
	ScriptStdOut(ScriptStdOutErr* pScriptStdOutErr);
	virtual ~ScriptStdOut();

	/** 
		python执行写操作 
	*/
	static PyObject* __py_write(PyObject* self, PyObject *args);
	static PyObject* __py_flush(PyObject* self, PyObject *args);

	bool install(void);
	bool uninstall(void);
	bool isInstall(void) const{ return isInstall_; }

	INLINE ScriptStdOutErr* pScriptStdOutErr() const;

protected:
	bool softspace_;
	PyObject* old_stdobj_;
	bool isInstall_;

	ScriptStdOutErr* pScriptStdOutErr_;
} ;

}
}

#ifdef CODE_INLINE
#include "scriptstdout.inl"
#endif

#endif // KBE_SCRIPTSTDOUT_H
