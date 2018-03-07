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

#include "datatype.h"
#include "datatypes.h"
#include "entitydef.h"
#include "fixeddict.h"
#include "fixedarray.h"
#include "entity_call.h"
#include "scriptdef_module.h"
#include "pyscript/vector2.h"
#include "pyscript/vector3.h"
#include "pyscript/vector4.h"
#include "pyscript/copy.h"
#include "pyscript/py_memorystream.h"

#ifndef CODE_INLINE
#include "datatype.inl"
#endif

namespace KBEngine{
static DATATYPE_UID _g_dataTypeUID = 1;

//-------------------------------------------------------------------------------------
DataType::DataType(DATATYPE_UID did):
id_(did),
aliasName_()
{
	if(id_ == 0)
		id_ = _g_dataTypeUID++;

	DataTypes::addDataType(id_, this);

	EntityDef::md5().append((void*)this->aliasName(), (int)strlen(this->aliasName()));
	EntityDef::md5().append((void*)&id_, sizeof(DATATYPE_UID));
}

//-------------------------------------------------------------------------------------
DataType::~DataType()
{
	finalise();
}

//-------------------------------------------------------------------------------------
bool DataType::finalise()
{
	_g_dataTypeUID = 1;
	return true;
}

//-------------------------------------------------------------------------------------
bool DataType::initialize(XML* xml, TiXmlNode* node)
{
	return true;
}

//-------------------------------------------------------------------------------------
UInt64Type::UInt64Type(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
UInt64Type::~UInt64Type()
{
}

//-------------------------------------------------------------------------------------
bool UInt64Type::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("UINT64");
		return false;
	}

	if (!PyLong_Check(pyValue))
		return false;

	PyLong_AsUnsignedLongLong(pyValue);
	if (!PyErr_Occurred()) 
		return true;
	
	PyErr_Clear();
	PyLong_AsUnsignedLong(pyValue);
	if (!PyErr_Occurred()) 
		return true;
	
	PyErr_Clear();
	long v = PyLong_AsLong(pyValue);
	if (!PyErr_Occurred()) 
	{
		if(v < 0)
		{
			OUT_TYPE_ERROR("UINT64");
			return false;
		}
		return true;
	}

	PyErr_Clear();
	OUT_TYPE_ERROR("UINT64");
	return false;
}

//-------------------------------------------------------------------------------------
PyObject* UInt64Type::parseDefaultStr(std::string defaultVal)
{
	uint64 val = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> val;
	}

	PyObject* pyval = PyLong_FromUnsignedLongLong(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "UInt64Type::parseDefaultStr: defaultVal(%s) error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromUnsignedLongLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
void UInt64Type::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	uint64 v = static_cast<uint64>(PyLong_AsUnsignedLongLong(pyValue));

	if (PyErr_Occurred())
	{	
		PyErr_Clear();
		v = (uint64)PyLong_AsUnsignedLong(pyValue);
		if (PyErr_Occurred())
		{	
			PyErr_Clear();
			v = (uint64)PyLong_AsLong(pyValue);

			if(PyErr_Occurred())
			{
				PyErr_Clear();
				PyErr_Format(PyExc_TypeError, "UInt64Type::addToStream: pyValue(%s) is wrong!", 
					(pyValue == NULL) ? "NULL": pyValue->ob_type->tp_name);

				PyErr_PrintEx(0);

				v = 0;
			}
		}
	}

	(*mstream) << v;
}

