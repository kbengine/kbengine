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

#include "datatypes.hpp"
#include "property.hpp"
#include "pyscript/vector2.hpp"
#include "pyscript/vector3.hpp"
#include "pyscript/vector4.hpp"
#include "pyscript/copy.hpp"

namespace KBEngine{

uint32 PropertyDescription::propertyDescriptionCount_ = 0;

//-------------------------------------------------------------------------------------
PropertyDescription::PropertyDescription(ENTITY_PROPERTY_UID utype, 
										 std::string dataTypeName, 
										 std::string name, uint32 flags, 
										 bool isPersistent, 
										DataType* dataType, 
										bool isIdentifier, 
										uint32 databaseLength, 
										std::string defaultStr, 
										uint8 detailLevel):
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
	
	// mailbox 无法保存
	if(isPersistent && strcmp(dataType_->getName(), "MAILBOX") == 0)
	{
		isPersistent_ = false;
	}

	PropertyDescription::propertyDescriptionCount_++;

	if(dataType != NULL)
	{
		if(g_componentType == CELLAPP_TYPE || g_componentType == BASEAPP_TYPE ||
			g_componentType == CLIENT_TYPE)
		{
			defaultVal_ = dataType->parseDefaultStr(defaultStr);
		}
	}
	else
	{
		ERROR_MSG("PropertyDescription::PropertyDescription: %s DataType is NULL, in property[%s].\n", 
			dataTypeName.c_str(), name_.c_str());
	}
}

//-------------------------------------------------------------------------------------
PropertyDescription::~PropertyDescription()
{
	S_RELEASE(defaultVal_);
	dataType_->decRef();
}

//-------------------------------------------------------------------------------------
void PropertyDescription::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	dataType_->addToStream(mstream, pyValue);
}

//-------------------------------------------------------------------------------------
PyObject* PropertyDescription::createFromStream(MemoryStream* mstream)
{
	return dataType_->createFromStream(mstream);
}

//-------------------------------------------------------------------------------------
void PropertyDescription::addPersistentToStream(MemoryStream* mstream, PyObject* pyValue)
{
	// 允许使用默认值来创建一个流
	if(pyValue == NULL)
	{
		pyValue = newDefaultVal();
		dataType_->addToStream(mstream, pyValue);
		Py_DECREF(pyValue);
		return;
	}

	dataType_->addToStream(mstream, pyValue);
}

//-------------------------------------------------------------------------------------
PropertyDescription* PropertyDescription::createDescription(ENTITY_PROPERTY_UID utype, 
															std::string& dataTypeName, 
															std::string& name, 
															uint32 flags, 
															bool isPersistent, 
															DataType* dataType, 
															bool isIdentifier, 
															uint32 databaseLength, 
															std::string& defaultStr, 
															uint8 detailLevel)
{
	PropertyDescription* propertyDescription = NULL;
	if(dataTypeName == "FIXED_DICT" || 
		dataType->getName() == "FIXED_DICT")
	{
		propertyDescription = new FixedDictDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, 
														defaultStr, detailLevel);
	}
	else if(dataTypeName == "ARRAY" ||
		dataType->getName() == "ARRAY")
	{
		propertyDescription = new ArrayDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, 
														defaultStr, detailLevel);
		
	}
	else if(dataTypeName == "VECTOR2" || 
		dataType->getName() == "VECTOR2")
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, 
														defaultStr, detailLevel, 2);
	}
	else if(dataTypeName == "VECTOR3" || 
		dataType->getName() == "VECTOR3")
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, 
														defaultStr, detailLevel, 3);
	}
	else if(dataTypeName == "VECTOR4" || 
		dataType->getName() == "VECTOR4")
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, 
														defaultStr, detailLevel, 4);
	}
	else
	{
		propertyDescription = new PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, 
														defaultStr, detailLevel);
	}

	return propertyDescription;
}

