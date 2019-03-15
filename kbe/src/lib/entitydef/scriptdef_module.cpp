// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "scriptdef_module.h"
#include "entitydef.h"
#include "py_entitydef.h"
#include "datatypes.h"
#include "common.h"
#include "common/smartpointer.h"
#include "entitydef/entity_call.h"
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
pVolatileinfo_(new VolatileInfo()),
name_(name),
usePropertyDescrAlias_(false),
useMethodDescrAlias_(false),
useComponentDescrAlias_(false),
componentDescr_uidmap_(),
componentDescr_(),
componentPropertyDescr_(),
persistent_(true),
isComponentModule_(false)
{
	EntityDef::md5().append((void*)name.c_str(), (int)name.size());
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
	S_RELEASE(pVolatileinfo_);

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

		PROPERTYDESCRIPTION_MAP::iterator iter1 = clientPropertyDescr_.begin();
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

		// 不能为0，0表示不可用，至少是1
		aliasID = 1;

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

		// 组件是否使用aliasID
		if (componentDescr_.size() <= 255)
		{
			useComponentDescrAlias_ = true;

			COMPONENTDESCRIPTION_MAP::iterator iter3 = componentDescr_.begin();
			for (; iter3 != componentDescr_.end(); ++iter3)
			{
				componentDescrVec_.push_back(iter3->second);
			}
		}

		COMPONENTDESCRIPTION_MAP::iterator comp_iter =	componentDescr_.begin();
		for (; comp_iter != componentDescr_.end(); ++comp_iter)
		{
			// 组件内的属性和方法计算aliasID
			comp_iter->second->onLoaded();
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
	if (isComponentModule())
	{
		std::string fmodule = "scripts/base/components/" + name_ + ".py";
		std::string fmodule_pyc = fmodule + "c";
		if (Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
			Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
		{
			setBase(true);
		}

		fmodule = "scripts/cell/components/" + name_ + ".py";
		fmodule_pyc = fmodule + "c";
		if (Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
			Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
		{
			setCell(true);
		}

		if (!hasClient())
		{
			// 如果是组件， 并且服务器上没有脚本或者exposed方法不需要产生代码
			if ((hasBase() && getBaseExposedMethodDescriptions().size() > 0) ||
				(hasCell() && getCellExposedMethodDescriptions().size() > 0))
				setClient(true);
		}

		return;
	}

	/*
		entity存在某部分(cell, base, client)的判定规则

		1: entitydef文件中存在实体某部分的方法或者属性，同时也必须也存在py脚本
		2: 用户在entities.xml明确声明存在某实体部分(为了unity3d或者html5类的前端无法加载py的环境考虑)
			entities.xml， <Spaces hasCell="true" hasClient="false", hasBase="true"></Spaces>
	*/

	int assertionHasClient = -1;
	int assertionHasBase = -1;
	int assertionHasCell = -1;

	std::string entitiesFile = Resmgr::getSingleton().getPyUserScriptsPath() + "entities.xml";

	// 打开这个entities.xml文件
	// 允许纯脚本定义，则可能没有这个文件
	if (access(entitiesFile.c_str(), 0) == 0)
	{
		SmartPointer<XML> xml(new XML());
		if (!xml->openSection(entitiesFile.c_str()) || !xml->isGood())
			return;

		// 获得entities.xml根节点, 如果没有定义一个entity那么直接返回true
		TiXmlNode* node = xml->getRootNode();
		if (node == NULL)
			return;

		// 开始遍历所有的entity节点
		XML_FOR_BEGIN(node)
		{
			std::string moduleName = xml.get()->getKey(node);
			if (name_ == moduleName)
			{
				const char* val = node->ToElement()->Attribute("hasClient");
				if (val)
				{
					if (kbe_strnicmp(val, "true", strlen(val)) == 0)
						assertionHasClient = 1;
					else
						assertionHasClient = 0;
				}

				EntityDef::md5().append((void*)&assertionHasClient, sizeof(int));

				val = node->ToElement()->Attribute("hasCell");
				if (val)
				{
					if (kbe_strnicmp(val, "true", strlen(val)) == 0)
						assertionHasCell = 1;
					else
						assertionHasCell = 0;
				}

				EntityDef::md5().append((void*)&assertionHasCell, sizeof(int));

				val = node->ToElement()->Attribute("hasBase");
				if (val)
				{
					if (kbe_strnicmp(val, "true", strlen(val)) == 0)
						assertionHasBase = 1;
					else
						assertionHasBase = 0;
				}

				EntityDef::md5().append((void*)&assertionHasBase, sizeof(int));
				break;
			}
		}
		XML_FOR_END(node);
	}

	// 检查PyEntityDef
	script::entitydef::DefContext* pDefContext = script::entitydef::DefContext::findDefContext(name_);
	if (pDefContext)
	{
		if (pDefContext->hasClient)
			assertionHasClient = 1;
		else
			assertionHasClient = 0;

		EntityDef::md5().append((void*)&assertionHasClient, sizeof(int));
	}

	std::string fmodule = "scripts/client/" + name_ + ".py";
	std::string fmodule_pyc = fmodule + "c";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		if (assertionHasClient < 0)
		{
			// 如果用户不存在明确声明并设置为没有对应实体部分
			// 这样做的原因是允许用户在def文件定义这部分的内容(因为interface的存在，interface中可能会存在客户端属性或者方法)
			// 但如果脚本不存在仍然认为用户当前不需要该部分
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setClient(true);
		}
		else
		{
			// 用户明确声明并进行了设定
			setClient(assertionHasClient == 1);
		}
	}
	else
	{
		if(assertionHasClient < 0)
		{
			// 如果用户不存在明确声明并设置为没有对应实体部分
			// 这样做的原因是允许用户在def文件定义这部分的内容(因为interface的存在，interface中可能会存在客户端属性或者方法)
			// 但如果脚本不存在仍然认为用户当前不需要该部分
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setClient(false);
		}
		else
		{
			// 用户明确声明并进行了设定
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
		if (assertionHasBase < 0)
		{
			// 如果用户不存在明确声明并设置为没有对应实体部分
			// 这样做的原因是允许用户在def文件定义这部分的内容(因为interface的存在，interface中可能会存在base属性或者方法)
			// 但如果脚本不存在仍然认为用户当前不需要该部分
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setBase(true);
		}
		else
		{
			// 用户明确声明并进行了设定
			setBase(assertionHasBase == 1);
		}
	}
	else
	{
		if(assertionHasBase < 0)
		{
			// 如果用户不存在明确声明并设置为没有对应实体部分
			// 这样做的原因是允许用户在def文件定义这部分的内容(因为interface的存在，interface中可能会存在base属性或者方法)
			// 但如果脚本不存在仍然认为用户当前不需要该部分
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setBase(false);
		}
		else
		{
			// 用户明确声明并进行了设定
			setBase(assertionHasBase == 1);
		}
	}

	fmodule = "scripts/cell/" + name_ + ".py";
	fmodule_pyc = fmodule + "c";
	if(Resmgr::getSingleton().matchRes(fmodule) != fmodule ||
		Resmgr::getSingleton().matchRes(fmodule_pyc) != fmodule_pyc)
	{
		if (assertionHasCell < 0)
		{
			// 如果用户不存在明确声明并设置为没有对应实体部分
			// 这样做的原因是允许用户在def文件定义这部分的内容(因为interface的存在，interface中可能会存在cell属性或者方法)
			// 但如果脚本不存在仍然认为用户当前不需要该部分
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setCell(true);
		}
		else
		{
			// 用户明确声明并进行了设定
			setCell(assertionHasCell == 1);
		}
	}
	else
	{
		if(assertionHasCell < 0)
		{
			// 如果用户不存在明确声明并设置为没有对应实体部分
			// 这样做的原因是允许用户在def文件定义这部分的内容(因为interface的存在，interface中可能会存在cell属性或者方法)
			// 但如果脚本不存在仍然认为用户当前不需要该部分
			// http://www.kbengine.org/cn/docs/configuration/entities.html 
			setCell(false);
		}
		else
		{
			// 用户明确声明并进行了设定
			setCell(assertionHasCell == 1);
		}
	}
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::addPropertyDescription(const char* attrName, 
										  PropertyDescription* propertyDescription, 
										  COMPONENT_TYPE componentType, bool ignoreConflict)
{
	if(!ignoreConflict && hasMethodName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addPropertyDescription: There is a method[{}] name conflict! componentType={}.\n",
			attrName, componentType));
		
		return false;
	}
	
	if (!ignoreConflict && hasComponentName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addPropertyDescription: There is a component[{}] name conflict!\n",
			attrName));

		return false;
	}

	bool isEntityComponent = propertyDescription->getDataType() && 
		std::string("ENTITY_COMPONENT") == propertyDescription->getDataType()->getName();

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
		ERROR_MSG(fmt::format("ScriptDefModule::addPropertyDescription: [{}] is exist! componentType={}.\n",
			attrName, componentType));

		return false;
	}

	(*propertyDescr)[attrName] = propertyDescription;
	(*propertyDescr_uidmap)[propertyDescription->getUType()] = propertyDescription;
	propertyDescription->incRef();

	if(isEntityComponent)
		componentPropertyDescr_[attrName] = propertyDescription;

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
	if(hasPropertyName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addCellMethodDescription: There is a property[{}] name conflict!\n",
			attrName));
		
		return false;
	}
	
	if (hasComponentName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addCellMethodDescription: There is a component[{}] name conflict!\n",
			attrName));

		return false;
	}

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
	if(hasPropertyName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addBaseMethodDescription: There is a property[{}] name conflict!\n",
			attrName));
		
		return false;
	}
	
	if (hasComponentName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addBaseMethodDescription: There is a component[{}] name conflict!\n",
			attrName));

		return false;
	}

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
	if(hasPropertyName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addClientMethodDescription: There is a property[{}] name conflict!\n",
			attrName));
		
		return false;
	}
	
	if (hasComponentName(attrName))
	{
		ERROR_MSG(fmt::format("ScriptDefModule::addClientMethodDescription: There is a component[{}] name conflict!\n",
			attrName));

		return false;
	}

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
bool ScriptDefModule::hasPropertyName(const std::string& name)
{
	return findPropertyDescription(name.c_str(), CELLAPP_TYPE) ||
		findPropertyDescription(name.c_str(), BASEAPP_TYPE) ||
		findPropertyDescription(name.c_str(), CLIENT_TYPE); 
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::hasMethodName(const std::string& name)
{
	return findMethodDescription(name.c_str(), CELLAPP_TYPE) ||
		findMethodDescription(name.c_str(), BASEAPP_TYPE) ||
		findMethodDescription(name.c_str(), CLIENT_TYPE);
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::hasComponentName(const std::string& name)
{
	return findComponentDescription(name.c_str());
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::hasName(const std::string& name)
{
	return hasPropertyName(name) || hasMethodName(name) || hasComponentName(name);
}

//-------------------------------------------------------------------------------------
bool ScriptDefModule::addComponentDescription(const char* compName,
	ScriptDefModule* compDescription)
{
	componentDescr_[compName] = compDescription;
	componentDescr_uidmap_[compDescription->getUType()] = compDescription;
	return true;
}

//-------------------------------------------------------------------------------------
ScriptDefModule* ScriptDefModule::findComponentDescription(const char* compName)
{
	COMPONENTDESCRIPTION_MAP::iterator iter = componentDescr_.find(compName);
	if (iter == componentDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findComponentDescription: [{}] not found!\n", compName);
		return NULL;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
ScriptDefModule* ScriptDefModule::findComponentDescription(ENTITY_PROPERTY_UID utype)
{
	COMPONENTDESCRIPTION_UIDMAP::iterator iter = componentDescr_uidmap_.find(utype);
	if (iter == componentDescr_uidmap_.end())
	{
		//ERROR_MSG("ScriptDefModule::findComponentDescription: [%ld] not found!\n", utype);
		return NULL;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
ScriptDefModule* ScriptDefModule::findComponentDescription(ENTITY_COMPONENT_ALIASID aliasID)
{
	if (componentDescrVec_.size() <= aliasID)
	{
		//ERROR_MSG("ScriptDefModule::findComponentDescription: [%s] not found!\n", aliasID);
		return NULL;
	}

	return componentDescrVec_[aliasID];
}

//-------------------------------------------------------------------------------------
PropertyDescription* ScriptDefModule::findComponentPropertyDescription(const char* attrName)
{
	COMPONENTPROPERTYDESCRIPTION_MAP::iterator iter = componentPropertyDescr_.find(attrName);
	if (iter == componentPropertyDescr_.end())
	{
		//ERROR_MSG("ScriptDefModule::findComponentPropertyDescription: [%s] not found!\n", attrName);
		return NULL;
	}
	return iter->second;
}

//-------------------------------------------------------------------------------------
}
