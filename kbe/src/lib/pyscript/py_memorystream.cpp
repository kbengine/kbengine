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


#include "py_memorystream.hpp"

namespace KBEngine{ namespace script{

PySequenceMethods PyMemoryStream::seqMethods =
{
	seq_length,		// inquiry sq_length;				len(x)
	0,				// binaryfunc sq_concat;			x + y
	0,				// intargfunc sq_repeat;			x * n
	0,				// intargfunc sq_item;				x[i]
	0,				//seq_slice,				// intintargfunc sq_slice;			x[i:j]
	0,				// intobjargproc sq_ass_item;		x[i] = v
	0,				//seq_ass_slice,			// intintobjargproc sq_ass_slice;	x[i:j] = v
	0,				// objobjproc sq_contains;			v in x
	0,				// binaryfunc sq_inplace_concat;	x += y
	0				// intargfunc sq_inplace_repeat;	x *= n
};

SCRIPT_METHOD_DECLARE_BEGIN(PyMemoryStream)
SCRIPT_METHOD_DECLARE("append",				append,			METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(PyMemoryStream)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(PyMemoryStream)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(PyMemoryStream, 0, &PyMemoryStream::seqMethods, 0, 0, 0)		
	
//-------------------------------------------------------------------------------------
PyMemoryStream::PyMemoryStream(PyTypeObject* pyType, bool isInitialised):
ScriptObject(pyType, isInitialised)
{
}

//-------------------------------------------------------------------------------------
PyMemoryStream::~PyMemoryStream()
{
}

//-------------------------------------------------------------------------------------
void PyMemoryStream::addToStream(MemoryStream* mstream)
{
	ArraySize size = stream().size();

	(*mstream) << size;
	if(size > 0)
	{
		ArraySize rpos = stream().rpos(), wpos = stream().wpos();
		(*mstream) << rpos;
		(*mstream) << wpos;
		(*mstream).append(stream().data(), size);
	}
}

//-------------------------------------------------------------------------------------
void PyMemoryStream::createFromStream(MemoryStream* mstream)
{
	ArraySize size;
	ArraySize rpos, wpos;

	(*mstream) >> size;
	if(size > 0)
	{
		(*mstream) >> rpos;
		(*mstream) >> wpos;

		stream().append(mstream->data() + mstream->rpos(), size);
		stream().rpos(rpos);
		stream().wpos(wpos);

		mstream->read_skip(size);
	}
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::tp_repr()
{
	PyObject* pybytes = this->pyBytes();
	PyObject* pyStr = PyObject_Str(pybytes);
	Py_DECREF(pybytes);
	return pyStr;
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::tp_str()
{
	return tp_repr();
}

//-------------------------------------------------------------------------------------
Py_ssize_t PyMemoryStream::seq_length(PyObject* self)
{
	PyMemoryStream* seq = static_cast<PyMemoryStream*>(self);
	return seq->length();
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__py_append(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyMemoryStream* pyobj = static_cast<PyMemoryStream*>(self);
	return NULL;	
}

//-------------------------------------------------------------------------------------
}
}