//-------------------------------------------------------------------------------------
PyObject* PropertyDescription::newDefaultVal(void)
{
	int ob_refcnt = defaultVal_->ob_refcnt;
	PyObject* pyobj = script::Copy::deepcopy(defaultVal_); 
	if(pyobj == defaultVal_ && pyobj->ob_refcnt == ob_refcnt)
		Py_INCREF(pyobj);
	
	return pyobj;
}

//-------------------------------------------------------------------------------------
int PropertyDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	PyObject* pyName = PyUnicode_InternFromString(getName());
	int result = PyObject_GenericSetAttr(parentObj, pyName, value);
	Py_DECREF(pyName);
	return result;	
}

//-------------------------------------------------------------------------------------
FixedDictDescription::FixedDictDescription(ENTITY_PROPERTY_UID utype, 
										   std::string dataTypeName,
										   std::string name, 
										   uint32 flags, 
										   bool isPersistent, 
											DataType* dataType, 
											bool isIdentifier, 
											uint32 databaseLength, 
											std::string defaultStr, 
											uint8 detailLevel):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
		dataType, isIdentifier, databaseLength, defaultStr, detailLevel)
{
	KBE_ASSERT(dataType->type() == DATA_TYPE_FIXEDDICT);

	/*
	FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = 
		static_cast<FixedDictType*>(dataType)->getKeyTypes();

	FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes.begin();
	for(; iter != keyTypes.end(); iter++)
	{
		PropertyDescription* pPropertyDescription = PropertyDescription::createDescription(0,
			std::string(iter->second->getName()), iter->first, flags, isPersistent, iter->second, false, 0, std::string(), detailLevel);
	}
	*/
}

//-------------------------------------------------------------------------------------
FixedDictDescription::~FixedDictDescription()
{
}

//-------------------------------------------------------------------------------------
int FixedDictDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	if(static_cast<FixedDictType*>(dataType_)->isSameType(value))
	{
		FixedDictType* dataType = static_cast<FixedDictType*>(this->getDataType());
		return PropertyDescription::onSetValue(parentObj, dataType->createNewFromObj(value));
	}

	return 0;
}

//-------------------------------------------------------------------------------------
void FixedDictDescription::addPersistentToStream(MemoryStream* mstream, PyObject* pyValue)
{
	static_cast<FixedDictType*>(dataType_)->addToStreamEx(mstream, pyValue, true);
}

//-------------------------------------------------------------------------------------
ArrayDescription::ArrayDescription(ENTITY_PROPERTY_UID utype, 
										   std::string dataTypeName,
										   std::string name, 
										   uint32 flags, 
										   bool isPersistent, 
											DataType* dataType, 
											bool isIdentifier, 
											uint32 databaseLength, 
											std::string defaultStr, 
											uint8 detailLevel):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
		dataType, isIdentifier, databaseLength, defaultStr, detailLevel)
{
}

//-------------------------------------------------------------------------------------
ArrayDescription::~ArrayDescription()
{
}

//-------------------------------------------------------------------------------------
int ArrayDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	if(static_cast<FixedArrayType*>(dataType_)->isSameType(value))
	{
		FixedArrayType* dataType = static_cast<FixedArrayType*>(this->getDataType());
		return PropertyDescription::onSetValue(parentObj, dataType->createNewFromObj(value));
	}

	return 0;	
}

//-------------------------------------------------------------------------------------
VectorDescription::VectorDescription(ENTITY_PROPERTY_UID utype, 
									 std::string dataTypeName, 
									 std::string name, 
									 uint32 flags, 
									 bool isPersistent, 
									DataType* dataType, 
									bool isIdentifier, 
									uint32 databaseLength, 
									std::string defaultStr, 
									uint8 detailLevel, 
									uint8 elemCount):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
		dataType, isIdentifier, databaseLength, defaultStr, detailLevel),
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
				PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName()));
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
				PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName()));
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
				PyObject* pyobj = PyObject_GetAttrString(parentObj, const_cast<char*>(getName()));
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
