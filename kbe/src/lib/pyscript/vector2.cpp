// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "vector2.h"
#include "vector3.h"
#include "pyscript/py_gc.h"

namespace KBEngine{ namespace script{

const int ScriptVector2::VECTOR_SIZE = sizeof(Vector2) / sizeof(float);

PySequenceMethods ScriptVector2::seqMethods =
{
	ScriptVector2::seq_length,		/* sq_length */
	0,								/* sq_concat */
	0,								/* sq_repeat */
	ScriptVector2::seq_item,		/* sq_item */
	0,//ScriptVector2::seq_slice,		/* sq_slice */
	ScriptVector2::seq_ass_item,	/* sq_ass_item */
	0,								/* sq_ass_slice */
	0,								/* sq_contains */
	0,								/* sq_inplace_concat */
	0,								/* sq_inplace_repeat */
};

PyNumberMethods ScriptVector2::numberMethods = 
{
	ScriptVector2::py_add,				//binaryfunc nb_add;
	ScriptVector2::py_subtract,			//binaryfunc nb_subtract;
	ScriptVector2::py_multiply,			//binaryfunc nb_multiply;
	//ScriptVector2::py_divide,			//binaryfunc nb_divide;
	0,									//binaryfunc nb_remainder;
	0,									//binaryfunc nb_divmod;
	0,									//ternaryfunc nb_power;
	ScriptVector2::py_negative,			//unaryfunc nb_negative;
	ScriptVector2::py_positive,			//unaryfunc nb_positive;
	0,									//unaryfunc nb_absolute;
	ScriptVector2::py_nonzero,			//inquiry nb_nonzero  nb_nonzero重命名为nb_bool,__nonzero__()重命名为__bool__();
	0,									//unaryfunc nb_invert;
	0,									//binaryfunc nb_lshift;
	0,									//binaryfunc nb_rshift;
	0,									//binaryfunc nb_and;
	0,									//binaryfunc nb_xor;
	0,									//binaryfunc nb_or;
	//0,								//coercion nb_coerce;
	0,									//unaryfunc nb_int;
	0,									// void *nb_reserved;
	//0,								//unaryfunc nb_long;
	0,									//unaryfunc nb_float;
	//0,								//unaryfunc nb_oct;
	//0,								//unaryfunc nb_hex;
	ScriptVector2::py_inplace_add,		//binaryfunc nb_inplace_add;
	ScriptVector2::py_inplace_subtract,	//binaryfunc nb_inplace_subtract;
	ScriptVector2::py_inplace_multiply,	//binaryfunc nb_inplace_multiply;
	//ScriptVector2::py_inplace_divide,	//binaryfunc nb_inplace_divide;
	0,									//binaryfunc nb_inplace_remainder;
	0,									//ternaryfunc nb_inplace_power;
	0,									//binaryfunc nb_inplace_lshift;
	0,									//binaryfunc nb_inplace_rshift;
	0,									//binaryfunc nb_inplace_and;
	0,									//binaryfunc nb_inplace_xor;
	0,									//binaryfunc nb_inplace_or;
	// Added in release 2.2
	0,									//binaryfunc nb_floor_divide;
	0,									//binaryfunc nb_true_divide;
	0,									//binaryfunc nb_inplace_floor_divide;
	0,									//binaryfunc nb_inplace_true_divide;
};
/*
static int tp_compare(PyObject* v, PyObject* w)
{
	if (ScriptVector2::check(v) && ScriptVector2::check(w)){
		const Vector2& a = ((ScriptVector2 *)v)->getVector();
		const Vector2& b = ((ScriptVector2 *)w)->getVector();
		return (a < b) ? -1 : (b < a) ? 1 : 0;
	}
	return 0;
}
*/
SCRIPT_METHOD_DECLARE_BEGIN(ScriptVector2)
SCRIPT_METHOD_DECLARE("distTo",							pyDistTo,					METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("distSqrTo",						pyDistSqrTo,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("cross2D",						pyCross2D,					METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("scale",							pyScale,					METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("dot",							pyDot,						METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("normalise",						pyNormalise,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("tuple",							pyTuple,					METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("list",							pyList,						METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("set",							pySet,						METH_VARARGS,		0)
SCRIPT_DIRECT_METHOD_DECLARE("__reduce_ex__",			__reduce_ex__,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ScriptVector2)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptVector2)
SCRIPT_GETSET_DECLARE("x",				pyGetX,							pySetX,				0,		0)
SCRIPT_GETSET_DECLARE("y",				pyGetY,							pySetY,				0,		0)
SCRIPT_GET_DECLARE("length",			pyGetVectorLength,				0,					0)
SCRIPT_GET_DECLARE("lengthSquared",		pyGetVectorLengthSquared,		0,					0)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptVector2, 0, &ScriptVector2::seqMethods, 0, 0, 0)	

//-------------------------------------------------------------------------------------
ScriptVector2::ScriptVector2(Vector2* v):
ScriptObject(getScriptType(), false),
val_(v),
isCopy_(true)
{
	script::PyGC::incTracing("Vector2");
}

//-------------------------------------------------------------------------------------
ScriptVector2::ScriptVector2(Vector2 v):
ScriptObject(getScriptType(), false),
isCopy_(false)
{
	val_ = new Vector2(v);
	script::PyGC::incTracing("Vector2");
}

//-------------------------------------------------------------------------------------
ScriptVector2::ScriptVector2(float x, float y):
ScriptObject(getScriptType(), false),
isCopy_(false)
{
	val_ = new Vector2(x, y);
	script::PyGC::incTracing("Vector2");
}

//-------------------------------------------------------------------------------------
ScriptVector2::~ScriptVector2()
{
	if(!isCopy_)
		delete val_;

	script::PyGC::decTracing("Vector2");
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::tp_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{					
	ScriptVector2* v = new ScriptVector2(0,0);

	if(PyTuple_Size(args) > 0)
	{
		PyObject* pyResult = v->__py_pySet((PyObject*)v, args);

		if(pyResult)
			Py_DECREF(pyResult);
		else
			return NULL;
	}

	return v;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::tp_repr()
{
	char str[128];
	Vector2 v = this->getVector();

	strcpy(str, "Vector2(");
	for(int i=0; i < VECTOR_SIZE; ++i)
	{
		if (i > 0)
			strcat(str, ", ");

		kbe_snprintf(str + strlen(str), 128, "%f", v[i]);
	}

	strcat(str, ")");
	return PyUnicode_FromString(str);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::tp_str()
{
	return tp_repr();
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::pyGetVectorLength()
{ 
	return PyFloat_FromDouble(KBEVec2Length(&getVector())); 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::pyGetVectorLengthSquared()
{ 
	return PyFloat_FromDouble(KBEVec2LengthSq(&getVector()));
}

//-------------------------------------------------------------------------------------
Py_ssize_t ScriptVector2::seq_length(PyObject* self)
{
	ScriptVector2* seq = static_cast<ScriptVector2*>(self);
	return seq->length();
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::seq_item(PyObject* self, Py_ssize_t index)
{
	if (index < 0 || VECTOR_SIZE <= index)
	{
		PyErr_SetString(PyExc_IndexError, "Vector2 index out of range");
		//PyErr_PrintEx(0);
		return NULL;
	}

	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	return PyFloat_FromDouble(sv->getVector()[static_cast<int32>(static_cast<int>(index))]);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::seq_slice(PyObject* self, Py_ssize_t startIndex, Py_ssize_t endIndex)
{
	if(startIndex < 0)
		startIndex = 0;

	if(endIndex > VECTOR_SIZE)
		endIndex = VECTOR_SIZE;

	if(endIndex < startIndex)
		endIndex = startIndex;

	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& my_v = sv->getVector();
	PyObject* pyResult = NULL;

	int length = (int)(endIndex - startIndex);

	if (length == VECTOR_SIZE)
	{
		pyResult = sv;
		Py_INCREF(pyResult);
	}
	else
		switch(length)
		{
			case 0:
				pyResult = PyTuple_New(0);
				break;
			case 1:
				pyResult = PyTuple_New(1);
				PyTuple_SET_ITEM(pyResult, 0, PyFloat_FromDouble(sv->getVector()[static_cast<int>(startIndex)]));
				break;
			case 2:
			{
				Vector2 v;
				
				for (int i = (int)startIndex; i < (int)endIndex; ++i){
					v[i - static_cast<int>(startIndex)] = my_v[i];
				}

				pyResult = new ScriptVector2(v);
				break;
			}
			case 3:
			{
				Vector3 v;
				for (int i = (int)startIndex; i < (int)endIndex; ++i){
					v[i - static_cast<int>(startIndex)] = my_v[i];
				}

				pyResult = new ScriptVector3(v);
				break;
			}
			default:
				PyErr_Format(PyExc_IndexError, "Bad slice indexes [%d, %d] for Vector%d", startIndex, endIndex, VECTOR_SIZE);
				PyErr_PrintEx(0);
				break;
		}

	return pyResult;
}

//-------------------------------------------------------------------------------------
int ScriptVector2::seq_ass_item(PyObject* self, Py_ssize_t index, PyObject* value)
{
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);

	if (index < 0 || VECTOR_SIZE <= index)
	{
		PyErr_SetString(PyExc_IndexError, "Vector assignment index out of range");
		PyErr_PrintEx(0);
		return -1;
	}

	Vector2& v = sv->getVector();
	v[static_cast<int>(index)] = float(PyFloat_AsDouble(value));
	return 0;
}

//-------------------------------------------------------------------------------------
int ScriptVector2::pySetX(PyObject *value)
{ 
	getVector().x = float(PyFloat_AsDouble(value)); 
	return 0; 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::pyGetX()
{ 
	return PyFloat_FromDouble(getVector().x); 
}

//-------------------------------------------------------------------------------------
int ScriptVector2::pySetY(PyObject *value)
{ 
	getVector().y = float(PyFloat_AsDouble(value)); 
	return 0; 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::pyGetY()
{ 
	return PyFloat_FromDouble(getVector().y); 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__reduce_ex__(PyObject* self, PyObject* protocol)
{
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Vector2");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	PyObject* args1 = PyTuple_New(VECTOR_SIZE);
	PyTuple_SET_ITEM(args1, 0, PyFloat_FromDouble(v.x));
	PyTuple_SET_ITEM(args1, 1, PyFloat_FromDouble(v.y));
	PyTuple_SET_ITEM(args, 1, args1);
	
	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}
	return args;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__unpickle__(PyObject* self, PyObject* args)
{
	float x, y;
	Py_ssize_t size = PyTuple_Size(args);
	if(size != VECTOR_SIZE)
	{
		ERROR_MSG("ScriptVector2::__unpickle__: args error! size != 2");
		S_Return;
	}

	if(!PyArg_ParseTuple(args, "ff", &x, &y))
	{
		ERROR_MSG("ScriptVector2::__unpickle__: args error!");
		S_Return;
	}

	return new ScriptVector2(x, y);
}

//-------------------------------------------------------------------------------------
void ScriptVector2::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method = {"Vector2", (PyCFunction)&ScriptVector2::__unpickle__, METH_VARARGS, 0};
	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "Vector2");
	Py_DECREF(pyFunc);
	ScriptVector2::_scriptType.tp_as_number = &ScriptVector2::numberMethods;
	// ScriptVector2::_scriptType.tp_reserved = tp_compare;
	ScriptVector2::_scriptType.tp_name = "Vector2";
}

//-------------------------------------------------------------------------------------
bool ScriptVector2::check(PyObject* value, bool isPrintErr)
{
	if(value == NULL || PySequence_Check(value) <= 0)
	{
		if(isPrintErr)
		{
			PyErr_Format(PyExc_TypeError, "ScriptVector2::check(): args is must a sequence.");
			PyErr_PrintEx(0);
		}

		return false;
	}

	Py_ssize_t size = PySequence_Size(value);
	if(size != VECTOR_SIZE)
	{
		if(isPrintErr)
		{
			PyErr_Format(PyExc_TypeError, "ScriptVector2::check(): len(args) != %d. can't set.", VECTOR_SIZE);
			PyErr_PrintEx(0);
		}

		return false;
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool ScriptVector2::convertPyObjectToVector2(Vector2& v, PyObject* obj)
{
	if(!check(obj))
		return false;

	PyObject* pyItem = PySequence_GetItem(obj, 0);
	v.x = float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	pyItem = PySequence_GetItem(obj, 1);
	v.y = float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_add(PyObject *a, PyObject *b)
{
	if(!check(a) || !check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector2 av;
	Vector2 bv;

	convertPyObjectToVector2(av, a);
	convertPyObjectToVector2(bv, b);
	return new ScriptVector2(av + bv);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_subtract(PyObject *a, PyObject *b)
{
	if(!check(a) || !check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector2 av;
	Vector2 bv;

	convertPyObjectToVector2(av, a);
	convertPyObjectToVector2(bv, b);
	return new ScriptVector2(av - bv);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_multiply(PyObject *a, PyObject *b)
{
	float f;
	
	if(check(a))
	{
		ScriptVector2* sv = static_cast<ScriptVector2*>(a);
		f = float(PyFloat_AsDouble(b));
		return new ScriptVector2(sv->getVector() * f);
	}
	else if(check(b))
	{
		ScriptVector2* sv = static_cast<ScriptVector2*>(b);
		f = float(PyFloat_AsDouble(a));
		return new ScriptVector2(sv->getVector() * f);
	}

	PyErr_Clear();
	Py_INCREF( Py_NotImplemented );
	return Py_NotImplemented;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_divide(PyObject *a, PyObject *b)
{
	if(!check(a))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}

	Vector2 av;
	convertPyObjectToVector2(av, a);
	float f = float(PyFloat_AsDouble(b));
	return new ScriptVector2(av / f);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_negative(PyObject *self)
{
	//ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_positive(PyObject *self)
{
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	return new ScriptVector2(sv->getVector() * -1.f);
}

//-------------------------------------------------------------------------------------
int ScriptVector2::py_nonzero(PyObject *self)
{
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	// 点乘
	Vector2 v = sv->getVector();
	float val = v.x * v.x + v.y * v.y;
	return val > 0.f;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_inplace_add(PyObject *self, PyObject *b)
{
	if(!check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector2 bv;
	convertPyObjectToVector2(bv, b);
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	v += bv;
	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_inplace_subtract(PyObject *self, PyObject *b)
{
	if(!check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector2 bv;
	convertPyObjectToVector2(bv, b);
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	v -= bv;
	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_inplace_multiply(PyObject *self, PyObject *b)
{
	float value = float(PyFloat_AsDouble(b));
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	sv->setVector(sv->getVector() * value);
	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::py_inplace_divide(PyObject *self, PyObject *b)
{
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	float f = float(PyFloat_AsDouble(b));
	v /= f;

	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyDistTo(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "args > 1 error!\n");
		PyErr_PrintEx(0);
		S_Return;
	}
	
	PyObject* pyVal = PyTuple_GET_ITEM(args, 0);
	if(!check(pyVal))
	{
		S_Return;
	}

	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	
	Vector2 v1;
	convertPyObjectToVector2(v1, pyVal);
	
	Vector2 rv = (v - v1);
	return PyFloat_FromDouble(KBEVec2Length(&rv)); //计算长度并返回
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyDistSqrTo(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "args > 1 error!\n");
		PyErr_PrintEx(0);
		S_Return;
	}
	
	PyObject* pyVal = PyTuple_GET_ITEM(args, 0);
	if(!check(pyVal))
	{
		S_Return;
	}

	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	
	Vector2 v1;
	convertPyObjectToVector2(v1, pyVal);
	
	Vector2 rv = (v - v1);
	return PyFloat_FromDouble(KBEVec2LengthSq(&rv)); //计算点乘并返回
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyCross2D(PyObject* self, PyObject* args)
{
	ScriptVector2* v = new ScriptVector2(0,0);
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);

	PyObject* pyResult = v->__py_pySet((PyObject*)v, args);
	if(pyResult)
		Py_DECREF(pyResult);

	const Vector2 otherV = v->getVector();
	const Vector2 thisV = sv->getVector();
	float result = thisV[0] * otherV[1] - thisV[1] * otherV[0];
	Py_DECREF(v);
	return PyFloat_FromDouble(result);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyScale(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) == 1)
	{
		ScriptVector2* sv = static_cast<ScriptVector2*>(self);
		Vector2& v = sv->getVector();
		PyObject* pItem = PyTuple_GetItem(args, 0);
		return new ScriptVector2(v * float(PyFloat_AsDouble(pItem)));
	}

	PyErr_SetString(PyExc_TypeError, "Vector.scale expects a float argument");
	PyErr_PrintEx(0);
	return NULL;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyDot(PyObject* self, PyObject* args)
{
	ScriptVector2* v = new ScriptVector2(0,0);

	PyObject* pyResult = v->__py_pySet((PyObject*)v, args);
	if(pyResult)
		Py_DECREF(pyResult);

	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	float result = KBEVec2Dot(const_cast<Vector2*>(&sv->getVector()), const_cast<Vector2*>(&v->getVector()));
	Py_DECREF(v);
	return PyFloat_FromDouble(result);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyNormalise(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Vector.normalise takes no arguments(nor does it brook any dissent :)");
		PyErr_PrintEx(0);
		return NULL;
	}

	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	KBEVec2Normalize(&v, &v);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyTuple(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Vector.tuple takes no arguments");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyTup = PyTuple_New(VECTOR_SIZE);
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();

	for(int i = 0; i < VECTOR_SIZE; ++i)
		PyTuple_SetItem(pyTup, i, PyFloat_FromDouble(v[i]));

	return pyTup;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pyList(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Vector.tuple takes no arguments");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyList = PyList_New(VECTOR_SIZE);
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	Vector2& v = sv->getVector();
	
	for (int i=0; i < VECTOR_SIZE; ++i)
		PyList_SetItem(pyList, i, PyFloat_FromDouble(v[i]));

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector2::__py_pySet(PyObject* self, PyObject* args)
{
	ScriptVector2* sv = static_cast<ScriptVector2*>(self);
	bool good = false;
	Vector2 v;

	// 如果参数只有1个元素
	int tupleSize = (int)PyTuple_Size(args);
	if(tupleSize == 1)
	{
		PyObject* pyItem = PyTuple_GetItem(args, 0);

		if(ScriptVector2::check(pyItem, false))
		{
			convertPyObjectToVector2(v, pyItem);
			good = true;
		}
		else
		{
			float f = float(PyFloat_AsDouble(pyItem));
			for (int i=0; i < VECTOR_SIZE; ++i)
			{
				v[i] = f;
			}
			good = true;
		}
	}
	else if(tupleSize >= VECTOR_SIZE)
	{
		convertPyObjectToVector2(v, args);
		good = true;
	}

	if(!good)
	{
		PyErr_Format(PyExc_TypeError, "Vector.set must be set to a tuple of %d floats, or one float", VECTOR_SIZE);
		PyErr_PrintEx(0);
		return NULL;
	}

	sv->setVector(v);
	S_Return;
}

//-------------------------------------------------------------------------------------

}
}
