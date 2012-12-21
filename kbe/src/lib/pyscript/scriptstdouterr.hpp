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

	/** 
		python执行写操作 
	*/
	static PyObject* __py_write(PyObject* self, PyObject *args);	
	static PyObject* __py_flush(PyObject* self, PyObject *args);

	/** 
		安装和卸载这个模块 
	*/
	bool install(void);
	bool uninstall(void);
	bool isInstall(void)const{ return isInstall_; }

	virtual void onPrint(const wchar_t* msg, uint32 msglen);

	INLINE std::wstring& buffer();
protected:
	bool softspace_;
	PyObject* sysModule_;
	PyObject* prevStderr_;
	PyObject* prevStdout_;
	bool isInstall_;
	std::wstring sbuffer_;
} ;

}
}

#ifdef CODE_INLINE
#include "scriptstdouterr.ipp"
#endif

#endif
