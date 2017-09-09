/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#ifndef KBE_ENTITY_MACRO_H
#define KBE_ENTITY_MACRO_H

#include "common/common.h"
#include "server/callbackmgr.h"		

namespace KBEngine{

#define ENTITY_METHOD_DECLARE_BEGIN(APP, CLASS)																\
	ENTITY_CPP_IMPL(APP, CLASS)																				\
	SCRIPT_METHOD_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_METHOD_DECLARE("__reduce_ex__",	reduce_ex__,					METH_VARARGS,				0)	\
	SCRIPT_METHOD_DECLARE("addTimer",		pyAddTimer,						METH_VARARGS,				0)	\
	SCRIPT_METHOD_DECLARE("delTimer",		pyDelTimer,						METH_VARARGS,				0)	\
	SCRIPT_METHOD_DECLARE("writeToDB",		pyWriteToDB,					METH_VARARGS,				0)	\
	SCRIPT_METHOD_DECLARE("destroy",		pyDestroyEntity,				METH_VARARGS | METH_KEYWORDS,0)	\

	
#define ENTITY_METHOD_DECLARE_END()																			\
	SCRIPT_METHOD_DECLARE_END()																				\


#define ENTITY_GETSET_DECLARE_BEGIN(CLASS)																	\
	SCRIPT_GETSET_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_GET_DECLARE("id",				pyGetID,						0,						0)		\
	SCRIPT_GET_DECLARE("isDestroyed",		pyGetIsDestroyed,				0,						0)		\
	SCRIPT_GET_DECLARE("className",			pyGetClassName,					0,						0)		\


#define ENTITY_GETSET_DECLARE_END()																			\
	SCRIPT_GETSET_DECLARE_END()																				\


#define CLIENT_ENTITY_METHOD_DECLARE_BEGIN(APP, CLASS)														\
	ENTITY_CPP_IMPL(APP, CLASS)																				\
	SCRIPT_METHOD_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_METHOD_DECLARE("__reduce_ex__",	reduce_ex__,					METH_VARARGS,			0)		\

	
#define CLIENT_ENTITY_METHOD_DECLARE_END()																	\
	SCRIPT_METHOD_DECLARE_END()																				\


#define CLIENT_ENTITY_GETSET_DECLARE_BEGIN(CLASS)															\
	SCRIPT_GETSET_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_GET_DECLARE("id",				pyGetID,						0,						0)		\
	SCRIPT_GET_DECLARE("spaceID",			pyGetSpaceID,					0,						0)		\
	SCRIPT_GET_DECLARE("isDestroyed",		pyGetIsDestroyed,				0,						0)		\
	SCRIPT_GET_DECLARE("className",			pyGetClassName,					0,						0)		\


#define CLIENT_ENTITY_GETSET_DECLARE_END()																	\
	SCRIPT_GETSET_DECLARE_END()																				\


#ifdef CLIENT_NO_FLOAT																					
	#define ADD_POS_DIR_TO_STREAM(s, pos, dir)																\
		int32 x = (int32)pos.x;																				\
		int32 y = (int32)pos.y;																				\
		int32 z = (int32)pos.z;																				\
																											\
		s << posuid << x << y << z;																			\
																											\
		x = (int32)dir.x;																					\
		y = (int32)dir.y;																					\
		z = (int32)dir.z;																					\
																											\
		s << diruid << x << y << z;																			\


	#define ADD_POS_DIR_TO_STREAM_ALIASID(s, pos, dir)														\
		int32 x = (int32)pos.x;																				\
		int32 y = (int32)pos.y;																				\
		int32 z = (int32)pos.z;																				\
																											\
		uint8 aliasID = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;											\
		s << aliasID << x << y << z;																		\
																											\
		x = (int32)dir.x;																					\
		y = (int32)dir.y;																					\
		z = (int32)dir.z;																					\
																											\
		aliasID = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;									\
		s << aliasID << x << y << z;																		\


