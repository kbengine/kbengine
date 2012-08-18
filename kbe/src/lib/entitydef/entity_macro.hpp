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


#ifndef __ENTITY_MACRO_H__
#define __ENTITY_MACRO_H__
#include "cstdkbe/cstdkbe.hpp"
#include "server/callbackmgr.hpp"		

namespace KBEngine{

#define ENTITY_METHOD_DECLARE_BEGIN(CLASS)																	\
	ENTITY_CPP_IMPL(CLASS)																					\
	SCRIPT_METHOD_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_METHOD_DECLARE("__reduce_ex__",	reduce_ex__,					METH_VARARGS,			0)		\
	SCRIPT_METHOD_DECLARE("addTimer",		pyAddTimer,						METH_VARARGS,			0)		\
	SCRIPT_METHOD_DECLARE("delTimer",		pyDelTimer,						METH_VARARGS,			0)		\

#define ENTITY_METHOD_DECLARE_END()																			\
	SCRIPT_METHOD_DECLARE_END()																				\

#define ENTITY_GETSET_DECLARE_BEGIN(CLASS)																	\
	SCRIPT_GETSET_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_GET_DECLARE("id",				pyGetID,						0,						0)		\
	SCRIPT_GET_DECLARE("spaceID",			pyGetSpaceID,					0,						0)		\


#define ENTITY_GETSET_DECLARE_END()																			\
	SCRIPT_GETSET_DECLARE_END()																				\


#define CAN_DEBUG_CREATE_ENTITY
#ifdef CAN_DEBUG_CREATE_ENTITY
	#define DEBUG_CREATE_ENTITY_NAMESPACE																	\
			char* ccattr_DEBUG_CREATE_ENTITY_NAMESPACE = wchar2char(PyUnicode_AsWideCharString(key, NULL));	\
			DEBUG_MSG("%s::debug_createNamespace:add %s.\n", getScriptName(),								\
																ccattr_DEBUG_CREATE_ENTITY_NAMESPACE);		\
			free(ccattr_DEBUG_CREATE_ENTITY_NAMESPACE);														\


