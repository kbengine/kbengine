// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "datatypes.h"
#include "entitydef.h"
#include "property.h"
#include "entity_component.h"
#include "pyscript/vector2.h"
#include "pyscript/vector3.h"
#include "pyscript/vector4.h"
#include "pyscript/copy.h"

#ifndef CODE_INLINE
#include "property.inl"
#endif


namespace KBEngine{

uint32 PropertyDescription::propertyDescriptionCount_ = 0;

//-------------------------------------------------------------------------------------
PropertyDescription::PropertyDescription(ENTITY_PROPERTY_UID utype, 
										std::string dataTypeName, 
										std::string name, uint32 flags, 
										bool isPersistent, 
										DataType* dataType, 
										bool isIdentifier, 
										std::string indexType,
										uint32 databaseLength, 
										std::string defaultStr, 
										DETAIL_TYPE detailLevel):
	name_(name),
	dataTypeName_(dataTypeName),
	flags_(flags),
	isPersistent_(isPersistent),
	dataType_(dataType),
	isIdentifier_(isIdentifier),
	databaseLength_(databaseLength),
	utype_(utype),
	defaultValStr_(defaultStr),
	detailLevel_(detailLevel),
	aliasID_(-1),
	indexType_(indexType)
{
	dataType_->incRef();

	// entityCall 无法保存
	if(isPersistent && strcmp(dataType_->getName(), "ENTITYCALL") == 0)
	{
		isPersistent_ = false;
	}

	EntityDef::md5().append((void*)name_.c_str(), (int)name_.size());
	EntityDef::md5().append((void*)defaultValStr_.c_str(), (int)defaultValStr_.size());
	EntityDef::md5().append((void*)dataTypeName.c_str(), (int)dataTypeName.size());
	EntityDef::md5().append((void*)&utype_, sizeof(ENTITY_PROPERTY_UID));
	EntityDef::md5().append((void*)&flags_, sizeof(uint32));
	EntityDef::md5().append((void*)&isPersistent_, sizeof(bool));
	EntityDef::md5().append((void*)&isIdentifier_, sizeof(bool));
	EntityDef::md5().append((void*)&databaseLength_, sizeof(uint32));
	EntityDef::md5().append((void*)&detailLevel_, sizeof(int8));

	DATATYPE_UID uid = dataType->id();
	EntityDef::md5().append((void*)&uid, sizeof(DATATYPE_UID));

	PropertyDescription::propertyDescriptionCount_++;

	if(dataType == NULL)
	{
		ERROR_MSG(fmt::format("PropertyDescription::PropertyDescription: {} DataType is NULL, in property[{}].\n",
			dataTypeName.c_str(), name_.c_str()));
	}
}

//-------------------------------------------------------------------------------------
PropertyDescription::~PropertyDescription()
{
	dataType_->decRef();
}

//-------------------------------------------------------------------------------------
bool PropertyDescription::isSameType(PyObject* pyValue)
{
	return dataType_->isSameType(pyValue);
}

//-------------------------------------------------------------------------------------
bool PropertyDescription::isSamePersistentType(PyObject* pyValue)
{
	return dataType_->isSameType(pyValue);
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
PyObject* PropertyDescription::parseDefaultStr(const std::string& defaultVal)
{
	return dataType_->parseDefaultStr(defaultVal);
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
PyObject* PropertyDescription::createFromPersistentStream(MemoryStream* mstream)
{
	return dataType_->createFromStream(mstream);
}

//-------------------------------------------------------------------------------------
PropertyDescription* PropertyDescription::createDescription(ENTITY_PROPERTY_UID utype, 
															const std::string& dataTypeName, 
															const std::string& name, 
															uint32 flags, 
															bool isPersistent, 
															DataType* dataType, 
															bool isIdentifier, 
															std::string indexType,
															uint32 databaseLength, 
															const std::string& defaultStr,
															DETAIL_TYPE detailLevel)
{
	PropertyDescription* propertyDescription = NULL;
	if(dataTypeName == "FIXED_DICT" || 
		strcmp(dataType->getName(), "FIXED_DICT") == 0)
	{
		propertyDescription = new FixedDictDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, indexType, databaseLength, 
														defaultStr, detailLevel);
	}
	else if(dataTypeName == "ARRAY" ||
		strcmp(dataType->getName(), "ARRAY") == 0)
	{
		propertyDescription = new ArrayDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, indexType, databaseLength, 
														defaultStr, detailLevel);
		
	}
	else if(dataTypeName == "VECTOR2" || 
		strcmp(dataType->getName(), "VECTOR2") == 0)
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, indexType, databaseLength, 
														defaultStr, detailLevel, 2);
	}
	else if(dataTypeName == "VECTOR3" || 
		strcmp(dataType->getName(), "VECTOR3") == 0)
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, indexType, databaseLength, 
														defaultStr, detailLevel, 3);
	}
	else if(dataTypeName == "VECTOR4" || 
		strcmp(dataType->getName(), "VECTOR4") == 0)
	{
		propertyDescription = new VectorDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, indexType, databaseLength, 
														defaultStr, detailLevel, 4);
	}
	else if (dataTypeName == "EntityComponent" ||
		strcmp(dataType->getName(), "EntityComponent") == 0)
	{
		propertyDescription = new EntityComponentDescription(utype, dataTypeName, name, flags, isPersistent,
			dataType, isIdentifier, indexType, databaseLength,
			defaultStr, detailLevel);
	}
	else
	{
		propertyDescription = new PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
														dataType, isIdentifier, indexType, databaseLength, 
														defaultStr, detailLevel);
	}

	return propertyDescription;
}