//-------------------------------------------------------------------------------------
PyObject* UInt64Type::createFromStream(MemoryStream* mstream)
{
	uint64 val = 0;
	if(mstream)
		(*mstream) >> val;	

	PyObject* pyval = PyLong_FromUnsignedLongLong(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Format(PyExc_TypeError, "UInt64Type::createFromStream: errval=%" PRIu64 ", default return is 0", val);
		PyErr_PrintEx(0);
		S_RELEASE(pyval);
		return PyLong_FromUnsignedLongLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
UInt32Type::UInt32Type(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
UInt32Type::~UInt32Type()
{
}

//-------------------------------------------------------------------------------------
bool UInt32Type::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("UINT32");
		return false;
	}

	if (!PyLong_Check(pyValue))
		return false;

	PyLong_AsUnsignedLong(pyValue);
	if (!PyErr_Occurred()) 
		return true;
	
	PyErr_Clear();
	long v = PyLong_AsLong(pyValue);
	if (!PyErr_Occurred()) 
	{
		if(v < 0)
		{
			OUT_TYPE_ERROR("UINT32");
			return false;
		}
		return true;
	}
	
	PyErr_Clear();
	OUT_TYPE_ERROR("UINT32");
	return false;
}

//-------------------------------------------------------------------------------------
PyObject* UInt32Type::parseDefaultStr(std::string defaultVal)
{
	uint32 val = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> val;
	}

	PyObject* pyval = PyLong_FromUnsignedLong(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "UInt32Type::parseDefaultStr: defaultVal(%s) error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromUnsignedLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
void UInt32Type::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	uint32 v = PyLong_AsUnsignedLong(pyValue);

	if (PyErr_Occurred())
	{	
		PyErr_Clear();
		v = (uint32)PyLong_AsLong(pyValue);

		if(PyErr_Occurred())
		{
			PyErr_Clear();

			PyErr_Format(PyExc_TypeError, "UInt32Type::addToStream: pyValue(%s) is wrong!", 
				(pyValue == NULL) ? "NULL": pyValue->ob_type->tp_name);

			PyErr_PrintEx(0);

			v = 0;
		}
	}

	(*mstream) << v;
}

//-------------------------------------------------------------------------------------
PyObject* UInt32Type::createFromStream(MemoryStream* mstream)
{
	uint32 val = 0;
	if(mstream)
		(*mstream) >> val;	

	PyObject* pyval = PyLong_FromUnsignedLong(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Format(PyExc_TypeError, "UInt32Type::createFromStream: errval=%u, default return is 0", val);
		PyErr_PrintEx(0);
		S_RELEASE(pyval);
		return PyLong_FromUnsignedLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
Int64Type::Int64Type(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
Int64Type::~Int64Type()
{
}

//-------------------------------------------------------------------------------------
bool Int64Type::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("INT64");
		return false;
	}

	if(!PyLong_Check(pyValue))
		return false;

	PyLong_AsLongLong(pyValue);
	if (!PyErr_Occurred()) 
		return true;

	PyErr_Clear();
	PyLong_AsLong(pyValue);
	if (!PyErr_Occurred()) 
	{
		return true;
	}

	PyErr_Clear();
	OUT_TYPE_ERROR("INT64");
	return false;
}

//-------------------------------------------------------------------------------------
PyObject* Int64Type::parseDefaultStr(std::string defaultVal)
{
	int64 val = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> val;
	}

	PyObject* pyval = PyLong_FromLongLong(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "Int64Type::parseDefaultStr: defaultVal(%s) error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromLongLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
void Int64Type::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	int64 v = PyLong_AsLongLong(pyValue);

	if (PyErr_Occurred())
	{	
		PyErr_Clear();
		v = (uint32)PyLong_AsLong(pyValue);

		if(PyErr_Occurred())
		{
			PyErr_Clear();

			PyErr_Format(PyExc_TypeError, "Int64Type::addToStream: pyValue(%s) is wrong!", 
				(pyValue == NULL) ? "NULL": pyValue->ob_type->tp_name);

			PyErr_PrintEx(0);

			v = 0;
		}
	}

	(*mstream) << v;
}

//-------------------------------------------------------------------------------------
PyObject* Int64Type::createFromStream(MemoryStream* mstream)
{
	int64 val = 0;
	if(mstream)
		(*mstream) >> val;	

	PyObject* pyval = PyLong_FromLongLong(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Format(PyExc_TypeError, "Int64Type::createFromStream: errval=%" PRI64 ", default return is 0", val);
		PyErr_PrintEx(0);
		S_RELEASE(pyval);
		return PyLong_FromLongLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
FloatType::FloatType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
FloatType::~FloatType()
{
}

//-------------------------------------------------------------------------------------
bool FloatType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("FLOAT");
		return false;
	}

	bool ret = PyFloat_Check(pyValue);
	if(!ret)
		OUT_TYPE_ERROR("FLOAT");
	return ret;
}

//-------------------------------------------------------------------------------------
PyObject* FloatType::parseDefaultStr(std::string defaultVal)
{
	float val = 0.0f;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> val;
	}

	PyObject* pyval = PyFloat_FromDouble(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "FloatType::parseDefaultStr: defaultVal(%s) error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyFloat_FromDouble(0.0f);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
void FloatType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	if(!PyFloat_Check(pyValue))
	{
		PyErr_Format(PyExc_TypeError, "FloatType::addToStream: pyValue(%s) is wrong!", 
			(pyValue == NULL) ? "NULL": pyValue->ob_type->tp_name);

		PyErr_PrintEx(0);
		return;
	}

	float val = (float)PyFloat_AsDouble(pyValue);
	(*mstream) << val;
}

//-------------------------------------------------------------------------------------
PyObject* FloatType::createFromStream(MemoryStream* mstream)
{
	float val = 0.0;
	if(mstream)
		(*mstream) >> val;	

	PyObject* pyval = PyFloat_FromDouble(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Format(PyExc_TypeError, "FloatType::createFromStream: errval=%f, default return is 0", val);
		PyErr_PrintEx(0);
		S_RELEASE(pyval);
		return PyFloat_FromDouble(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
DoubleType::DoubleType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
DoubleType::~DoubleType()
{
}

//-------------------------------------------------------------------------------------
bool DoubleType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("DOUBLE");
		return false;
	}

	bool ret = PyFloat_Check(pyValue);
	if(!ret)
		OUT_TYPE_ERROR("DOUBLE");
	return ret;
}

//-------------------------------------------------------------------------------------
PyObject* DoubleType::parseDefaultStr(std::string defaultVal)
{
	double val = 0.0f;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> val;
	}

	PyObject* pyval = PyFloat_FromDouble(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "DoubleType::parseDefaultStr: defaultVal(%s) error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyFloat_FromDouble(0.0f);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
void DoubleType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	if(!PyFloat_Check(pyValue))
	{
		PyErr_Format(PyExc_TypeError, "DoubleType::addToStream: pyValue(%s) is wrong!", 
			(pyValue == NULL) ? "NULL": pyValue->ob_type->tp_name);

		PyErr_PrintEx(0);
		return;
	}

	(*mstream) << PyFloat_AsDouble(pyValue);
}

//-------------------------------------------------------------------------------------
PyObject* DoubleType::createFromStream(MemoryStream* mstream)
{
	double val = 0.0;
	if(mstream)
		(*mstream) >> val;	

	PyObject* pyval = PyFloat_FromDouble(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Format(PyExc_TypeError, "UInt32Type::createFromStream: errval=%lf, default return is 0", val);
		PyErr_PrintEx(0);
		S_RELEASE(pyval);
		return PyFloat_FromDouble(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
Vector2Type::Vector2Type(DATATYPE_UID did) :
DataType(did)
{
}

//-------------------------------------------------------------------------------------
Vector2Type::~Vector2Type()
{
}

//-------------------------------------------------------------------------------------
bool Vector2Type::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		PyErr_Format(PyExc_TypeError, 
			"must be set to a VECTOR2 type.");
		
		PyErr_PrintEx(0);
		return false;
	}

	if(!PySequence_Check(pyValue) || (uint32)PySequence_Size(pyValue) != 2)
	{
		PyErr_Format(PyExc_TypeError, 
			"must be set to a VECTOR2 type.");
		
		PyErr_PrintEx(0);
		return false;
	}

	for(uint32 index=0; index<2; ++index)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, index);
		if(!PyFloat_Check(pyVal) && !PyLong_Check(pyVal) && !PyLong_AsLongLong(pyVal))
		{
			PyErr_Format(PyExc_TypeError, 
				"VECTOR2 item is not digit.");
			
			PyErr_PrintEx(0);
			return false;
		}

		Py_DECREF(pyVal);
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Vector2Type::parseDefaultStr(std::string defaultVal)
{
	float x = 0.0f, y = 0.0f;

	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> x >> y;
	}

	return new script::ScriptVector2(float(x), float(y));
}

//-------------------------------------------------------------------------------------
void Vector2Type::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	for(ArraySize index=0; index<2; ++index)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, index);
#ifdef CLIENT_NO_FLOAT
		int32 v = (int32)PyFloat_AsDouble(pyVal);
#else
		float v = (float)PyFloat_AsDouble(pyVal);
#endif
		(*mstream) << v;
		Py_DECREF(pyVal);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Vector2Type::createFromStream(MemoryStream* mstream)
{
#ifdef CLIENT_NO_FLOAT
		int32 x = 0, y = 0;
#else
		float x = 0.0f, y = 0.0f;
#endif

	if(mstream)
		(*mstream) >> x >> y;	

	return new script::ScriptVector2(float(x), float(y));
}

//-------------------------------------------------------------------------------------
Vector3Type::Vector3Type(DATATYPE_UID did) :
DataType(did)
{
}

//-------------------------------------------------------------------------------------
Vector3Type::~Vector3Type()
{
}

//-------------------------------------------------------------------------------------
bool Vector3Type::isSameType(PyObject* pyValue)
{
	if (pyValue == NULL)
	{
		PyErr_Format(PyExc_TypeError,
			"must be set to a VECTOR3 type.");

		PyErr_PrintEx(0);
		return false;
	}

	if (!PySequence_Check(pyValue) || (uint32)PySequence_Size(pyValue) != 3)
	{
		PyErr_Format(PyExc_TypeError,
			"must be set to a VECTOR3 type.");

		PyErr_PrintEx(0);
		return false;
	}

	for (uint32 index = 0; index<3; ++index)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, index);
		if (!PyFloat_Check(pyVal) && !PyLong_Check(pyVal) && !PyLong_AsLongLong(pyVal))
		{
			PyErr_Format(PyExc_TypeError,
				"VECTOR3 item is not digit.");

			PyErr_PrintEx(0);
			return false;
		}

		Py_DECREF(pyVal);
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Vector3Type::parseDefaultStr(std::string defaultVal)
{
	float x = 0.0f, y = 0.0f, z = 0.0f;

	if (!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> x >> y >> z;
	}

	return new script::ScriptVector3(float(x), float(y), float(z));
}

//-------------------------------------------------------------------------------------
void Vector3Type::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	for (ArraySize index = 0; index<3; ++index)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, index);
#ifdef CLIENT_NO_FLOAT
		int32 v = (int32)PyFloat_AsDouble(pyVal);
#else
		float v = (float)PyFloat_AsDouble(pyVal);
#endif
		(*mstream) << v;
		Py_DECREF(pyVal);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Vector3Type::createFromStream(MemoryStream* mstream)
{
#ifdef CLIENT_NO_FLOAT
	int32 x = 0, y = 0, z = 0;
#else
	float x = 0.0f, y = 0.0f, z = 0.0f;
#endif

	if (mstream)
		(*mstream) >> x >> y >> z;

	return new script::ScriptVector3(float(x), float(y), float(z));
}

//-------------------------------------------------------------------------------------
Vector4Type::Vector4Type(DATATYPE_UID did) :
DataType(did)
{
}

//-------------------------------------------------------------------------------------
Vector4Type::~Vector4Type()
{
}

//-------------------------------------------------------------------------------------
bool Vector4Type::isSameType(PyObject* pyValue)
{
	if (pyValue == NULL)
	{
		PyErr_Format(PyExc_TypeError,
			"must be set to a VECTOR4 type.");

		PyErr_PrintEx(0);
		return false;
	}

	if (!PySequence_Check(pyValue) || (uint32)PySequence_Size(pyValue) != 4)
	{
		PyErr_Format(PyExc_TypeError,
			"must be set to a VECTOR4 type.");

		PyErr_PrintEx(0);
		return false;
	}

	for (uint32 index = 0; index<4; ++index)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, index);
		if (!PyFloat_Check(pyVal) && !PyLong_Check(pyVal) && !PyLong_AsLongLong(pyVal))
		{
			PyErr_Format(PyExc_TypeError,
				"VECTOR4 item is not digit.");

			PyErr_PrintEx(0);
			return false;
		}

		Py_DECREF(pyVal);
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Vector4Type::parseDefaultStr(std::string defaultVal)
{
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	if (!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> x >> y >> z >> w;
	}

	return new script::ScriptVector4(float(x), float(y), float(z), float(w));
}

//-------------------------------------------------------------------------------------
void Vector4Type::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	for (ArraySize index = 0; index<4; ++index)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, index);
#ifdef CLIENT_NO_FLOAT
		int32 v = (int32)PyFloat_AsDouble(pyVal);
