// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENTITY_MACRO_H
#define KBE_ENTITY_MACRO_H

#include "common/common.h"
#include "server/callbackmgr.h"		

namespace KBEngine{

#define ENTITY_METHOD_DECLARE_BEGIN(APP, CLASS)																						\
	ENTITY_CPP_IMPL(APP, CLASS)																										\
	SCRIPT_METHOD_DECLARE_BEGIN(CLASS)																								\
	SCRIPT_METHOD_DECLARE("__reduce_ex__",					reduce_ex__,						METH_VARARGS,					0)	\
	SCRIPT_METHOD_DECLARE("__getDEP__",						pyGetDatachangeEventPtr,			METH_VARARGS,					0)	\
	SCRIPT_METHOD_DECLARE("addTimer",						pyAddTimer,							METH_VARARGS,					0)	\
	SCRIPT_METHOD_DECLARE("delTimer",						pyDelTimer,							METH_VARARGS,					0)	\
	SCRIPT_METHOD_DECLARE("writeToDB",						pyWriteToDB,						METH_VARARGS,					0)	\
	SCRIPT_METHOD_DECLARE("destroy",						pyDestroyEntity,					METH_VARARGS | METH_KEYWORDS,	0)	\
	SCRIPT_METHOD_DECLARE("registerEvent",					pyRegisterEvent,					METH_VARARGS | METH_KEYWORDS,	0)	\
	SCRIPT_METHOD_DECLARE("deregisterEvent",				pyDeregisterEvent,					METH_VARARGS | METH_KEYWORDS,	0)	\
	SCRIPT_METHOD_DECLARE("fireEvent",						pyFireEvent,						METH_VARARGS | METH_KEYWORDS,	0)	\
	SCRIPT_METHOD_DECLARE("getComponent",					pyGetComponent,						METH_VARARGS | METH_KEYWORDS,	0)	\

	
#define ENTITY_METHOD_DECLARE_END()																									\
	SCRIPT_METHOD_DECLARE_END()																										\


#define ENTITY_GETSET_DECLARE_BEGIN(CLASS)																							\
	SCRIPT_GETSET_DECLARE_BEGIN(CLASS)																								\
	SCRIPT_GET_DECLARE("id",								pyGetID,							0,								0)	\
	SCRIPT_GET_DECLARE("isDestroyed",						pyGetIsDestroyed,					0,								0)	\
	SCRIPT_GET_DECLARE("className",							pyGetClassName,						0,								0)	\


#define ENTITY_GETSET_DECLARE_END()																									\
	SCRIPT_GETSET_DECLARE_END()																										\


#define CLIENT_ENTITY_METHOD_DECLARE_BEGIN(APP, CLASS)																				\
	ENTITY_CPP_IMPL(APP, CLASS)																										\
	SCRIPT_METHOD_DECLARE_BEGIN(CLASS)																								\
	SCRIPT_METHOD_DECLARE("__reduce_ex__",					reduce_ex__,						METH_VARARGS,					0)	\
	SCRIPT_METHOD_DECLARE("__getDEP__",						pyGetDatachangeEventPtr,			METH_VARARGS,					0)	\
	SCRIPT_METHOD_DECLARE("registerEvent",					pyRegisterEvent,					METH_VARARGS | METH_KEYWORDS,	0)	\
	SCRIPT_METHOD_DECLARE("deregisterEvent",				pyDeregisterEvent,					METH_VARARGS | METH_KEYWORDS,	0)	\
	SCRIPT_METHOD_DECLARE("fireEvent",						pyFireEvent,						METH_VARARGS | METH_KEYWORDS,	0)	\

	
#define CLIENT_ENTITY_METHOD_DECLARE_END()																							\
	SCRIPT_METHOD_DECLARE_END()																										\


#define CLIENT_ENTITY_GETSET_DECLARE_BEGIN(CLASS)																					\
	SCRIPT_GETSET_DECLARE_BEGIN(CLASS)																								\
	SCRIPT_GET_DECLARE("id",								pyGetID,							0,								0)	\
	SCRIPT_GET_DECLARE("spaceID",							pyGetSpaceID,						0,								0)	\
	SCRIPT_GET_DECLARE("isDestroyed",						pyGetIsDestroyed,					0,								0)	\
	SCRIPT_GET_DECLARE("className",							pyGetClassName,						0,								0)	\


#define CLIENT_ENTITY_GETSET_DECLARE_END()																	\
	SCRIPT_GETSET_DECLARE_END()																				\


#ifdef CLIENT_NO_FLOAT																					
	#define ADD_POS_DIR_TO_STREAM(s, pos, dir)																\
		int32 x = (int32)pos.x;																				\
		int32 y = (int32)pos.y;																				\
		int32 z = (int32)pos.z;																				\
																											\
		s << (ENTITY_PROPERTY_UID)0 << posuid << x << y << z;												\
																											\
		x = (int32)dir.x;																					\
		y = (int32)dir.y;																					\
		z = (int32)dir.z;																					\
																											\
		s << (ENTITY_PROPERTY_UID)0 << diruid << x << y << z;												\


	#define ADD_POS_DIR_TO_STREAM_ALIASID(s, pos, dir)														\
		int32 x = (int32)pos.x;																				\
		int32 y = (int32)pos.y;																				\
		int32 z = (int32)pos.z;																				\
																											\
		uint8 aliasID = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;											\
		s << (uint8)0 << aliasID << x << y << z;															\
																											\
		x = (int32)dir.x;																					\
		y = (int32)dir.y;																					\
		z = (int32)dir.z;																					\
																											\
		aliasID = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;									\
		s << (uint8)0 << aliasID << x << y << z;															\


	#define STREAM_TO_POS_DIR(s, pos, dir)																	\
	{																										\
		int32 x = 0;																						\
		int32 y = 0;																						\
		int32 z = 0;																						\
		ENTITY_PROPERTY_UID uid;																			\
																											\
		s >> uid >> uid >> x >> y >> z;																		\
																											\
		pos.x = float(x);																					\
		pos.y = float(y);																					\
		pos.z = float(z);																					\
																											\
		s >> uid >> uid >> x >> y >> z;																		\
		dir.x = float(x);																					\
		dir.y = float(y);																					\
		dir.z = float(z);																					\
	}																										\


#else																									
	#define ADD_POS_DIR_TO_STREAM(s, pos, dir)																\
		s << (ENTITY_PROPERTY_UID)0 << posuid << pos.x << pos.y << pos.z;									\
		s << (ENTITY_PROPERTY_UID)0 << diruid << dir.x << dir.y << dir.z;									\


	#define ADD_POS_DIR_TO_STREAM_ALIASID(s, pos, dir)														\
		uint8 aliasID = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;											\
		s << (uint8)0 << aliasID << pos.x << pos.y << pos.z;												\
		aliasID = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;									\
		s << (uint8)0 << aliasID << dir.x << dir.y << dir.z;												\
	

