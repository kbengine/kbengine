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


#include "scriptdef_module.hpp"
#include "entitydef.hpp"
#include "datatypes.hpp"
#include "common.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "entitydef/entity_mailbox.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
ScriptDefModule::ScriptDefModule(std::string name):
scriptType_(NULL),
uType_(0),
hasCell_(false),
hasBase_(false),
hasClient_(false),
name_(name)
{
	detailLevel_.level[DETAIL_LEVEL_NEAR] = new DetailLevel::Level();
	detailLevel_.level[DETAIL_LEVEL_MEDIUM] = new DetailLevel::Level();
	detailLevel_.level[DETAIL_LEVEL_FAR] = new DetailLevel::Level();
	detailLevel_.level[DETAIL_LEVEL_UNKNOW] = new DetailLevel::Level();
	
	detailLevel_.level[DETAIL_LEVEL_NEAR]->radius = CELL_BORDER_WIDTH;
	detailLevel_.level[DETAIL_LEVEL_NEAR]->hyst = 0.0f;
	
	detailLevel_.level[DETAIL_LEVEL_MEDIUM]->radius = 0.0f;
	detailLevel_.level[DETAIL_LEVEL_MEDIUM]->hyst = 0.0f;
	
	detailLevel_.level[DETAIL_LEVEL_FAR]->radius = 0.0f;
	detailLevel_.level[DETAIL_LEVEL_FAR]->hyst = 0.0f;
	
	detailLevel_.level[DETAIL_LEVEL_UNKNOW]->radius = 999999999.0f;
	detailLevel_.level[DETAIL_LEVEL_UNKNOW]->hyst = 0.0f;
}

//-------------------------------------------------------------------------------------
ScriptDefModule::~ScriptDefModule()
{
	S_RELEASE(scriptType_);
	PROPERTYDESCRIPTION_MAP::iterator iter1 = cellPropertyDescr_.begin();
	for(; iter1 != cellPropertyDescr_.end(); iter1++)
		iter1->second->decRef();

	iter1 = basePropertyDescr_.begin();
	for(; iter1 != basePropertyDescr_.end(); iter1++)
		iter1->second->decRef();

	iter1 = clientPropertyDescr_.begin();
	for(; iter1 != clientPropertyDescr_.end(); iter1++)
		iter1->second->decRef();

	METHODDESCRIPTION_MAP::iterator iter2 = methodCellDescr_.begin();
	for(; iter2 != methodCellDescr_.end(); iter2++)
		SAFE_RELEASE(iter2->second);
		
	METHODDESCRIPTION_MAP::iterator iter3 = methodBaseDescr_.begin();
	for(; iter3 != methodBaseDescr_.end(); iter3++)
		SAFE_RELEASE(iter3->second);
		
	METHODDESCRIPTION_MAP::iterator iter4 = methodClientDescr_.begin();
	for(; iter4 != methodClientDescr_.end(); iter4++)
		SAFE_RELEASE(iter4->second);
}

//-------------------------------------------------------------------------------------
ENTITY_SCRIPT_UID ScriptDefModule::getUType(void)
{
	return uType_;
}

