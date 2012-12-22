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

#include "blob.hpp"
#include "datatypes.hpp"

namespace KBEngine{ 

SCRIPT_METHOD_DECLARE_BEGIN(Blob)
SCRIPT_METHOD_DECLARE("__reduce_ex__",	reduce_ex__,		METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Blob)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Blob)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(Blob, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Blob::Blob(std::string& strDictInitData):
PyMemoryStream(getScriptType(), false)
{
	initialize(strDictInitData);
}

//-------------------------------------------------------------------------------------
Blob::Blob(PyObject* pyDictInitData):
PyMemoryStream(getScriptType(), false)
{
	initialize(pyDictInitData);
}

//-------------------------------------------------------------------------------------
Blob::Blob(MemoryStream* streamInitData):
PyMemoryStream(getScriptType(), false)
{
	initialize(streamInitData);
}

//-------------------------------------------------------------------------------------
Blob::Blob():
PyMemoryStream(getScriptType(), false)
{
	initialize("");
}

//-------------------------------------------------------------------------------------
Blob::~Blob()
{
}

//-------------------------------------------------------------------------------------
void Blob::initialize(std::string strDictInitData)
{
	if(strDictInitData.size() > 0)
		stream_.append(strDictInitData.data(), strDictInitData.size());
}

//-------------------------------------------------------------------------------------
void Blob::initialize(PyObject* pyBytesInitData)
{
	char *buffer;
	Py_ssize_t length;

	if(PyBytes_AsStringAndSize(pyBytesInitData, &buffer, &length) < 0)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	if(length > 0)
		stream_.append(buffer, length);
}

//-------------------------------------------------------------------------------------
void Blob::initialize(MemoryStream* streamInitData)
{
	if(streamInitData)
		stream_ = *streamInitData;
}

//-------------------------------------------------------------------------------------
PyObject* Blob::__py_reduce_ex__(PyObject* self, PyObject* protocol)
{
	Blob* blob = static_cast<Blob*>(self);
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Blob");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	PyObject* args1 = PyTuple_New(3);

	PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(blob->stream().rpos()));
	PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLong(blob->stream().wpos()));
	PyTuple_SET_ITEM(args1, 2, blob->pyBytes());

	PyTuple_SET_ITEM(args, 1, args1);

	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}
	return args;
}

//-------------------------------------------------------------------------------------
PyObject* Blob::__unpickle__(PyObject* self, PyObject* args)
{
	Py_ssize_t size = PyTuple_Size(args);
	if(size != 3)
	{
		ERROR_MSG("Blob::__unpickle__: args is error! size != 3");
		S_Return;
	}

	PyObject* pyRpos = PyTuple_GET_ITEM(args, 0);
	PyObject* pyWpos = PyTuple_GET_ITEM(args, 1);

	PyObject* pybytes = PyTuple_GET_ITEM(args, 2);
	if(pybytes == NULL)
	{
		ERROR_MSG("Blob::__unpickle__: args is error!");
		S_Return;
	}
	
	Blob* pBlob = new Blob(pybytes);
	pBlob->stream().rpos(PyLong_AsUnsignedLong(pyRpos));
	pBlob->stream().wpos(PyLong_AsUnsignedLong(pyWpos));
	return pBlob;
}

//-------------------------------------------------------------------------------------
void Blob::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method = 
		{"Blob", (PyCFunction)&Blob::__unpickle__, METH_VARARGS, 0};

	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "Blob");
	Py_DECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
PyObject* Blob::py_new()
{
	return new Blob();
}

//-------------------------------------------------------------------------------------
}