#else
		float v = (float)PyFloat_AsDouble(pyVal);
#endif
		(*mstream) << v;
		Py_DECREF(pyVal);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Vector4Type::createFromStream(MemoryStream* mstream)
{
#ifdef CLIENT_NO_FLOAT
	int32 x = 0, y = 0, z = 0, w = 0;
#else
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
#endif

	if (mstream)
		(*mstream) >> x >> y >> z >> w;

	return new script::ScriptVector4(float(x), float(y), float(z), float(w));
}

//-------------------------------------------------------------------------------------
StringType::StringType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
StringType::~StringType()
{
}

//-------------------------------------------------------------------------------------
bool StringType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("STRING");
		return false;
	}

	bool ret = PyUnicode_Check(pyValue);
	if(!ret)
	{
		OUT_TYPE_ERROR("STRING");
	}
	else
	{
		PyObject* pyfunc = PyObject_GetAttrString(pyValue, "encode"); 
		if(pyfunc == NULL)
		{
			SCRIPT_ERROR_CHECK();
			ret = false;
		}
		else
		{
			PyObject* pyRet = PyObject_CallFunction(pyfunc, 
				const_cast<char*>("(s)"), "ascii");
			
			S_RELEASE(pyfunc);
			
			if(!pyRet)
			{
				SCRIPT_ERROR_CHECK();
				ret = false;
			}
			else
			{
				S_RELEASE(pyRet);
			}
		}
	}
	
	return ret;
}

//-------------------------------------------------------------------------------------
PyObject* StringType::parseDefaultStr(std::string defaultVal)
{
	PyObject* pyobj = PyUnicode_FromString(defaultVal.c_str());

	if (pyobj && !PyErr_Occurred()) 
		return pyobj;

	PyErr_Clear();
	PyErr_Format(PyExc_TypeError, "StringType::parseDefaultStr: defaultVal(%s) error! val=[%s]", 
		pyobj != NULL ? pyobj->ob_type->tp_name : "NULL", defaultVal.c_str());

	PyErr_PrintEx(0);
	S_RELEASE(pyobj);

	return PyUnicode_FromString("");
}

//-------------------------------------------------------------------------------------
void StringType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyValue, NULL);
	strutil::wchar2char(PyUnicode_AsWideCharStringRet0, mstream);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);
}

//-------------------------------------------------------------------------------------
PyObject* StringType::createFromStream(MemoryStream* mstream)
{
	std::string val = "";
	if(mstream)
		(*mstream) >> val;

	PyObject* pyobj = PyUnicode_FromString(val.c_str());

	if (pyobj && !PyErr_Occurred()) 
		return pyobj;

	PyErr_PrintEx(0);
	S_RELEASE(pyobj);

	return NULL;
}

//-------------------------------------------------------------------------------------
UnicodeType::UnicodeType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
UnicodeType::~UnicodeType()
{
}

//-------------------------------------------------------------------------------------
bool UnicodeType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("UNICODE");
		return false;
	}

	bool ret = PyUnicode_Check(pyValue);
	if(!ret)
		OUT_TYPE_ERROR("UNICODE");

	return ret;
}

//-------------------------------------------------------------------------------------
PyObject* UnicodeType::parseDefaultStr(std::string defaultVal)
{
	PyObject* pyobj = PyUnicode_DecodeUTF8(defaultVal.data(), defaultVal.size(), "");

	if(pyobj && !PyErr_Occurred()) 
	{
		return pyobj;
	}

	PyErr_Clear();
	PyErr_Format(PyExc_TypeError, "UnicodeType::parseDefaultStr: defaultVal(%s) error! val=[%s]", 
		pyobj != NULL ? pyobj->ob_type->tp_name : "NULL", defaultVal.c_str());

	PyErr_PrintEx(0);
	S_RELEASE(pyobj);

	return PyUnicode_DecodeUTF8("", 0, "");
}

