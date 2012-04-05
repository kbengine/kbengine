#include "property.hpp"
#include "pyscript/Vector2.hpp"
#include "pyscript/Vector3.hpp"
#include "pyscript/Vector4.hpp"
namespace KBEngine{

uint32	PropertyDescription::propertyDescriptionCount_ = 0;

//-------------------------------------------------------------------------------------
PropertyDescription::PropertyDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel):
	m_dataTypeName_(dataTypeName),
	m_name_(name),
	m_flags_(flags),
	m_isPersistent_(isPersistent),
	m_dataType_(dataType),
	m_isIdentifier_(isIdentifier),
	m_databaseLength_(databaseLength),
	m_detailLevel_(detailLevel)
{
	m_dataType_->incRef();
	
	PropertyDescription::propertyDescriptionCount_++;
	m_utype_ = PropertyDescription::propertyDescriptionCount_;

	if(dataType != NULL)
		m_defaultVal_ = dataType->parseDefaultStr(defaultStr);
	else
		ERROR_MSG("PropertyDescription::PropertyDescription: %s DataType is NULL, in property[%s].\n", 
			dataTypeName.c_str(), m_name_.c_str());
}

//-------------------------------------------------------------------------------------
PropertyDescription::~PropertyDescription()
{
	SAFE_RELEASE(m_defaultVal_);
	m_dataType_->decRef();
}

//-------------------------------------------------------------------------------------
PropertyDescription* PropertyDescription::createDescription(std::string& dataTypeName, 
	std::string& name, uint32 flags, bool isPersistent, DataType* dataType, bool isIdentifier, 
	uint32 databaseLength, std::string& defaultStr, uint8& detailLevel)
{
	PropertyDescription* propertyDescription = NULL;
	if(dataTypeName == "FIXED_DICT" || dataType->getName() == "FIXED_DICT")
	{
		propertyDescription = new FixedDictDescription(dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel);
	}
	else if(dataTypeName == "ARRAY" || dataType->getName() == "ARRAY")
	{
		propertyDescription = new ArrayDescription(dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel);
		
	}
	else if(dataTypeName == "VECTOR2" || dataType->getName() == "VECTOR2")
	{
		propertyDescription = new VectorDescription(dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel, 2);
	}
	else if(dataTypeName == "VECTOR3" || dataType->getName() == "VECTOR3")
	{
		propertyDescription = new VectorDescription(dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel, 3);
	}
	else if(dataTypeName == "VECTOR4" || dataType->getName() == "VECTOR4")
	{
		propertyDescription = new VectorDescription(dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, databaseLength, defaultStr, detailLevel, 4);
	}
	else
	{
		propertyDescription = new PropertyDescription(dataTypeName, name, flags, isPersistent, 
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
FixedDictDescription::FixedDictDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel):
	PropertyDescription(dataTypeName, name, flags, isPersistent, dataType, isIdentifier, databaseLength, defaultStr, detailLevel)
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
ArrayDescription::ArrayDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel):
	PropertyDescription(dataTypeName, name, flags, isPersistent, dataType, isIdentifier, databaseLength, defaultStr, detailLevel)
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
	
	Array* array1 = static_cast<Array*>(pyobj);
	Py_XDECREF(pyobj);
	return 0;	
}

//-------------------------------------------------------------------------------------
VectorDescription::VectorDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
	DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel, uint8 elemCount):
	PropertyDescription(dataTypeName, name, flags, isPersistent, dataType, isIdentifier, databaseLength, defaultStr, detailLevel),
	m_elemCount_(elemCount)
{
}

//-------------------------------------------------------------------------------------
VectorDescription::~VectorDescription()
{
}

//-------------------------------------------------------------------------------------
int VectorDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	switch(m_elemCount_)
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
				v->pySet(v, value);
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
				v->pySet(v, value);
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
				v->pySet(v, value);
				Py_XDECREF(pyobj);
			}
		}
		break;
	};
	
	return 0;	
}


}
