/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __ENTITY_MACRO_H__
#define __ENTITY_MACRO_H__
#include "cstdkbe/cstdkbe.hpp"

namespace KBEngine{

#define ENTITY_METHOD_DECLARE_BEGIN(CLASS)																	\
	SCRIPT_METHOD_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_METHOD_DECLARE("__reduce_ex__",	__reduce_ex__,					METH_VARARGS,			0)		\


#define ENTITY_METHOD_DECLARE_END()																			\
	SCRIPT_METHOD_DECLARE_END()																				\

#define ENTITY_GETSET_DECLARE_BEGIN(CLASS)																	\
	SCRIPT_GETSET_DECLARE_BEGIN(CLASS)																		\
	SCRIPT_GET_DECLARE(	"id",				pyGetID,						0,						0)		\
	SCRIPT_GET_DECLARE("spaceID",			pyGetSpaceID,					0,						0)		\


#define ENTITY_GETSET_DECLARE_END()																			\
	SCRIPT_GETSET_DECLARE_END()																				\



#define ENTITY_HEADER(CLASS)																				\
protected:																									\
	ENTITY_ID		id_;																					\
	ScriptModule*	scriptModule_;																			\
	const ScriptModule::PROPERTYDESCRIPTION_MAP*	lpPropertyDescrs_;										\
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
				getScriptModuleName(), id_);																\
			return;																							\
		}																									\
																											\
		PyObject *key, *value;																				\
		int pos = 0;																						\
																											\
		while(PyDict_Next(dictData, &pos, &key, &value))													\
		{																									\
			PyObject_SetAttr(this, key, value);																\
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
			(*mstream) << (uint32)propertyDescription->getUType();											\
			propertyDescription->getDataType()->addToStream(mstream, pyVal);								\
		}																									\
																											\
		Py_XDECREF(cellData);																				\
	}																										\
																											\
	static PyObject* __reduce_ex__(PyObject* self, PyObject* protocol)										\
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
		return args;																						\
	}																										\
																											\
	static PyObject* __pyget_pyGetID(Entity *self, void *closure)											\
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
	static PyObject* __pyget_pyGetSpaceID(Entity *self, void *closure)										\
	{																										\
		return PyLong_FromLong(self->getSpaceID());															\
	}																										\
																											\
	inline const char* getScriptModuleName(void)const														\
	{																										\
		return scriptModule_->getScriptType()->tp_name; 													\
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
																											\
		if(lpPropertyDescrs_)																				\
		{																									\
																											\
			ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = lpPropertyDescrs_->find(ccattr);	\
			if(iter != lpPropertyDescrs_->end())															\
			{																								\
				char err[255];																				\
				sprintf(err, "property[%s] is in [%s] def. del failed.", ccattr, getScriptModuleName());	\
				PyErr_SetString(PyExc_TypeError, err);														\
				PyErr_PrintEx(0);																			\
				delete ccattr;																				\
				return 0;																					\
			}																								\
		}																									\
																											\
		if(scriptModule_->findMethodDescription(ccattr, g_componentType) != NULL)							\
		{																									\
			char err[255];																					\
			sprintf(err, "method[%s] is in [%s] def. del failed.", ccattr, getScriptModuleName());			\
			PyErr_SetString(PyExc_TypeError, err);															\
			PyErr_PrintEx(0);																				\
			delete ccattr;																					\
			return 0;																						\
		}																									\
																											\
		delete ccattr;																						\
		return ScriptObject::onScriptDelAttribute(attr);													\
	}																										\
																											\
	int onScriptSetAttribute(PyObject* attr, PyObject* value)												\
	{																										\
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
					delete ccattr;																			\
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
					delete ccattr;																			\
					return result;																			\
				}																							\
			}																								\
		}																									\
																											\
		delete ccattr;																						\
		return ScriptObject::onScriptSetAttribute(attr, value);												\
	}																										\
																											\
	PyObject * onScriptGetAttribute(PyObject* attr)															\
	{																										\
		return ScriptObject::onScriptGetAttribute(attr);													\
	}																										\
																											\


#define ENTITY_CONSTRUCTION(CLASS)																			\
	id_(id),																								\
	scriptModule_(scriptModule),																			\
	lpPropertyDescrs_(&scriptModule->getPropertyDescrs())													\

#define ENTITY_DECONSTRUCTION(CLASS)																		\
	INFO_MSG(#CLASS"::~"#CLASS"(): %s %ld\n", getScriptModuleName(), id_);									\
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