//-------------------------------------------------------------------------------------
void UnicodeType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	PyObject* pyobj = PyUnicode_AsUTF8String(pyValue);
	if(pyobj == NULL)
	{
		OUT_TYPE_ERROR("UNICODE");
		return;
	}	

	mstream->appendBlob(PyBytes_AS_STRING(pyobj), (ArraySize)PyBytes_GET_SIZE(pyobj));
	Py_DECREF(pyobj);
}

//-------------------------------------------------------------------------------------
PyObject* UnicodeType::createFromStream(MemoryStream* mstream)
{
	std::string val = "";
	if(mstream)
	{
		mstream->readBlob(val);
	}

	PyObject* pyobj = PyUnicode_DecodeUTF8(val.data(), val.size(), "");

	if(pyobj && !PyErr_Occurred()) 
	{
		return pyobj;
	}

	S_RELEASE(pyobj);
	::PyErr_PrintEx(0);

	return NULL;
}

//-------------------------------------------------------------------------------------
PythonType::PythonType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
PythonType::~PythonType()
{
}

//-------------------------------------------------------------------------------------
bool PythonType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("PYTHON");
		return false;
	}

	bool ret = script::Pickler::pickle(pyValue).empty();
	if(ret)
		OUT_TYPE_ERROR("PYTHON");

	return !ret;
}

//-------------------------------------------------------------------------------------
PyObject* PythonType::parseDefaultStr(std::string defaultVal)
{
	if(defaultVal.size() > 0)
	{
		PyObject* module = PyImport_AddModule("__main__");
		if(module == NULL)
		{
			PyErr_SetString(PyExc_SystemError, 
				"PythonType::createObject:PyImport_AddModule __main__ error!");
			
			PyErr_PrintEx(0);	
			S_Return;
		}

		PyObject* mdict = PyModule_GetDict(module); // Borrowed reference.
		
		return PyRun_String(const_cast<char*>(defaultVal.c_str()), 
							Py_eval_input, mdict, mdict);
	}
		
	S_Return;
}

//-------------------------------------------------------------------------------------
void PythonType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	std::string datas = script::Pickler::pickle(pyValue);
	mstream->appendBlob(datas);
}

//-------------------------------------------------------------------------------------
PyObject* PythonType::createFromStream(MemoryStream* mstream)
{
	std::string datas = "";
	mstream->readBlob(datas);

	if (datas.size() == 0)
		Py_RETURN_NONE;

	return script::Pickler::unpickle(datas);
}

//-------------------------------------------------------------------------------------
PyDictType::PyDictType(DATATYPE_UID did):
PythonType(did)
{
}

//-------------------------------------------------------------------------------------
PyDictType::~PyDictType()
{
}

//-------------------------------------------------------------------------------------
bool PyDictType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("PY_DICT");
		return false;
	}

	if(!PyDict_Check(pyValue))
	{
		OUT_TYPE_ERROR("PY_DICT");
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* PyDictType::parseDefaultStr(std::string defaultVal)
{
	PyObject* pyVal = PythonType::parseDefaultStr(defaultVal);
	if(PyDict_Check(pyVal))
	{
		return pyVal;
	}

	if(pyVal)
		Py_DECREF(pyVal);

	return PyDict_New();
}

//-------------------------------------------------------------------------------------
PyObject* PyDictType::createFromStream(MemoryStream* mstream)
{
	std::string datas = "";
	mstream->readBlob(datas);

	if (datas.size() == 0)
		return PyDict_New();

	return script::Pickler::unpickle(datas);
}

//-------------------------------------------------------------------------------------
PyTupleType::PyTupleType(DATATYPE_UID did):
PythonType(did)
{
}

//-------------------------------------------------------------------------------------
PyTupleType::~PyTupleType()
{
}

//-------------------------------------------------------------------------------------
bool PyTupleType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("PY_TUPLE");
		return false;
	}

	if(!PyTuple_Check(pyValue))
	{
		OUT_TYPE_ERROR("PY_TUPLE");
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* PyTupleType::parseDefaultStr(std::string defaultVal)
{
	PyObject* pyVal = PythonType::parseDefaultStr(defaultVal);
	if(PyTuple_Check(pyVal))
	{
		return pyVal;
	}

	if(pyVal)
		Py_DECREF(pyVal);

	return PyTuple_New(0);
}

//-------------------------------------------------------------------------------------
PyObject* PyTupleType::createFromStream(MemoryStream* mstream)
{
	std::string datas = "";
	mstream->readBlob(datas);

	if (datas.size() == 0)
		return PyTuple_New(0);

	return script::Pickler::unpickle(datas);
}

//-------------------------------------------------------------------------------------
PyListType::PyListType(DATATYPE_UID did):
PythonType(did)
{
}

//-------------------------------------------------------------------------------------
PyListType::~PyListType()
{
}

//-------------------------------------------------------------------------------------
bool PyListType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("PY_LIST");
		return false;
	}

	if(!PyList_Check(pyValue))
	{
		OUT_TYPE_ERROR("PY_LIST");
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* PyListType::parseDefaultStr(std::string defaultVal)
{
	PyObject* pyVal = PythonType::parseDefaultStr(defaultVal);
	if(PyList_Check(pyVal))
	{
		return pyVal;
	}

	if(pyVal)
		Py_DECREF(pyVal);

	return PyList_New(0);
}

//-------------------------------------------------------------------------------------
PyObject* PyListType::createFromStream(MemoryStream* mstream)
{
	std::string datas = "";
	mstream->readBlob(datas);

	if (datas.size() == 0)
		return PyList_New(0);

	return script::Pickler::unpickle(datas);
}

//-------------------------------------------------------------------------------------
BlobType::BlobType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
BlobType::~BlobType()
{
}

//-------------------------------------------------------------------------------------
bool BlobType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("BLOB");
		return false;
	}

	if (!PyBytes_Check(pyValue) && 
		!PyObject_TypeCheck(pyValue, script::PyMemoryStream::getScriptType()))
	{
		OUT_TYPE_ERROR("BLOB");
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* BlobType::parseDefaultStr(std::string defaultVal)
{
	return  PyBytes_FromStringAndSize(defaultVal.data(), defaultVal.size());
}

//-------------------------------------------------------------------------------------
void BlobType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	if (!PyBytes_Check(pyValue))
	{
		script::PyMemoryStream* pPyMemoryStream = static_cast<script::PyMemoryStream*>(pyValue);
		MemoryStream& m = pPyMemoryStream->stream();
		mstream->appendBlob((const char*)m.data() + m.rpos(), (ArraySize)m.length());
	}
	else
	{
		Py_ssize_t datasize = PyBytes_GET_SIZE(pyValue);
		char* datas = PyBytes_AsString(pyValue);
		mstream->appendBlob(datas, (ArraySize)datasize);
	}
}

