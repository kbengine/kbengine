// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "pickler.h"
#include "py_memorystream.h"
#include "py_gc.h"

#ifndef CODE_INLINE
#include "py_memorystream.inl"
#endif

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
SCRIPT_METHOD_DECLARE("pop",				pop,			METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("bytes",				bytes,			METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("rpos",				rpos,			METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("wpos",				wpos,			METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("fill",				fill,			METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("__reduce_ex__",		reduce_ex__,	METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(PyMemoryStream)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(PyMemoryStream)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(PyMemoryStream, 0, &PyMemoryStream::seqMethods, 0, 0, 0)		
	
//-------------------------------------------------------------------------------------
PyMemoryStream::PyMemoryStream(PyTypeObject* pyType, bool isInitialised, bool readonly):
ScriptObject(pyType, isInitialised),
readonly_(readonly)
{
}

//-------------------------------------------------------------------------------------
PyMemoryStream::PyMemoryStream(std::string& strDictInitData, bool readonly) :
ScriptObject(getScriptType(), false),
readonly_(readonly)
{
	initialize(strDictInitData);
	script::PyGC::incTracing("MemoryStream");
}

//-------------------------------------------------------------------------------------
PyMemoryStream::PyMemoryStream(PyObject* pyDictInitData, bool readonly) :
ScriptObject(getScriptType(), false),
readonly_(readonly)
{
	initialize(pyDictInitData);
	script::PyGC::incTracing("MemoryStream");
}

//-------------------------------------------------------------------------------------
PyMemoryStream::PyMemoryStream(MemoryStream* streamInitData, bool readonly) :
ScriptObject(getScriptType(), false),
readonly_(readonly)
{
	initialize(streamInitData);
	script::PyGC::incTracing("MemoryStream");
}

//-------------------------------------------------------------------------------------
PyMemoryStream::PyMemoryStream(bool readonly) :
ScriptObject(getScriptType(), false),
readonly_(readonly)
{
	initialize("");
	script::PyGC::incTracing("MemoryStream");
}

//-------------------------------------------------------------------------------------
PyMemoryStream::~PyMemoryStream()
{
}

//-------------------------------------------------------------------------------------
void PyMemoryStream::initialize(std::string strDictInitData)
{
	if (strDictInitData.size() > 0)
		stream_.append(strDictInitData.data(), strDictInitData.size());
}

//-------------------------------------------------------------------------------------
void PyMemoryStream::initialize(PyObject* pyBytesInitData)
{
	char *buffer;
	Py_ssize_t length;

	if (PyBytes_AsStringAndSize(pyBytesInitData, &buffer, &length) < 0)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	if (length > 0)
		stream_.append(buffer, length);
}

//-------------------------------------------------------------------------------------
void PyMemoryStream::initialize(MemoryStream* streamInitData)
{
	if (streamInitData)
		stream_ = *streamInitData;
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__py_reduce_ex__(PyObject* self, PyObject* protocol)
{
	PyMemoryStream* pPyMemoryStream = static_cast<PyMemoryStream*>(self);
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("MemoryStream");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	PyObject* args1 = PyTuple_New(3);

	PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(pPyMemoryStream->stream().rpos()));
	PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLong(pPyMemoryStream->stream().wpos()));
	PyTuple_SET_ITEM(args1, 2, pPyMemoryStream->pyBytes());

	PyTuple_SET_ITEM(args, 1, args1);

	if (unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}

	return args;
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__unpickle__(PyObject* self, PyObject* args)
{
	Py_ssize_t size = PyTuple_Size(args);
	if (size != 3)
	{
		ERROR_MSG("PyMemoryStream::__unpickle__: args is wrong! (size != 3)");
		S_Return;
	}

	PyObject* pyRpos = PyTuple_GET_ITEM(args, 0);
	PyObject* pyWpos = PyTuple_GET_ITEM(args, 1);

	PyObject* pybytes = PyTuple_GET_ITEM(args, 2);
	if (pybytes == NULL)
	{
		ERROR_MSG("PyMemoryStream::__unpickle__: args is wrong!");
		S_Return;
	}

	PyMemoryStream* pPyMemoryStream = new PyMemoryStream(pybytes);
	pPyMemoryStream->stream().rpos(PyLong_AsUnsignedLong(pyRpos));
	pPyMemoryStream->stream().wpos(PyLong_AsUnsignedLong(pyWpos));
	return pPyMemoryStream;
}

//-------------------------------------------------------------------------------------
void PyMemoryStream::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method =
	{ "MemoryStream", (PyCFunction)&PyMemoryStream::__unpickle__, METH_VARARGS, 0 };

	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "MemoryStream");
	Py_DECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::py_new()
{
	return new PyMemoryStream();
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
PyObject* PyMemoryStream::__py_bytes(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyMemoryStream* pyobj = static_cast<PyMemoryStream*>(self);
	return pyobj->pyBytes();
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__py_append(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyMemoryStream* pyobj = static_cast<PyMemoryStream*>(self);
	if(pyobj->readonly())
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::append: read only!");
		PyErr_PrintEx(0);
		S_Return;
	}

	int argCount = PyTuple_Size(args);
	if(argCount != 2)
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::append: args error! arg1 is type[UINT8|STRING|...], arg2 is val.");
		PyErr_PrintEx(0);
	}
	
	char* type;
	PyObject* pyVal = NULL;

	if(!PyArg_ParseTuple(args, "s|O", &type, &pyVal))
	{
		PyErr_Format(PyExc_TypeError, "PyMemoryStream::append: args error!");
		PyErr_PrintEx(0);
		S_Return;
	}

	if(strcmp(type, "UINT8") == 0)
	{
		uint8 v = (uint8)PyLong_AsUnsignedLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "UINT16") == 0)
	{
		uint16 v = (uint16)PyLong_AsUnsignedLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "UINT32") == 0)
	{
		uint32 v = PyLong_AsUnsignedLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "UINT64") == 0)
	{
		uint64 v = PyLong_AsUnsignedLongLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "INT8") == 0)
	{
		int8 v = (int8)PyLong_AsLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "INT16") == 0)
	{
		int16 v = (int16)PyLong_AsLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "INT32") == 0)
	{
		int32 v = PyLong_AsLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "INT64") == 0)
	{
		int64 v = PyLong_AsLongLong(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "FLOAT") == 0)
	{
		float v = (float)PyFloat_AsDouble(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "DOUBLE") == 0)
	{
		double v = (double)PyFloat_AsDouble(pyVal);
		pyobj->stream() << v;
	}
	else if(strcmp(type, "STRING") == 0)
	{
		const char* s = PyUnicode_AsUTF8AndSize(pyVal, NULL);
		pyobj->stream() << s;
	}
	else if(strcmp(type, "UNICODE") == 0)
	{
		Py_ssize_t size;
		const char* s = PyUnicode_AsUTF8AndSize(pyVal, &size);
		if (s == NULL)
		{
			PyErr_Format(PyExc_TypeError, "PyMemoryStream::append: val is not UNICODE!");
			PyErr_PrintEx(0);
			S_Return;
		}

		pyobj->stream().appendBlob(s, size);
	}
	else if(strcmp(type, "PYTHON") == 0 || strcmp(type, "PY_DICT") == 0
		 || strcmp(type, "PY_TUPLE") == 0  || strcmp(type, "PY_LIST") == 0)
	{
		std::string datas = Pickler::pickle(pyVal);
		pyobj->stream().appendBlob(datas);
	}
	else if(strcmp(type, "BLOB") == 0)
	{
		if(!PyObject_TypeCheck(pyVal, PyMemoryStream::getScriptType()) && !PyBytes_Check(pyVal))
		{
			PyErr_Format(PyExc_TypeError, "PyMemoryStream::append: val is not BLOB!");
			PyErr_PrintEx(0);
			S_Return;
		}

		if(PyBytes_Check(pyVal))
		{
			char *buffer;
			Py_ssize_t length;
			
			if(PyBytes_AsStringAndSize(pyVal, &buffer, &length) < 0)
			{
				SCRIPT_ERROR_CHECK();
				S_Return;
			}

			pyobj->stream().appendBlob(buffer, length);
		}
		else
		{
			PyMemoryStream* obj = static_cast<PyMemoryStream*>(pyVal);
			pyobj->stream().appendBlob(&obj->stream());
		}
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "PyMemoryStream::append: type %s no support!", type);
		PyErr_PrintEx(0);
		S_Return;
	}

	S_Return;	
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__py_pop(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyMemoryStream* pyobj = static_cast<PyMemoryStream*>(self);

	if(pyobj->readonly())
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::pop: read only!");
		PyErr_PrintEx(0);
		S_Return;
	}

	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::pop: args error! arg1 is type[UINT8|STRING|...].");
		PyErr_PrintEx(0);
	}
	
	char* type;
	if(!PyArg_ParseTuple(args, "s", &type))
	{
		PyErr_Format(PyExc_TypeError, "PyMemoryStream::pop: args error!");
		PyErr_PrintEx(0);
		S_Return;
	}

	try
	{
		if(strcmp(type, "UINT8") == 0)
		{
			uint8 v;
			pyobj->stream() >> v;
			return PyLong_FromUnsignedLong(v);
		}
		else if(strcmp(type, "UINT16") == 0)
		{
			uint16 v;
			pyobj->stream() >> v;
			return PyLong_FromUnsignedLong(v);
		}
		else if(strcmp(type, "UINT32") == 0)
		{
			uint32 v;
			pyobj->stream() >> v;
			return PyLong_FromUnsignedLong(v);
		}
		else if(strcmp(type, "UINT64") == 0)
		{
			uint64 v;
			pyobj->stream() >> v;
			return PyLong_FromUnsignedLongLong(v);
		}
		else if(strcmp(type, "INT8") == 0)
		{
			int8 v;
			pyobj->stream() >> v;
			return PyLong_FromLong(v);
		}
		else if(strcmp(type, "INT16") == 0)
		{
			int16 v;
			pyobj->stream() >> v;
			return PyLong_FromLong(v);
		}
		else if(strcmp(type, "INT32") == 0)
		{
			int32 v;
			pyobj->stream() >> v;
			return PyLong_FromLong(v);
		}
		else if(strcmp(type, "INT64") == 0)
		{
			int64 v;
			pyobj->stream() >> v;
			return PyLong_FromLongLong(v);
		}
		else if(strcmp(type, "FLOAT") == 0)
		{
			float v;
			pyobj->stream() >> v;
			return PyFloat_FromDouble(v);
		}
		else if(strcmp(type, "DOUBLE") == 0)
		{
			double v;
			pyobj->stream() >> v;
			return PyFloat_FromDouble(v);
		}
		else if(strcmp(type, "STRING") == 0)
		{
			std::string s;
			pyobj->stream() >> s;
			return PyUnicode_FromString(s.c_str());
			
		}
		else if(strcmp(type, "UNICODE") == 0)
		{
			std::string s;
			pyobj->stream().readBlob(s);

			PyObject* ret = PyUnicode_DecodeUTF8(s.data(), s.length(), NULL);
			if(ret == NULL)
			{
				SCRIPT_ERROR_CHECK();
				PyErr_Format(PyExc_TypeError, "PyMemoryStream::pop: val is not UNICODE!");
				PyErr_PrintEx(0);
				S_Return;
			}

			return ret;
		}
		else if(strcmp(type, "PYTHON") == 0 || strcmp(type, "PY_DICT") == 0
			 || strcmp(type, "PY_TUPLE") == 0  || strcmp(type, "PY_LIST") == 0)
		{
			std::string s;
			pyobj->stream().readBlob(s);
			return Pickler::unpickle(s);
		}
		else if (strcmp(type, "BLOB") == 0)
		{
			std::string s;
			pyobj->stream().readBlob(s);

			PyObject* ret = PyBytes_FromStringAndSize((char*)s.data(), s.length());
			if (ret == NULL)
			{
				SCRIPT_ERROR_CHECK();
				PyErr_Format(PyExc_TypeError, "PyMemoryStream::pop: val is not BLOB!");
				PyErr_PrintEx(0);
				S_Return;
			}

			return ret;
		}
		else
		{
			PyErr_Format(PyExc_TypeError, "PyMemoryStream::pop: type %s no support!", type);
			PyErr_PrintEx(0);
			S_Return;
		}
	}
	catch(MemoryStreamException &e)
	{
		PyErr_Format(PyExc_Exception, "PyMemoryStream::pop: stream error!");
		PyErr_PrintEx(0);
		S_Return;
	}

	S_Return;	
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__py_rpos(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyMemoryStream* pyobj = static_cast<PyMemoryStream*>(self);

	int argCount = PyTuple_Size(args);
	if (argCount > 1)
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::rpos: args error! arg1 is type[UINT32].");
		PyErr_PrintEx(0);
	}
	else if (argCount == 1)
	{
		uint32 pos = 0;
		if (!PyArg_ParseTuple(args, "I", &pos))
		{
			PyErr_Format(PyExc_TypeError, "PyMemoryStream::rpos: args error!");
			PyErr_PrintEx(0);
			S_Return;
		}

		pyobj->stream().rpos(pos);
	}
	else
	{
		return PyLong_FromUnsignedLong(pyobj->stream().rpos());
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__py_wpos(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyMemoryStream* pyobj = static_cast<PyMemoryStream*>(self);

	int argCount = PyTuple_Size(args);
	if (argCount > 1)
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::wpos: args error! arg1 is type[UINT32].");
		PyErr_PrintEx(0);
	}

	if (argCount > 0 && pyobj->readonly())
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::wpos: read only!");
		PyErr_PrintEx(0);
		S_Return;
	}

	if (argCount == 1)
	{
		uint32 pos = 0;
		if (!PyArg_ParseTuple(args, "I", &pos))
		{
			PyErr_Format(PyExc_TypeError, "PyMemoryStream::wpos: args error!");
			PyErr_PrintEx(0);
			S_Return;
		}

		pyobj->stream().wpos(pos);
	}
	else
	{
		return PyLong_FromUnsignedLong(pyobj->stream().wpos());
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* PyMemoryStream::__py_fill(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyMemoryStream* pyobj = static_cast<PyMemoryStream*>(self);

	if (pyobj->readonly())
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::fill: read only!");
		PyErr_PrintEx(0);
		S_Return;
	}

	int argCount = PyTuple_Size(args);
	if (argCount != 1)
	{
		PyErr_Format(PyExc_AssertionError, "PyMemoryStream::fill: args error! arg1 is [bytes|PyMemoryStream].");
		PyErr_PrintEx(0);
	}

	PyObject* pyVal = NULL;

	if (!PyArg_ParseTuple(args, "O", &pyVal))
	{
		PyErr_Format(PyExc_TypeError, "PyMemoryStream::fill: args error!");
		PyErr_PrintEx(0);
		S_Return;
	}


	if (!PyObject_TypeCheck(pyVal, PyMemoryStream::getScriptType()) && !PyBytes_Check(pyVal))
	{
		PyErr_Format(PyExc_TypeError, "PyMemoryStream::fill: val is not [bytes|PyMemoryStream]!");
		PyErr_PrintEx(0);
		S_Return;
	}

	if (PyBytes_Check(pyVal))
	{
		char *buffer;
		Py_ssize_t length;

		if (PyBytes_AsStringAndSize(pyVal, &buffer, &length) < 0)
		{
			SCRIPT_ERROR_CHECK();
			S_Return;
		}

		pyobj->stream().append(buffer, length);
	}
	else
	{
		PyMemoryStream* obj = static_cast<PyMemoryStream*>(pyVal);
		pyobj->stream().append(obj->stream());
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
}
}

