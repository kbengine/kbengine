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

#define ENTITY_METHOD_DECLARE_BEGIN(APP, CLASS)																\
	ENTITY_CPP_IMPL(APP, CLASS)																				\
	SCRIPT_METHOD_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_METHOD_DECLARE("__reduce_ex__",	reduce_ex__,					METH_VARARGS,			0)		\
	SCRIPT_METHOD_DECLARE("addTimer",		pyAddTimer,						METH_VARARGS,			0)		\
	SCRIPT_METHOD_DECLARE("delTimer",		pyDelTimer,						METH_VARARGS,			0)		\
	SCRIPT_METHOD_DECLARE("writeToDB",		pyWriteToDB,					METH_VARARGS,			0)		\
	SCRIPT_METHOD_DECLARE("destroy",		pyDestroyEntity,				METH_VARARGS,			0)		\

	
#define ENTITY_METHOD_DECLARE_END()																			\
	SCRIPT_METHOD_DECLARE_END()																				\

#define ENTITY_GETSET_DECLARE_BEGIN(CLASS)																	\
	SCRIPT_GETSET_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_GET_DECLARE("id",				pyGetID,						0,						0)		\
	SCRIPT_GET_DECLARE("spaceID",			pyGetSpaceID,					0,						0)		\
	SCRIPT_GET_DECLARE("isDestroyed",		pyGetIsDestroyed,				0,						0)		\


#define ENTITY_GETSET_DECLARE_END()																			\
	SCRIPT_GETSET_DECLARE_END()																				\


#ifdef CLIENT_NO_FLOAT																					
	#define ADD_POS_DIR_TO_STREAM(s, pos, dir)																\
		int32 x = (int32)pos.x;																				\
		int32 y = (int32)pos.y;																				\
		int32 z = (int32)pos.z;																				\
																											\
		s << posuid << posdirLen << x << y << z;															\
																											\
		x = (int32)dir.x;																					\
		y = (int32)dir.y;																					\
		z = (int32)dir.z;																					\
																											\
		s << diruid << posdirLen << x << y << z;															\

#else																									
	#define ADD_POS_DIR_TO_STREAM(s, pos, dir)																\
		s << posuid << posdirLen << pos.x << pos.y << pos.z;												\
		s << diruid << posdirLen << dir.x << dir.y << dir.z;												\

#endif	


#define ADD_POSDIR_TO_STREAM(s, pos, dir)																	\
	{																										\
		ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;								\
		ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;					\
																											\
		Mercury::FixedMessages::MSGInfo* msgInfo =															\
					Mercury::FixedMessages::getSingleton().isFixed("Property::position");					\
																											\
		if(msgInfo != NULL)																					\
		{																									\
			posuid = msgInfo->msgid;																		\
			msgInfo = NULL;																					\
		}																									\
																											\
		msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");					\
		if(msgInfo != NULL)																					\
		{																									\
			diruid = msgInfo->msgid;																		\
			msgInfo = NULL;																					\
		}																									\
																											\
		ArraySize posdirLen = 3;																			\
		ADD_POS_DIR_TO_STREAM(s, pos, dir)																	\
																											\
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
			char* ccattr_DEBUG_CREATE_ENTITY_NAMESPACE = wchar2char(PyUnicode_AsWideCharStringRet1);		\
			PyObject* pytsval = PyObject_Str(value);														\
			wchar_t* cwpytsval = PyUnicode_AsWideCharString(pytsval, NULL);									\
			char* cccpytsval = wchar2char(cwpytsval);														\
			Py_DECREF(pytsval);																				\
			DEBUG_MSG("%s(refc=%u, id=%d)::debug_createNamespace:add %s(%s).\n", getScriptName(),			\
												static_cast<PyObject*>(this)->ob_refcnt, this->getID(),		\
																ccattr_DEBUG_CREATE_ENTITY_NAMESPACE,		\
																cccpytsval);								\
			free(ccattr_DEBUG_CREATE_ENTITY_NAMESPACE);														\
			PyMem_Free(PyUnicode_AsWideCharStringRet1);														\
			free(cccpytsval);																				\
			PyMem_Free(cwpytsval);																			\
		}																									\