//-------------------------------------------------------------------------------------
PyObject* BlobType::createFromStream(MemoryStream* mstream)
{
	std::string datas;
	mstream->readBlob(datas);
	return PyBytes_FromStringAndSize(datas.data(), datas.size());
}

//-------------------------------------------------------------------------------------
EntityCallType::EntityCallType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
EntityCallType::~EntityCallType()
{
}

//-------------------------------------------------------------------------------------
bool EntityCallType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("ENTITYCALL");
		return false;
	}

	if(!(PyObject_TypeCheck(pyValue, EntityCall::getScriptType()) || pyValue == Py_None))
	{
		PyTypeObject* type = script::ScriptObject::getScriptObjectType("Entity");
		if(!type || !(PyObject_IsInstance(pyValue, (PyObject *)type)))
		{
			OUT_TYPE_ERROR("ENTITYCALL");
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* EntityCallType::parseDefaultStr(std::string defaultVal)
{
	Py_RETURN_NONE;
}

//-------------------------------------------------------------------------------------
void EntityCallType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	COMPONENT_ID cid = 0;
	ENTITY_ID id = 0;
	uint16 type = 0;
	ENTITY_SCRIPT_UID utype = 0;

	if(pyValue != Py_None)
	{
		PyTypeObject* stype = script::ScriptObject::getScriptObjectType("Entity");
		{
			// 是否是一个entity?
			if(PyObject_IsInstance(pyValue, (PyObject *)stype))
			{
				PyObject* pyid = PyObject_GetAttrString(pyValue, "id");

				if (pyid)
				{
					id = PyLong_AsLong(pyid);
					Py_DECREF(pyid);

					cid = g_componentID;

					if (g_componentType == BASEAPP_TYPE)
						type = (uint16)ENTITYCALL_TYPE_BASE;
					else if (g_componentType == CELLAPP_TYPE)
						type = (uint16)ENTITYCALL_TYPE_CELL;
					else
						type = (uint16)ENTITYCALL_TYPE_CLIENT;

					PyObject* pyClass = PyObject_GetAttrString(pyValue, "__class__");
					PyObject* pyClassName = PyObject_GetAttrString(pyClass, "__name__");
					wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyClassName, NULL);
					char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);

					PyMem_Free(PyUnicode_AsWideCharStringRet0);
					Py_DECREF(pyClass);
					Py_DECREF(pyClassName);

					ScriptDefModule* pScriptDefModule = EntityDef::findScriptModule(ccattr);
					free(ccattr);

					utype = pScriptDefModule->getUType();
				}
				else
				{
					// 某些情况下会为NULL， 例如：使用了weakproxy，而entitycall已经变为NULL了
					SCRIPT_ERROR_CHECK();
					id = 0;
					cid = 0;
				}
			}
		}
		
		// 只能是entitycall
		if(id == 0)
		{
			EntityCallAbstract* pEntityCallAbstract = static_cast<EntityCallAbstract*>(pyValue);
			cid = pEntityCallAbstract->componentID();
			id = pEntityCallAbstract->id();
			type = static_cast<int16>(pEntityCallAbstract->type());;
			utype = pEntityCallAbstract->utype();
		}
	}

	(*mstream) << id;
	(*mstream) << cid;
	(*mstream) << type;
	(*mstream) << utype;
}

//-------------------------------------------------------------------------------------
PyObject* EntityCallType::createFromStream(MemoryStream* mstream)
{
	if(mstream)
	{
		COMPONENT_ID cid = 0;
		ENTITY_ID id = 0;
		uint16 type;
		ENTITY_SCRIPT_UID utype;

		(*mstream) >> id >> cid >> type >> utype;

		// 允许传输Py_None
		if(id > 0)
		{
			PyObject* entity = EntityCall::tryGetEntity(cid, id);
			if(entity != NULL)
			{
				Py_INCREF(entity);
				return entity;
			}

			return new EntityCall(EntityDef::findScriptModule(utype), NULL, cid, 
							id, (ENTITYCALL_TYPE)type);
		}
	}

	Py_RETURN_NONE;
}

//-------------------------------------------------------------------------------------
FixedArrayType::FixedArrayType(DATATYPE_UID did):
DataType(did)
{
}

//-------------------------------------------------------------------------------------
FixedArrayType::~FixedArrayType()
{
	dataType_->decRef();
}

//-------------------------------------------------------------------------------------
PyObject* FixedArrayType::createNewItemFromObj(PyObject* pyobj)
{
	if(!isSameItemType(pyobj))
	{
		Py_RETURN_NONE;
	}

	return dataType_->createNewFromObj(pyobj);
}

//-------------------------------------------------------------------------------------
PyObject* FixedArrayType::createNewFromObj(PyObject* pyobj)
{
	if(!isSameType(pyobj))
	{
		Py_RETURN_NONE;
	}

	if(PyObject_TypeCheck(pyobj, FixedArray::getScriptType()))
	{
		Py_INCREF(pyobj);
		return pyobj;
	}

	FixedArray* pFixedArray = new FixedArray(this);
	pFixedArray->initialize(pyobj);
	return pFixedArray;
}

//-------------------------------------------------------------------------------------
bool FixedArrayType::initialize(XML* xml, TiXmlNode* node, const std::string& parentName)
{
	dataType_ = NULL;
	TiXmlNode* arrayNode = xml->enterNode(node, "of");
	if(arrayNode == NULL)
	{
		ERROR_MSG("FixedArrayType::initialize: not found \"of\".\n");
		return false;
	}

	std::string strType = xml->getValStr(arrayNode);

	if(strType == "ARRAY")
	{
		FixedArrayType* dataType = new FixedArrayType();

		if(dataType->initialize(xml, arrayNode, std::string("_") + parentName +
			dataType->aliasName() + "_ArrayType"))
		{
			dataType_ = dataType;
			dataType_->incRef();

			DataTypes::addDataType(std::string("_") + parentName +
				dataType->aliasName() + "_ArrayType", dataType);
		}
		else
		{
			ERROR_MSG("FixedArrayType::initialize: Array is wrong.\n");
			return false;
		}
	}
	else
	{
		DataType* dataType = DataTypes::getDataType(strType);
		if(dataType != NULL)
		{
			dataType_ = dataType;
			dataType_->incRef();
		}
		else
		{
			ERROR_MSG(fmt::format("FixedArrayType::initialize: key[{}] did not find type[{}]!\n", 
				"ARRAY", strType.c_str()));
			
			return false;
		}			
	}

	if(dataType_ == NULL)
	{
		ERROR_MSG("FixedArrayType::initialize: dataType is NULL.\n");
		return false;
	}

	DATATYPE_UID uid = dataType_->id();
	EntityDef::md5().append((void*)&uid, sizeof(DATATYPE_UID));
	EntityDef::md5().append((void*)strType.c_str(), (int)strType.size());
	return true;
}

