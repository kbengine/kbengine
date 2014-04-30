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
#include "resmgr/resmgr.hpp"
#include "pyscript/script.hpp"

#ifndef CODE_INLINE
#include "scriptdef_module.ipp"
#endif


namespace KBEngine{

//-------------------------------------------------------------------------------------
ScriptDefModule::ScriptDefModule(std::string name):
scriptType_(NULL),
uType_(0),
hasCell_(false),
hasBase_(false),
hasClient_(false),
volatileinfo_(),
name_(name)
{
	EntityDef::md5().append((void*)name.c_str(), name.size());
}

//-------------------------------------------------------------------------------------
ScriptDefModule::~ScriptDefModule()
{
	finalise();
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::finalise(void)
{
	S_RELEASE(scriptType_);
	PROPERTYDESCRIPTION_MAP::iterator iter1 = cellPropertyDescr_.begin();
	for(; iter1 != cellPropertyDescr_.end(); iter1++)
		iter1->second->decRef();
	
	cellPropertyDescr_.clear();

	iter1 = basePropertyDescr_.begin();
	for(; iter1 != basePropertyDescr_.end(); iter1++)
		iter1->second->decRef();

	basePropertyDescr_.clear();

	iter1 = clientPropertyDescr_.begin();
	for(; iter1 != clientPropertyDescr_.end(); iter1++)
		iter1->second->decRef();

	clientPropertyDescr_.clear();

	METHODDESCRIPTION_MAP::iterator iter2 = methodCellDescr_.begin();
	for(; iter2 != methodCellDescr_.end(); iter2++)
		SAFE_RELEASE(iter2->second);
		
	methodCellDescr_.clear();

	METHODDESCRIPTION_MAP::iterator iter3 = methodBaseDescr_.begin();
	for(; iter3 != methodBaseDescr_.end(); iter3++)
		SAFE_RELEASE(iter3->second);
	
	methodBaseDescr_.clear();

	METHODDESCRIPTION_MAP::iterator iter4 = methodClientDescr_.begin();
	for(; iter4 != methodClientDescr_.end(); iter4++)
		SAFE_RELEASE(iter4->second);

	methodClientDescr_.clear();
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::onLoaded(void)
{
	int aliasID = 0;
	PROPERTYDESCRIPTION_MAP::iterator iter1 = cellPropertyDescr_.begin();
	for(; iter1 != cellPropertyDescr_.end(); iter1++)
	{
		if(iter1->second->hasClient())
		{
			iter1->second->aliasID(aliasID++);
		}
	}

	iter1 = basePropertyDescr_.begin();
	for(; iter1 != basePropertyDescr_.end(); iter1++)
	{
		if(iter1->second->hasClient())
		{
			iter1->second->aliasID(aliasID++);
		}
	}

	iter1 = clientPropertyDescr_.begin();
	for(; iter1 != clientPropertyDescr_.end(); iter1++)
	{
		if(iter1->second->hasClient())
		{
			iter1->second->aliasID(aliasID++);
		}
	}
	
	if(aliasID > 255)
	{
		iter1 = cellPropertyDescr_.begin();
		for(; iter1 != cellPropertyDescr_.end(); iter1++)
		{
			if(iter1->second->hasClient())
			{
				iter1->second->aliasID(-1);
			}
		}

		iter1 = basePropertyDescr_.begin();
		for(; iter1 != basePropertyDescr_.end(); iter1++)
		{
			if(iter1->second->hasClient())
			{
				iter1->second->aliasID(-1);
			}
		}

		iter1 = clientPropertyDescr_.begin();
		for(; iter1 != clientPropertyDescr_.end(); iter1++)
		{
			if(iter1->second->hasClient())
			{
				iter1->second->aliasID(-1);
			}
		}
	}

	aliasID = 0;

	METHODDESCRIPTION_MAP::iterator iter2 = methodClientDescr_.begin();
	for(; iter2 != methodClientDescr_.end(); iter2++)
	{
		iter2->second->aliasID(aliasID++);
	}

	if(aliasID > 255)
	{
		METHODDESCRIPTION_MAP::iterator iter2 = methodClientDescr_.begin();
		for(; iter2 != methodClientDescr_.end(); iter2++)
		{
			iter2->second->aliasID(-1);
		}
	}

	if(g_debugEntity)
	{
		c_str();
	}
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::c_str()
{
	PROPERTYDESCRIPTION_MAP::iterator iter1 = cellPropertyDescr_.begin();
	for(; iter1 != cellPropertyDescr_.end(); iter1++)
	{
		DEBUG_MSG(boost::format("ScriptDefModule::c_str: %1%.%2% uid=%3%, flags=%4%, aliasID=%5%.\n") % 
			getName() % iter1->second->getName() % iter1->second->getUType() % entityDataFlagsToString(iter1->second->getFlags()) % iter1->second->aliasID());
	}

	iter1 = basePropertyDescr_.begin();
	for(; iter1 != basePropertyDescr_.end(); iter1++)
	{
		DEBUG_MSG(boost::format("ScriptDefModule::c_str: %1%.%2% uid=%3%, flags=%4%, aliasID=%5%.\n") % 
			getName() % iter1->second->getName() % iter1->second->getUType() % entityDataFlagsToString(iter1->second->getFlags()) % iter1->second->aliasID());
	}

	iter1 = clientPropertyDescr_.begin();
	for(; iter1 != clientPropertyDescr_.end(); iter1++)
	{
		DEBUG_MSG(boost::format("ScriptDefModule::c_str: %1%.%2% uid=%3%, flags=%4%, aliasID=%5%.\n") % 
			getName() % iter1->second->getName() % iter1->second->getUType() % entityDataFlagsToString(iter1->second->getFlags()) % iter1->second->aliasID());
	}

	METHODDESCRIPTION_MAP::iterator iter2 = methodCellDescr_.begin();
	for(; iter2 != methodCellDescr_.end(); iter2++)
	{
		DEBUG_MSG(boost::format("ScriptDefModule::c_str: %1%.CellMethod %2% uid=%3%, argssize=%4%, aliasID=%5%%6%.\n") % 
			getName() % iter2->second->getName() % iter2->second->getUType() % 
			iter2->second->getArgSize() % iter2->second->aliasID() % (iter2->second->isExposed() ? ", exposed=true" : ", exposed=false"));
	}

	METHODDESCRIPTION_MAP::iterator iter3 = methodBaseDescr_.begin();
	for(; iter3 != methodBaseDescr_.end(); iter3++)
	{
		DEBUG_MSG(boost::format("ScriptDefModule::c_str: %1%.BaseMethod %2% uid=%3%, argssize=%4%, aliasID=%5%%6%.\n") % 
			getName() % iter3->second->getName() % iter3->second->getUType() % 
			iter3->second->getArgSize() % iter3->second->aliasID() % (iter3->second->isExposed() ? ", exposed=true" : ", exposed=false"));
	}

	METHODDESCRIPTION_MAP::iterator iter4 = methodClientDescr_.begin();
	for(; iter4 != methodClientDescr_.end(); iter4++)
	{
		DEBUG_MSG(boost::format("ScriptDefModule::c_str: %1%.ClientMethod %2% uid=%3%, argssize=%4%, aliasID=%5%.\n") % 
			getName() % iter4->second->getName() % iter4->second->getUType() % iter4->second->getArgSize() % iter4->second->aliasID());
	}

	DEBUG_MSG(boost::format("ScriptDefModule::c_str: [%1%], cellPropertys=%2%, basePropertys=%2%, "
		"clientPropertys=%4%, cellMethods=%5%(%6%), baseMethods=%7%(%8%), clientMethods=%9%\n") %
		getName() % 
		getCellPropertyDescriptions().size() % 
		getBasePropertyDescriptions().size() % 
		getClientPropertyDescriptions().size() % 
		getCellMethodDescriptions().size() % 
		getCellExposedMethodDescriptions().size() % 
		getBaseMethodDescriptions().size() % 
		getBaseExposedMethodDescriptions().size() % 
		getClientMethodDescriptions().size());
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::setUType(ENTITY_SCRIPT_UID utype)
{ 
	uType_ = utype; 
	EntityDef::md5().append((void*)&uType_, sizeof(ENTITY_SCRIPT_UID));
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
void ScriptDefModule::autoMatchCompOwn()
{
	setClient(false);
	setBase(false);
	setCell(false);

	std::string fmodule = "scripts/client/" + name_ + ".py";
	std::string fmodule_pyc = "scripts/client/"SCRIPT_BIN_CACHEDIR"/" + name_ + "."SCRIPT_BIN_TAG".pyc";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		setClient(true);
	}
	
	if(g_componentType == CLIENT_TYPE)
	{
		setBase(true);
		setCell(true);
		return;
	}

	fmodule = "scripts/base/" + name_ + ".py";
	fmodule_pyc = "scripts/base/"SCRIPT_BIN_CACHEDIR"/" + name_ + "."SCRIPT_BIN_TAG".pyc";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		setBase(true);
	}

	fmodule = "scripts/cell/" + name_ + ".py";
	fmodule_pyc = "scripts/cell/"SCRIPT_BIN_CACHEDIR"/" + name_ + "."SCRIPT_BIN_TAG".pyc";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		setCell(true);
	}
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::addPropertyDescription(const char* attrName, 
										  PropertyDescription* propertyDescription, 
										  COMPONENT_TYPE componentType)
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

			setCell(true);
			break;
	case BASEAPP_TYPE:
			f_propertyDescription = findBasePropertyDescription(attrName);
			propertyDescr = &getBasePropertyDescriptions();
			propertyDescr_uidmap = &getBasePropertyDescriptions_uidmap();
			setBase(true);
			break;
	default:
			f_propertyDescription = findClientPropertyDescription(attrName);
			propertyDescr = &getClientPropertyDescriptions();
			propertyDescr_uidmap = &getClientPropertyDescriptions_uidmap();
			setClient(true);
			break;
	};

	if(f_propertyDescription)
	{
		ERROR_MSG(boost::format("ScriptDefModule::addPropertyDescription: [%1%] is exist! componentType=%2%.\n") %
			attrName % componentType);

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
PropertyDescription* ScriptDefModule::findPropertyDescription(const char* attrName, 
															  COMPONENT_TYPE componentType)
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
PropertyDescription* ScriptDefModule::findPropertyDescription(ENTITY_PROPERTY_UID utype, 
															  COMPONENT_TYPE componentType)
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
MethodDescription* ScriptDefModule::findMethodDescription(const char* attrName, 
														  COMPONENT_TYPE componentType)
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
MethodDescription* ScriptDefModule::findMethodDescription(ENTITY_METHOD_UID utype, 
														  COMPONENT_TYPE componentType)
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
bool ScriptDefModule::addCellMethodDescription(const char* attrName, 
											   MethodDescription* methodDescription)
{
	MethodDescription* f_methodDescription = findCellMethodDescription(attrName);
	if(f_methodDescription)
	{
		ERROR_MSG(boost::format("ScriptDefModule::addCellMethodDescription: [%1%] is exist!\n") % attrName);
		return false;
	}
	
	setCell(true);
	methodCellDescr_[attrName] = methodDescription;
	methodCellDescr_uidmap_[methodDescription->getUType()] = methodDescription;

	if(methodDescription->isExposed())
		methodCellExposedDescr_[attrName] = methodDescription;

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
bool ScriptDefModule::addBaseMethodDescription(const char* attrName, 
											   MethodDescription* methodDescription)
{
	MethodDescription* f_methodDescription = findBaseMethodDescription(attrName);
	if(f_methodDescription)
	{
		ERROR_MSG(boost::format("ScriptDefModule::addBaseMethodDescription: [%1%] is exist!\n") % 
			attrName);

		return false;
	}
	
	setBase(true);
	methodBaseDescr_[attrName] = methodDescription;
	methodBaseDescr_uidmap_[methodDescription->getUType()] = methodDescription;

	if(methodDescription->isExposed())
		methodBaseExposedDescr_[attrName] = methodDescription;

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
bool ScriptDefModule::addClientMethodDescription(const char* attrName, 
												 MethodDescription* methodDescription)
{
	MethodDescription* f_methodDescription = findClientMethodDescription(attrName);
	if(f_methodDescription)
	{
		ERROR_MSG(boost::format("ScriptDefModule::addClientMethodDescription: [%1%] is exist!\n") %
			attrName);

		return false;
	}

	setClient(true);
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
