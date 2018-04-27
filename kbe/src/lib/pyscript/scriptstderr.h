// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPTSTDERR_H
#define KBE_SCRIPTSTDERR_H

#include "helper/debug_helper.h"
#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class ScriptStdOutErr;
class ScriptStdErr: public ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ScriptStdErr, ScriptObject)
public:	
	ScriptStdErr(ScriptStdOutErr* pScriptStdOutErr);
	virtual ~ScriptStdErr();

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
#include "scriptstderr.inl"
#endif

#endif // KBE_SCRIPTSTDERR_H