	#define DEBUG_OP_ATTRIBUTE(op, ccattr)																	\
			char* ccattr_DEBUG_OP_ATTRIBUTE = wchar2char(PyUnicode_AsWideCharString(ccattr, NULL));			\
			DEBUG_MSG("%s::debug_op_attr:op=%s, %s.\n", getScriptName(),									\
															op, ccattr_DEBUG_OP_ATTRIBUTE);					\
			free(ccattr_DEBUG_OP_ATTRIBUTE);																\

#else
	#define DEBUG_CREATE_ENTITY_NAMESPACE			
	#define DEBUG_OP_ATTRIBUTE(op, ccattr)NULL;
#endif


#define ENTITY_HEADER(CLASS)																				\
protected:																									\
	ENTITY_ID		id_;																					\
	ScriptModule*	scriptModule_;																			\
	const ScriptModule::PROPERTYDESCRIPTION_MAP* lpPropertyDescrs_;											\
	uint32 spaceID_;																						\
	ScriptTimers scriptTimers_;																				\
	PY_CALLBACKMGR pyCallbackMgr_;																			\
public:																										\
	void initializeScript()																					\
	{																										\
		PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("__init__"),						\
										const_cast<char*>(""));												\
		if(pyResult != NULL)																				\
			Py_DECREF(pyResult);																			\
		else																								\
			SCRIPT_ERROR_CHECK();																			\
	}																										\
																											\
	void createNamespace(PyObject* dictData)																\
	{																										\
		if(dictData == NULL)																				\
			return;																							\
																											\
		if(!PyDict_Check(dictData)){																		\
			ERROR_MSG(#CLASS"::createNamespace: create"#CLASS"[%s:%ld] args is not a dict.\n",				\
				getScriptName(), id_);																		\
			return;																							\
		}																									\
																											\
		Py_ssize_t pos = 0;																					\
		PyObject *key, *value;																				\
		PyObject* pydict = PyObject_GetAttrString(this, "__dict__");										\
		PyObject* cellDataDict = PyObject_GetAttrString(this, "cellData");									\
		if(cellDataDict == NULL)																			\
			PyErr_Clear();																					\
																											\
		while(PyDict_Next(dictData, &pos, &key, &value))													\
		{																									\
			DEBUG_CREATE_ENTITY_NAMESPACE																	\
			if(PyDict_Contains(pydict, key) > 0)															\
			{																								\
				PyDict_SetItem(pydict, key, value);															\
				continue;																					\
			}																								\
																											\
			if(cellDataDict != NULL && PyDict_Contains(cellDataDict, key) > 0)								\
    			PyDict_SetItem(cellDataDict, key, value);													\
			else																							\
				PyDict_SetItem(pydict, key, value);															\
		}																									\
																											\
		SCRIPT_ERROR_CHECK();																				\
	}																										\
																											\
	PyObject* getCellDataByFlags(uint32 flags)																\
	{																										\
		PyObject* cellData = PyDict_New();																	\
		PyObject* pydict = PyObject_GetAttrString(this, "__dict__");										\
																											\
		ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =												\
						scriptModule_->getCellPropertyDescriptions();										\
		ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();						\
		for(; iter != propertyDescrs.end(); iter++)															\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			if((flags & propertyDescription->getFlags()) > 0)												\
			{																								\
				PyObject* pyVal = PyDict_GetItemString(pydict, propertyDescription->getName().c_str());		\
				PyDict_SetItemString(cellData, propertyDescription->getName().c_str(), pyVal);				\
			}																								\
		}																									\
																											\
		Py_XDECREF(pydict);																					\
		SCRIPT_ERROR_CHECK();																				\
		return cellData;																					\
	}																										\
																											\
	void getCellDataByDetailLevel(int8 detailLevel, MemoryStream* mstream)									\
	{																										\
		PyObject* cellData = PyObject_GetAttrString(this, "__dict__");										\
																											\
		ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =												\
				scriptModule_->getCellPropertyDescriptionsByDetailLevel(detailLevel);						\
		ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();						\
		for(; iter != propertyDescrs.end(); iter++)															\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			PyObject* pyVal = PyDict_GetItemString(cellData, propertyDescription->getName().c_str());		\
			(*mstream) << propertyDescription->getUType();													\
			propertyDescription->getDataType()->addToStream(mstream, pyVal);								\
		}																									\
																											\
		Py_XDECREF(cellData);																				\
		SCRIPT_ERROR_CHECK();																				\
	}																										\
																											\
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol)									\
	{																										\
		CLASS* entity = static_cast<CLASS*>(self);															\
		PyObject* args = PyTuple_New(2);																	\
		PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Mailbox");								\
		PyTuple_SET_ITEM(args, 0, unpickleMethod);															\
		PyObject* args1 = PyTuple_New(4);																	\
		PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(entity->getID()));								\
		PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLong(g_componentType));								\
		PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(entity->getScriptModule()->getUType()));			\
		PyTuple_SET_ITEM(args1, 3, PyLong_FromUnsignedLong(MAILBOX_TYPE_BASE));								\
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
	void onTimer(ScriptID timerID, int useraAgs)															\
	{																										\
		PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onTimer"),						\
			const_cast<char*>("Ii"), timerID, useraAgs);													\
																											\
		if(pyResult != NULL)																				\
			Py_DECREF(pyResult);																			\
		else																								\
			SCRIPT_ERROR_CHECK();																			\
	}																										\
																											\
	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }													\
																											\
	static PyObject* __pyget_pyGetID(CLASS *self, void *closure)											\
	{																										\
		return PyLong_FromLong(self->getID());																\
	}																										\
																											\
	inline ENTITY_ID getID()const																			\
	{																										\
		return id_;																							\
	}																										\
																											\
	inline void setID(int id)																				\
	{																										\
		id_ = id; 																							\
	}																										\
																											\
	inline uint32 getSpaceID()const																			\
	{																										\
		return spaceID_;																					\
	}																										\
	inline void setSpaceID(int id)																			\
	{																										\
		spaceID_ = id;																						\
	}																										\
	static PyObject* __pyget_pyGetSpaceID(CLASS *self, void *closure)										\
	{																										\
		return PyLong_FromLong(self->getSpaceID());															\
	}																										\
																											\
	inline ScriptModule* getScriptModule(void)const															\
	{																										\
		return scriptModule_; 																				\
	}																										\
																											\
	int onScriptDelAttribute(PyObject* attr)																\
	{																										\
		char* ccattr = wchar2char(PyUnicode_AsWideCharString(attr, NULL));									\
		DEBUG_OP_ATTRIBUTE("del", attr)																		\
																											\
		if(lpPropertyDescrs_)																				\
		{																									\
																											\
			ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = lpPropertyDescrs_->find(ccattr);	\
			if(iter != lpPropertyDescrs_->end())															\
			{																								\
				char err[255];																				\
				kbe_snprintf(err, 255, "property[%s] is in [%s] def. del failed.", ccattr, getScriptName());\
				PyErr_SetString(PyExc_TypeError, err);														\
				PyErr_PrintEx(0);																			\
				free(ccattr);																				\
				return 0;																					\
			}																								\
		}																									\
																											\
		if(scriptModule_->findMethodDescription(ccattr, g_componentType) != NULL)							\
		{																									\
			char err[255];																					\
			kbe_snprintf(err, 255, "method[%s] is in [%s] def. del failed.", ccattr, getScriptName());		\
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
		char* ccattr = wchar2char(PyUnicode_AsWideCharString(attr, NULL));									\
																											\
		if(lpPropertyDescrs_)																				\
		{																									\
			ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = lpPropertyDescrs_->find(ccattr);	\
			if(iter != lpPropertyDescrs_->end())															\
			{																								\
				PropertyDescription* propertyDescription = iter->second;									\
				DataType* dataType = propertyDescription->getDataType();									\
																											\
				if(!dataType->isSameType(value)){															\
					free(ccattr);																			\
					return 0;																				\
				}																							\
				else																						\
				{																							\
					int result = propertyDescription->onSetValue(this, value);								\
																											\
					/* 如果def属性数据有改变， 那么可能需要广播 */											\
					if(result != -1)																		\
						onDefDataChanged(propertyDescription, value);										\
																											\
					free(ccattr);																			\
					return result;																			\
				}																							\
			}																								\
		}																									\
																											\
		free(ccattr);																						\
		return ScriptObject::onScriptSetAttribute(attr, value);												\
	}																										\
																											\
	PyObject * onScriptGetAttribute(PyObject* attr)															\
	{																										\
		DEBUG_OP_ATTRIBUTE("get", attr)																		\
		return ScriptObject::onScriptGetAttribute(attr);													\
	}																										\
																											\
	DECLARE_PY_MOTHOD_ARG3(pyAddTimer, float, float, int32);												\
	DECLARE_PY_MOTHOD_ARG1(pyDelTimer, ScriptID);															\


#define ENTITY_CPP_IMPL(CLASS)																				\
	class EntityScriptTimerHandler : public TimerHandler													\
	{																										\
	public:																									\
		EntityScriptTimerHandler(CLASS * entity ) : pEntity_( entity )										\
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


#define ENTITY_CONSTRUCTION(CLASS)																			\
	id_(id),																								\
	scriptModule_(scriptModule),																			\
	lpPropertyDescrs_(&scriptModule->getPropertyDescrs()),													\
	spaceID_(0),																							\
	scriptTimers_(),																						\
	pyCallbackMgr_()																						\


#define ENTITY_DECONSTRUCTION(CLASS)																		\
	INFO_MSG(#CLASS"::~"#CLASS"(): %s %ld\n", getScriptName(), id_);										\
	scriptModule_ = NULL;																					\


#define ENTITY_INIT_PROPERTYS(CLASS)																		\
	ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = lpPropertyDescrs_->begin();				\
	for(; iter != lpPropertyDescrs_->end(); iter++)															\
	{																										\
		PropertyDescription* propertyDescription = iter->second;											\
		DataType* dataType = propertyDescription->getDataType();											\
																											\
		if(dataType)																						\
		{																									\
			MemoryStream* ms = propertyDescription->getDefaultVal();										\
			PyObject* pyVal = dataType->createObject(ms);													\
			PyObject_SetAttrString(static_cast<PyObject*>(this),											\
						propertyDescription->getName().c_str(), pyVal);										\
			Py_DECREF(pyVal);																				\
																											\
			/* DEBUG_MSG(#CLASS"::"#CLASS": added [%s] property.\n", 
			propertyDescription->getName().c_str());*/														\
			if(ms)																							\
				ms->rpos(0);																				\
		}																									\
		else																								\
		{																									\
		ERROR_MSG(#CLASS"::"#CLASS": %s PropertyDescription the dataType is NULL.\n",						\
				propertyDescription->getName().c_str());													\
		}																									\
	}																										\
																											\



}
#endif
