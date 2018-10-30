// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "vector3.h"
#include "vector2.h"
#include "pyscript/py_gc.h"

namespace KBEngine{ namespace script{

const int ScriptVector3::VECTOR_SIZE = sizeof(Vector3) / sizeof(float);

PySequenceMethods ScriptVector3::seqMethods =
{
	ScriptVector3::seq_length,			/* sq_length */
	0,									/* sq_concat */
	0,									/* sq_repeat */
	ScriptVector3::seq_item,			/* sq_item */
	0,//ScriptVector3::seq_slice,			/* sq_slice */
	ScriptVector3::seq_ass_item,		/* sq_ass_item */
	0,									/* sq_ass_slice */
	0,									/* sq_contains */
	0,									/* sq_inplace_concat */
	0,									/* sq_inplace_repeat */
};

PyNumberMethods ScriptVector3::numberMethods = 
{
	ScriptVector3::py_add,				//binaryfunc nb_add;
	ScriptVector3::py_subtract,			//binaryfunc nb_subtract;
	ScriptVector3::py_multiply,			//binaryfunc nb_multiply;
	//ScriptVector3::py_divide,			//binaryfunc nb_divide;
	0,									//binaryfunc nb_remainder;
	0,									//binaryfunc nb_divmod;
	0,									//ternaryfunc nb_power;
	ScriptVector3::py_negative,			//unaryfunc nb_negative;
	ScriptVector3::py_positive,			//unaryfunc nb_positive;
	0,									//unaryfunc nb_absolute;
	ScriptVector3::py_nonzero,			//inquiry nb_nonzero  nb_nonzero重命名为nb_bool,__nonzero__()重命名为__bool__();
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
	ScriptVector3::py_inplace_add,		//binaryfunc nb_inplace_add;
	ScriptVector3::py_inplace_subtract,	//binaryfunc nb_inplace_subtract;
	ScriptVector3::py_inplace_multiply,	//binaryfunc nb_inplace_multiply;
	//ScriptVector3::py_inplace_divide,	//binaryfunc nb_inplace_divide;
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
	if (ScriptVector3::check(v) && ScriptVector3::check(w)){
		const Vector3& a = ((ScriptVector3 *)v)->getVector();
		const Vector3& b = ((ScriptVector3 *)w)->getVector();
		return (a < b) ? -1 : (b < a) ? 1 : 0;
	}
	return 0;
}
*/
SCRIPT_METHOD_DECLARE_BEGIN(ScriptVector3)
SCRIPT_METHOD_DECLARE("flatDistTo",						pyFlatDistTo,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("flatDistSqrTo",					pyFlatDistSqrTo,			METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("distTo",							pyDistTo,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("distSqrTo",						pyDistSqrTo,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("cross2D",						pyCross2D,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("scale",							pyScale,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("dot",							pyDot,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("normalise",						pyNormalise,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("tuple",							pyTuple,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("list",							pyList,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("set",							pySet,						METH_VARARGS,				0)
SCRIPT_DIRECT_METHOD_DECLARE("__reduce_ex__",			__reduce_ex__,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ScriptVector3)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptVector3)
SCRIPT_GETSET_DECLARE("x",								pyGetX,						pySetX,						0,		0)
SCRIPT_GETSET_DECLARE("y",								pyGetY,						pySetY,						0,		0)
SCRIPT_GETSET_DECLARE("z",								pyGetZ,						pySetZ,						0,		0)
SCRIPT_GET_DECLARE("length",							pyGetVectorLength,			0,							0)
SCRIPT_GET_DECLARE("lengthSquared",						pyGetVectorLengthSquared,	0,							0)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptVector3, 0, &ScriptVector3::seqMethods, 0, 0, 0)	

//-------------------------------------------------------------------------------------
ScriptVector3::ScriptVector3(Vector3* v, PYVector3ChangedCallback* pyVector3ChangedCallback):
ScriptObject(getScriptType(), false),
val_(v),
isRef_(true),
_pyVector3ChangedCallback(pyVector3ChangedCallback)
{
	script::PyGC::incTracing("Vector3");
}

//-------------------------------------------------------------------------------------
ScriptVector3::ScriptVector3(Vector3 v):
ScriptObject(getScriptType(), false),
isRef_(false),
_pyVector3ChangedCallback(NULL)
{
	val_ = new Vector3(v);
	script::PyGC::incTracing("Vector3");
}

//-------------------------------------------------------------------------------------
ScriptVector3::ScriptVector3(float x, float y, float z):
ScriptObject(getScriptType(), false),
isRef_(false),
_pyVector3ChangedCallback(NULL)
{
	val_ = new Vector3(x, y, z);
	script::PyGC::incTracing("Vector3");
}

//-------------------------------------------------------------------------------------
ScriptVector3::~ScriptVector3()
{
	if(!isRef_)
		delete val_;

	_pyVector3ChangedCallback = NULL;
	script::PyGC::decTracing("Vector3");
}

//-------------------------------------------------------------------------------------
void ScriptVector3::onLoseRef()
{
	setPYVector3ChangedCallback(NULL);
	isRef_ = false;
	val_ = new Vector3((*val_));
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::tp_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{					
	ScriptVector3* v = new ScriptVector3(0,0,0);

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
PyObject* ScriptVector3::tp_repr()
{
	char str[128];
	Vector3 v = this->getVector();

	strcpy(str, "Vector3(");
	for(int i=0; i < VECTOR_SIZE; ++i)
	{
		if (i > 0)
			strcat(str, ", ");
		kbe_snprintf(str + strlen(str), 128, "%f", v[i]);
	}

	strcat(str, "), ref=");
	
	if(isRef_)
		strcat(str, "true");
	else
		strcat(str, "false");

	return PyUnicode_FromString(str);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::tp_str()
{
	return tp_repr();
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::pyGetVectorLength()
{ 
	return PyFloat_FromDouble(KBEVec3Length(&getVector())); 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::pyGetVectorLengthSquared()
{ 
	return PyFloat_FromDouble(KBEVec3LengthSq(&getVector()));
}

//-------------------------------------------------------------------------------------
Py_ssize_t ScriptVector3::seq_length(PyObject* self)
{
	ScriptVector3* seq = static_cast<ScriptVector3*>(self);
	return seq->length();
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::seq_item(PyObject* self, Py_ssize_t index)
{
	if (index < 0 || VECTOR_SIZE <= index)
	{
		PyErr_SetString(PyExc_IndexError, "Vector3 index out of range");
		//PyErr_PrintEx(0);
		return NULL;
	}

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	return PyFloat_FromDouble(sv->getVector()[static_cast<int>(index)]);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::seq_slice(PyObject* self, Py_ssize_t startIndex, Py_ssize_t endIndex)
{
	if(startIndex < 0)
		startIndex = 0;

	if(endIndex > VECTOR_SIZE)
		endIndex = VECTOR_SIZE;

	if(endIndex < startIndex)
		endIndex = startIndex;

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& my_v = sv->getVector();
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
int ScriptVector3::seq_ass_item(PyObject* self, Py_ssize_t index, PyObject* value)
{
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);

	if (index < 0 || VECTOR_SIZE <= index)
	{
		PyErr_SetString(PyExc_IndexError, "Vector assignment index out of range");
		PyErr_PrintEx(0);
		return -1;
	}

	Vector3& v = sv->getVector();
	v[static_cast<int>(index)] = float(PyFloat_AsDouble(value));
	return 0;
}

//-------------------------------------------------------------------------------------
int ScriptVector3::pySetX(PyObject *value)
{ 
	getVector().x = float(PyFloat_AsDouble(value)); 
	onPyPositionChanged();
	return 0; 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::pyGetX()
{ 
	return PyFloat_FromDouble(getVector().x); 
}

//-------------------------------------------------------------------------------------
int ScriptVector3::pySetY(PyObject *value)
{ 
	getVector().y = float(PyFloat_AsDouble(value)); 
	onPyPositionChanged();
	return 0; 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::pyGetY()
{ 
	return PyFloat_FromDouble(getVector().y); 
}

//-------------------------------------------------------------------------------------
int ScriptVector3::pySetZ(PyObject *value)
{
	getVector().z = float(PyFloat_AsDouble(value)); 
	onPyPositionChanged();
	return 0; 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::pyGetZ()
{ 
	return PyFloat_FromDouble(getVector().z); 
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__reduce_ex__(PyObject* self, PyObject* protocol)
{
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Vector3");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	PyObject* args1 = PyTuple_New(VECTOR_SIZE);
	PyTuple_SET_ITEM(args1, 0, PyFloat_FromDouble(v.x));
	PyTuple_SET_ITEM(args1, 1, PyFloat_FromDouble(v.y));
	PyTuple_SET_ITEM(args1, 2, PyFloat_FromDouble(v.z));
	PyTuple_SET_ITEM(args, 1, args1);
	
	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}

	return args;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__unpickle__(PyObject* self, PyObject* args)
{
	float x, y, z;
	Py_ssize_t size = PyTuple_Size(args);
	if(size != VECTOR_SIZE)
	{
		ERROR_MSG("ScriptVector3::__unpickle__: args error! size != 3");
		S_Return;
	}

	if(!PyArg_ParseTuple(args, "fff", &x, &y, &z))
	{
		ERROR_MSG("ScriptVector3::__unpickle__: args error!");
		S_Return;
	}

	return new ScriptVector3(x, y, z);
}

//-------------------------------------------------------------------------------------
void ScriptVector3::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method = {"Vector3", (PyCFunction)&ScriptVector3::__unpickle__, METH_VARARGS, 0};
	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "Vector3");
	Py_DECREF(pyFunc);
	ScriptVector3::_scriptType.tp_as_number = &ScriptVector3::numberMethods;
	//ScriptVector3::_scriptType.tp_compare = tp_compare;
	ScriptVector3::_scriptType.tp_name = "Vector3";
}

//-------------------------------------------------------------------------------------
bool ScriptVector3::check(PyObject* value, bool isPrintErr)
{
	if(value == NULL || PySequence_Check(value) <= 0)
	{
		if(isPrintErr)
		{
			PyErr_Format(PyExc_TypeError, "ScriptVector3::check(): args is must a sequence.");
			PyErr_PrintEx(0);
		}

		return false;
	}

	Py_ssize_t size = PySequence_Size(value);
	if(size != VECTOR_SIZE)
	{
		if(isPrintErr)
		{
			PyErr_Format(PyExc_TypeError, "ScriptVector3::check(): len(args) != %d. can't set.", VECTOR_SIZE);
			PyErr_PrintEx(0);
		}

		return false;
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool ScriptVector3::convertPyObjectToVector3(Vector3& v, PyObject* obj)
{
	if(!check(obj))
		return false;

	PyObject* pyItem = PySequence_GetItem(obj, 0);
	v.x = float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	pyItem = PySequence_GetItem(obj, 1);
	v.y = float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	pyItem = PySequence_GetItem(obj, 2);
	v.z = float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_add(PyObject *a, PyObject *b)
{
	if(!check(a) || !check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector3 av;
	Vector3 bv;

	convertPyObjectToVector3(av, a);
	convertPyObjectToVector3(bv, b);
	return new ScriptVector3(av + bv);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_subtract(PyObject *a, PyObject *b)
{
	if(!check(a) || !check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector3 av;
	Vector3 bv;

	convertPyObjectToVector3(av, a);
	convertPyObjectToVector3(bv, b);
	return new ScriptVector3(av - bv);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_multiply(PyObject *a, PyObject *b)
{
	if(!check(a))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}

	Vector3 av;
	convertPyObjectToVector3(av, a);
	float f = float(PyFloat_AsDouble(b));
	return new ScriptVector3(av * f);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_divide(PyObject *a, PyObject *b)
{
	if(!check(a))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}

	Vector3 av;
	convertPyObjectToVector3(av, a);
	float f = float(PyFloat_AsDouble(b));
	return new ScriptVector3(av / f);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_negative(PyObject *self)
{
	//ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_positive(PyObject *self)
{
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	return new ScriptVector3(sv->getVector() * -1.f);
}

//-------------------------------------------------------------------------------------
int ScriptVector3::py_nonzero(PyObject *self)
{
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	// 点乘
	Vector3 v = sv->getVector();
	float val = v.x * v.x + v.y * v.y + v.z * v.z;
	return val > 0.f;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_inplace_add(PyObject *self, PyObject *b)
{
	if(!check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector3 bv;
	convertPyObjectToVector3(bv, b);
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	v += bv;
	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_inplace_subtract(PyObject *self, PyObject *b)
{
	if(!check(b))
	{
		PyErr_Clear();
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	
	Vector3 bv;
	convertPyObjectToVector3(bv, b);
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	v -= bv;
	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_inplace_multiply(PyObject *self, PyObject *b)
{
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	
	if (check(b, false))
	{
		Vector3 bv;
		convertPyObjectToVector3(bv, b);
	
		// 叉乘
		v.x = (v.y * bv.z) - (v.z * bv.y);
		v.y = (v.z * bv.x) - (v.x * bv.z);
		v.z = (v.x * bv.y) - (v.y * bv.x);
	}
	else
	{
		float f = float(PyFloat_AsDouble(b));
		v *= f;
	}

	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::py_inplace_divide(PyObject *self, PyObject *b)
{
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	float f = float(PyFloat_AsDouble(b));
	v /= f;

	Py_INCREF(sv);
	return sv;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyFlatDistSqrTo(PyObject* self, PyObject* args)
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

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	
	Vector3 v1;
	convertPyObjectToVector3(v1, pyVal);
	
	float x = v.x - v1.x;
	float z = v.z - v1.z;
	return PyFloat_FromDouble(x*x + z*z);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyFlatDistTo(PyObject* self, PyObject* args)
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

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	
	Vector3 v1;
	convertPyObjectToVector3(v1, pyVal);
	return PyFloat_FromDouble(KBEVec3CalcVec2Length(v, v1));
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyDistTo(PyObject* self, PyObject* args)
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

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	
	Vector3 v1;
	convertPyObjectToVector3(v1, pyVal);
	
	Vector3 rv = (v - v1);
	return PyFloat_FromDouble(KBEVec3Length(&rv)); //计算长度并返回
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyDistSqrTo(PyObject* self, PyObject* args)
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

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	
	Vector3 v1;
	convertPyObjectToVector3(v1, pyVal);
	
	Vector3 rv = (v - v1);
	return PyFloat_FromDouble(KBEVec3LengthSq(&rv)); //计算点乘并返回
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyCross2D(PyObject* self, PyObject* args)
{
	ScriptVector3* v = new ScriptVector3(0,0,0);
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);

	PyObject* pyResult = v->__py_pySet((PyObject*)v, args);
	if(pyResult)
		Py_DECREF(pyResult);

	const Vector3 otherV = v->getVector();
	const Vector3 thisV = sv->getVector();
	float result = thisV[0] * otherV[2] - thisV[2] * otherV[0];
	Py_DECREF(v);
	return PyFloat_FromDouble(result);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyScale(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) == 1)
	{
		ScriptVector3* sv = static_cast<ScriptVector3*>(self);
		Vector3& v = sv->getVector();
		PyObject* pItem = PyTuple_GetItem(args, 0);
		return new ScriptVector3(v * float(PyFloat_AsDouble(pItem)));
	}

	PyErr_SetString(PyExc_TypeError, "Vector.scale expects a float argument");
	PyErr_PrintEx(0);
	return NULL;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyDot(PyObject* self, PyObject* args)
{
	ScriptVector3* v = new ScriptVector3(0,0,0);

	PyObject* pyResult = v->__py_pySet((PyObject*)v, args);
	if(pyResult)
		Py_DECREF(pyResult);

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	float result = KBEVec3Dot(const_cast<Vector3*>(&sv->getVector()), const_cast<Vector3*>(&v->getVector()));
	Py_DECREF(v);
	return PyFloat_FromDouble(result);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyNormalise(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Vector.normalise takes no arguments(nor does it brook any dissent :)");
		PyErr_PrintEx(0);
		return NULL;
	}

	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	KBEVec3Normalize(&v, &v);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyTuple(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Vector.tuple takes no arguments");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyTup = PyTuple_New(VECTOR_SIZE);
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();

	for(int i = 0; i < VECTOR_SIZE; ++i)
		PyTuple_SetItem(pyTup, i, PyFloat_FromDouble(v[i]));

	return pyTup;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pyList(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Vector.tuple takes no arguments");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyList = PyList_New(VECTOR_SIZE);
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	Vector3& v = sv->getVector();
	
	for (int i=0; i < VECTOR_SIZE; ++i)
		PyList_SetItem(pyList, i, PyFloat_FromDouble(v[i]));

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptVector3::__py_pySet(PyObject* self, PyObject* args)
{
	ScriptVector3* sv = static_cast<ScriptVector3*>(self);
	bool good = false;
	Vector3 v;

	// 如果参数只有1个元素
	int tupleSize = (int)PyTuple_Size(args);
	if(tupleSize == 1)
	{
		PyObject* pyItem = PyTuple_GetItem(args, 0);

		if(ScriptVector3::check(pyItem, false))
		{
			convertPyObjectToVector3(v, pyItem);
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
		convertPyObjectToVector3(v, args);
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
void ScriptVector3::setVector(const Vector3& v)
{
	*val_ = v; 
}

//-------------------------------------------------------------------------------------
void ScriptVector3::setVectorFromPy(const Vector3& v)
{
	setVector(v);
	onPyPositionChanged();
}

//-------------------------------------------------------------------------------------
void ScriptVector3::onPyPositionChanged()
{
	if(isRef_ && _pyVector3ChangedCallback != NULL)
	{
		(*_pyVector3ChangedCallback)();
	}
}

//-------------------------------------------------------------------------------------

}
}