//-------------------------------------------------------------------------------------
bool FixedArrayType::isSameItemType(PyObject* pyValue)
{
	return dataType_->isSameType(pyValue);
}

//-------------------------------------------------------------------------------------
bool FixedArrayType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("ARRAY");
		return false;
	}
	
	if(!PySequence_Check(pyValue))
	{
		OUT_TYPE_ERROR("ARRAY");
		return false;
	}

	Py_ssize_t size = PySequence_Size(pyValue);
	for(Py_ssize_t i=0; i<size; ++i)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, i);
		bool ok = dataType_->isSameType(pyVal);
		Py_DECREF(pyVal);

		if(!ok)
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* FixedArrayType::parseDefaultStr(std::string defaultVal)
{
	FixedArray* pFixedArray = new FixedArray(this);
	pFixedArray->initialize("");
	return pFixedArray;
}

//-------------------------------------------------------------------------------------
void FixedArrayType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	addToStreamEx(mstream, pyValue, false);
}

//-------------------------------------------------------------------------------------
void FixedArrayType::addToStreamEx(MemoryStream* mstream, PyObject* pyValue, bool onlyPersistents)
{
	ArraySize size = (ArraySize)PySequence_Size(pyValue);
	(*mstream) << size;

	for(ArraySize i=0; i<size; ++i)
	{
		PyObject* pyVal = PySequence_GetItem(pyValue, i);

		if(dataType_->type() == DATA_TYPE_FIXEDDICT)
			((FixedDictType*)dataType_)->addToStreamEx(mstream, pyVal, onlyPersistents);
		else if(dataType_->type() == DATA_TYPE_FIXEDARRAY)
			((FixedArrayType*)dataType_)->addToStreamEx(mstream, pyVal, onlyPersistents);
		else
			dataType_->addToStream(mstream, pyVal);

		Py_DECREF(pyVal);
	}
}

//-------------------------------------------------------------------------------------
PyObject* FixedArrayType::createFromStream(MemoryStream* mstream)
{
	return createFromStreamEx(mstream, false);
}

//-------------------------------------------------------------------------------------
PyObject* FixedArrayType::createFromStreamEx(MemoryStream* mstream, bool onlyPersistents)
{
	ArraySize size;

	FixedArray* pFixedArray = new FixedArray(this);
	pFixedArray->initialize("");

	if(mstream)
	{
		(*mstream) >> size;	
		
		std::vector<PyObject*>& vals = pFixedArray->getValues();
		for(ArraySize i=0; i<size; ++i)
		{
			if(mstream->length() == 0)
			{
				ERROR_MSG(fmt::format("FixedArrayType::createFromStream: {} invalid(size={}), stream no space!\n",
					aliasName(), size));

				break;
			}

			PyObject* pyVal = NULL;
			
			if(dataType_->type() == DATA_TYPE_FIXEDDICT)
				pyVal = ((FixedDictType*)dataType_)->createFromStreamEx(mstream, onlyPersistents);
			else if(dataType_->type() == DATA_TYPE_FIXEDARRAY)
				pyVal = ((FixedArrayType*)dataType_)->createFromStreamEx(mstream, onlyPersistents);
			else
				pyVal = dataType_->createFromStream(mstream);
	
			if(pyVal)
			{
				vals.push_back(pyVal);
			}
			else
			{
				ERROR_MSG(fmt::format("FixedArrayType::createFromStream: {}, pyVal is NULL! size={}\n", 
					aliasName(), size));

				break;
			}
		}
	}

	return pFixedArray;
}

//-------------------------------------------------------------------------------------
FixedDictType::FixedDictType(DATATYPE_UID did):
DataType(did),
keyTypes_(),
implObj_(NULL),
pycreateObjFromDict_(NULL),
pygetDictFromObj_(NULL),
pyisSameType_(NULL),
moduleName_()
{
}

//-------------------------------------------------------------------------------------
FixedDictType::~FixedDictType()
{
	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		iter->second->dataType->decRef();
	}

	keyTypes_.clear();

	S_RELEASE(pycreateObjFromDict_);
	S_RELEASE(pygetDictFromObj_);
	S_RELEASE(pyisSameType_);
	S_RELEASE(implObj_);
}

//-------------------------------------------------------------------------------------
std::string FixedDictType::getKeyNames(void)
{
	std::string keyNames = "";

	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		keyNames += iter->first + ",";
	}

	return keyNames;
}

//-------------------------------------------------------------------------------------
std::string FixedDictType::debugInfos(void)
{
	std::string retstr = "";

	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		retstr += iter->first;
		retstr += "(";
		retstr += iter->second->dataType->aliasName();
		retstr += "), ";
	}
	
	if(retstr.size() > 0)
		retstr.erase(retstr.size() - 2, 2);

	return retstr;
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictType::createNewItemFromObj(const char* keyName, PyObject* pyobj)
{
	DataType* dataType = isSameItemType(keyName, pyobj);
	if(!dataType)
	{
		Py_RETURN_NONE;
	}

	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		if(iter->first == keyName)
		{
			return iter->second->dataType->createNewFromObj(pyobj);
		}
	}

	Py_RETURN_NONE;
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictType::createNewFromObj(PyObject* pyobj)
{
	if(!isSameType(pyobj))
	{
		Py_RETURN_NONE;
	}

	if(this->hasImpl())
	{
		return impl_createObjFromDict(pyobj);
	}

	// 可能在传入参数的时候已经是FixedDict类型了, 因为parseDefaultStr
	// 会初始为最终对象类型
	if(PyObject_TypeCheck(pyobj, FixedDict::getScriptType()))
	{
		Py_INCREF(pyobj);
		return pyobj;
	}

	FixedDict* pFixedDict = new FixedDict(this);
	pFixedDict->initialize(pyobj);
	return pFixedDict;
}

