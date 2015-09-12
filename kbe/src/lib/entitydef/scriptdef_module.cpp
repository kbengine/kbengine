/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "scriptdef_module.h"
#include "entitydef.h"
#include "datatypes.h"
#include "common.h"
#include "common/smartpointer.h"
#include "entitydef/entity_mailbox.h"
#include "resmgr/resmgr.h"
#include "pyscript/script.h"
#include "server/serverconfig.h"
#include "client_lib/config.h"
#include "network/bundle.h"

#ifndef CODE_INLINE
#include "scriptdef_module.inl"
#endif


namespace KBEngine{

//-------------------------------------------------------------------------------------
ScriptDefModule::ScriptDefModule(std::string name, ENTITY_SCRIPT_UID utype):
scriptType_(NULL),
uType_(utype),
persistentPropertyDescr_(),
cellPropertyDescr_(),
basePropertyDescr_(),
clientPropertyDescr_(),
persistentPropertyDescr_uidmap_(),
cellPropertyDescr_uidmap_(),
basePropertyDescr_uidmap_(),
clientPropertyDescr_uidmap_(),
propertyDescr_aliasmap_(),
methodCellDescr_(),
methodBaseDescr_(),
methodClientDescr_(),
methodBaseExposedDescr_(),
methodCellExposedDescr_(),
methodCellDescr_uidmap_(),
methodBaseDescr_uidmap_(),
methodClientDescr_uidmap_(),
methodDescr_aliasmap_(),
hasCell_(false),
hasBase_(false),
hasClient_(false),
volatileinfo_(),
name_(name),
usePropertyDescrAlias_(false),
useMethodDescrAlias_(false)
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
	for(; iter1 != cellPropertyDescr_.end(); ++iter1)
		iter1->second->decRef();
	
	cellPropertyDescr_.clear();

	iter1 = basePropertyDescr_.begin();
	for(; iter1 != basePropertyDescr_.end(); ++iter1)
		iter1->second->decRef();

	basePropertyDescr_.clear();

	iter1 = clientPropertyDescr_.begin();
	for(; iter1 != clientPropertyDescr_.end(); ++iter1)
		iter1->second->decRef();

	clientPropertyDescr_.clear();

	METHODDESCRIPTION_MAP::iterator iter2 = methodCellDescr_.begin();
	for(; iter2 != methodCellDescr_.end(); ++iter2)
		SAFE_RELEASE(iter2->second);
		
	methodCellDescr_.clear();

	METHODDESCRIPTION_MAP::iterator iter3 = methodBaseDescr_.begin();
	for(; iter3 != methodBaseDescr_.end(); ++iter3)
		SAFE_RELEASE(iter3->second);
	
	methodBaseDescr_.clear();

	METHODDESCRIPTION_MAP::iterator iter4 = methodClientDescr_.begin();
	for(; iter4 != methodClientDescr_.end(); ++iter4)
		SAFE_RELEASE(iter4->second);