//-------------------------------------------------------------------------------------
PyTypeObject* ScriptDefModule::getScriptType(void)
{
	return scriptType_;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptDefModule::createObject(void)
{
	PyObject * pObject = PyType_GenericAlloc(scriptType_, 0);
	if (pObject == NULL)
	{
		PyErr_Print();
		ERROR_MSG("ScriptDefModule::createObject: GenericAlloc is failed.\n");
	}
	return pObject;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptDefModule::getInitDict(void)
{
	return NULL;
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::addPropertyDescription(const char* attrName, 
										  PropertyDescription* propertyDescription, COMPONENT_TYPE componentType)
{
	PropertyDescription* f_propertyDescription = NULL;
	PROPERTYDESCRIPTION_MAP*  propertyDescr;
	PROPERTYDESCRIPTION_UIDMAP*  propertyDescr_uidmap;

	switch(componentType)
	{
	case CELLAPP_TYPE:
			f_propertyDescription = findCellPropertyDescription(attrName);
			propertyDescr = &getCellPropertyDescriptions();
			propertyDescr_uidmap = &getCellPropertyDescriptions_uidmap();
			
			// 判断他们是什么级别的属性， 将其保存到对应detailLevel的地方
			if((propertyDescription->getFlags() & ENTITY_CLIENT_DATA_FLAGS) > 0){
				cellDetailLevelPropertyDescrs_[propertyDescription->getDetailLevel()][attrName] = propertyDescription;
			}
			break;
	case BASEAPP_TYPE:
			f_propertyDescription = findBasePropertyDescription(attrName);
			propertyDescr = &getBasePropertyDescriptions();
			propertyDescr_uidmap = &getBasePropertyDescriptions_uidmap();
			break;
	default:
			f_propertyDescription = findClientPropertyDescription(attrName);
			propertyDescr = &getClientPropertyDescriptions();
			propertyDescr_uidmap = &getClientPropertyDescriptions_uidmap();
			break;
	};

	if(f_propertyDescription)
	{
		ERROR_MSG("ScriptDefModule::addPropertyDescription: [%s] is exist! componentType=%d.\n", attrName, componentType);
		return false;
	}

	(*propertyDescr)[attrName] = propertyDescription;
	(*propertyDescr_uidmap)[propertyDescription->getUType()] = propertyDescription;
	propertyDescription->incRef();


	// 判断是否是存储属性， 是就存储到persistentPropertyDescr_
	if(propertyDescription->isPersistent())
	{
		PROPERTYDESCRIPTION_MAP::const_iterator pciter = 
			persistentPropertyDescr_.find(attrName);

		if(pciter == persistentPropertyDescr_.end())
		{
			persistentPropertyDescr_[attrName] = propertyDescription;
			persistentPropertyDescr_uidmap_[propertyDescription->getUType()] = propertyDescription;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findCellPropertyDescription(const char* attrName)
{
	PROPERTYDESCRIPTION_MAP::iterator iter = cellPropertyDescr_.find(attrName);
	if(iter == cellPropertyDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findCellPropertyDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findBasePropertyDescription(const char* attrName)
{
	PROPERTYDESCRIPTION_MAP::iterator iter = basePropertyDescr_.find(attrName);
	if(iter == basePropertyDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findBasePropertyDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findClientPropertyDescription(const char* attrName)
{
	PROPERTYDESCRIPTION_MAP::iterator iter = clientPropertyDescr_.find(attrName);
	if(iter == clientPropertyDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findClientPropertyDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findPersistentPropertyDescription(const char* attrName)
{
	PROPERTYDESCRIPTION_MAP::iterator iter = persistentPropertyDescr_.find(attrName);
	if(iter == persistentPropertyDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findClientPropertyDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findCellPropertyDescription(ENTITY_PROPERTY_UID utype)
{
	PROPERTYDESCRIPTION_UIDMAP::iterator iter = cellPropertyDescr_uidmap_.find(utype);

	if(iter == cellPropertyDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findCellPropertyDescription: [%ld] not found!\n", utype);
		return NULL;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findBasePropertyDescription(ENTITY_PROPERTY_UID utype)
{
	PROPERTYDESCRIPTION_UIDMAP::iterator iter = basePropertyDescr_uidmap_.find(utype);

	if(iter == basePropertyDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findBasePropertyDescription: [%ld] not found!\n", utype);
		return NULL;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findClientPropertyDescription(ENTITY_PROPERTY_UID utype)
{
	PROPERTYDESCRIPTION_UIDMAP::iterator iter = clientPropertyDescr_uidmap_.find(utype);

	if(iter == clientPropertyDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findClientPropertyDescription: [%ld] not found!\n", utype);
		return NULL;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findPersistentPropertyDescription(ENTITY_PROPERTY_UID utype)
{
	PROPERTYDESCRIPTION_UIDMAP::iterator iter = persistentPropertyDescr_uidmap_.find(utype);

	if(iter == persistentPropertyDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findClientPropertyDescription: [%ld] not found!\n", utype);
		return NULL;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findPropertyDescription(const char* attrName, COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case CELLAPP_TYPE:
			return findCellPropertyDescription(attrName);
			break;
	case BASEAPP_TYPE:
			return findBasePropertyDescription(attrName);
			break;
	default:
			return findClientPropertyDescription(attrName);
			break;
	};

	return NULL;
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findPropertyDescription(ENTITY_PROPERTY_UID utype, COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case CELLAPP_TYPE:
			return findCellPropertyDescription(utype);
			break;
	case BASEAPP_TYPE:
			return findBasePropertyDescription(utype);
			break;
	default:
			return findClientPropertyDescription(utype);
			break;
	};

	return NULL;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findMethodDescription(const char* attrName, COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case CELLAPP_TYPE:
			return findCellMethodDescription(attrName);
			break;
	case BASEAPP_TYPE:
			return findBaseMethodDescription(attrName);
			break;
	default:
			return findClientMethodDescription(attrName);
			break;
	};

	return NULL;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findMethodDescription(ENTITY_METHOD_UID utype, COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case CELLAPP_TYPE:
			return findCellMethodDescription(utype);
			break;
	case BASEAPP_TYPE:
			return findBaseMethodDescription(utype);
			break;
	default:
			return findClientMethodDescription(utype);
			break;
	};

	return NULL;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findCellMethodDescription(const char* attrName)
{
	METHODDESCRIPTION_MAP::iterator iter = methodCellDescr_.find(attrName);
	if(iter == methodCellDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findCellMethodDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findCellMethodDescription(ENTITY_METHOD_UID utype)
{
	METHODDESCRIPTION_UIDMAP::iterator iter = methodCellDescr_uidmap_.find(utype);
	if(iter == methodCellDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findCellMethodDescription: [%ld] not found!\n", utype);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::addCellMethodDescription(const char* attrName, MethodDescription* methodDescription)
{
	MethodDescription* f_methodDescription = findCellMethodDescription(attrName);
	if(f_methodDescription)
	{
		ERROR_MSG("ScriptDefModule::addCellMethodDescription: [%s] is exist!\n", attrName);
		return false;
	}

	methodCellDescr_[attrName] = methodDescription;
	methodCellDescr_uidmap_[methodDescription->getUType()] = methodDescription;
	return true;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findBaseMethodDescription(const char* attrName)
{
	METHODDESCRIPTION_MAP::iterator iter = methodBaseDescr_.find(attrName);
	if(iter == methodBaseDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findBaseMethodDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findBaseMethodDescription(ENTITY_METHOD_UID utype)
{
	METHODDESCRIPTION_UIDMAP::iterator iter = methodBaseDescr_uidmap_.find(utype);
	if(iter == methodBaseDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findBaseMethodDescription: [%ld] not found!\n", utype);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::addBaseMethodDescription(const char* attrName, MethodDescription* methodDescription)
{
	MethodDescription* f_methodDescription = findBaseMethodDescription(attrName);
	if(f_methodDescription)
	{
		ERROR_MSG("ScriptDefModule::addBaseMethodDescription: [%s] is exist!\n", attrName);
		return false;
	}

	methodBaseDescr_[attrName] = methodDescription;
	methodBaseDescr_uidmap_[methodDescription->getUType()] = methodDescription;
	return true;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findClientMethodDescription(const char* attrName)
{
	METHODDESCRIPTION_MAP::iterator iter = methodClientDescr_.find(attrName);
	if(iter == methodClientDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findClientMethodDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findClientMethodDescription(ENTITY_METHOD_UID utype)
{
	METHODDESCRIPTION_UIDMAP::iterator iter = methodClientDescr_uidmap_.find(utype);
	if(iter == methodClientDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findClientMethodDescription: [%ld] not found!\n", utype);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::addClientMethodDescription(const char* attrName, MethodDescription* methodDescription)
{
	MethodDescription* f_methodDescription = findClientMethodDescription(attrName);
	if(f_methodDescription)
	{
		ERROR_MSG("ScriptDefModule::addClientMethodDescription: [%s] is exist!\n", attrName);
		return false;
	}

	methodClientDescr_[attrName] = methodDescription;
	methodClientDescr_uidmap_[methodDescription->getUType()] = methodDescription;
	return true;
}

//-------------------------------------------------------------------------------------
ScriptDefModule::PROPERTYDESCRIPTION_MAP& ScriptDefModule::getPropertyDescrs()										
{																										
	ScriptDefModule::PROPERTYDESCRIPTION_MAP* lpPropertyDescrs = NULL;										
																										
	switch(g_componentType)																				
	{																									
		case CELLAPP_TYPE:																				
			lpPropertyDescrs = &getCellPropertyDescriptions();							
			break;																						
		case BASEAPP_TYPE:																				
			lpPropertyDescrs = &getBasePropertyDescriptions();							
			break;																						
		default:																						
			lpPropertyDescrs = &getClientPropertyDescriptions();							
			break;																						
	};																									
																									
	return *lpPropertyDescrs;																			
}																										

//-------------------------------------------------------------------------------------
}