	#define STREAM_TO_POS_DIR(s, pos, dir)																	\
	{																										\
		ENTITY_PROPERTY_UID uid;																			\
		s >> uid >> uid >> pos.x >> pos.y >> pos.z;															\
		s >> uid >> uid >> dir.x >> dir.y >> dir.z;															\
	}																										\


#endif	


#define ADD_POSDIR_TO_STREAM(s, pos, dir)																	\
	{																										\
		ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;								\
		ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;					\
																											\
		Network::FixedMessages::MSGInfo* msgInfo =															\
					Network::FixedMessages::getSingleton().isFixed("Property::position");					\
																											\
		if(msgInfo != NULL)																					\
		{																									\
			posuid = msgInfo->msgid;																		\
			msgInfo = NULL;																					\
		}																									\
																											\
		msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");					\
		if(msgInfo != NULL)																					\
		{																									\
			diruid = msgInfo->msgid;																		\
			msgInfo = NULL;																					\
		}																									\
																											\
		ADD_POS_DIR_TO_STREAM(s, pos, dir)																	\
																											\
	}																										\

#define ADD_POSDIR_TO_PYDICT(pydict, pos, dir)																\
	{																										\
		PyObject* pypos = PyTuple_New(3);																	\
		PyObject* pydir = PyTuple_New(3);																	\
																											\
		PyTuple_SET_ITEM(pypos, 0, PyFloat_FromDouble(pos.x));												\
		PyTuple_SET_ITEM(pypos, 1, PyFloat_FromDouble(pos.y));												\
		PyTuple_SET_ITEM(pypos, 2, PyFloat_FromDouble(pos.z));												\
																											\
		PyTuple_SET_ITEM(pydir, 0, PyFloat_FromDouble(dir.x));												\
		PyTuple_SET_ITEM(pydir, 1, PyFloat_FromDouble(dir.y));												\
		PyTuple_SET_ITEM(pydir, 2, PyFloat_FromDouble(dir.z));												\
																											\
		PyDict_SetItemString(pydict, "position", pypos);													\
		PyDict_SetItemString(pydict, "direction", pydir);													\
		Py_DECREF(pypos);																					\
		Py_DECREF(pydir);																					\
	}																										\

/*
	debug info.
*/
#define CAN_DEBUG_CREATE_ENTITY
#ifdef CAN_DEBUG_CREATE_ENTITY
#define DEBUG_CREATE_ENTITY_NAMESPACE																		\
		if(g_debugEntity)																					\
		{																									\
			const char* ccattr_DEBUG_CREATE_ENTITY_NAMESPACE = PyUnicode_AsUTF8AndSize(key, NULL);			\
			PyObject* pytsval = PyObject_Str(value);														\
			const char* cccpytsval = PyUnicode_AsUTF8AndSize(pytsval, NULL);								\
			Py_DECREF(pytsval);																				\
			DEBUG_MSG(fmt::format("{}(refc={}, id={})::debug_createNamespace:add {}({}).\n",				\
												scriptName(),												\
												static_cast<PyObject*>(this)->ob_refcnt,					\
												this->id(),													\
																ccattr_DEBUG_CREATE_ENTITY_NAMESPACE,		\
																cccpytsval));								\
		}																									\


#define DEBUG_OP_ATTRIBUTE(op, ccattr)																		\
		if(g_debugEntity)																					\
		{																									\
			const char* ccattr_DEBUG_OP_ATTRIBUTE = PyUnicode_AsUTF8AndSize(ccattr, NULL);					\
			DEBUG_MSG(fmt::format("{}(refc={}, id={})::debug_op_attr:op={}, {}.\n",							\
												scriptName(),												\
												static_cast<PyObject*>(this)->ob_refcnt, this->id(),		\
															op, ccattr_DEBUG_OP_ATTRIBUTE));				\
		}																									\


#define DEBUG_PERSISTENT_PROPERTY(op, ccattr)																\
	{																										\
		if(g_debugEntity)																					\
		{																									\
			DEBUG_MSG(fmt::format("{}(refc={}, id={})::debug_op_Persistent:op={}, {}.\n",					\
												scriptName(),												\
												static_cast<PyObject*>(this)->ob_refcnt, this->id(),		\
															op, ccattr));									\
		}																									\
	}																										\


#define DEBUG_REDUCE_EX(tentity)																			\
		if(g_debugEntity)																					\
		{																									\
			DEBUG_MSG(fmt::format("{}(refc={}, id={})::debug_reduct_ex: utype={}.\n",						\
												tentity->scriptName(),										\
												static_cast<PyObject*>(tentity)->ob_refcnt,					\
												tentity->id(),												\
												tentity->pScriptModule()->getUType()));						\
		}																									\


#else
	#define DEBUG_CREATE_ENTITY_NAMESPACE			
	#define DEBUG_OP_ATTRIBUTE(op, ccattr)
	#define DEBUG_PERSISTENT_PROPERTY(op, ccattr)
	#define DEBUG_REDUCE_EX(tentity)
#endif


#define ENTITY_DESTROYED_CHECK(RETURN, OPNAME, ENTITY)														\
{																											\
	if(ENTITY->isDestroyed())																				\
	{																										\
		PyErr_Format(PyExc_Exception, "%s::%s: %d is destroyed!\n",											\
			OPNAME, ENTITY->scriptName(), ENTITY->id());													\
		PyErr_PrintEx(0);																					\
		RETURN;																								\
	}																										\
}																											\


// 实体的标志
#define ENTITY_FLAGS_UNKNOWN						0x00000000
#define ENTITY_FLAGS_DESTROYING						0x00000001
#define ENTITY_FLAGS_INITING						0x00000002
#define ENTITY_FLAGS_TELEPORT_START					0x00000004
#define ENTITY_FLAGS_TELEPORT_STOP					0x00000008
#define ENTITY_FLAGS_DESTROY_AFTER_GETCELL			0x00000010

#define ENTITY_HEADER(CLASS)																				\
public:																										\
	typedef KBEUnordered_map< std::string, std::vector<PyObjectPtr> > ENTITY_EVENTS;						\
protected:																									\
	ENTITY_ID													id_;										\
	ScriptDefModule*											pScriptModule_;								\
	const ScriptDefModule::PROPERTYDESCRIPTION_MAP*				pPropertyDescrs_;							\
	SPACE_ID													spaceID_;									\
	ScriptTimers												scriptTimers_;								\
	PY_CALLBACKMGR												pyCallbackMgr_;								\
	bool														isDestroyed_;								\
	uint32														flags_;										\
	ENTITY_EVENTS												events_;									\
public:																										\
																											\
	bool initing() const{ return hasFlags(ENTITY_FLAGS_INITING); }											\
																											\
	void onInitializeScript();																				\
	void initializeScript()																					\
	{																										\
		removeFlags(ENTITY_FLAGS_INITING);																	\
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);																	\
																											\
		const ScriptDefModule::COMPONENTDESCRIPTION_MAP* pComponentDescrs =									\
			&pScriptModule_->getComponentDescrs();															\
																											\
		ScriptDefModule::COMPONENTDESCRIPTION_MAP::const_iterator iter1 = pComponentDescrs->begin();		\
		for (; iter1 != pComponentDescrs->end(); ++iter1)													\
		{																									\
			PyObject* pComponentProperty = PyObject_GetAttrString(this, iter1->first.c_str());				\
			if(pComponentProperty)																			\
			{																								\
				if(PyObject_TypeCheck(pComponentProperty, EntityComponent::getScriptType()))				\
				{																							\
					EntityComponent* pEntityComponent = static_cast<EntityComponent*>(pComponentProperty);	\
					pEntityComponent->initializeScript();													\
				}																							\
				else																						\
				{																							\
					PyErr_Format(PyExc_AssertionError, "%s.%s is not property of EntityComponent!",			\
						scriptName(), iter1->first.c_str());												\
					PyErr_PrintEx(0);																		\
				}																							\
																											\
				Py_DECREF(pComponentProperty);																\
			}																								\
			else																							\
			{																								\
				PyErr_Clear();																				\
			}																								\
		}																									\
																											\
		if(PyObject_HasAttrString(this, "__init__"))														\
		{																									\
			PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("__init__"),					\
											const_cast<char*>(""));											\
			if(pyResult != NULL)																			\
				Py_DECREF(pyResult);																		\
			else																							\
				SCRIPT_ERROR_CHECK();																		\
		}																									\
																											\
		iter1 = pComponentDescrs->begin();																	\
		for (; iter1 != pComponentDescrs->end(); ++iter1)													\
		{																									\
			PyObject* pComponentProperty = PyObject_GetAttrString(this, iter1->first.c_str());				\
			if(pComponentProperty)																			\
			{																								\
				if(PyObject_TypeCheck(pComponentProperty, EntityComponent::getScriptType()))				\
				{																							\
					EntityComponent* pEntityComponent = static_cast<EntityComponent*>(pComponentProperty);	\
					pEntityComponent->updateOwner(id(), this);												\
					pEntityComponent->onAttached();															\
				}																							\
				else																						\
				{																							\
					PyErr_Format(PyExc_AssertionError, "%s.%s is not property of EntityComponent!",			\
						scriptName(), iter1->first.c_str());												\
					PyErr_PrintEx(0);																		\
				}																							\
																											\
				Py_DECREF(pComponentProperty);																\
			}																								\
			else																							\
			{																								\
				PyErr_Clear();																				\
			}																								\
		}																									\
																											\
		onInitializeScript();																				\
	}																										\
																											\
	void initializeEntity(PyObject* dictData, bool persistentData = false)									\
	{																										\
		createNamespace(dictData, persistentData);															\
		initializeScript();																					\
	}																										\
																											\
	bool _reload(bool fullReload);																			\
	bool reload(bool fullReload)																			\
	{																										\
		if(fullReload)																						\
		{																									\
			pScriptModule_ = EntityDef::findScriptModule(scriptName());										\
			KBE_ASSERT(pScriptModule_);																		\
			pPropertyDescrs_ = &pScriptModule_->getPropertyDescrs();										\
		}																									\
																											\
		if(PyObject_SetAttrString(this, "__class__", (PyObject*)pScriptModule_->getScriptType()) == -1)		\
		{																									\
			WARNING_MSG(fmt::format("Entity::reload: "														\
				"{} {} could not change __class__ to new class!\n",											\
				pScriptModule_->getName(), id_));															\
			PyErr_Print();																					\
			return false;																					\
		}																									\
																											\
		initProperty(true);																					\
		return _reload(fullReload);																			\
	}																										\
																											\
	void createNamespace(PyObject* dictData, bool persistentData = false)									\
	{																										\
		if(dictData == NULL)																				\
			return;																							\
																											\
		if(!PyDict_Check(dictData)){																		\
			ERROR_MSG(fmt::format(#CLASS"::createNamespace: create"#CLASS"[{}:{}] "							\
				"args is not a dict.\n",																	\
				scriptName(), id_));																		\
			return;																							\
		}																									\
																											\
		EntityDef::context().currComponentType = g_componentType;											\
		EntityDef::context().currEntityID = id();															\
																											\
		Py_ssize_t pos = 0;																					\
		PyObject *key, *value;																				\
		PyObject* cellDataDict = PyObject_GetAttrString(this, "cellData");									\
																											\
		if(cellDataDict == NULL)																			\
		{																									\
			PyErr_Clear();																					\
			EntityComponent::convertDictDataToEntityComponent(id(), this, pScriptModule_, dictData, persistentData); \
		}																									\
		else																								\
		{																									\
			PyObject* cellDataDictNew = PyDict_GetItemString(dictData, "cellData");							\
			if (cellDataDictNew)																			\
			{																								\
				if(PyDict_Check(cellDataDictNew))															\
				{																							\
					PyDict_Update(cellDataDict, cellDataDictNew);											\
				}																							\
				else																						\
				{																							\
					ERROR_MSG(fmt::format(#CLASS"::createNamespace: create"#CLASS"[{}:{}] "					\
						"cellData is not a dict.\n",														\
						scriptName(), id_));																\
				}																							\
																											\
				PyDict_DelItemString(dictData, "cellData");													\
			}																								\
			else																							\
			{																								\
				PyErr_Clear();																				\
			}																								\
		}																									\
																											\
		while(PyDict_Next(dictData, &pos, &key, &value))													\
		{																									\
			DEBUG_CREATE_ENTITY_NAMESPACE																	\
			if(PyObject_HasAttr(this, key) > 0)																\
			{																								\
				const char* ccattr = PyUnicode_AsUTF8AndSize(key, NULL);									\
																											\
				PropertyDescription* pCompPropertyDescription =												\
					pScriptModule_->findComponentPropertyDescription(ccattr);								\
																											\
				if (pCompPropertyDescription)																\
				{																							\
					if(PyDict_Check(value) /* createDictDataFromPersistentStream 流程导致非字典 */)			\
					{																						\
						EntityComponent* pEntityComponent = (EntityComponent*)PyObject_GetAttr(this, key);	\
						pEntityComponent->updateFromDict(this, value);										\
						Py_DECREF(pEntityComponent);														\
					}																						\
					else																					\
					{																						\
						PyObject_SetAttr(this, key, value);													\
					}																						\
				}																							\
				else																						\
				{																							\
					PyObject_SetAttr(this, key, value);														\
				}																							\
																											\
				continue;																					\
			}																								\
																											\
			if(cellDataDict != NULL && PyDict_Contains(cellDataDict, key) > 0)								\
			{																								\
				PyObject* pyVal = PyDict_GetItem(cellDataDict, key);										\
				if (PyDict_Check(pyVal))																	\
				{																							\
					if (0 != PyDict_Update(pyVal, value) /* EntityComponent required */)					\
					{																						\
						SCRIPT_ERROR_CHECK();																\
						KBE_ASSERT(false);																	\
					}																						\
				}																							\
				else																						\
				{																							\
					PyDict_SetItem(cellDataDict, key, value);												\
				}																							\
			}																								\
			else																							\
			{																								\
				const char* ccattr = PyUnicode_AsUTF8AndSize(key, NULL);									\
																											\
				PropertyDescription* pCompPropertyDescription =												\
					pScriptModule_->findComponentPropertyDescription(ccattr);								\
																											\
																											\
				if (pCompPropertyDescription)																\
				{																							\
					/* 一般在base上可能放在cellData中是字典，而没有cell的实体需要pass这个设置 */					\
					if(PyDict_Check(value))																	\
						continue;																			\
				}																							\
																											\
				PyObject_SetAttr(this, key, value);															\
			}																								\
		}																									\
																											\
		SCRIPT_ERROR_CHECK();																				\
																											\
		Py_XDECREF(cellDataDict);																			\
	}																										\
																											\
	void addCellDataToStream(COMPONENT_TYPE sendTo, uint32 flags,											\
										MemoryStream* mstream, bool useAliasID = false);					\
																											\
	PyObject* createCellDataFromStream(MemoryStream* mstream)												\
	{																										\
		EntityDef::context().currComponentType = CELLAPP_TYPE;												\
		EntityDef::context().currEntityID = id();															\
																											\
		PyObject* cellData = PyDict_New();																	\
		ENTITY_PROPERTY_UID uid;																			\
		Vector3 pos, dir;																					\
		STREAM_TO_POS_DIR(*mstream, pos, dir);																\
		ADD_POSDIR_TO_PYDICT(cellData, pos, dir);															\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP& propertyDescrs =										\
								pScriptModule_->getCellPropertyDescriptions_uidmap();						\
																											\
		size_t count = propertyDescrs.size();																\
																											\
		{																									\
			ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP::iterator iter = propertyDescrs.begin();			\
			for(; iter != propertyDescrs.end(); ++iter)														\
			{																								\
				/* 由于存在一种情况， 组件def中没有内容， 但有cell脚本，此时baseapp上无法判断他是否有cell属性，所以写celldata时没有数据写入 */ \
				if (iter->second->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT)						\
				{																							\
					EntityComponentType* pEntityComponentType = (EntityComponentType*)iter->second->getDataType();	\
					if (pEntityComponentType->pScriptDefModule()->getCellPropertyDescriptions().size() == 0)\
					{																						\
						--count;																			\
						continue;																			\
					}																						\
				}																							\
			}																								\
		}																									\
																											\
		while(mstream->length() > 0 && count-- > 0)															\
		{																									\
			(*mstream) >> uid /* 父属性 */ >> uid;															\
			ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP::iterator iter = propertyDescrs.find(uid);			\
			if(iter == propertyDescrs.end())																\
			{																								\
				ERROR_MSG(fmt::format("{}::createCellDataFromStream: not found uid({})! entityID={}\n", scriptName(), uid, id()));	\
				break;																						\
			}																								\
																											\
			PyObject* pyobj = NULL;																			\
			if (iter->second->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT)							\
			{																								\
				pyobj = ((EntityComponentType*)iter->second->getDataType())->createCellDataFromStream(mstream);\
			}																								\
			else																							\
			{																								\
				pyobj = iter->second->createFromStream(mstream);											\
			}																								\
																											\
			if(pyobj == NULL)																				\
			{																								\
				SCRIPT_ERROR_CHECK();																		\
				pyobj = iter->second->parseDefaultStr("");													\
				PyDict_SetItemString(cellData, iter->second->getName(), pyobj);								\
				Py_DECREF(pyobj);																			\
			}																								\
			else																							\
			{																								\
				PyDict_SetItemString(cellData, iter->second->getName(), pyobj);								\
				Py_DECREF(pyobj);																			\
			}																								\
		}																									\
																											\
		return cellData;																					\
	}																										\
																											\
	void addCellDataToStreamByDetailLevel(int8 detailLevel, MemoryStream* mstream, bool useAliasID = false)	\
	{																										\
		EntityDef::context().currComponentType = CELLAPP_TYPE;												\
		EntityDef::context().currEntityID = id();															\
																											\
		PyObject* cellData = PyObject_GetAttrString(this, "__dict__");										\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =											\
				pScriptModule_->getCellPropertyDescriptionsByDetailLevel(detailLevel);						\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();				\
		for(; iter != propertyDescrs.end(); ++iter)															\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			PyObject* pyVal = PyDict_GetItemString(cellData, propertyDescription->getName());				\
																											\
			if(useAliasID && pScriptModule_->usePropertyDescrAlias())										\
			{																								\
				(*mstream) << propertyDescription->aliasIDAsUint8();										\
			}																								\
			else																							\
			{																								\
				(*mstream) << propertyDescription->getUType();												\
			}																								\
																											\
			propertyDescription->getDataType()->addToStream(mstream, pyVal);								\
		}																									\
																											\
		Py_XDECREF(cellData);																				\
		SCRIPT_ERROR_CHECK();																				\
	}																										\
																											\
	void addClientDataToStream(MemoryStream* s, bool otherClient = false)									\
	{																										\
		EntityDef::context().currComponentType = CLIENT_TYPE;												\
		EntityDef::context().currEntityID = id();															\
																											\
		PyObject* pydict = PyObject_GetAttrString(this, "__dict__");										\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =											\
				pScriptModule()->getClientPropertyDescriptions();											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();					\
		for(; iter != propertyDescrs.end(); ++iter)															\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			if(otherClient)																					\
			{																								\
				if((propertyDescription->getFlags() & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) <= 0)			\
					continue;																				\
			}																								\
																											\
			PyObject *key = PyUnicode_FromString(propertyDescription->getName());							\
																											\
			if(PyDict_Contains(pydict, key) > 0)															\
			{																								\
				if(pScriptModule()->usePropertyDescrAlias())												\
				{																							\
					(*s) << (uint8)0;																		\
	    			(*s) << propertyDescription->aliasIDAsUint8();											\
				}																							\
				else																						\
				{																							\
					(*s) << (ENTITY_PROPERTY_UID)0;															\
	    			(*s) << propertyDescription->getUType();												\
				}																							\
																											\
	    		propertyDescription->getDataType()->addToStream(s, PyDict_GetItem(pydict, key));			\
			}																								\
																											\
			Py_DECREF(key);																					\
		}																									\
																											\
		Py_XDECREF(pydict);																					\
	}																										\
																											\
	void addPositionAndDirectionToStream(MemoryStream& s, bool useAliasID = false);							\
																											\
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol)									\
	{																										\
		CLASS* entity = static_cast<CLASS*>(self);															\
		DEBUG_REDUCE_EX(entity);																			\
		PyObject* args = PyTuple_New(2);																	\
		PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("EntityCall");							\
		PyTuple_SET_ITEM(args, 0, unpickleMethod);															\
		PyObject* args1 = PyTuple_New(4);																	\
		PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(entity->id()));									\
		PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLongLong(g_componentID));								\
		PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(entity->pScriptModule()->getUType()));			\
		if(g_componentType == BASEAPP_TYPE)																	\
			PyTuple_SET_ITEM(args1, 3, PyLong_FromUnsignedLong(ENTITYCALL_TYPE_BASE));						\
		else																								\
			PyTuple_SET_ITEM(args1, 3, PyLong_FromUnsignedLong(ENTITYCALL_TYPE_CELL));						\
		PyTuple_SET_ITEM(args, 1, args1);																	\
																											\
		if(unpickleMethod == NULL){																			\
			Py_DECREF(args);																				\
			return NULL;																					\
		}																									\
		SCRIPT_ERROR_CHECK();																				\
		return args;																						\
	}																										\
																											\
	inline ScriptTimers& scriptTimers(){ return scriptTimers_; }											\
	void onTimer(ScriptID timerID, int useraAgs);															\
																											\
	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }													\
																											\
	static PyObject* __pyget_pyGetID(CLASS *self, void *closure)											\
	{																										\
		return PyLong_FromLong(self->id());																	\
	}																										\
																											\
	INLINE ENTITY_ID id() const																				\
	{																										\
		return id_;																							\
	}																										\
																											\
	INLINE void id(ENTITY_ID v)																				\
	{																										\
		id_ = v; 																							\
	}																										\
																											\
	INLINE bool hasFlags(uint32 v) const																	\
	{																										\
		return (flags_ & v) > 0;																			\
	}																										\
																											\
	INLINE uint32 flags() const																				\
	{																										\
		return flags_;																						\
	}																										\
																											\
	INLINE void flags(uint32 v)																				\
	{																										\
		flags_ = v; 																						\
	}																										\
																											\
	INLINE uint32 addFlags(uint32 v)																		\
	{																										\
		flags_ |= v;																						\
		return flags_;																						\
	}																										\
																											\
	INLINE uint32 removeFlags(uint32 v)																		\
	{																										\
		flags_ &= ~v; 																						\
		return flags_;																						\
	}																										\
																											\
	INLINE SPACE_ID spaceID() const																			\
	{																										\
		return spaceID_;																					\
	}																										\
	INLINE void spaceID(SPACE_ID id)																		\
	{																										\
		spaceID_ = id;																						\
	}																										\
	static PyObject* __pyget_pyGetSpaceID(CLASS *self, void *closure)										\
	{																										\
		return PyLong_FromLong(self->spaceID());															\
	}																										\
																											\
	INLINE ScriptDefModule* pScriptModule(void) const														\
	{																										\
		return pScriptModule_; 																				\
	}																										\
																											\
	int onScriptDelAttribute(PyObject* attr)																\
	{																										\
		const char* ccattr = PyUnicode_AsUTF8AndSize(attr, NULL);											\
		DEBUG_OP_ATTRIBUTE("del", attr)																		\
																											\
		if(pPropertyDescrs_)																				\
		{																									\
			ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pPropertyDescrs_->find(ccattr);	\
			if(iter != pPropertyDescrs_->end())																\
			{																								\
				char err[255];																				\
				kbe_snprintf(err, 255, "property[%s] defined in %s.def, del failed!", ccattr, scriptName());\
				PyErr_SetString(PyExc_TypeError, err);														\
				PyErr_PrintEx(0);																			\
				return 0;																					\
			}																								\
		}																									\
																											\
		if(pScriptModule_->findMethodDescription(ccattr, g_componentType) != NULL)							\
		{																									\
			char err[255];																					\
			kbe_snprintf(err, 255, "method[%s] defined in %s.def, del failed!", ccattr, scriptName());		\
			PyErr_SetString(PyExc_TypeError, err);															\
			PyErr_PrintEx(0);																				\
			return 0;																						\
		}																									\
																											\
		return ScriptObject::onScriptDelAttribute(attr);													\
	}																										\
																											\
	int onScriptSetAttribute(PyObject* attr, PyObject* value)												\
	{																										\
		DEBUG_OP_ATTRIBUTE("set", attr)																		\
		const char* ccattr = PyUnicode_AsUTF8AndSize(attr, NULL);											\
																											\
		if(pPropertyDescrs_)																				\
		{																									\
			ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pPropertyDescrs_->find(ccattr);	\
			if(iter != pPropertyDescrs_->end())																\
			{																								\
				PropertyDescription* propertyDescription = iter->second;									\
				DataType* dataType = propertyDescription->getDataType();									\
																											\
				if(!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed_)										\
				{																							\
					PyErr_Format(PyExc_AssertionError, "can't set %s.%s to %s. entity is destroyed!",		\
													scriptName(), ccattr, value->ob_type->tp_name);			\
					PyErr_PrintEx(0);																		\
					return 0;																				\
				}																							\
																											\
				if(!dataType->isSameType(value))															\
				{																							\
					PyErr_Format(PyExc_ValueError, "can't set %s.%s to %s.",								\
													scriptName(), ccattr, value->ob_type->tp_name);			\
					PyErr_PrintEx(0);																		\
					return 0;																				\
				}																							\
				else																						\
				{																							\
					Py_ssize_t ob_refcnt = value->ob_refcnt;												\
					PyObject* pySetObj = propertyDescription->onSetValue(this, value);						\
																											\
					/* 如果def属性数据有改变， 那么可能需要广播 */												\
					if(pySetObj != NULL)																	\
					{																						\
						onDefDataChanged(NULL, propertyDescription, pySetObj);								\
						if(pySetObj == value && pySetObj->ob_refcnt - ob_refcnt > 1)						\
							Py_DECREF(pySetObj);															\
					}																						\
																											\
					return pySetObj == NULL ? -1 : 0;														\
				}																							\
			}																								\
		}																									\
																											\
		return ScriptObject::onScriptSetAttribute(attr, value);												\
	}																										\
																											\
	PyObject * onScriptGetAttribute(PyObject* attr);														\
																											\
	DECLARE_PY_MOTHOD_ARG3(pyAddTimer, float, float, int32);												\
																											\
	static PyObject* __py_pyWriteToDB(PyObject* self, PyObject* args)										\
	{																										\
		uint16 currargsSize = (uint16)PyTuple_Size(args);													\
		CLASS* pobj = static_cast<CLASS*>(self);															\
																											\
		if((g_componentType == CELLAPP_TYPE && currargsSize > 2) ||											\
			(g_componentType == BASEAPP_TYPE && currargsSize > 3))											\
		{																									\
			PyErr_Format(PyExc_AssertionError,																\
							"%s::writeToDB: args max require %d args, gived %d!\n",							\
				pobj->scriptName(), 1, currargsSize);														\
																											\
			PyErr_PrintEx(0);																				\
			S_Return;																						\
		}																									\
																											\
		int extra = 0;																						\
		std::string strextra;																				\
		PyObject* pycallback = NULL;																		\
																											\
		if(g_componentType == CELLAPP_TYPE)																	\
		{																									\
			PyObject* baseMB = PyObject_GetAttrString(self, "base");										\
			if(baseMB == NULL || baseMB == Py_None)															\
			{																								\
				PyErr_Clear();																				\
				PyErr_SetString(PyExc_AssertionError,														\
				"This method can only be called on a real entity that has a base entity. ");				\
				PyErr_PrintEx(0);																			\
			}																								\
		}																									\
		else if(g_componentType == BASEAPP_TYPE)															\
		{																									\
			extra = -1;	/* shouldAutoLoad -1默认不改变设置 */												\
		}																									\
																											\
		if(currargsSize == 1)																				\
		{																									\
			if(g_componentType == BASEAPP_TYPE)																\
			{																								\
				if(!PyArg_ParseTuple(args, "O", &pycallback))												\
				{																							\
					PyErr_Format(PyExc_AssertionError, "%s::writeToDB: args error!", pobj->scriptName());	\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(!PyCallable_Check(pycallback))															\
				{																							\
					if(pycallback != Py_None)																\
					{																						\
						PyErr_Format(PyExc_TypeError, "%s::writeToDB: args1 not is callback!", pobj->scriptName());\
						PyErr_PrintEx(0);																	\
						S_Return;																			\
					}																						\
					else																					\
					{																						\
						pycallback = NULL;																	\
					}																						\
				}																							\
			}																								\
			else																							\
			{																								\
				if(!PyArg_ParseTuple(args, "i", &extra))													\
				{																							\
					PyErr_Format(PyExc_AssertionError, "%s::writeToDB: args error!", pobj->scriptName());	\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
			}																								\
		}																									\
		else if(currargsSize == 2)																			\
		{																									\
			if(g_componentType == BASEAPP_TYPE)																\
			{																								\
				if(!PyArg_ParseTuple(args, "O|i", &pycallback, &extra))										\
				{																							\
					PyErr_Format(PyExc_AssertionError, "%s::writeToDB: args error!", pobj->scriptName());	\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(!PyCallable_Check(pycallback))															\
				{																							\
					if(pycallback != Py_None)																\
					{																						\
						PyErr_Format(PyExc_TypeError, "%s::writeToDB: args1 not is callback!", pobj->scriptName());	\
						PyErr_PrintEx(0);																	\
						S_Return;																			\
					}																						\
					else																					\
					{																						\
						pycallback = NULL;																	\
					}																						\
				}																							\
			}																								\
			else																							\
			{																								\
				PyObject* pystr_extra = NULL;																\
				if(!PyArg_ParseTuple(args, "i|O", &extra, &pystr_extra))									\
				{																							\
					PyErr_Format(PyExc_AssertionError, "%s::writeToDB: args error!", pobj->scriptName());	\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(pystr_extra)																				\
				{																							\
					strextra = PyUnicode_AsUTF8AndSize(pystr_extra, NULL);									\
				}																							\
																											\
				if(!g_kbeSrvConfig.dbInterface(strextra))													\
				{																							\
					PyErr_Format(PyExc_TypeError, "%s::writeToDB: args2, "									\
													"incorrect dbInterfaceName(%s)!",						\
													pobj->scriptName(), strextra.c_str());					\
					PyErr_PrintEx(0);																		\
					S_Return;																				\
				}																							\
			}																								\
		}																									\
		else if(currargsSize == 3)																			\
		{																									\
			if(g_componentType == BASEAPP_TYPE)																\
			{																								\
				PyObject* pystr_extra = NULL;																\
				if(!PyArg_ParseTuple(args, "O|i|O", &pycallback, &extra, &pystr_extra))						\
				{																							\
					PyErr_Format(PyExc_AssertionError, "%s::writeToDB: args error!", pobj->scriptName());	\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(!PyCallable_Check(pycallback))															\
				{																							\
					if(pycallback != Py_None)																\
					{																						\
						PyErr_Format(PyExc_TypeError, "%s::writeToDB: args1 not is callback!", pobj->scriptName());	\
						PyErr_PrintEx(0);																	\
						S_Return;																			\
					}																						\
					else																					\
					{																						\
						pycallback = NULL;																	\
					}																						\
				}																							\
																											\
				if(pystr_extra)																				\
				{																							\
					strextra = PyUnicode_AsUTF8AndSize(pystr_extra, NULL);									\
				}																							\
																											\
				if(!g_kbeSrvConfig.dbInterface(strextra))													\
				{																							\
					PyErr_Format(PyExc_TypeError, "%s::writeToDB: args3, "									\
										"incorrect dbInterfaceName(%s)!",									\
											pobj->scriptName(), strextra.c_str());							\
					PyErr_PrintEx(0);																		\
					S_Return;																				\
				}																							\
			}																								\
			else																							\
			{																								\
				KBE_ASSERT(false);																			\
			}																								\
		}																									\
																											\
		pobj->writeToDB(pycallback, (void*)&extra, (void*)strextra.c_str());								\
		S_Return;																							\
	}																										\
																											\
	void writeToDB(void* data, void* extra1, void* extra2);													\
																											\
	void destroy(bool callScript = true)																	\
	{																										\
		if(hasFlags(ENTITY_FLAGS_DESTROYING))																\
			return;																							\
																											\
		if(!isDestroyed_)																					\
		{																									\
			isDestroyed_ = true;																			\
			addFlags(ENTITY_FLAGS_DESTROYING);																\
			EntityComponent::onEntityDestroy(this, pScriptModule_, callScript, true);						\
			onDestroy(callScript);																			\
			scriptTimers_.cancelAll();																		\
			removeFlags(ENTITY_FLAGS_DESTROYING);															\
			EntityComponent::onEntityDestroy(this, pScriptModule_, callScript, false);						\
			events_.clear();																				\
			Py_DECREF(this);																				\
		}																									\
	}																										\
	INLINE bool isDestroyed() const																			\
	{																										\
		return isDestroyed_;																				\
	}																										\
	DECLARE_PY_GET_MOTHOD(pyGetIsDestroyed);																\
																											\
	void destroyEntity();																					\
	static PyObject* __py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs);				\
																											\
	DECLARE_PY_GET_MOTHOD(pyGetClassName);																	\
																											\
	void initProperty(bool isReload = false);																\
																											\
	static PyObject* __py_pyGetDatachangeEventPtr(PyObject* self, PyObject* args)							\
	{																										\
		CLASS* pobj = static_cast<CLASS*>(self);															\
		static EntityComponent::OnDataChangedEvent dataChangedEvent;										\
			dataChangedEvent = std::tr1::bind(&CLASS::onDefDataChanged, pobj,								\
			std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3);			\
		return PyLong_FromVoidPtr((void*)&dataChangedEvent);												\
	}																										\
																											\
	ENTITY_EVENTS& events() { return events_; }																\
																											\
	bool registerEvent(const std::string& evnName, PyObject* pyCallback)									\
	{																										\
		ENTITY_DESTROYED_CHECK(return false, "registerEvent", this);										\
																											\
		if (!PyCallable_Check(pyCallback))																	\
		{																									\
			PyErr_Format(PyExc_TypeError, "%s::registerEvent: '%.200s' object is not callable! eventName=%s, entityID=%d",\
				scriptName(), (pyCallback ? pyCallback->ob_type->tp_name : "NULL"), evnName.c_str(), id());		\
			PyErr_PrintEx(0);																				\
			return false;																					\
		}																									\
																											\
		std::vector<PyObjectPtr>& evnVecs = events_[evnName];												\
		std::vector<PyObjectPtr>::iterator iter = evnVecs.begin();											\
		for(; iter != evnVecs.end(); ++iter)																\
		{																									\
			if((*iter).get() == pyCallback)																	\
			{																								\
				PyErr_Format(PyExc_TypeError, "%s::registerEvent: This callable('%.200s') has been registered! eventName=%s, entityID=%d",\
					scriptName(), (pyCallback ? pyCallback->ob_type->tp_name : "NULL"), evnName.c_str(), id());	\
				PyErr_PrintEx(0);																			\
				return false;																				\
			}																								\
		}																									\
																											\
		events_[evnName].push_back(pyCallback);																\
        return true;                                                                                        \
	}																										\
																											\
	void fireEvent(const std::string& evnName, PyObject* pyArgs = NULL)										\
	{																										\
		ENTITY_DESTROYED_CHECK(return, "fireEvent", this);													\
																											\
		std::vector<PyObjectPtr>& evnVecs = events_[evnName];												\
		std::vector<PyObjectPtr>::iterator iter = evnVecs.begin();											\
		for(; iter != evnVecs.end(); ++iter)																\
		{																									\
			PyObject* pyResult = NULL;																		\
			if (pyArgs == NULL)																				\
			{																								\
				pyResult = PyObject_CallObject((*iter).get(), NULL);										\
			}																								\
			else																							\
			{																								\
				pyResult = PyObject_CallObject((*iter).get(), pyArgs);										\
			}																								\
																											\
			if(pyResult == NULL)																			\
			{																								\
				SCRIPT_ERROR_CHECK();																		\
			}																								\
			else																							\
			{																								\
				Py_DECREF(pyResult);																		\
			}																								\
		}																									\
	}																										\
																											\
	bool deregisterEvent(const std::string& evnName, PyObject* pyCallback)									\
	{																										\
		std::vector<PyObjectPtr>& evnVecs = events_[evnName];												\
		std::vector<PyObjectPtr>::iterator iter = evnVecs.begin();											\
		for(; iter != evnVecs.end(); ++iter)																\
		{																									\
			if((*iter).get() == pyCallback)																	\
			{																								\
				evnVecs.erase(iter);																		\
				return true;																				\
			}																								\
		}																									\
																											\
		return false;																						\
	}																										\
																											\
    static PyObject* __py_pyRegisterEvent(PyObject* self, PyObject* args)									\
	{																										\
		uint16 currargsSize = (uint16)PyTuple_Size(args);													\
		CLASS* pobj = static_cast<CLASS*>(self);															\
																											\
		if(currargsSize != 2)																				\
		{																									\
			PyErr_Format(PyExc_AssertionError,																\
							"%s::registerEvent: args require %d args, gived %d!\n",							\
							pobj->scriptName(), 1, currargsSize);											\
																											\
			PyErr_PrintEx(0);																				\
			Py_RETURN_FALSE;																				\
		}																									\
																											\
		const char* eventName = NULL;																		\
		PyObject* pycallback = NULL;																		\
		if(!PyArg_ParseTuple(args, "sO", &eventName, &pycallback))											\
		{																									\
			PyErr_Format(PyExc_AssertionError, "%s::registerEvent:: args error!", pobj->scriptName());		\
			PyErr_PrintEx(0);																				\
			pycallback = NULL;																				\
			Py_RETURN_FALSE;																				\
		}																									\
																											\
		if(!eventName)																						\
		{																									\
			PyErr_Format(PyExc_AssertionError, "%s::registerEvent:: eventName error!", pobj->scriptName());	\
			PyErr_PrintEx(0);																				\
			pycallback = NULL;																				\
			Py_RETURN_FALSE;																				\
		}																									\
																											\
		return PyBool_FromLong(pobj->registerEvent(eventName, pycallback));									\
	}																										\
																											\
    static PyObject* __py_pyDeregisterEvent(PyObject* self, PyObject* args)									\
	{																										\
		uint16 currargsSize = (uint16)PyTuple_Size(args);													\
		CLASS* pobj = static_cast<CLASS*>(self);															\
																											\
		if(currargsSize != 2)																				\
		{																									\
			PyErr_Format(PyExc_AssertionError,																\
							"%s::deregisterEvent: args require %d args, gived %d!\n",						\
							pobj->scriptName(), 1, currargsSize);											\
																											\
			PyErr_PrintEx(0);																				\
			Py_RETURN_FALSE;																				\
		}																									\
																											\
		const char* eventName = NULL;																		\
		PyObject* pycallback = NULL;																		\
		if(!PyArg_ParseTuple(args, "sO", &eventName, &pycallback))											\
		{																									\
			PyErr_Format(PyExc_AssertionError, "%s::deregisterEvent:: args error!", pobj->scriptName());	\
			PyErr_PrintEx(0);																				\
			pycallback = NULL;																				\
			Py_RETURN_FALSE;																				\
		}																									\
																											\
		if(!eventName)																						\
		{																									\
			PyErr_Format(PyExc_AssertionError, "%s::deregisterEvent:: eventName error!", pobj->scriptName());\
			PyErr_PrintEx(0);																				\
			pycallback = NULL;																				\
			Py_RETURN_FALSE;																				\
		}																									\
																											\
		return PyBool_FromLong(pobj->deregisterEvent(eventName, pycallback));								\
	}																										\
																											\
    static PyObject* __py_pyFireEvent(PyObject* self, PyObject* args)										\
	{																										\
		uint16 currargsSize = (uint16)PyTuple_Size(args);													\
		CLASS* pobj = static_cast<CLASS*>(self);															\
																											\
		if(currargsSize == 0)																				\
		{																									\
			PyErr_Format(PyExc_AssertionError,																\
							"%s::fireEvent: args require %d args, gived %d!\n",								\
							pobj->scriptName(), 1, currargsSize);											\
																											\
			PyErr_PrintEx(0);																				\
			Py_RETURN_FALSE;																				\
		}																									\
																											\
		char* eventName = NULL;																				\
		if(currargsSize == 1)																				\
		{																									\
			if(!PyArg_ParseTuple(args, "s", &eventName))													\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::fireEvent:: args error! entityID=%d", pobj->scriptName(), pobj->id());		\
				PyErr_PrintEx(0);																			\
				Py_RETURN_FALSE;																			\
			}																								\
																											\
			if(!eventName)																					\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::fireEvent:: eventName error!", pobj->scriptName());	\
				PyErr_PrintEx(0);																			\
				Py_RETURN_FALSE;																			\
			}																								\
																											\
			pobj->fireEvent(eventName);																		\
		}																									\
		else if(currargsSize == 2)																			\
		{																									\
			PyObject* pyobj = NULL;																			\
			if (!PyArg_ParseTuple(args, "sO", &eventName, &pyobj))											\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::fireEvent:: args error! entityID=%d", pobj->scriptName(), pobj->id());		\
				PyErr_PrintEx(0);																			\
				Py_RETURN_FALSE;																			\
			}																								\
																											\
			if(!eventName)																					\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::fireEvent:: eventName error!", pobj->scriptName());	\
				PyErr_PrintEx(0);																			\
				Py_RETURN_FALSE;																			\
			}																								\
																											\
			PyObject* pyargs = PyTuple_New(1);																\
			Py_INCREF(pyobj);																				\
			PyTuple_SET_ITEM(pyargs, 0, pyobj);																\
			pobj->fireEvent(eventName, pyargs);																\
			Py_DECREF(pyargs);																				\
		}																									\
		else																								\
		{																									\
			PyObject* pyEvnName = PyTuple_GET_ITEM(args, 0);												\
																											\
			if(!PyUnicode_Check(pyEvnName))																	\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::fireEvent:: eventName error!", pobj->scriptName());	\
				PyErr_PrintEx(0);																			\
				Py_RETURN_FALSE;																			\
			}																								\
																											\
			eventName = const_cast<char*>(PyUnicode_AsUTF8AndSize(pyEvnName, NULL));						\
																											\
			PyObject* pyargs = PyTuple_GetSlice(args, 1, currargsSize);										\
			pobj->fireEvent(eventName, pyargs);																\
			Py_DECREF(pyargs);																				\
		}																									\
																											\
		Py_RETURN_TRUE;																						\
	}																										\
																											\
	PyObject* pyGetComponent(const std::string& componentName, bool all)									\
	{																										\
		std::vector<EntityComponent*> founds =																\
			EntityComponent::getComponents(componentName, this, pScriptModule_);							\
																											\
		if (!all)																							\
		{																									\
			if(founds.size() > 0)																			\
				return founds[0];																			\
																											\
			Py_RETURN_NONE;																					\
		}																									\
		else																								\
		{																									\
			PyObject* pyObj = PyTuple_New(founds.size());													\
																											\
			for (int i = 0; i < (int)founds.size(); ++i)													\
			{																								\
				PyTuple_SetItem(pyObj, i, founds[i]);														\
			}																								\
																											\
			return pyObj;																					\
		}																									\
																											\
        return NULL;																						\
	}																										\
																											\
    static PyObject* __py_pyGetComponent(PyObject* self, PyObject* args)									\
	{																										\
		uint16 currargsSize = (uint16)PyTuple_Size(args);													\
		CLASS* pobj = static_cast<CLASS*>(self);															\
																											\
		if(currargsSize == 0 || currargsSize > 2)															\
		{																									\
			PyErr_Format(PyExc_AssertionError,																\
							"%s::getComponent: args require 1-2 args, gived %d!\n",							\
							pobj->scriptName(), currargsSize);												\
																											\
			PyErr_PrintEx(0);																				\
			Py_RETURN_NONE;																					\
		}																									\
																											\
		char* componentName = NULL;																			\
		if(currargsSize == 1)																				\
		{																									\
			if(!PyArg_ParseTuple(args, "s", &componentName))												\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::getComponent:: args error!", pobj->scriptName());	\
				PyErr_PrintEx(0);																			\
				Py_RETURN_NONE;																				\
			}																								\
																											\
			if(!componentName)																				\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::getComponent:: componentName error!", pobj->scriptName());\
				PyErr_PrintEx(0);																			\
				Py_RETURN_NONE;																				\
			}																								\
																											\
			return pobj->pyGetComponent(componentName, false);												\
		}																									\
		else if(currargsSize == 2)																			\
		{																									\
			PyObject* pyobj = NULL;																			\
			if (!PyArg_ParseTuple(args, "sO", &componentName, &pyobj))										\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::getComponent:: args error!", pobj->scriptName());	\
				PyErr_PrintEx(0);																			\
				Py_RETURN_NONE;																				\
			}																								\
																											\
			if(!componentName)																				\
			{																								\
				PyErr_Format(PyExc_AssertionError, "%s::getComponent:: componentName error!", pobj->scriptName());\
				PyErr_PrintEx(0);																			\
				Py_RETURN_NONE;																				\
			}																								\
																											\
			return pobj->pyGetComponent(componentName, (pyobj == Py_True));									\
		}																									\
																											\
		Py_RETURN_NONE;																						\
	}																										\