#define DEBUG_OP_ATTRIBUTE(op, ccattr)																		\
		if(g_debugEntity)																					\
		{																									\
			wchar_t* PyUnicode_AsWideCharStringRet2 = PyUnicode_AsWideCharString(ccattr, NULL);				\
			char* ccattr_DEBUG_OP_ATTRIBUTE = wchar2char(PyUnicode_AsWideCharStringRet2);					\
			DEBUG_MSG("%s(refc=%u, id=%d)::debug_op_attr:op=%s, %s.\n", getScriptName(),					\
												static_cast<PyObject*>(this)->ob_refcnt, this->getID(),		\
															op, ccattr_DEBUG_OP_ATTRIBUTE);					\
			free(ccattr_DEBUG_OP_ATTRIBUTE);																\
			PyMem_Free(PyUnicode_AsWideCharStringRet2);														\
		}																									\


#define DEBUG_PERSISTENT_PROPERTY(op, ccattr)																\
	{																										\
		if(g_debugEntity)																					\
		{																									\
			DEBUG_MSG("%s(refc=%u, id=%d)::debug_op_Persistent:op=%s, %s.\n", getScriptName(),				\
												static_cast<PyObject*>(this)->ob_refcnt, this->getID(),		\
															op, ccattr);									\
		}																									\
	}																										\


#define DEBUG_REDUCE_EX(tentity)																			\
		if(g_debugEntity)																					\
		{																									\
			DEBUG_MSG("%s(refc=%u, id=%d)::debug_reduct_ex: utype=%u.\n", tentity->getScriptName(),			\
												static_cast<PyObject*>(tentity)->ob_refcnt, tentity->getID(),\
												tentity->getScriptModule()->getUType());					\
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
			OPNAME, ENTITY->getScriptName(), ENTITY->getID());												\
		PyErr_PrintEx(0);																					\
		RETURN;																								\
	}																										\
}																											\


#define ENTITY_HEADER(CLASS)																				\
protected:																									\
	ENTITY_ID		id_;																					\
	ScriptDefModule*	scriptModule_;																		\
	const ScriptDefModule::PROPERTYDESCRIPTION_MAP* lpPropertyDescrs_;										\
	SPACE_ID spaceID_;																						\
	ScriptTimers scriptTimers_;																				\
	PY_CALLBACKMGR pyCallbackMgr_;																			\
	bool isDestroyed_;																						\
	Mercury::Bundle* pBundle_;																				\
	bool initing_;																							\