//-------------------------------------------------------------------------------------
PyObject* PropertyDescription::newDefaultVal()
{
	return dataType_->parseDefaultStr(defaultValStr_);
}

//-------------------------------------------------------------------------------------
PyObject* PropertyDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	PyObject* pyName = PyUnicode_InternFromString(getName());
	int result = PyObject_GenericSetAttr(parentObj, pyName, value);
	Py_DECREF(pyName);
	
	if(result == -1)
		return NULL;

	return value;	
}

//-------------------------------------------------------------------------------------
FixedDictDescription::FixedDictDescription(ENTITY_PROPERTY_UID utype, 
											std::string dataTypeName,
											std::string name, 
											uint32 flags, 
											bool isPersistent, 
											DataType* dataType, 
											bool isIdentifier, 
											std::string indexType,
											uint32 databaseLength, 
											std::string defaultStr, 
											DETAIL_TYPE detailLevel):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
		dataType, isIdentifier, indexType, databaseLength, defaultStr, detailLevel)
{
	KBE_ASSERT(dataType->type() == DATA_TYPE_FIXEDDICT);

	/*
	FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = 
		static_cast<FixedDictType*>(dataType)->getKeyTypes();

	FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes.begin();
	for(; iter != keyTypes.end(); ++iter)
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
PyObject* FixedDictDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	if(static_cast<FixedDictType*>(dataType_)->isSameType(value))
	{
		FixedDictType* dataType = static_cast<FixedDictType*>(this->getDataType());

		PyObject* pyobj = dataType->createNewFromObj(value);
		PropertyDescription::onSetValue(parentObj, pyobj);

		Py_DECREF(pyobj);
		return pyobj;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void FixedDictDescription::addPersistentToStream(MemoryStream* mstream, PyObject* pyValue)
{
	// 允许使用默认值来创建一个流
	if(pyValue == NULL)
	{
		pyValue = newDefaultVal();
		static_cast<FixedDictType*>(dataType_)->addToStreamEx(mstream, pyValue, true);
		Py_DECREF(pyValue);
		return;
	}

	static_cast<FixedDictType*>(dataType_)->addToStreamEx(mstream, pyValue, true);
}

//-------------------------------------------------------------------------------------
PyObject* FixedDictDescription::createFromPersistentStream(MemoryStream* mstream)
{
	return ((FixedDictType*)dataType_)->createFromStreamEx(mstream, true);
}

//-------------------------------------------------------------------------------------
ArrayDescription::ArrayDescription(ENTITY_PROPERTY_UID utype, 
									std::string dataTypeName,
									std::string name, 
									uint32 flags, 
									bool isPersistent, 
									DataType* dataType, 
									bool isIdentifier, 
									std::string indexType,
									uint32 databaseLength, 
									std::string defaultStr, 
									DETAIL_TYPE detailLevel):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
		dataType, isIdentifier, indexType, databaseLength, defaultStr, detailLevel)
{
}

//-------------------------------------------------------------------------------------
ArrayDescription::~ArrayDescription()
{
}

//-------------------------------------------------------------------------------------
PyObject* ArrayDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	if(static_cast<FixedArrayType*>(dataType_)->isSameType(value))
	{
		FixedArrayType* dataType = static_cast<FixedArrayType*>(this->getDataType());
		PyObject* pyobj = dataType->createNewFromObj(value);
		PropertyDescription::onSetValue(parentObj, pyobj);

		Py_DECREF(pyobj);
		return pyobj;
	}

	return NULL;	
}

//-------------------------------------------------------------------------------------
void ArrayDescription::addPersistentToStream(MemoryStream* mstream, PyObject* pyValue)
{
	// 允许使用默认值来创建一个流
	if(pyValue == NULL)
	{
		pyValue = newDefaultVal();
		static_cast<FixedArrayType*>(dataType_)->addToStreamEx(mstream, pyValue, true);
		Py_DECREF(pyValue);
		return;
	}

	static_cast<FixedArrayType*>(dataType_)->addToStreamEx(mstream, pyValue, true);
}

//-------------------------------------------------------------------------------------
PyObject* ArrayDescription::createFromPersistentStream(MemoryStream* mstream)
{
	return ((FixedArrayType*)dataType_)->createFromStreamEx(mstream, true);
}

//-------------------------------------------------------------------------------------
VectorDescription::VectorDescription(ENTITY_PROPERTY_UID utype, 
									std::string dataTypeName, 
									std::string name, 
									uint32 flags, 
									bool isPersistent, 
									DataType* dataType, 
									bool isIdentifier, 
									std::string indexType,
									uint32 databaseLength, 
									std::string defaultStr, 
									DETAIL_TYPE detailLevel, 
									uint8 elemCount):
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent, 
		dataType, isIdentifier, indexType, databaseLength, defaultStr, detailLevel),
	elemCount_(elemCount)
{
}

//-------------------------------------------------------------------------------------
VectorDescription::~VectorDescription()
{
}

//-------------------------------------------------------------------------------------
PyObject* VectorDescription::onSetValue(PyObject* parentObj, PyObject* value)
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
					return NULL;

				script::ScriptVector2* v = static_cast<script::ScriptVector2*>(pyobj);
				v->__py_pySet(v, value);
				Py_XDECREF(pyobj);
				return v;
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
					return NULL;

				script::ScriptVector3* v = static_cast<script::ScriptVector3*>(pyobj);
				v->__py_pySet(v, value);
				Py_XDECREF(pyobj);
				return v;
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
					return NULL;

				script::ScriptVector4* v = static_cast<script::ScriptVector4*>(pyobj);
				v->__py_pySet(v, value);
				Py_XDECREF(pyobj);
				return v;
			}
		}
		break;
	};
	
	return NULL;	
}

//-------------------------------------------------------------------------------------
EntityComponentDescription::EntityComponentDescription(ENTITY_PROPERTY_UID utype,
	std::string dataTypeName,
	std::string name,
	uint32 flags,
	bool isPersistent,
	DataType* dataType,
	bool isIdentifier,
	std::string indexType,
	uint32 databaseLength,
	std::string defaultStr,
	DETAIL_TYPE detailLevel) :
	PropertyDescription(utype, dataTypeName, name, flags, isPersistent,
		dataType, isIdentifier, indexType, databaseLength, defaultStr, detailLevel)
{
}

//-------------------------------------------------------------------------------------
EntityComponentDescription::~EntityComponentDescription()
{
}

//-------------------------------------------------------------------------------------
PyObject* EntityComponentDescription::newDefaultVal()
{
	EntityComponent* pEntityComponent = static_cast<EntityComponent*>(PropertyDescription::newDefaultVal());
	pEntityComponent->pPropertyDescription(this);
	return pEntityComponent;
}

//-------------------------------------------------------------------------------------
PyObject* EntityComponentDescription::onSetValue(PyObject* parentObj, PyObject* value)
{
	return PropertyDescription::onSetValue(parentObj, value);
}

//-------------------------------------------------------------------------------------
bool EntityComponentDescription::isSamePersistentType(PyObject* pyValue)
{
	return ((EntityComponentType*)dataType_)->isSamePersistentType(pyValue);
}

//-------------------------------------------------------------------------------------
void EntityComponentDescription::addPersistentToStream(MemoryStream* mstream, PyObject* pyValue)
{
	// 允许使用默认值来创建一个流
	if (pyValue == NULL)
	{
		pyValue = newDefaultVal();
		((EntityComponentType*)dataType_)->addPersistentToStream(mstream, pyValue);
		Py_DECREF(pyValue);
		return;
	}

	((EntityComponentType*)dataType_)->addPersistentToStream(mstream, pyValue);
}

//-------------------------------------------------------------------------------------
void EntityComponentDescription::addPersistentToStreamTemplates(ScriptDefModule* pScriptModule, MemoryStream* mstream)
{
	((EntityComponentType*)dataType_)->addPersistentToStreamTemplates(pScriptModule, mstream);
}

//-------------------------------------------------------------------------------------
PyObject* EntityComponentDescription::createFromPersistentStream(MemoryStream* mstream)
{
	EntityComponent* pEntityComponent = static_cast<EntityComponent*>(static_cast<EntityComponentType*>(dataType_)->createFromPersistentStream(NULL, mstream));
	pEntityComponent->pPropertyDescription(this);
	return pEntityComponent;
}

//-------------------------------------------------------------------------------------
PyObject* EntityComponentDescription::createFromPersistentStream(ScriptDefModule* pScriptModule, MemoryStream* mstream)
{
	EntityComponent* pEntityComponent = static_cast<EntityComponent*>(static_cast<EntityComponentType*>(dataType_)->createFromPersistentStream(pScriptModule, mstream));
	pEntityComponent->pPropertyDescription(this);
	return pEntityComponent;
}

//-------------------------------------------------------------------------------------
PyObject* EntityComponentDescription::createFromStream(MemoryStream* mstream)
{
	EntityComponent* pEntityComponent = static_cast<EntityComponent*>(PropertyDescription::createFromStream(mstream));
	pEntityComponent->pPropertyDescription(this);
	return pEntityComponent;
}

//-------------------------------------------------------------------------------------
PyObject* EntityComponentDescription::parseDefaultStr(const std::string& defaultStr)
{
	EntityComponent* pEntityComponent = static_cast<EntityComponent*>(PropertyDescription::parseDefaultStr(defaultStr));
	pEntityComponent->pPropertyDescription(this);
	return pEntityComponent;
}

//-------------------------------------------------------------------------------------
}
