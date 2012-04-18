#include "array.hpp"

namespace KBEngine{ 

SCRIPT_METHOD_DECLARE_BEGIN(Array)
SCRIPT_METHOD_DECLARE("__reduce_ex__", __reduce_ex__, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("append", _append, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("count", _count, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("extend", _extend, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("index", _index, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("insert", _insert, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("pop", _pop, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE("remove", _remove, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(Array)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Array)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(Array, 0, &Sequence::seqMethods, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Array::Array(DataType* dataType, std::string& strInitData):
Sequence(getScriptType(), false)
{
	_dataType = static_cast<ArrayType*>(dataType);
	_dataType->incRef();
	initialize(strInitData);
}

//-------------------------------------------------------------------------------------
Array::Array(DataType* dataType):
Sequence(getScriptType(), false)
{
	_dataType = static_cast<ArrayType*>(dataType);
	_dataType->incRef();
	initialize("");
}


//-------------------------------------------------------------------------------------
Array::~Array()
{
	_dataType->decRef();
}

//-------------------------------------------------------------------------------------
void Array::initialize(std::string strInitData)
{
}

//-------------------------------------------------------------------------------------
PyObject* Array::__reduce_ex__(PyObject* self, PyObject* protocol)
{
	Array* arr = static_cast<Array*>(self);
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Array");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	int len = arr->length();
	PyObject* args1 = PyTuple_New(1);
	PyObject* pyList = PyList_New(len);

	if(len > 0)
	{
		std::vector<PyObject*>& values = arr->getValues();
		for(int i=0; i<len; i++)
			PyList_SET_ITEM(pyList, i, values[i]);
	}

	PyTuple_SET_ITEM(args1, 0, pyList);
	PyTuple_SET_ITEM(args, 1, args1);

	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}
	return args;
}

//-------------------------------------------------------------------------------------
PyObject* Array::__unpickle__(PyObject* self, PyObject* args)
{
	Py_ssize_t size = PyTuple_Size(args);
	if(size != 1)
	{
		ERROR_MSG("Array::__unpickle__: args is error! size != 1");
		S_Return;
	}
	
	PyObject* pyList = PyTuple_GET_ITEM(args, 0);
	if(pyList == NULL)
	{
		ERROR_MSG("Array::__unpickle__: args is error!");
		S_Return;
	}

	Py_INCREF(pyList);
	return pyList;
}

//-------------------------------------------------------------------------------------
void Array::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method = {"Array", (PyCFunction)&Array::__unpickle__, METH_VARARGS, 0};
	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "Array");
	Py_DECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
bool Array::isSameType(PyObject* pyValue)
{
	return _dataType->isSameType(pyValue);
}

//-------------------------------------------------------------------------------------
PyObject* Array::_append(PyObject* self, PyObject* args, PyObject* kwargs)
{
	Array* ary = static_cast<Array*>(self);
	uint32 seq_size = ary->length();
	return PyBool_FromLong(seq_ass_slice(self, seq_size, seq_size, &*args) == 0);	
}

//-------------------------------------------------------------------------------------
PyObject* Array::_count(PyObject* self, PyObject* args, PyObject* kwargs)
{
	Array* ary = static_cast<Array*>(self);
	PyObject* pyItem = PyTuple_GetItem(args, 0);
	int count = 0, cur;
	for (uint32 i = 0; (cur = ary->findFrom(i, &*pyItem)) >= 0; i = cur + 1)
		++count;
	return PyLong_FromLong(count);	
}

//-------------------------------------------------------------------------------------
PyObject* Array::_extend(PyObject* self, PyObject* args, PyObject* kwargs)
{
	Array* ary = static_cast<Array*>(self);
	uint32 seq_size = ary->length();
	PyObject* pyItem = PyTuple_GetItem(args, 0);
	return PyBool_FromLong(seq_ass_slice(self, seq_size, seq_size, &*pyItem) == 0);	
}

//-------------------------------------------------------------------------------------
PyObject* Array::_index(PyObject* self, PyObject* args, PyObject* kwargs)
{
	Array* ary = static_cast<Array*>(self);
	PyObject* pyItem = PyTuple_GetItem(args, 0);
	int index = ary->findFrom(0, &*pyItem);
	if (index == -1)
	{
		PyErr_SetString(PyExc_ValueError, "Array::index: value not found");
		return NULL;
	}
	return PyLong_FromLong(index);
}

//-------------------------------------------------------------------------------------
PyObject* Array::_insert(PyObject* self, PyObject* args, PyObject* kwargs)
{
	int before = PyLong_AsLong(PyTuple_GetItem(args, 0));
	PyObject* pyobj = PyTuple_GetItem(args, 1);
	
	//Array* ary = static_cast<Array*>(self);
	PyObject* pyTuple = PyTuple_New(1);
	PyTuple_SET_ITEM(&*pyTuple, 0, pyobj);

	const int argsize = PyTuple_Size(args);
	if(argsize > 2)
	{
		PyErr_SetString(PyExc_ValueError, "Array::insert: args is error!");
		return NULL;
	}
	
	return PyBool_FromLong(seq_ass_slice(self, before, before, &*pyTuple) == 0);
}

//-------------------------------------------------------------------------------------
PyObject* Array::_pop(PyObject* self, PyObject* args, PyObject* kwargs)
{
	Array* ary = static_cast<Array*>(self);
	std::vector<PyObject*>& values = ary->getValues();
	
	if (values.empty())
	{
		PyErr_SetString(PyExc_IndexError, "Array.pop: empty array");
		return NULL;
	}

	PyObject* pyItem = PyTuple_GetItem(args, 0);
	int index = PyLong_AsLong(pyItem);
	if (index < 0) index += values.size();
	if (uint32(index) >= values.size())
	{
		PyErr_SetString(PyExc_IndexError, "Array.pop: index out of range");
		return NULL;
	}

	PyObject* pyValue = values[index];
	PyObject* pyTuple = PyTuple_New(0);
	if (seq_ass_slice(self, index, index + 1, &*pyTuple) != 0)
		return NULL;

	Py_INCREF(pyValue);
	return pyValue;
}

//-------------------------------------------------------------------------------------
PyObject* Array::_remove(PyObject* self, PyObject* args, PyObject* kwargs)
{
	Array* ary = static_cast<Array*>(self);
	PyObject* pyItem = PyTuple_GetItem(args, 0);
	int index = ary->findFrom(0, &*pyItem);
	if (index == -1)
	{
		PyErr_SetString(PyExc_ValueError, "Array.remove: value not found");
		return PyLong_FromLong(-1);
	}

	PyObject* pyTuple = PyTuple_New(0);
	return PyBool_FromLong(seq_ass_slice(self, index, index + 1, &*pyTuple) == 0);
}

//-------------------------------------------------------------------------------------
}
