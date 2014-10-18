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


#ifndef KBE_PY_MEMORYSTREAM_HPP
#define KBE_PY_MEMORYSTREAM_HPP

#include "scriptobject.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"

namespace KBEngine{ namespace script{

class PyMemoryStream : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(PyMemoryStream, ScriptObject)
public:	
	static PySequenceMethods seqMethods;

	PyMemoryStream(PyTypeObject* pyType, bool isInitialised = false, bool readonly = false);
	virtual ~PyMemoryStream();

	INLINE MemoryStream& stream();

	INLINE PyObject* pyBytes();

	INLINE bool readonly()const;

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
	INLINE int length(void)const;
protected:
	MemoryStream stream_;
	bool readonly_;
} ;

}
}

#ifdef CODE_INLINE
#include "py_memorystream.ipp"
#endif

#endif // KBE_PY_MEMORYSTREAM_HPP