public:																										\
	void initializeScript()																					\
	{																										\
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
	PyObject* addCellDataToStream(uint32 flags)																\
	{																										\
		PyObject* cellData = PyDict_New();																	\
		PyObject* pydict = PyObject_GetAttrString(this, "__dict__");										\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =											\
						scriptModule_->getCellPropertyDescriptions();										\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();				\
		for(; iter != propertyDescrs.end(); iter++)															\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			if((flags & propertyDescription->getFlags()) > 0)												\
			{																								\
				PyObject* pyVal = PyDict_GetItemString(pydict, propertyDescription->getName());				\
				PyDict_SetItemString(cellData, propertyDescription->getName(), pyVal);						\
			}																								\
		}																									\
																											\
		Py_XDECREF(pydict);																					\
		SCRIPT_ERROR_CHECK();																				\
		return cellData;																					\
	}																										\
																											\
	void addCellDataToStreamByDetailLevel(int8 detailLevel, MemoryStream* mstream)							\
	{																										\
		PyObject* cellData = PyObject_GetAttrString(this, "__dict__");										\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =											\
				scriptModule_->getCellPropertyDescriptionsByDetailLevel(detailLevel);						\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();				\
		for(; iter != propertyDescrs.end(); iter++)															\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			PyObject* pyVal = PyDict_GetItemString(cellData, propertyDescription->getName());				\
			(*mstream) << propertyDescription->getUType();													\
			propertyDescription->getDataType()->addToStream(mstream, pyVal);								\
		}																									\
																											\
		Py_XDECREF(cellData);																				\
		SCRIPT_ERROR_CHECK();																				\
	}																										\
																											\
	void addClientDataToStream(MemoryStream* s)																\
	{																										\
																											\
		PyObject* pydict = PyObject_GetAttrString(this, "__dict__");										\
																											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =											\
				getScriptModule()->getClientPropertyDescriptions();											\
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();					\
		for(; iter != propertyDescrs.end(); iter++)															\
		{																									\
			PropertyDescription* propertyDescription = iter->second;										\
			PyObject *key = PyUnicode_FromString(propertyDescription->getName());							\
																											\
			if(PyDict_Contains(pydict, key) > 0)															\
			{																								\
	    		(*s) << propertyDescription->getUType();													\
	    		propertyDescription->getDataType()->addToStream(s, PyDict_GetItem(pydict, key));			\
			}																								\
																											\
			Py_DECREF(key);																					\
		}																									\
																											\
		Py_XDECREF(pydict);																					\
	}																										\
																											\
	void addPositionAndDirectionToStream(MemoryStream& s);													\
																											\
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol)									\
	{																										\
		CLASS* entity = static_cast<CLASS*>(self);															\
		DEBUG_REDUCE_EX(entity);																			\
		PyObject* args = PyTuple_New(2);																	\
		PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Mailbox");								\
		PyTuple_SET_ITEM(args, 0, unpickleMethod);															\
		PyObject* args1 = PyTuple_New(4);																	\
		PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(entity->getID()));								\
		PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLongLong(g_componentID));								\
		PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(entity->getScriptModule()->getUType()));			\
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
	INLINE ENTITY_ID getID()const																			\
	{																										\
		return id_;																							\
	}																										\
																											\
	INLINE void setID(int id)																				\
	{																										\
		id_ = id; 																							\
	}																										\
																											\
	INLINE SPACE_ID getSpaceID()const																		\
	{																										\
		return spaceID_;																					\
	}																										\
	INLINE void setSpaceID(SPACE_ID id)																		\
	{																										\
		spaceID_ = id;																						\
	}																										\
	static PyObject* __pyget_pyGetSpaceID(CLASS *self, void *closure)										\
	{																										\
		return PyLong_FromLong(self->getSpaceID());															\
	}																										\
																											\
	INLINE ScriptDefModule* getScriptModule(void)const														\
	{																										\
		return scriptModule_; 																				\
	}																										\
																											\
	INLINE Mercury::Bundle* pBundle()const{ return pBundle_; }												\
																											\
	int onScriptDelAttribute(PyObject* attr)																\
	{																										\
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);					\
		char* ccattr = wchar2char(PyUnicode_AsWideCharStringRet0);											\
		PyMem_Free(PyUnicode_AsWideCharStringRet0);															\
		DEBUG_OP_ATTRIBUTE("del", attr)																		\
																											\
		if(lpPropertyDescrs_)																				\
		{																									\
																											\
			ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = lpPropertyDescrs_->find(ccattr);\
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
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);					\
		char* ccattr = wchar2char(PyUnicode_AsWideCharStringRet0);											\
		PyMem_Free(PyUnicode_AsWideCharStringRet0);															\
																											\
		if(lpPropertyDescrs_)																				\
		{																									\
			ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = lpPropertyDescrs_->find(ccattr);\
			if(iter != lpPropertyDescrs_->end())															\
			{																								\
				PropertyDescription* propertyDescription = iter->second;									\
				DataType* dataType = propertyDescription->getDataType();									\
																											\
				if(!dataType->isSameType(value))															\
				{																							\
					PyErr_Format(PyExc_ValueError, "can't set %s.%s to %s.",								\
													getScriptName(), ccattr, value->ob_type->tp_name);		\
					PyErr_PrintEx(0);																		\
					free(ccattr);																			\
					return 0;																				\
				}																							\
				else																						\
				{																							\
					PyObject* pySetObj = propertyDescription->onSetValue(this, value);						\
																											\
					/* 如果def属性数据有改变， 那么可能需要广播 */											\
					if(pySetObj != NULL)																	\
					{																						\
						onDefDataChanged(propertyDescription, pySetObj);									\
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
	PyObject * onScriptGetAttribute(PyObject* attr)															\
	{																										\
		DEBUG_OP_ATTRIBUTE("get", attr)																		\
		return ScriptObject::onScriptGetAttribute(attr);													\
	}																										\
																											\
	DECLARE_PY_MOTHOD_ARG3(pyAddTimer, float, float, int32);												\
	DECLARE_PY_MOTHOD_ARG1(pyDelTimer, ScriptID);															\
																											\
	static PyObject* __py_pyWriteToDB(PyObject* self, PyObject* args)										\
	{																										\
		uint16 currargsSize = PyTuple_Size(args);															\
		CLASS* pobj = static_cast<CLASS*>(self);															\
																											\
		if((g_componentType == CELLAPP_TYPE && currargsSize > 0) ||											\
			(g_componentType == BASEAPP_TYPE && currargsSize > 1))											\
		{																									\
			PyErr_Format(PyExc_AssertionError,																\
							"%s: args max require %d args, gived %d! is script[%s].\n",						\
				__FUNCTION__, 1, currargsSize, pobj->getScriptName());										\
			PyErr_PrintEx(0);																				\
		}																									\
																											\
		PyObject* pycallback = NULL;																		\
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
																											\
		if(currargsSize == 1)																				\
		{																									\
			if(PyArg_ParseTuple(args, "O", &pycallback) == -1)												\
			{																								\
				PyErr_Format(PyExc_TypeError, "KBEngine::writeToDB: args is error!");						\
				PyErr_PrintEx(0);																			\
				pycallback = NULL;																			\
				S_Return;																					\
			}																								\
																											\
			if(!PyCallable_Check(pycallback))																\
			{																								\
				PyErr_Format(PyExc_TypeError, "KBEngine::writeToDB: args1 not is callback!");				\
				PyErr_PrintEx(0);																			\
				Py_DECREF(pycallback);																		\
				S_Return;																					\
			}																								\
		}																									\
																											\
		pobj->writeToDB(pycallback);																		\
		S_Return;																							\
	}																										\
																											\
	void writeToDB(void* data);																				\
																											\
	void destroy()																							\
	{																										\
		if(!isDestroyed_)																					\
		{																									\
			isDestroyed_ = true;																			\
			onDestroy();																					\
		}																									\
		Py_DECREF(this);																					\
	}																										\
	INLINE bool isDestroyed()const																			\
	{																										\
		return isDestroyed_;																				\
	}																										\
	DECLARE_PY_GET_MOTHOD(pyGetIsDestroyed);																\
																											\
	void destroyEntity(void);																				\
	DECLARE_PY_MOTHOD_ARG0(pyDestroyEntity);																\



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
																											\
	void CLASS::destroyEntity(void)																			\
	{																										\
		APP::getSingleton().destroyEntity(id_);																\
	}																										\
	PyObject* CLASS::pyGetIsDestroyed()																		\
	{																										\
		return PyBool_FromLong(isDestroyed());																\
	}																										\
																											\
	void CLASS::addPositionAndDirectionToStream(MemoryStream& s)											\
	{																										\
		ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;								\
		ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;					\
																											\
		Mercury::FixedMessages::MSGInfo* msgInfo =															\
					Mercury::FixedMessages::getSingleton().isFixed("Property::position");					\
																											\
		if(msgInfo != NULL)																					\
		{																									\
			posuid = msgInfo->msgid;																		\
			msgInfo = NULL;																					\
		}																									\
																											\
		msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");					\
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
		ArraySize posdirLen = 3;																			\
		Vector3 pos, dir;																					\
		script::ScriptVector3::convertPyObjectToVector3(pos, pyPos);										\
		script::ScriptVector3::convertPyObjectToVector3(dir, pyDir);										\
																											\
		ADD_POS_DIR_TO_STREAM(s, pos, dir)																	\
																											\
		if(g_componentType != BASEAPP_TYPE)																	\
		{																									\
			Py_XDECREF(pyPos);																				\
			Py_XDECREF(pyDir);																				\
		}																									\
																											\
	}																										\


#define ENTITY_CONSTRUCTION(CLASS)																			\
	id_(id),																								\
	scriptModule_(const_cast<ScriptDefModule*>(scriptModule)),												\
	lpPropertyDescrs_(&scriptModule_->getPropertyDescrs()),													\
	spaceID_(0),																							\
	scriptTimers_(),																						\
	pyCallbackMgr_(),																						\
	isDestroyed_(false),																					\
	pBundle_(new Mercury::Bundle()),																		\
	initing_(true)																							\


#define ENTITY_DECONSTRUCTION(CLASS)																		\
	INFO_MSG("%s::~%s(): %d\n", getScriptName(), getScriptName(), id_);										\
	scriptModule_ = NULL;																					\
	isDestroyed_ = true;																					\
	initing_ = false;																						\
	SAFE_RELEASE(pBundle_);																					\


#define ENTITY_INIT_PROPERTYS(CLASS)																		\
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = lpPropertyDescrs_->begin();				\
	for(; iter != lpPropertyDescrs_->end(); iter++)															\
	{																										\
		PropertyDescription* propertyDescription = iter->second;											\
		DataType* dataType = propertyDescription->getDataType();											\
																											\
		if(dataType)																						\
		{																									\
			PyObject* defObj = propertyDescription->newDefaultVal();										\
			PyObject_SetAttrString(static_cast<PyObject*>(this),											\
						propertyDescription->getName(), defObj);											\
			Py_DECREF(defObj);																				\
																											\
			/* DEBUG_MSG(#CLASS"::"#CLASS": added [%s] property.\n", 
			propertyDescription->getName());*/																\
		}																									\
		else																								\
		{																									\
		ERROR_MSG(#CLASS"::"#CLASS": %s PropertyDescription the dataType is NULL.\n",						\
				propertyDescription->getName());															\
		}																									\
	}																										\
	initing_ = false;																						\
																											\



}
#endif
