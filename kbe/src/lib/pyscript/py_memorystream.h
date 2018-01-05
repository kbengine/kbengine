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


#ifndef KBE_PY_MEMORYSTREAM_H
#define KBE_PY_MEMORYSTREAM_H

#include "scriptobject.h"
#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/memorystream.h"

namespace KBEngine{ namespace script{

class PyMemoryStream : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(PyMemoryStream, ScriptObject)
public:	
	static PySequenceMethods seqMethods;

	PyMemoryStream(bool readonly = false);
	PyMemoryStream(std::string& strInitData, bool readonly = false);
	PyMemoryStream(PyObject* pyBytesInitData, bool readonly = false);
	PyMemoryStream(MemoryStream* streamInitData, bool readonly = false);

	PyMemoryStream(PyTypeObject* pyType, bool isInitialised = false, bool readonly = false);
	virtual ~PyMemoryStream();


	/**
	支持pickler 方法
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);

	/**
	unpickle方法
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/**
	脚本被安装时被调用
	*/
	static void onInstallScript(PyObject* mod);

	static PyObject* py_new();

	/**
	初始化固定字典
	*/
	void initialize(std::string strDictInitData);
	void initialize(PyObject* pyDictInitData);
	void initialize(MemoryStream* streamInitData);

	INLINE MemoryStream& stream();

	INLINE PyObject* pyBytes();

	INLINE bool readonly() const;

	void addToStream(MemoryStream* mstream);

	void createFromStream(MemoryStream* mstream);

	/** 
		获得对象的描述 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();

	static PyObject* __py_append(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* __py_pop(PyObject* self, PyObject* args, PyObject* kwargs);	
	
	static Py_ssize_t seq_length(PyObject* self);
	INLINE int length(void) const;

	static PyObject* __py_bytes(PyObject* self, PyObject* args, PyObject* kwargs);	
	
	static PyObject* __py_rpos(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* __py_wpos(PyObject* self, PyObject* args, PyObject* kwargs);

	static PyObject* __py_fill(PyObject* self, PyObject* args, PyObject* kwargs);

protected:
	MemoryStream stream_;
	bool readonly_;
} ;

}
}

#ifdef CODE_INLINE
#include "py_memorystream.inl"
#endif

#endif // KBE_PY_MEMORYSTREAM_H
