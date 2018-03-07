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