#define ENTITY_CPP_IMPL(APP, CLASS)																			\
	class EntityScriptTimerHandler : public TimerHandler													\
	{																										\
	public:																									\
		EntityScriptTimerHandler(CLASS * entity) : pEntity_( entity )										\
		{																									\
		}																									\
																											\
	private:																								\
		virtual void handleTimeout(TimerHandle handle, void * pUser)										\
		{																									\
			ScriptTimers* scriptTimers = &pEntity_->scriptTimers();											\
			int id = ScriptTimersUtil::getIDForHandle( scriptTimers, handle );								\
			pEntity_->onTimer(id, intptr( pUser ));															\
		}																									\
																											\
		virtual void onRelease( TimerHandle handle, void * /*pUser*/ )										\
		{																									\
			ScriptTimers* scriptTimers = &pEntity_->scriptTimers();											\
			scriptTimers->releaseTimer(handle);																\
			delete this;																					\
		}																									\
																											\
		CLASS* pEntity_;																					\
	};																										\
																											\
	PyObject* CLASS::pyAddTimer(float interval, float repeat, int32 userArg)								\
	{																										\
		EntityScriptTimerHandler* pHandler = new EntityScriptTimerHandler(this);							\
		ScriptTimers * pTimers = &scriptTimers_;															\
		int id = ScriptTimersUtil::addTimer(&pTimers,														\
				interval, repeat,																			\
				userArg, pHandler);																			\
																											\
		if (id == 0)																						\
		{																									\
			PyErr_SetString(PyExc_ValueError, "Unable to add timer");										\
			PyErr_PrintEx(0);																				\
			delete pHandler;																				\
																											\
			return NULL;																					\
		}																									\
																											\
		return PyLong_FromLong(id);																			\
	}																										\
																											\
	static PyObject* __py_pyDelTimer(PyObject* self, PyObject* args)										\
	{																										\
		uint16 currargsSize = PyTuple_Size(args);															\
		CLASS* pobj = static_cast<CLASS*>(self);															\
																											\
		if (currargsSize != 1)																				\
		{																									\
			PyErr_Format(PyExc_AssertionError,																\
				"%s::delTimer: args require 1 args(id|int or \"All\"|str), gived %d!\n",					\
				pobj->scriptName(), currargsSize);															\
																											\
			PyErr_PrintEx(0);																				\
			return PyLong_FromLong(-1);																		\
		}																									\
																											\
		ScriptID timerID = 0;																				\
		PyObject* pyargobj = NULL;																			\
																											\
		if (!PyArg_ParseTuple(args, "O", &pyargobj))														\
		{																									\
			PyErr_Format(PyExc_TypeError,																	\
				"%s::delTimer: args(id|int or \"All\"|str) error!",											\
				pobj->scriptName());																		\
																											\
			PyErr_PrintEx(0);																				\
			return PyLong_FromLong(-1);																		\
		}																									\
																											\
		if (pyargobj == NULL)																				\
		{																									\
			PyErr_Format(PyExc_TypeError,																	\
				"%s::delTimer: args(id|int or \"All\"|str) error!",											\
				pobj->scriptName());																		\
																											\
			PyErr_PrintEx(0);																				\
			return PyLong_FromLong(-1);																		\
		}																									\
																											\
		if (PyUnicode_Check(pyargobj))																		\
		{																									\
			if (strcmp(PyUnicode_AsUTF8AndSize(pyargobj, NULL), "All") == 0)								\
			{																								\
				pobj->scriptTimers().cancelAll();															\
			}																								\
			else																							\
			{																								\
				PyErr_Format(PyExc_TypeError,																\
					"%s::delTimer: args not is \"All\"!",													\
					pobj->scriptName());																	\
																											\
				PyErr_PrintEx(0);																			\
				return PyLong_FromLong(-1);																	\
			}																								\
																											\
			return PyLong_FromLong(0);																		\
		}																									\
		else                                                                                                \
		{																									\
			if (!PyLong_Check(pyargobj))																	\
			{																								\
				PyErr_Format(PyExc_TypeError,																\
					"%s::delTimer: args(id|int) error!",													\
					pobj->scriptName());																	\
																											\
				PyErr_PrintEx(0);																			\
				return PyLong_FromLong(-1);																	\
			}																								\
																											\
			timerID = PyLong_AsLong(pyargobj);																\
		}																									\
																											\
		if(!ScriptTimersUtil::delTimer(&pobj->scriptTimers(), timerID))										\
		{																									\
			return PyLong_FromLong(-1);																		\
		}																									\
																											\
		return PyLong_FromLong(timerID);																	\
	}																										\
																											\
	void CLASS::destroyEntity()																				\
	{																										\
		APP::getSingleton().destroyEntity(id_, true);														\
	}																										\
																											\
	PyObject* CLASS::pyGetIsDestroyed()																		\
	{																										\
		return PyBool_FromLong(isDestroyed());																\
	}																										\
																											\
	PyObject* CLASS::pyGetClassName()																		\
	{																										\
		return PyUnicode_FromString(scriptName());															\
	}																										\
																											\
	void CLASS::addPositionAndDirectionToStream(MemoryStream& s, bool useAliasID)							\
	{																										\
		ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;								\
		ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;					\
																											\
		Network::FixedMessages::MSGInfo* msgInfo =															\
					Network::FixedMessages::getSingleton().isFixed("Property::position");					\
																											\
		if(msgInfo != NULL)																					\
		{																									\
			posuid = msgInfo->msgid;																		\
			msgInfo = NULL;																					\
		}																									\
																											\
		msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");					\
		if(msgInfo != NULL)																					\
		{																									\
			diruid = msgInfo->msgid;																		\
			msgInfo = NULL;																					\
		}																									\
																											\
		PyObject* pyPos = NULL;																				\
		PyObject* pyDir = NULL;																				\
																											\
																											\
		if(g_componentType == BASEAPP_TYPE)																	\
		{																									\
			PyObject* cellDataDict = PyObject_GetAttrString(this, "cellData");								\
			if(cellDataDict == NULL)																		\
			{																								\
				PyErr_Clear();																				\
				return;																						\
			}																								\
			else																							\
			{																								\
				pyPos = PyDict_GetItemString(cellDataDict, "position");										\
				pyDir = PyDict_GetItemString(cellDataDict, "direction");									\
			}																								\
																											\
			Py_XDECREF(cellDataDict);																		\
			if(pyPos == NULL && pyDir == NULL)																\
			{																								\
				PyErr_Clear();																				\
				return;																						\
			}																								\
		}																									\
		else																								\
		{																									\
			pyPos = PyObject_GetAttrString(this, "position");												\
			pyDir = PyObject_GetAttrString(this, "direction");												\
		}																									\
																											\
																											\
		Vector3 pos, dir;																					\
		script::ScriptVector3::convertPyObjectToVector3(pos, pyPos);										\
		script::ScriptVector3::convertPyObjectToVector3(dir, pyDir);										\
																											\
		if(pScriptModule()->usePropertyDescrAlias() && useAliasID)											\
		{																									\
			ADD_POS_DIR_TO_STREAM_ALIASID(s, pos, dir)														\
		}																									\
		else																								\
		{																									\
			ADD_POS_DIR_TO_STREAM(s, pos, dir)																\
		}																									\
																											\
		if(g_componentType != BASEAPP_TYPE)																	\
		{																									\
			Py_XDECREF(pyPos);																				\
			Py_XDECREF(pyDir);																				\
		}																									\
	}																										\
																											\
	void CLASS::initProperty(bool isReload)																	\
	{																										\
		EntityDef::context().currComponentType = g_componentType;											\
		EntityDef::context().currEntityID = id();															\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP* oldpropers = NULL;										\
		if(isReload)																						\
		{																									\
			ScriptDefModule* pOldScriptDefModule =															\
										EntityDef::findOldScriptModule(pScriptModule_->getName());			\
			if(!pOldScriptDefModule)																		\
			{																								\
				ERROR_MSG(fmt::format("{}::initProperty: not found old_module!\n",							\
					pScriptModule_->getName()));															\
				KBE_ASSERT(false && "Entity::initProperty: not found old_module");							\
			}																								\
																											\
			oldpropers =																					\
											&pOldScriptDefModule->getPropertyDescrs();						\
		}																									\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pPropertyDescrs_->begin();			\
		for(; iter != pPropertyDescrs_->end(); ++iter)														\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			DataType* dataType = propertyDescription->getDataType();										\
																											\
			if(oldpropers)																					\
			{																								\
				ScriptDefModule::PROPERTYDESCRIPTION_MAP::iterator olditer = oldpropers->find(iter->first);	\
				if(olditer != oldpropers->end())															\
				{																							\
					if(strcmp(olditer->second->getDataType()->getName(),									\
							propertyDescription->getDataType()->getName()) == 0 &&							\
						strcmp(olditer->second->getDataType()->getName(),									\
							propertyDescription->getDataType()->getName()) == 0)							\
						continue;																			\
				}																							\
			}																								\
																											\
			if(dataType)																					\
			{																								\
				PyObject* defObj = propertyDescription->newDefaultVal();									\
																											\
				if(dataType->type() == DATA_TYPE_ENTITY_COMPONENT)											\
					((EntityComponent*)defObj)->updateOwner(id(), this);									\
																											\
				PyObject_SetAttrString(static_cast<PyObject*>(this),										\
							propertyDescription->getName(), defObj);										\
				Py_DECREF(defObj);																			\
																											\
				/* DEBUG_MSG(fmt::format(#CLASS"::"#CLASS": added [{}] property ref={}.\n",					\
								propertyDescription->getName(), defObj->ob_refcnt));*/						\
			}																								\
			else																							\
			{																								\
				ERROR_MSG(fmt::format("{}::initProperty: {} dataType is NULL！ entityID={}\n",				\
					scriptName(), propertyDescription->getName(), id()));									\
			}																								\
		}																									\
																											\
		/* 由于组件初始化时会自动initProperty，所以只有重加载时需要显示的去调用组件的initProperty */				\
		if(isReload)																						\
		{																									\
			const ScriptDefModule::COMPONENTDESCRIPTION_MAP* pComponentDescrs =								\
				&pScriptModule_->getComponentDescrs();														\
																											\
			ScriptDefModule::COMPONENTDESCRIPTION_MAP::const_iterator iter1 = pComponentDescrs->begin();	\
			for (; iter1 != pComponentDescrs->end(); ++iter1)												\
			{																								\
				PyObject* pComponentProperty = PyObject_GetAttrString(this, iter1->first.c_str());			\
				if(pComponentProperty)																		\
				{																							\
					if(PyObject_TypeCheck(pComponentProperty, EntityComponent::getScriptType()))			\
					{																						\
						EntityComponent* pEntityComponent = static_cast<EntityComponent*>(pComponentProperty);\
						pEntityComponent->initProperty();													\
					}																						\
					else																					\
					{																						\
						PyErr_Format(PyExc_AssertionError, "%s.%s is not property of EntityComponent!",		\
							scriptName(), iter1->first.c_str());											\
						PyErr_PrintEx(0);																	\
					}																						\
																											\
					Py_DECREF(pComponentProperty);															\
				}																							\
				else																						\
				{																							\
					PyErr_Clear();																			\
				}																							\
			}																								\
		}																									\
	}																										\
																											\



#define ENTITY_CONSTRUCTION(CLASS)																			\
	id_(id),																								\
	pScriptModule_(const_cast<ScriptDefModule*>(pScriptModule)),											\
	pPropertyDescrs_(&pScriptModule_->getPropertyDescrs()),													\
	spaceID_(0),																							\
	scriptTimers_(),																						\
	pyCallbackMgr_(),																						\
	isDestroyed_(false),																					\
	flags_(ENTITY_FLAGS_INITING)																			\


#define ENTITY_DECONSTRUCTION(CLASS)																		\
	DEBUG_MSG(fmt::format("{}::~{}(): {}\n", scriptName(), scriptName(), id_));								\
	pScriptModule_ = NULL;																					\
	isDestroyed_ = true;																					\
	removeFlags(ENTITY_FLAGS_INITING);																		\


#define ENTITY_INIT_PROPERTYS(CLASS)																		\



}
#endif // KBE_ENTITY_MACRO_H