	#define STREAM_TO_POS_DIR(s, pos, dir)																	\
	{																										\
		int32 x = 0;																						\
		int32 y = 0;																						\
		int32 z = 0;																						\
		ENTITY_PROPERTY_UID uid;																			\
																											\
		s >> uid >> x >> y >> z;																			\
																											\
		pos.x = float(x);																					\
		pos.y = float(y);																					\
		pos.z = float(z);																					\
																											\
		s >> uid >> x >> y >> z;																			\
		dir.x = float(x);																					\
		dir.y = float(y);																					\
		dir.z = float(z);																					\
	}																										\


#else																									
	#define ADD_POS_DIR_TO_STREAM(s, pos, dir)																\
		s << posuid << pos.x << pos.y << pos.z;																\
		s << diruid << dir.x << dir.y << dir.z;																\


	#define ADD_POS_DIR_TO_STREAM_ALIASID(s, pos, dir)														\
		uint8 aliasID = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;											\
		s << aliasID << pos.x << pos.y << pos.z;															\
		aliasID = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;									\
		s << aliasID << dir.x << dir.y << dir.z;															\
	

	#define STREAM_TO_POS_DIR(s, pos, dir)																	\
	{																										\
		ENTITY_PROPERTY_UID uid;																			\
		s >> uid >> pos.x >> pos.y >> pos.z;																\
		s >> uid >> dir.x >> dir.y >> dir.z;																\
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
			wchar_t* PyUnicode_AsWideCharStringRet1 = PyUnicode_AsWideCharString(key, NULL);				\
			char* ccattr_DEBUG_CREATE_ENTITY_NAMESPACE= strutil::wchar2char(PyUnicode_AsWideCharStringRet1);\
			PyObject* pytsval = PyObject_Str(value);														\
			wchar_t* cwpytsval = PyUnicode_AsWideCharString(pytsval, NULL);									\
			char* cccpytsval = strutil::wchar2char(cwpytsval);												\
			Py_DECREF(pytsval);																				\
			DEBUG_MSG(fmt::format("{}(refc={}, id={})::debug_createNamespace:add {}({}).\n",				\
												scriptName(),												\
												static_cast<PyObject*>(this)->ob_refcnt,					\
												this->id(),													\
																ccattr_DEBUG_CREATE_ENTITY_NAMESPACE,		\
																cccpytsval));								\
			free(ccattr_DEBUG_CREATE_ENTITY_NAMESPACE);														\
			PyMem_Free(PyUnicode_AsWideCharStringRet1);														\
			free(cccpytsval);																				\
			PyMem_Free(cwpytsval);																			\
		}																									\


#define DEBUG_OP_ATTRIBUTE(op, ccattr)																		\
		if(g_debugEntity)																					\
		{																									\
			wchar_t* PyUnicode_AsWideCharStringRet2 = PyUnicode_AsWideCharString(ccattr, NULL);				\
			char* ccattr_DEBUG_OP_ATTRIBUTE = strutil::wchar2char(PyUnicode_AsWideCharStringRet2);			\
			DEBUG_MSG(fmt::format("{}(refc={}, id={})::debug_op_attr:op={}, {}.\n",							\
												scriptName(),												\
												static_cast<PyObject*>(this)->ob_refcnt, this->id(),		\
															op, ccattr_DEBUG_OP_ATTRIBUTE));				\
			free(ccattr_DEBUG_OP_ATTRIBUTE);																\
			PyMem_Free(PyUnicode_AsWideCharStringRet2);														\
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
#define ENTITY_FLAGS_UNKNOWN			0x00000000
#define ENTITY_FLAGS_DESTROYING			0x00000001
#define ENTITY_FLAGS_INITING			0x00000002
#define ENTITY_FLAGS_TELEPORT_START		0x00000004
#define ENTITY_FLAGS_TELEPORT_END		0x00000008

#define ENTITY_HEADER(CLASS)																				\
protected:																									\
	ENTITY_ID										id_;													\
	ScriptDefModule*								pScriptModule_;											\
	const ScriptDefModule::PROPERTYDESCRIPTION_MAP* pPropertyDescrs_;										\
	SPACE_ID										spaceID_;												\
	ScriptTimers									scriptTimers_;											\
	PY_CALLBACKMGR									pyCallbackMgr_;											\
	bool											isDestroyed_;											\
	uint32											flags_;													\
public:																										\
	bool initing() const{ return hasFlags(ENTITY_FLAGS_INITING); }											\
																											\
	void initializeScript()																					\
	{																										\
		removeFlags(ENTITY_FLAGS_INITING);																	\
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);																	\
		if(PyObject_HasAttrString(this, "__init__"))														\
		{																									\
			PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("__init__"),					\
											const_cast<char*>(""));											\
			if(pyResult != NULL)																			\
				Py_DECREF(pyResult);																		\
			else																							\
				SCRIPT_ERROR_CHECK();																		\
		}																									\
	}																										\
																											\
	void initializeEntity(PyObject* dictData)																\
	{																										\
		createNamespace(dictData);																			\
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
			WARNING_MSG(fmt::format("Base::reload: "														\
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
	void createNamespace(PyObject* dictData)																\
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
		Py_ssize_t pos = 0;																					\
		PyObject *key, *value;																				\
		PyObject* cellDataDict = PyObject_GetAttrString(this, "cellData");									\
		if(cellDataDict == NULL)																			\
			PyErr_Clear();																					\
																											\
		while(PyDict_Next(dictData, &pos, &key, &value))													\
		{																									\
			DEBUG_CREATE_ENTITY_NAMESPACE																	\
			if(PyObject_HasAttr(this, key) > 0)																\
			{																								\
				PyObject_SetAttr(this, key, value);															\
				continue;																					\
			}																								\
																											\
			if(cellDataDict != NULL && PyDict_Contains(cellDataDict, key) > 0)								\
    			PyDict_SetItem(cellDataDict, key, value);													\
			else																							\
				PyObject_SetAttr(this, key, value);															\
		}																									\
																											\
		SCRIPT_ERROR_CHECK();																				\
		Py_XDECREF(cellDataDict);																			\
	}																										\
																											\
	void addCellDataToStream(uint32 flags, MemoryStream* mstream, bool useAliasID = false);					\
																											\
	PyObject* createCellDataFromStream(MemoryStream* mstream)												\
	{																										\
		PyObject* cellData = PyDict_New();																	\
		ENTITY_PROPERTY_UID uid;																			\
		Vector3 pos, dir;																					\
		STREAM_TO_POS_DIR(*mstream, pos, dir);																\
		ADD_POSDIR_TO_PYDICT(cellData, pos, dir);															\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP& propertyDescrs =										\
								pScriptModule_->getCellPropertyDescriptions_uidmap();						\
																											\
		size_t count = 0;																					\
																											\
		while(mstream->length() > 0 && count < propertyDescrs.size())										\
		{																									\
			(*mstream) >> uid;																				\
			ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP::iterator iter = propertyDescrs.find(uid);			\
			if(iter == propertyDescrs.end())																\
			{																								\
				ERROR_MSG(fmt::format(#CLASS"::createCellDataFromStream: not found uid({})\n", uid));		\
				break;																						\
			}																								\
																											\
			PyObject* pyobj = iter->second->createFromStream(mstream);										\
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
																											\
			++count;																						\
		}																									\
																											\
		return cellData;																					\
	}																										\
																											\
	void addCellDataToStreamByDetailLevel(int8 detailLevel, MemoryStream* mstream, bool useAliasID = false)	\
	{																										\
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
	    			(*s) << propertyDescription->aliasIDAsUint8();											\
				}																							\
				else																						\
				{																							\
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
		PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Mailbox");								\
		PyTuple_SET_ITEM(args, 0, unpickleMethod);															\
		PyObject* args1 = PyTuple_New(4);																	\
		PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(entity->id()));									\
		PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLongLong(g_componentID));								\
		PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(entity->pScriptModule()->getUType()));			\
		if(g_componentType == BASEAPP_TYPE)																	\
			PyTuple_SET_ITEM(args1, 3, PyLong_FromUnsignedLong(MAILBOX_TYPE_BASE));							\
		else																								\
			PyTuple_SET_ITEM(args1, 3, PyLong_FromUnsignedLong(MAILBOX_TYPE_CELL));							\
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
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);					\
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);									\
		PyMem_Free(PyUnicode_AsWideCharStringRet0);															\
		DEBUG_OP_ATTRIBUTE("del", attr)																		\
																											\
		if(pPropertyDescrs_)																				\
		{																									\
																											\
			ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pPropertyDescrs_->find(ccattr);	\
			if(iter != pPropertyDescrs_->end())																\
			{																								\
				char err[255];																				\
				kbe_snprintf(err, 255, "property[%s] is in [%s] def. del failed.", ccattr, scriptName());	\
				PyErr_SetString(PyExc_TypeError, err);														\
				PyErr_PrintEx(0);																			\
				free(ccattr);																				\
				return 0;																					\
			}																								\
		}																									\
																											\
		if(pScriptModule_->findMethodDescription(ccattr, g_componentType) != NULL)							\
		{																									\
			char err[255];																					\
			kbe_snprintf(err, 255, "method[%s] is in [%s] def. del failed.", ccattr, scriptName());			\
			PyErr_SetString(PyExc_TypeError, err);															\
			PyErr_PrintEx(0);																				\
			free(ccattr);																					\
			return 0;																						\
		}																									\
																											\
		free(ccattr);																						\
		return ScriptObject::onScriptDelAttribute(attr);													\
	}																										\
																											\
	int onScriptSetAttribute(PyObject* attr, PyObject* value)												\
	{																										\
		DEBUG_OP_ATTRIBUTE("set", attr)																		\
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);					\
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);									\
		PyMem_Free(PyUnicode_AsWideCharStringRet0);															\
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
					free(ccattr);																			\
					return 0;																				\
				}																							\
																											\
				if(!dataType->isSameType(value))															\
				{																							\
					PyErr_Format(PyExc_ValueError, "can't set %s.%s to %s.",								\
													scriptName(), ccattr, value->ob_type->tp_name);			\
					PyErr_PrintEx(0);																		\
					free(ccattr);																			\
					return 0;																				\
				}																							\
				else																						\
				{																							\
					Py_ssize_t ob_refcnt = value->ob_refcnt;												\
					PyObject* pySetObj = propertyDescription->onSetValue(this, value);						\
																											\
					/* 如果def属性数据有改变， 那么可能需要广播 */											\
					if(pySetObj != NULL)																	\
					{																						\
						onDefDataChanged(propertyDescription, pySetObj);									\
						if(pySetObj == value && pySetObj->ob_refcnt - ob_refcnt > 1)						\
							Py_DECREF(pySetObj);															\
					}																						\
																											\
					free(ccattr);																			\
					return pySetObj == NULL ? -1 : 0;														\
				}																							\
			}																								\
		}																									\
																											\
		free(ccattr);																						\
		return ScriptObject::onScriptSetAttribute(attr, value);												\
	}																										\
																											\
	PyObject * onScriptGetAttribute(PyObject* attr);														\
																											\
	DECLARE_PY_MOTHOD_ARG3(pyAddTimer, float, float, int32);												\
	DECLARE_PY_MOTHOD_ARG1(pyDelTimer, ScriptID);															\
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
							"%s: args max require %d args, gived %d! is script[%s].\n",						\
				__FUNCTION__, 1, currargsSize, pobj->scriptName());											\
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
				if(PyArg_ParseTuple(args, "O", &pycallback) == -1)											\
				{																							\
					PyErr_Format(PyExc_AssertionError, "KBEngine::writeToDB: args error!");					\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(!PyCallable_Check(pycallback))															\
				{																							\
					if(pycallback != Py_None)																\
					{																						\
						PyErr_Format(PyExc_TypeError, "KBEngine::writeToDB: args1 not is callback!");		\
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
				if(PyArg_ParseTuple(args, "i", &extra) == -1)												\
				{																							\
					PyErr_Format(PyExc_AssertionError, "KBEngine::writeToDB: args error!");					\
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
				if(PyArg_ParseTuple(args, "O|i", &pycallback, &extra) == -1)								\
				{																							\
					PyErr_Format(PyExc_AssertionError, "KBEngine::writeToDB: args error!");					\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(!PyCallable_Check(pycallback))															\
				{																							\
					if(pycallback != Py_None)																\
					{																						\
						PyErr_Format(PyExc_TypeError, "KBEngine::writeToDB: args1 not is callback!");		\
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
				if(PyArg_ParseTuple(args, "i|O", &extra, &pystr_extra) == -1)								\
				{																							\
					PyErr_Format(PyExc_AssertionError, "KBEngine::writeToDB: args error!");					\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(pystr_extra)																				\
				{																							\
					wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pystr_extra, NULL);\
					char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);						\
					strextra = ccattr;																		\
					PyMem_Free(PyUnicode_AsWideCharStringRet0);												\
					free(ccattr);																			\
				}																							\
																											\
				if(!g_kbeSrvConfig.dbInterface(strextra))													\
				{																							\
					PyErr_Format(PyExc_TypeError, "KBEngine::writeToDB: args2, "							\
													"incorrect dbInterfaceName(%s)!",						\
													strextra.c_str());										\
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
				if(PyArg_ParseTuple(args, "O|i|O", &pycallback, &extra, &pystr_extra) == -1)				\
				{																							\
					PyErr_Format(PyExc_AssertionError, "KBEngine::writeToDB: args error!");					\
					PyErr_PrintEx(0);																		\
					pycallback = NULL;																		\
					S_Return;																				\
				}																							\
																											\
				if(!PyCallable_Check(pycallback))															\
				{																							\
					if(pycallback != Py_None)																\
					{																						\
						PyErr_Format(PyExc_TypeError, "KBEngine::writeToDB: args1 not is callback!");		\
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
					wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pystr_extra, NULL);\
					char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);						\
					strextra = ccattr;																		\
					PyMem_Free(PyUnicode_AsWideCharStringRet0);												\
					free(ccattr);																			\
				}																							\
																											\
				if(!g_kbeSrvConfig.dbInterface(strextra))													\
				{																							\
					PyErr_Format(PyExc_TypeError, "KBEngine::writeToDB: args3, "							\
										"incorrect dbInterfaceName(%s)!",									\
											strextra.c_str());												\
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
			onDestroy(callScript);																			\
			scriptTimers_.cancelAll();																		\
			removeFlags(ENTITY_FLAGS_DESTROYING);															\
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
	PyObject* CLASS::pyDelTimer(ScriptID timerID)															\
	{																										\
		if(!ScriptTimersUtil::delTimer(&scriptTimers_, timerID))											\
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
																											\
	}																										\
																											\
	void CLASS::initProperty(bool isReload)																	\
	{																										\
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
				PyObject_SetAttrString(static_cast<PyObject*>(this),										\
							propertyDescription->getName(), defObj);										\
				Py_DECREF(defObj);																			\
																											\
				/* DEBUG_MSG(fmt::format(#CLASS"::"#CLASS": added [{}] property ref={}.\n",
								propertyDescription->getName(), defObj->ob_refcnt));*/						\
			}																								\
			else																							\
			{																								\
				ERROR_MSG(fmt::format(#CLASS"::initProperty: {} dataType is NULL.\n",						\
					propertyDescription->getName()));														\
			}																								\
		}																									\
																											\
	}																										\


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
