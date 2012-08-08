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


#include "property.hpp"
#include "pyscript/Vector2.hpp"
#include "pyscript/Vector3.hpp"
#include "pyscript/Vector4.hpp"
namespace KBEngine{

uint32	PropertyDescription::propertyDescriptionCount_ = 0;

//-------------------------------------------------------------------------------------
PropertyDescription::PropertyDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel):
	name_(name),
	dataTypeName_(dataTypeName),
	flags_(flags),
	isPersistent_(isPersistent),
	dataType_(dataType),
	isIdentifier_(isIdentifier),
	databaseLength_(databaseLength),
	utype_(utype),
	defaultVal_(NULL),
	detailLevel_(detailLevel)
{
	dataType_->incRef();
	
	PropertyDescription::propertyDescriptionCount_++;

	if(dataType != NULL)
		defaultVal_ = dataType->parseDefaultStr(defaultStr);
	else
	{
		ERROR_MSG("PropertyDescription::PropertyDescription: %s DataType is NULL, in property[%s].\n", 
			dataTypeName.c_str(), name_.c_str());
	}
}

//-------------------------------------------------------------------------------------
PropertyDescription::~PropertyDescription()
{
	SAFE_RELEASE(defaultVal_);
	dataType_->decRef();
}

//-------------------------------------------------------------------------------------
PropertyDescription* PropertyDescription::createDescription(ENTITY_PROPERTY_UID utype, std::string& dataTypeName, 
	std::string& name, uint32 flags, bool isPersistent, DataType* dataType, bool isIdentifier, 
	uint32 databaseLength, std::string& defaultStr, uint8 detailLevel)
{
	PropertyDescription* propertyDescription = NULL;
	if(dataTypeName == "FIXED_DICT" || dataType->getName() == "FIXED_DICT")
	{
		propertyDescription = new FixedDictDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel);
	}
	else if(dataTypeName == "ARRAY" || dataType->getName() == "ARRAY")
	{
		propertyDescription = new ArrayDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel);
		
	}
	else if(dataTypeName == "VECTOR2" || dataType->getName() == "VECTOR2")
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel, 2);
	}
	else if(dataTypeName == "VECTOR3" || dataType->getName() == "VECTOR3")
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel, 3);
	}
	else if(dataTypeName == "VECTOR4" || dataType->getName() == "VECTOR4")
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel, 4);
	}
	else
	{
		propertyDescription = new PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel);
	}
	return propertyDescription;
}

//-------------------------------------------------------------------------------------
int PropertyDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	PyObject* pyName = PyUnicode_InternFromString(getName().c_str());
	int result = PyObject_GenericSetAttr(parentObj, pyName, value);
	Py_DECREF(pyName);
	return result;	
}

//-------------------------------------------------------------------------------------
FixedDictDescription::FixedDictDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, dataType, isIdentifier, databaseLength, defaultStr, detailLevel)
{
}

//-------------------------------------------------------------------------------------
FixedDictDescription::~FixedDictDescription()
{
}

//-------------------------------------------------------------------------------------
int FixedDictDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName().c_str()));
	if(pyobj == NULL)
		return -1;
	
	FixedDict* fixedDict = static_cast<FixedDict*>(pyobj);
	fixedDict->update(value);
	Py_XDECREF(pyobj);
	return 0;	
}

//-------------------------------------------------------------------------------------
ArrayDescription::ArrayDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, dataType, isIdentifier, databaseLength, defaultStr, detailLevel)
{
}

//-------------------------------------------------------------------------------------
ArrayDescription::~ArrayDescription()
{
}

//-------------------------------------------------------------------------------------
int ArrayDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName().c_str()));
	if(pyobj == NULL)
		return -1;
	
	//Array* array1 = static_cast<Array*>(pyobj);
	Py_XDECREF(pyobj);
	return 0;	
}

//-------------------------------------------------------------------------------------
VectorDescription::VectorDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel, uint8 elemCount):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, dataType, isIdentifier, databaseLength, defaultStr, detailLevel),
	elemCount_(elemCount)
{
}

//-------------------------------------------------------------------------------------
VectorDescription::~VectorDescription()
{
}

//-------------------------------------------------------------------------------------
int VectorDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	switch(elemCount_)
	{
	case 2:
		{
			if(PyObject_TypeCheck(value, script::ScriptVector2::getScriptType()))
			{
				return PropertyDescription::onSetValue(parentObj, value);
			}
			else
			{
				PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName().c_str()));
				if(pyobj == NULL)
					return -1;

				script::ScriptVector2* v = static_cast<script::ScriptVector2*>(pyobj);
				v->__py_pySet(v, value);
				Py_XDECREF(pyobj);
			}
		}
		break;
	case 3:
		{
			if(PyObject_TypeCheck(value, script::ScriptVector3::getScriptType()))
			{
				return PropertyDescription::onSetValue(parentObj, value);
			}
			else
			{
				PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName().c_str()));
				if(pyobj == NULL)
					return -1;

				script::ScriptVector3* v = static_cast<script::ScriptVector3*>(pyobj);
				v->__py_pySet(v, value);
				Py_XDECREF(pyobj);
			}
		}
		break;
	case 4:
		{
			if(PyObject_TypeCheck(value, script::ScriptVector4::getScriptType()))
			{
				return PropertyDescription::onSetValue(parentObj, value);
			}
			else
			{
				PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName().c_str()));
				if(pyobj == NULL)
					return -1;

				script::ScriptVector4* v = static_cast<script::ScriptVector4*>(pyobj);
				v->__py_pySet(v, value);
				Py_XDECREF(pyobj);
			}
		}
		break;
	};
	
	return 0;	
}


}