//-------------------------------------------------------------------------------------
bool FixedDictType::initialize(XML* xml, TiXmlNode* node, std::string& parentName)
{
	TiXmlNode* propertiesNode = xml->enterNode(node, "Properties");
	if(propertiesNode == NULL)
	{
		ERROR_MSG("FixedDictType::initialize: not found \"Properties\".\n");
		return false;
	}

	std::string strType = "", typeName = "";

	XML_FOR_BEGIN(propertiesNode)
	{
		typeName = xml->getKey(propertiesNode);

		TiXmlNode* typeNode = xml->enterNode(propertiesNode->FirstChild(), "Type");
		TiXmlNode* PersistentNode = xml->enterNode(propertiesNode->FirstChild(), "Persistent");
		TiXmlNode* DatabaseLengthNode = xml->enterNode(propertiesNode->FirstChild(), "DatabaseLength");

		bool persistent = true;
		if(PersistentNode)
		{
			std::string strval = xml->getValStr(PersistentNode);
			if(strval == "false")
			{
				persistent = false;
			}
		}

		uint32 databaseLength = 0;
		if (DatabaseLengthNode)
		{
			databaseLength = xml->getValInt(DatabaseLengthNode);
		}

		if(typeNode)
		{
			strType = xml->getValStr(typeNode);

			if(strType == "ARRAY")
			{
				FixedArrayType* dataType = new FixedArrayType();
				DictItemDataTypePtr pDictItemDataType(new DictItemDataType());
				pDictItemDataType->dataType = dataType;

				if(dataType->initialize(xml, typeNode, std::string("_") + parentName + std::string("_") + typeName + "_ArrayType"))
				{
					DATATYPE_UID uid = dataType->id();
					EntityDef::md5().append((void*)&uid, sizeof(DATATYPE_UID));
					EntityDef::md5().append((void*)strType.c_str(), (int)strType.size());
					EntityDef::md5().append((void*)typeName.c_str(), (int)typeName.size());

					keyTypes_.push_back(std::pair< std::string, DictItemDataTypePtr >(typeName, pDictItemDataType));
					dataType->incRef();

					if(dataType->getDataType()->type() == DATA_TYPE_ENTITYCALL)
					{
						persistent = false;
					}

					pDictItemDataType->persistent = persistent;
					pDictItemDataType->databaseLength = databaseLength;
					EntityDef::md5().append((void*)&persistent, sizeof(bool));
					EntityDef::md5().append((void*)&databaseLength, sizeof(uint32));
					DataTypes::addDataType(std::string("_") + parentName + std::string("_") + typeName + "_ArrayType", dataType);
				}
				else
				{
					ERROR_MSG(fmt::format("FixedDictType::initialize: key[{}] did not find array-type[{}]!\n", 
						typeName.c_str(), strType.c_str()));

					return false;
				}
			}
			else
			{
				DataType* dataType = DataTypes::getDataType(strType);
				DictItemDataTypePtr pDictItemDataType(new DictItemDataType());
				pDictItemDataType->dataType = dataType;

				if(dataType != NULL)
				{
					DATATYPE_UID uid = dataType->id();
					EntityDef::md5().append((void*)&uid, sizeof(DATATYPE_UID));
					EntityDef::md5().append((void*)strType.c_str(), (int)strType.size());
					EntityDef::md5().append((void*)typeName.c_str(), (int)typeName.size());

					keyTypes_.push_back(std::pair< std::string, DictItemDataTypePtr >(typeName, pDictItemDataType));
					dataType->incRef();
					
					if(dataType->type() == DATA_TYPE_ENTITYCALL)
					{
						persistent = false;
					}

					pDictItemDataType->persistent = persistent;
					pDictItemDataType->databaseLength = databaseLength;
					EntityDef::md5().append((void*)&persistent, sizeof(bool));
					EntityDef::md5().append((void*)&databaseLength, sizeof(uint32));
				}
				else
				{
					ERROR_MSG(fmt::format("FixedDictType::initialize: key[{}] did not find type[{}]!\n", 
						typeName.c_str(), strType.c_str()));
					
					return false;
				}
			}
		}
		else
		{
			ERROR_MSG(fmt::format("FixedDictType::initialize: key[{}] no label[\"Type\"], key[{}] will be ignored!\n",
				typeName, typeName));
		}
	}
	XML_FOR_END(propertiesNode);

	TiXmlNode* implementedByNode = xml->enterNode(node, "implementedBy");
	if(implementedByNode)
	{
		strType = xml->getValStr(implementedByNode);

		if(g_componentType == CELLAPP_TYPE || g_componentType == BASEAPP_TYPE ||
				g_componentType == CLIENT_TYPE)
		{
			if(strType.size() > 0 && !loadImplModule(strType))
				return false;

			moduleName_ = strType;
		}

		if(strType.size() > 0)
			EntityDef::md5().append((void*)strType.c_str(), (int)strType.size());
	}

	if (keyTypes_.size() == 0)
	{
		ERROR_MSG(fmt::format("FixedDictType::initialize(): FIXED_DICT({}) no keys! \n",
			this->aliasName()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool FixedDictType::loadImplModule(std::string moduleName)
{
	KBE_ASSERT(implObj_ == NULL);
	
	std::vector<std::string> res_;
	strutil::kbe_split<char>(moduleName, '.', res_);
	
	if(res_.size() != 2)
	{
		ERROR_MSG(fmt::format("FixedDictType::loadImplModule: {} impl error! like:[moduleName.inst]\n",
			moduleName.c_str()));

		return false;
	}

	PyObject* implModule = PyImport_ImportModule(res_[0].c_str());
	if (!implModule)
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}
	
	implObj_ = PyObject_GetAttrString(implModule, res_[1].c_str());
	Py_DECREF(implModule);

	if (!implObj_)
	{
		SCRIPT_ERROR_CHECK()
		return false;
	}

	pycreateObjFromDict_ = PyObject_GetAttrString(implObj_, "createObjFromDict");
	if (!pycreateObjFromDict_)
	{
		SCRIPT_ERROR_CHECK()
		return false;
	}
	

	pygetDictFromObj_ = PyObject_GetAttrString(implObj_, "getDictFromObj");
	if (!pygetDictFromObj_)
	{
		SCRIPT_ERROR_CHECK()
		return false;
	}
	
	pyisSameType_ = PyObject_GetAttrString(implObj_, "isSameType");
	if (!pyisSameType_)
	{
		SCRIPT_ERROR_CHECK()
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictType::impl_createObjFromDict(PyObject* dictData)
{
	// 可能在传入参数的时候已经是用户类型了, 因为parseDefaultStr
	// 会初始为最终对象类型
	if(impl_isSameType(dictData))
	{
		Py_INCREF(dictData);
		return dictData;
	}

	PyObject* pyRet = PyObject_CallFunction(pycreateObjFromDict_, 
		const_cast<char*>("(O)"), dictData);
	
	if(pyRet == NULL || !impl_isSameType(pyRet))
	{
		SCRIPT_ERROR_CHECK();

		ERROR_MSG(fmt::format("FixedDictType::impl_createObjFromDict: {}.isSameType() is failed!\n",
			moduleName_.c_str()));
		
		Py_RETURN_NONE;
	}

	return pyRet;
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictType::impl_getDictFromObj(PyObject* pyobj)
{
	PyObject* pyRet = PyObject_CallFunction(pygetDictFromObj_, 
		const_cast<char*>("(O)"), pyobj);
	
	if(pyRet == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return parseDefaultStr("");
	}

	return pyRet;
}

//-------------------------------------------------------------------------------------
bool FixedDictType::impl_isSameType(PyObject* pyobj)
{
	PyObject* pyRet = PyObject_CallFunction(pyisSameType_, 
		const_cast<char*>("(O)"), pyobj);
	
	if(pyRet == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}
	
	bool ret = Py_True == pyRet;
	Py_DECREF(pyRet);
	return ret;
}

//-------------------------------------------------------------------------------------
DataType* FixedDictType::isSameItemType(const char* keyName, PyObject* pyValue)
{
	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		if(iter->first == keyName)
		{
			if(pyValue == NULL || !iter->second->dataType->isSameType(pyValue))
			{
				PyErr_Format(PyExc_TypeError, 
					"set FIXED_DICT(%s) error! at key: %s(%s), keyNames=[%s].", 
					this->aliasName(), 
					iter->first.c_str(),
					(pyValue == NULL ? "NULL" : pyValue->ob_type->tp_name),
					debugInfos().c_str());
				
				PyErr_PrintEx(0);
				return NULL;
			}
			else
			{
				return iter->second->dataType;
			}
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool FixedDictType::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("DICT");
		return false;
	}

	if(hasImpl())
	{
		// 这里返回false后还继续判断的原因是isSameType因为相关特性
		// fixeddict或者用户产生的类别都应该是合法的
		if(impl_isSameType(pyValue))
			return true;
	}

	if(PyObject_TypeCheck(pyValue, FixedDict::getScriptType()))
	{
		if(static_cast<FixedDict*>(pyValue)->getDataType()->id() == this->id())
			return true;
		else
			return false;
	}

	if(!PyDict_Check(pyValue))
	{
		OUT_TYPE_ERROR("DICT");
		return false;
	}

	Py_ssize_t dictSize = PyDict_Size(pyValue);
	if(dictSize != (Py_ssize_t)keyTypes_.size())
	{
		PyErr_Format(PyExc_TypeError, 
			"FIXED_DICT(%s) key does not match! giveKeySize=%d, dictKeySize=%d, dictKeyNames=[%s].", 
			this->aliasName(), dictSize, keyTypes_.size(), 
			debugInfos().c_str());
		
		PyErr_PrintEx(0);
		return false;
	}

	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		PyObject* pyObject = PyDict_GetItemString(pyValue, const_cast<char*>(iter->first.c_str()));
		if(pyObject == NULL || !iter->second->dataType->isSameType(pyObject))
		{
				PyErr_Format(PyExc_TypeError, 
					"set FIXED_DICT(%s) error! at key: %s(%s), keyNames=[%s].", 
					this->aliasName(), 
					iter->first.c_str(),
					(pyObject == NULL ? "NULL" : pyObject->ob_type->tp_name),
					debugInfos().c_str());
			
			PyErr_PrintEx(0);
			return false;
		}
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictType::parseDefaultStr(std::string defaultVal)
{
	PyObject* val = PyDict_New();

	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		PyObject* item = iter->second->dataType->parseDefaultStr("");
		PyDict_SetItemString(val, iter->first.c_str(), item);
		Py_DECREF(item);
	}

	FixedDict* pydict = new FixedDict(this);
	pydict->initialize(val);
	Py_DECREF(val);

	if(hasImpl())
	{
		PyObject* pyValue = impl_createObjFromDict(pydict);
		Py_DECREF(pydict);
		return pyValue;
	}
	
	return pydict;
}

//-------------------------------------------------------------------------------------
void FixedDictType::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	addToStreamEx(mstream, pyValue, false);
}

//-------------------------------------------------------------------------------------
void FixedDictType::addToStreamEx(MemoryStream* mstream, PyObject* pyValue, bool onlyPersistents)
{
	if(hasImpl())
	{
		pyValue = impl_getDictFromObj(pyValue);
	}

	PyObject* pydict = pyValue;
	if(PyObject_TypeCheck(pyValue, FixedDict::getScriptType()))
	{
		pydict = static_cast<FixedDict*>(pyValue)->getDictObject();
	}
	
	FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		if(onlyPersistents)
		{
			if(!iter->second->persistent)
				continue;
		}

		PyObject* pyObject = 
			PyDict_GetItemString(pydict, const_cast<char*>(iter->first.c_str()));
		
		if(pyObject == NULL)
		{
			ERROR_MSG(fmt::format("FixedDictType::addToStreamEx: {} not found key[{}]. keyNames[{}]\n",
				this->aliasName_, iter->first, this->debugInfos()));

			// KBE_ASSERT(pyObject != NULL);
			PyObject* pobj = iter->second->dataType->parseDefaultStr("");

			if(iter->second->dataType->type() == DATA_TYPE_FIXEDDICT)
				((FixedDictType*)iter->second->dataType)->addToStreamEx(mstream, pobj, onlyPersistents);
			else if(iter->second->dataType->type() == DATA_TYPE_FIXEDARRAY)
				((FixedArrayType*)iter->second->dataType)->addToStreamEx(mstream, pobj, onlyPersistents);
			else
				iter->second->dataType->addToStream(mstream, pobj);

			Py_DECREF(pobj);
			continue;
		}
		
		if(!iter->second->dataType->isSameType(pyObject))
		{
			// KBE_ASSERT(pyObject != NULL);
			PyObject* pobj = iter->second->dataType->parseDefaultStr("");

			if(iter->second->dataType->type() == DATA_TYPE_FIXEDDICT)
				((FixedDictType*)iter->second->dataType)->addToStreamEx(mstream, pobj, onlyPersistents);
			else if(iter->second->dataType->type() == DATA_TYPE_FIXEDARRAY)
				((FixedArrayType*)iter->second->dataType)->addToStreamEx(mstream, pobj, onlyPersistents);
			else
				iter->second->dataType->addToStream(mstream, pobj);

			Py_DECREF(pobj);
			continue;
		}

		if(iter->second->dataType->type() == DATA_TYPE_FIXEDDICT)
			((FixedDictType*)iter->second->dataType)->addToStreamEx(mstream, pyObject, onlyPersistents);
		else if(iter->second->dataType->type() == DATA_TYPE_FIXEDARRAY)
			((FixedArrayType*)iter->second->dataType)->addToStreamEx(mstream, pyObject, onlyPersistents);
		else
			iter->second->dataType->addToStream(mstream, pyObject);
	}

	if(hasImpl())
	{
		Py_DECREF(pyValue);
	}
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictType::createFromStream(MemoryStream* mstream)
{
	return createFromStreamEx(mstream, false);
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictType::createFromStreamEx(MemoryStream* mstream, bool onlyPersistents)
{
	FixedDict* pydict = new FixedDict(this, onlyPersistents);
	pydict->initialize(mstream, onlyPersistents);

	if(hasImpl())
	{
		PyObject* pyValue = impl_createObjFromDict(pydict);
		Py_DECREF(pydict);
		return pyValue;
	}

	return pydict;
}

//-------------------------------------------------------------------------------------
}