	methodClientDescr_.clear();
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::onLoaded(void)
{
	if(EntityDef::entitydefAliasID())
	{
		int aliasID = ENTITY_BASE_PROPERTY_ALIASID_MAX;
		PROPERTYDESCRIPTION_MAP::iterator iter1 = cellPropertyDescr_.begin();
		for(; iter1 != cellPropertyDescr_.end(); ++iter1)
		{
			if(iter1->second->hasClient())
			{
				propertyDescr_aliasmap_[aliasID] = iter1->second;
				iter1->second->aliasID(aliasID++);
			}
		}

		iter1 = basePropertyDescr_.begin();
		for(; iter1 != basePropertyDescr_.end(); ++iter1)
		{
			if(iter1->second->hasClient())
			{
				propertyDescr_aliasmap_[aliasID] = iter1->second;
				iter1->second->aliasID(aliasID++);
			}
		}

		iter1 = clientPropertyDescr_.begin();
		for(; iter1 != clientPropertyDescr_.end(); ++iter1)
		{
			if(iter1->second->hasClient())
			{
				propertyDescr_aliasmap_[aliasID] = iter1->second;
				iter1->second->aliasID(aliasID++);
			}
		}
		
		if(aliasID > 255)
		{
			iter1 = cellPropertyDescr_.begin();
			for(; iter1 != cellPropertyDescr_.end(); ++iter1)
			{
				if(iter1->second->hasClient())
				{
					iter1->second->aliasID(-1);
				}
			}

			iter1 = basePropertyDescr_.begin();
			for(; iter1 != basePropertyDescr_.end(); ++iter1)
			{
				if(iter1->second->hasClient())
				{
					iter1->second->aliasID(-1);
				}
			}

			iter1 = clientPropertyDescr_.begin();
			for(; iter1 != clientPropertyDescr_.end(); ++iter1)
			{
				if(iter1->second->hasClient())
				{
					iter1->second->aliasID(-1);
				}
			}

			propertyDescr_aliasmap_.clear();
		}
		else
		{
			usePropertyDescrAlias_ = true;
		}

		aliasID = 0;

		METHODDESCRIPTION_MAP::iterator iter2 = methodClientDescr_.begin();
		for(; iter2 != methodClientDescr_.end(); ++iter2)
		{
			methodDescr_aliasmap_[aliasID] = iter2->second;
			iter2->second->aliasID(aliasID++);
		}

		if(aliasID > 255)
		{
			METHODDESCRIPTION_MAP::iterator iter2 = methodClientDescr_.begin();
			for(; iter2 != methodClientDescr_.end(); ++iter2)
			{
				iter2->second->aliasID(-1);
				methodDescr_aliasmap_.clear();
			}
		}
		else
		{
			useMethodDescrAlias_ = true;
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
	for(; iter1 != cellPropertyDescr_.end(); ++iter1)
	{
		DEBUG_MSG(fmt::format("ScriptDefModule::c_str: {}.{} uid={}, flags={}, aliasID={}.\n",
			getName(), iter1->second->getName(), iter1->second->getUType(), entityDataFlagsToString(iter1->second->getFlags()), iter1->second->aliasID()));
	}

	iter1 = basePropertyDescr_.begin();
	for(; iter1 != basePropertyDescr_.end(); ++iter1)
	{
		DEBUG_MSG(fmt::format("ScriptDefModule::c_str: {}.{} uid={}, flags={}, aliasID={}.\n",
			getName(), iter1->second->getName(), iter1->second->getUType(), entityDataFlagsToString(iter1->second->getFlags()), iter1->second->aliasID()));
	}

	iter1 = clientPropertyDescr_.begin();
	for(; iter1 != clientPropertyDescr_.end(); ++iter1)
	{
		DEBUG_MSG(fmt::format("ScriptDefModule::c_str: {}.{} uid={}, flags={}, aliasID={}.\n",
			getName(), iter1->second->getName(), iter1->second->getUType(), entityDataFlagsToString(iter1->second->getFlags()), iter1->second->aliasID()));
	}

	METHODDESCRIPTION_MAP::iterator iter2 = methodCellDescr_.begin();
	for(; iter2 != methodCellDescr_.end(); ++iter2)
	{
		DEBUG_MSG(fmt::format("ScriptDefModule::c_str: {}.CellMethod {} uid={}, argssize={}, aliasID={}{}.\n",
			getName(), iter2->second->getName(), iter2->second->getUType(),
			iter2->second->getArgSize(), iter2->second->aliasID(), (iter2->second->isExposed() ? ", exposed=true" : ", exposed=false")));
	}

	METHODDESCRIPTION_MAP::iterator iter3 = methodBaseDescr_.begin();
	for(; iter3 != methodBaseDescr_.end(); ++iter3)
	{
		DEBUG_MSG(fmt::format("ScriptDefModule::c_str: {}.BaseMethod {} uid={}, argssize={}, aliasID={}{}.\n",
			getName(), iter3->second->getName(), iter3->second->getUType(),
			iter3->second->getArgSize(), iter3->second->aliasID(), (iter3->second->isExposed() ? ", exposed=true" : ", exposed=false")));
	}

	METHODDESCRIPTION_MAP::iterator iter4 = methodClientDescr_.begin();
	for(; iter4 != methodClientDescr_.end(); ++iter4)
	{
		DEBUG_MSG(fmt::format("ScriptDefModule::c_str: {}.ClientMethod {} uid={}, argssize={}, aliasID={}.\n",
			getName(), iter4->second->getName(), iter4->second->getUType(), iter4->second->getArgSize(), iter4->second->aliasID()));
	}

	DEBUG_MSG(fmt::format("ScriptDefModule::c_str: [{}], cellPropertys={}, basePropertys={}, "
		"clientPropertys={}, cellMethods={}({}), baseMethods={}({}), clientMethods={}\n",
		getName(), 
		getCellPropertyDescriptions().size(), 
		getBasePropertyDescriptions().size(), 
		getClientPropertyDescriptions().size(), 
		getCellMethodDescriptions().size(), 
		getCellExposedMethodDescriptions().size(), 
		getBaseMethodDescriptions().size(), 
		getBaseExposedMethodDescriptions().size(), 
		getClientMethodDescriptions().size()));
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::setUType(ENTITY_SCRIPT_UID utype)
{ 
	uType_ = utype; 
	EntityDef::md5().append((void*)&uType_, sizeof(ENTITY_SCRIPT_UID));
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::addSmartUTypeToStream(MemoryStream* pStream)
{
	if(EntityDef::scriptModuleAliasID())
		(*pStream) << getAliasID();
	else
		(*pStream) << getUType();
}

//-------------------------------------------------------------------------------------
void ScriptDefModule::addSmartUTypeToBundle(Network::Bundle* pBundle)
{
	if(EntityDef::scriptModuleAliasID())
		(*pBundle) << getAliasID();
	else
		(*pBundle) << getUType();
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
	/*
		entity����ĳ����(cell, base, client)���ж�����

		1: entitydef�ļ��д���ʵ��ĳ���ֵķ����������ԣ�ͬʱҲ����Ҳ����py�ű�
		2: �û���entities.xml��ȷ��������ĳʵ�岿��(Ϊ��unity3d����html5���ǰ���޷�����py�Ļ�������)
			entities.xml�� <Spaces hasCell="true" hasClient="false", hasBase="true"></Spaces>
	*/

	std::string entitiesFile = Resmgr::getSingleton().getPyUserScriptsPath() + "entities.xml";

	// �����entities.xml�ļ�
	SmartPointer<XML> xml(new XML());
	if(!xml->openSection(entitiesFile.c_str()) || !xml->isGood())
		return;
	
	// ���entities.xml���ڵ�, ���û�ж���һ��entity��ôֱ�ӷ���true
	TiXmlNode* node = xml->getRootNode();
	if(node == NULL)
		return;

	int assertionHasClient = -1;
	int assertionHasBase = -1;
	int assertionHasCell = -1;

	// ��ʼ�������е�entity�ڵ�
	XML_FOR_BEGIN(node)
	{
		std::string moduleName = xml.get()->getKey(node);
		if(name_ == moduleName)
		{
			const char* val = node->ToElement()->Attribute("hasClient");
			if(val)
			{
				if(kbe_strnicmp(val, "true", strlen(val)) == 0)
					assertionHasClient = 1;
				else
					assertionHasClient = 0;
			}

			EntityDef::md5().append((void*)&assertionHasClient, sizeof(int));

			val = node->ToElement()->Attribute("hasCell");
			if(val)
			{
				if(kbe_strnicmp(val, "true", strlen(val)) == 0)
					assertionHasCell = 1;
				else
					assertionHasCell = 0;
			}

			EntityDef::md5().append((void*)&assertionHasCell, sizeof(int));

			val = node->ToElement()->Attribute("hasBase");
			if(val)
			{
				if(kbe_strnicmp(val, "true", strlen(val)) == 0)
					assertionHasBase = 1;
				else
					assertionHasBase = 0;
			}

			EntityDef::md5().append((void*)&assertionHasBase, sizeof(int));
			break;
		}
	}
	XML_FOR_END(node);

	std::string fmodule = "scripts/client/" + name_ + ".py";
	std::string fmodule_pyc = fmodule + "c";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		setClient(true);
	}
	else
	{
		if(assertionHasClient < 0)
		{
			// ����û���������ȷ����������Ϊû�ж�Ӧʵ�岿��
			// ��������ԭ���������û���def�ļ������ⲿ�ֵ�����(��Ϊinterface�Ĵ��ڣ�interface�п��ܻ���ڿͻ������Ի��߷���)
			// ������ű���������Ȼ��Ϊ�û���ǰ����Ҫ�ò���
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setClient(false);
		}
		else
		{
			// �û���ȷ�������������趨
			setClient(assertionHasClient == 1);
		}
	}

	if(g_componentType == CLIENT_TYPE)
	{
		setBase(true);
		setCell(true);
		return;
	}

	fmodule = "scripts/base/" + name_ + ".py";
	fmodule_pyc = fmodule + "c";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		setBase(true);
	}
	else
	{
		if(assertionHasBase < 0)
		{
			// ����û���������ȷ����������Ϊû�ж�Ӧʵ�岿��
			// ��������ԭ���������û���def�ļ������ⲿ�ֵ�����(��Ϊinterface�Ĵ��ڣ�interface�п��ܻ����base���Ի��߷���)
			// ������ű���������Ȼ��Ϊ�û���ǰ����Ҫ�ò���
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setBase(false);
		}
		else
		{
			// �û���ȷ�������������趨
			setBase(assertionHasBase == 1);
		}
	}

	fmodule = "scripts/cell/" + name_ + ".py";
	fmodule_pyc = fmodule + "c";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		setCell(true);
	}
	else
	{
		if(assertionHasCell < 0)
		{
			// ����û���������ȷ����������Ϊû�ж�Ӧʵ�岿��
			// ��������ԭ���������û���def�ļ������ⲿ�ֵ�����(��Ϊinterface�Ĵ��ڣ�interface�п��ܻ����cell���Ի��߷���)
			// ������ű���������Ȼ��Ϊ�û���ǰ����Ҫ�ò���
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setCell(false);
		}
		else
		{
			// �û���ȷ�������������趨
			setCell(assertionHasCell == 1);
		}
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
			
			// �ж�������ʲô��������ԣ� ���䱣�浽��ӦdetailLevel�ĵط�
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
		ERROR_MSG(fmt::format("ScriptDefModule::addPropertyDescription: [{}] is exist! componentType={}.\n",
			attrName, componentType));

		return false;
	}

	(*propertyDescr)[attrName] = propertyDescription;
	(*propertyDescr_uidmap)[propertyDescription->getUType()] = propertyDescription;
	propertyDescription->incRef();


	// �ж��Ƿ��Ǵ洢���ԣ� �Ǿʹ洢��persistentPropertyDescr_
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
PropertyDescription* ScriptDefModule::findAliasPropertyDescription(ENTITY_DEF_ALIASID aliasID)
{
	PROPERTYDESCRIPTION_ALIASMAP::iterator iter = propertyDescr_aliasmap_.find(aliasID);

	if(iter == propertyDescr_aliasmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findAliasPropertyDescription: [%ld] not found!\n", aliasID);
		return NULL;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
MethodDescription* ScriptDefModule::findAliasMethodDescription(ENTITY_DEF_ALIASID aliasID)
{
	METHODDESCRIPTION_ALIASMAP::iterator iter = methodDescr_aliasmap_.find(aliasID);
	if(iter == methodDescr_aliasmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findAliasMethodDescription: [%s] not found!\n", aliasID);
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
		ERROR_MSG(fmt::format("ScriptDefModule::addCellMethodDescription: [{}] is exist!\n", attrName));
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
		ERROR_MSG(fmt::format("ScriptDefModule::addBaseMethodDescription: [{}] is exist!\n", 
			attrName));

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
		ERROR_MSG(fmt::format("ScriptDefModule::addClientMethodDescription: [{}] is exist!\n",
			attrName));

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
