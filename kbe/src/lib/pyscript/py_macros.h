// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef _PY_MACROS_H
#define _PY_MACROS_H

#include "common/common.h"
#include "helper/debug_helper.h"

namespace KBEngine{ namespace script{

#define PY_METHOD_ARG_char								char
#define PY_METHOD_ARG_char_ARG							char
#define PY_METHOD_ARG_char_PYARGTYPE					"b"

#define PY_METHOD_ARG_charptr							char *
#define PY_METHOD_ARG_charptr_ARG						char *
#define PY_METHOD_ARG_charptr_PYARGTYPE					"s"

#define PY_METHOD_ARG_const_charptr						const char *
#define PY_METHOD_ARG_const_charptr_ARG					char *
#define PY_METHOD_ARG_const_charptr_PYARGTYPE			"s"

#define PY_METHOD_ARG_PyObject_ptr						PyObject *
#define PY_METHOD_ARG_PyObject_ptr_ARG					PyObject *
#define PY_METHOD_ARG_PyObject_ptr_PYARGTYPE			"O"

#define PY_METHOD_ARG_float								float
#define PY_METHOD_ARG_float_ARG							float
#define PY_METHOD_ARG_float_PYARGTYPE					"f"

#define PY_METHOD_ARG_double							double
#define PY_METHOD_ARG_double_ARG						double
#define PY_METHOD_ARG_double_PYARGTYPE					"d"

#define PY_METHOD_ARG_uint8								uint8
#define PY_METHOD_ARG_uint8_ARG							uint8
#define PY_METHOD_ARG_uint8_PYARGTYPE					"B"

#define PY_METHOD_ARG_uint16							uint16
#define PY_METHOD_ARG_uint16_ARG						uint16
#define PY_METHOD_ARG_uint16_PYARGTYPE					"H"

#define PY_METHOD_ARG_uint32							uint32
#define PY_METHOD_ARG_uint32_ARG						uint32
#define PY_METHOD_ARG_uint32_PYARGTYPE					"I"

#define PY_METHOD_ARG_uint64							uint64
#define PY_METHOD_ARG_uint64_ARG						uint64
#define PY_METHOD_ARG_uint64_PYARGTYPE					"K"

#define PY_METHOD_ARG_int8								int8
#define PY_METHOD_ARG_int8_ARG							int8
#define PY_METHOD_ARG_int8_PYARGTYPE					"b"

#define PY_METHOD_ARG_int16								int16
#define PY_METHOD_ARG_int16_ARG							int16
#define PY_METHOD_ARG_int16_PYARGTYPE					"h"

#define PY_METHOD_ARG_int								int32
#define PY_METHOD_ARG_int								int32
#define PY_METHOD_ARG_int_ARG							int32
#define PY_METHOD_ARG_int_PYARGTYPE						"i"

#define PY_METHOD_ARG_int32								int32
#define PY_METHOD_ARG_int32_ARG							int32
#define PY_METHOD_ARG_int32_PYARGTYPE					"i"

#define PY_METHOD_ARG_int64								int64
#define PY_METHOD_ARG_int64_ARG							int64
#define PY_METHOD_ARG_int64_PYARGTYPE					"L"

#define PY_METHOD_ARG_ScriptID							ScriptID
#define PY_METHOD_ARG_ScriptID_ARG						ScriptID
#define PY_METHOD_ARG_ScriptID_PYARGTYPE				"i"

#define PY_METHOD_ARG_TIMER_ID							TIMER_ID
#define PY_METHOD_ARG_TIMER_ID_ARG						TIMER_ID
#define PY_METHOD_ARG_TIMER_ID_PYARGTYPE				"I"

#define PY_METHOD_ARG_SPACE_ID							SPACE_ID
#define PY_METHOD_ARG_SPACE_ID_ARG						SPACE_ID
#define PY_METHOD_ARG_SPACE_ID_PYARGTYPE				"I"

#define PY_METHOD_ARG_ENTITY_ID							ENTITY_ID
#define PY_METHOD_ARG_ENTITY_ID_ARG						ENTITY_ID
#define PY_METHOD_ARG_ENTITY_ID_PYARGTYPE				"i"

//-----------------------------------------------------------------------------------------------------------
/** 定义暴露给脚本的方法宏
*/
#define SCRIPT_METHOD_DECLARE_BEGIN(CLASS)													bool CLASS::_##CLASS##_py_installed = false; PyMethodDef CLASS::_##CLASS##_scriptMethods[] = {
#define TEMPLATE_SCRIPT_METHOD_DECLARE_BEGIN(TEMPLATE_HEADER, TEMPLATE_CLASS, CLASSNAME)	TEMPLATE_HEADER bool TEMPLATE_CLASS::_##CLASSNAME##_py_installed = false;  TEMPLATE_HEADER PyMethodDef TEMPLATE_CLASS::_##CLASSNAME##_scriptMethods[] = {
#define SCRIPT_METHOD_DECLARE(METHOD_NAME, METHOD_FUNC, FLAGS, DOC)							{METHOD_NAME, (PyCFunction)&__py_##METHOD_FUNC, FLAGS, DOC},
#define SCRIPT_DIRECT_METHOD_DECLARE(METHOD_NAME, METHOD_FUNC, FLAGS, DOC)					{METHOD_NAME, (PyCFunction)&METHOD_FUNC, FLAGS, DOC},
#define SCRIPT_METHOD_DECLARE_END()															{0, 0, 0, 0}};

// 向模块追加方法
#define APPEND_SCRIPT_MODULE_METHOD(MODULE, NAME, FUNC, FLAGS, SELF)						\
	static PyMethodDef __pymethod_##NAME = {#NAME, (PyCFunction) FUNC, FLAGS, NULL};		\
	if(PyModule_AddObject(MODULE, #NAME, PyCFunction_New(&__pymethod_##NAME, SELF)) != 0)	\
	{																						\
		SCRIPT_ERROR_CHECK();																\
		ERROR_MSG("append " #NAME " to pyscript error!\n");									\
	}																						\
	
/** 定义暴露给脚本的属性宏
*/
#define SCRIPT_MEMBER_DECLARE_BEGIN(CLASS)													PyMemberDef CLASS::_##CLASS##_scriptMembers[] =	{
#define TEMPLATE_SCRIPT_MEMBER_DECLARE_BEGIN(TEMPLATE_HEADER, TEMPLATE_CLASS, CLASSNAME)	TEMPLATE_HEADER PyMemberDef TEMPLATE_CLASS::_##CLASSNAME##_scriptMembers[] =	{
#define SCRIPT_MEMBER_DECLARE(MEMBER_NAME, MEMBER_REF, MEMBER_TYPE, FLAGS, DOC)				{const_cast<char*>(MEMBER_NAME), MEMBER_TYPE, offsetof(ThisClass, MEMBER_REF), FLAGS, DOC},
#define SCRIPT_MEMBER_DECLARE_END()															{0, 0, 0, 0, 0}};

/** 定义暴露给脚本的getset属性宏
*/
#define SCRIPT_GETSET_DECLARE_BEGIN(CLASS)													PyGetSetDef CLASS::_##CLASS##_scriptGetSeters[] =	{
#define TEMPLATE_SCRIPT_GETSET_DECLARE_BEGIN(TEMPLATE_HEADER, TEMPLATE_CLASS, CLASSNAME)	TEMPLATE_HEADER PyGetSetDef TEMPLATE_CLASS::_##CLASSNAME##_scriptGetSeters[] =	{
#define SCRIPT_GETSET_DECLARE(NAME, GET, SET, DOC, CLOSURE)									{const_cast<char*>(NAME), (getter)__pyget_##GET, (setter)__pyset_##SET, DOC, CLOSURE},
#define SCRIPT_GET_DECLARE(NAME, GET, DOC, CLOSURE)											{const_cast<char*>(NAME), (getter)__pyget_##GET, (setter)__py_readonly_descr, DOC, const_cast<char*>(NAME)},
#define SCRIPT_SET_DECLARE(NAME, SET, DOC, CLOSURE)											{const_cast<char*>(NAME), (getter)__pyset_##SET, (setter)__py_writeonly_descr, DOC, const_cast<char*>(NAME)},
#define SCRIPT_GETSET_DECLARE_END()															{0, 0, 0, 0, 0}};

//-----------------------------------------------------------------------------------------------------------
/* 声明一个脚本get方法 */
#define DECLARE_PY_GET_MOTHOD(MNAME)												\
	PyObject* MNAME();																\
	static PyObject* __pyget_##MNAME(PyObject *self, void *closure)					\
	{																				\
		return static_cast<ThisClass*>(self)->MNAME();								\
	}																				\


/* 声明一个脚本set方法 */
#define DECLARE_PY_SET_MOTHOD(MNAME)												\
	int MNAME(PyObject *value);														\
	static int __pyset_##MNAME(PyObject *self,										\
									PyObject *value, void *closure)					\
	{																				\
		return static_cast<ThisClass*>(self)->MNAME(value);							\
	}																				\

/* 声明一个脚本getset方法 */
#define DECLARE_PY_GETSET_MOTHOD(GETNAME, SETNAME)									\
	DECLARE_PY_GET_MOTHOD(GETNAME)													\
	DECLARE_PY_SET_MOTHOD(SETNAME)													\

//-----------------------------------------------------------------------------------------------------------
#define DECLARE_PY_MOTHOD_ARG0(FUNCNAME)																							\
	PyObject* FUNCNAME(void);																										\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		return static_cast<ThisClass*>(self)->FUNCNAME();																			\
	}																																\
																																	\

#define DECLARE_PY_MOTHOD_ARG1(FUNCNAME, ARG_TYPE1)																					\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1);																					\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1 arg1;																								\
																																	\
		const uint8 argsSize = 1;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE,														\
										&arg1))																						\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s) error!\n", __FUNCTION__, #ARG_TYPE1);									\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s), gived %d! is script[%s].\n",							\
				__FUNCTION__, argsSize, #ARG_TYPE1, currargsSize, pobj->scriptName());												\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1);																								\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG2(FUNCNAME, ARG_TYPE1, ARG_TYPE2)																		\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
																																	\
		const uint8 argsSize = 2;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE,														\
										&arg1, &arg2))																				\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s) error!\n", __FUNCTION__, #ARG_TYPE1, #ARG_TYPE2);					\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s), gived %d! is script[%s].\n",						\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, currargsSize, pobj->scriptName());									\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2);																							\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG3(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3)															\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
																																	\
		const uint8 argsSize = 3;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE,														\
										&arg1, &arg2, &arg3))																		\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s) error!\n", __FUNCTION__, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3);	\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s), gived %d! is script[%s].\n",					\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, currargsSize, pobj->scriptName());						\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3);																					\
	}																																\
																																	\

#define DECLARE_PY_MOTHOD_ARG4(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3, ARG_TYPE4)												\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3,																						\
					PY_METHOD_ARG_##ARG_TYPE4);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
		PY_METHOD_ARG_##ARG_TYPE4##_ARG arg4;																						\
																																	\
		const uint8 argsSize = 4;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE4##_PYARGTYPE,														\
										&arg1, &arg2, &arg3, &arg4))																\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s, %s) error!\n", __FUNCTION__,									\
										#ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4);											\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s, %s), gived %d! is script[%s].\n",				\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, currargsSize, pobj->scriptName());			\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3, arg4);																				\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG5(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3, ARG_TYPE4, ARG_TYPE5)										\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3,																						\
					PY_METHOD_ARG_##ARG_TYPE4,																						\
					PY_METHOD_ARG_##ARG_TYPE5);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
		PY_METHOD_ARG_##ARG_TYPE4##_ARG arg4;																						\
		PY_METHOD_ARG_##ARG_TYPE5##_ARG arg5;																						\
																																	\
		const uint8 argsSize = 5;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE4##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE5##_PYARGTYPE,														\
										&arg1, &arg2, &arg3, &arg4, &arg5))															\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s, %s, %s) error!\n", __FUNCTION__,								\
						#ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5);												\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s, %s, %s), gived %d! is script[%s].\n",			\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, currargsSize, pobj->scriptName());	\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3, arg4, arg5);																		\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG6(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3, ARG_TYPE4, ARG_TYPE5, ARG_TYPE6)							\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3,																						\
					PY_METHOD_ARG_##ARG_TYPE4,																						\
					PY_METHOD_ARG_##ARG_TYPE5,																						\
					PY_METHOD_ARG_##ARG_TYPE6);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
		PY_METHOD_ARG_##ARG_TYPE4##_ARG arg4;																						\
		PY_METHOD_ARG_##ARG_TYPE5##_ARG arg5;																						\
		PY_METHOD_ARG_##ARG_TYPE6##_ARG arg6;																						\
																																	\
		const uint8 argsSize = 6;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE4##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE5##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE6##_PYARGTYPE,														\
										&arg1, &arg2, &arg3, &arg4, &arg5, &arg6))													\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s, %s, %s, %s) error!\n", __FUNCTION__,							\
							#ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6);								\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s, %s, %s, %s), gived %d! is script[%s].\n",		\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,						\
				currargsSize, pobj->scriptName());																					\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3, arg4, arg5, arg6);																	\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG7(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3, ARG_TYPE4, ARG_TYPE5, ARG_TYPE6, ARG_TYPE7)				\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3,																						\
					PY_METHOD_ARG_##ARG_TYPE4,																						\
					PY_METHOD_ARG_##ARG_TYPE5,																						\
					PY_METHOD_ARG_##ARG_TYPE6,																						\
					PY_METHOD_ARG_##ARG_TYPE7);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
		PY_METHOD_ARG_##ARG_TYPE4##_ARG arg4;																						\
		PY_METHOD_ARG_##ARG_TYPE5##_ARG arg5;																						\
		PY_METHOD_ARG_##ARG_TYPE6##_ARG arg6;																						\
		PY_METHOD_ARG_##ARG_TYPE7##_ARG arg7;																						\
																																	\
		const uint8 argsSize = 7;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE4##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE5##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE6##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE7##_PYARGTYPE,														\
										&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7))											\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s, %s, %s, %s, %s) error!\n", __FUNCTION__,						\
							#ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7);					\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s, %s, %s, %s, %s), gived %d! is script[%s].\n",	\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7,			\
				currargsSize, pobj->scriptName());																					\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3, arg4, arg5, arg6, arg7);															\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG8(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3, ARG_TYPE4, ARG_TYPE5, ARG_TYPE6, ARG_TYPE7, ARG_TYPE8)	\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3,																						\
					PY_METHOD_ARG_##ARG_TYPE4,																						\
					PY_METHOD_ARG_##ARG_TYPE5,																						\
					PY_METHOD_ARG_##ARG_TYPE6,																						\
					PY_METHOD_ARG_##ARG_TYPE7,																						\
					PY_METHOD_ARG_##ARG_TYPE8);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
		PY_METHOD_ARG_##ARG_TYPE4##_ARG arg4;																						\
		PY_METHOD_ARG_##ARG_TYPE5##_ARG arg5;																						\
		PY_METHOD_ARG_##ARG_TYPE6##_ARG arg6;																						\
		PY_METHOD_ARG_##ARG_TYPE7##_ARG arg7;																						\
		PY_METHOD_ARG_##ARG_TYPE8##_ARG arg8;																						\
																																	\
		const uint8 argsSize = 8;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE4##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE5##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE6##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE7##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE8##_PYARGTYPE,														\
										&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8))									\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s, %s, %s, %s, %s, %s) error!\n", __FUNCTION__,					\
							#ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7,	#ARG_TYPE8);		\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s, %s, %s, %s, %s, %s), gived %d! is script[%s].\n",	\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7,	#ARG_TYPE8,	\
				currargsSize, pobj->scriptName());																					\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);														\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG9(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3, ARG_TYPE4, ARG_TYPE5, ARG_TYPE6, ARG_TYPE7, ARG_TYPE8,	\
											ARG_TYPE9)																				\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3,																						\
					PY_METHOD_ARG_##ARG_TYPE4,																						\
					PY_METHOD_ARG_##ARG_TYPE5,																						\
					PY_METHOD_ARG_##ARG_TYPE6,																						\
					PY_METHOD_ARG_##ARG_TYPE7,																						\
					PY_METHOD_ARG_##ARG_TYPE8,																						\
					PY_METHOD_ARG_##ARG_TYPE9);																						\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
		PY_METHOD_ARG_##ARG_TYPE4##_ARG arg4;																						\
		PY_METHOD_ARG_##ARG_TYPE5##_ARG arg5;																						\
		PY_METHOD_ARG_##ARG_TYPE6##_ARG arg6;																						\
		PY_METHOD_ARG_##ARG_TYPE7##_ARG arg7;																						\
		PY_METHOD_ARG_##ARG_TYPE8##_ARG arg8;																						\
		PY_METHOD_ARG_##ARG_TYPE9##_ARG arg9;																						\
																																	\
		const uint8 argsSize = 9;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE4##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE5##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE6##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE7##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE8##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE9##_PYARGTYPE,														\
										&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9))								\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s, %s, %s, %s, %s, %s, %s) error!\n", __FUNCTION__,				\
						#ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7,	#ARG_TYPE8, #ARG_TYPE9);\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s, %s, %s, %s, %s, %s, %s), gived %d! is script[%s].\n",	\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7,	#ARG_TYPE8,		\
				#ARG_TYPE9, currargsSize, pobj->scriptName());																		\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);												\
	}																																\
																																	\


#define DECLARE_PY_MOTHOD_ARG10(FUNCNAME, ARG_TYPE1, ARG_TYPE2, ARG_TYPE3, ARG_TYPE4, ARG_TYPE5, ARG_TYPE6, ARG_TYPE7, ARG_TYPE8,	\
											ARG_TYPE9, ARG_TYPE10)																	\
	PyObject* FUNCNAME(PY_METHOD_ARG_##ARG_TYPE1,																					\
					PY_METHOD_ARG_##ARG_TYPE2,																						\
					PY_METHOD_ARG_##ARG_TYPE3,																						\
					PY_METHOD_ARG_##ARG_TYPE4,																						\
					PY_METHOD_ARG_##ARG_TYPE5,																						\
					PY_METHOD_ARG_##ARG_TYPE6,																						\
					PY_METHOD_ARG_##ARG_TYPE7,																						\
					PY_METHOD_ARG_##ARG_TYPE8,																						\
					PY_METHOD_ARG_##ARG_TYPE9,																						\
					PY_METHOD_ARG_##ARG_TYPE10);																					\
	static PyObject* __py_##FUNCNAME(PyObject* self, PyObject* args, PyObject* kwds)												\
	{																																\
		PY_METHOD_ARG_##ARG_TYPE1##_ARG arg1;																						\
		PY_METHOD_ARG_##ARG_TYPE2##_ARG arg2;																						\
		PY_METHOD_ARG_##ARG_TYPE3##_ARG arg3;																						\
		PY_METHOD_ARG_##ARG_TYPE4##_ARG arg4;																						\
		PY_METHOD_ARG_##ARG_TYPE5##_ARG arg5;																						\
		PY_METHOD_ARG_##ARG_TYPE6##_ARG arg6;																						\
		PY_METHOD_ARG_##ARG_TYPE7##_ARG arg7;																						\
		PY_METHOD_ARG_##ARG_TYPE8##_ARG arg8;																						\
		PY_METHOD_ARG_##ARG_TYPE9##_ARG arg9;																						\
		PY_METHOD_ARG_##ARG_TYPE10##_ARG arg10;																						\
																																	\
		const uint8 argsSize = 10;																									\
		uint16 currargsSize = (uint16)PyTuple_Size(args);																			\
		ThisClass* pobj = static_cast<ThisClass*>(self);																			\
																																	\
		if(currargsSize == argsSize)																								\
		{																															\
			if(!PyArg_ParseTuple(args, PY_METHOD_ARG_##ARG_TYPE1##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE2##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE3##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE4##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE5##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE6##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE7##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE8##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE9##_PYARGTYPE "|"													\
										PY_METHOD_ARG_##ARG_TYPE10##_PYARGTYPE,														\
										&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10))						\
			{																														\
				PyErr_Format(PyExc_TypeError, "%s: args(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s) error!\n", __FUNCTION__,			\
				#ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7,	#ARG_TYPE8,						\
				#ARG_TYPE9, #ARG_TYPE10);																							\
				PyErr_PrintEx(0);																									\
				S_Return;																											\
			}																														\
		}																															\
		else																														\
		{																															\
			PyErr_Format(PyExc_AssertionError, "%s: args require %d args(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s), gived %d! is script[%s].\n",	\
				__FUNCTION__, argsSize, #ARG_TYPE1, #ARG_TYPE2, #ARG_TYPE3, #ARG_TYPE4, #ARG_TYPE5, #ARG_TYPE6,	#ARG_TYPE7,	#ARG_TYPE8,	\
				#ARG_TYPE9, #ARG_TYPE10, currargsSize, pobj->scriptName());															\
																																	\
			PyErr_PrintEx(0);																										\
																																	\
			S_Return;																												\
		}																															\
																																	\
		return pobj->FUNCNAME(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);											\
	}																																\
																																	\


//-----------------------------------------------------------------------------------------------------------
/** 定义宏用于安全的调用一个对象的方法
*/
#define SCRIPT_OBJECT_CALL_ARGS0(OBJ, METHOT_NAME, GETERROR)											\
{																										\
	if(static_cast<PyObject*>(OBJ) == NULL)																\
	{																									\
		ERROR_MSG("call " #METHOT_NAME ", OBJ is NULL!\n");												\
	}																									\
	else																								\
	{																									\
		if (PyObject_HasAttrString(OBJ, METHOT_NAME))													\
		{																								\
			PyObject* pyResult = PyObject_CallMethod((OBJ), (METHOT_NAME), 								\
																const_cast<char*>(""));					\
			if(pyResult != NULL) {																		\
				Py_DECREF(pyResult);																	\
			}																							\
			else																						\
			{																							\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
		else																							\
		{																								\
			if (GETERROR)																				\
			{																							\
				PyErr_Format(PyExc_AttributeError, "'%s' object has no attribute '%s'", 				\
					OBJ->ob_type->tp_name, METHOT_NAME);												\
																										\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
	}																									\
}																										\
			
			
#define SCRIPT_OBJECT_CALL_ARGS1(OBJ, METHOT_NAME, FORMAT, ARG1, GETERROR)								\
{																										\
	if(static_cast<PyObject*>(OBJ) == NULL)																\
	{																									\
		ERROR_MSG("call " #METHOT_NAME ", OBJ is NULL!\n");												\
	}																									\
	else																								\
	{																									\
		if (PyObject_HasAttrString(OBJ, METHOT_NAME))													\
		{																								\
			PyObject* pyResult = PyObject_CallMethod((OBJ), 											\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1)													\
													);													\
			if(pyResult != NULL) {																		\
				Py_DECREF(pyResult);																	\
			}																							\
			else																						\
			{																							\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
		else																							\
		{																								\
			if (GETERROR)																				\
			{																							\
				PyErr_Format(PyExc_AttributeError, "'%s' object has no attribute '%s'", 				\
					OBJ->ob_type->tp_name, METHOT_NAME);												\
																										\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
	}																									\
}																										\
			
			
#define SCRIPT_OBJECT_CALL_ARGS2(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2, GETERROR)						\
{																										\
	if(static_cast<PyObject*>(OBJ) == NULL)																\
	{																									\
		ERROR_MSG("call " #METHOT_NAME ", OBJ is NULL!\n");												\
	}																									\
	else																								\
	{																									\
		if (PyObject_HasAttrString(OBJ, METHOT_NAME))													\
		{																								\
			PyObject* pyResult = PyObject_CallMethod((OBJ), 											\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2)													\
													);													\
			if(pyResult != NULL) {																		\
				Py_DECREF(pyResult);																	\
			}																							\
			else																						\
			{																							\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
		else																							\
		{																								\
			if (GETERROR)																				\
			{																							\
				PyErr_Format(PyExc_AttributeError, "'%s' object has no attribute '%s'", 				\
					OBJ->ob_type->tp_name, METHOT_NAME);												\
																										\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
	}																									\
}																										\


#define SCRIPT_OBJECT_CALL_ARGS3(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2, ARG3, GETERROR)					\
{																										\
	if(static_cast<PyObject*>(OBJ) == NULL)																\
	{																									\
		ERROR_MSG("call " #METHOT_NAME ", OBJ is NULL!\n");												\
	}																									\
	else																								\
	{																									\
		if (PyObject_HasAttrString(OBJ, METHOT_NAME))													\
		{																								\
			PyObject* pyResult = PyObject_CallMethod((OBJ), 											\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2),													\
												(ARG3)													\
													);													\
			if(pyResult != NULL) {																		\
				Py_DECREF(pyResult);																	\
			}																							\
			else																						\
			{																							\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
		else																							\
		{																								\
			if (GETERROR)																				\
			{																							\
				PyErr_Format(PyExc_AttributeError, "'%s' object has no attribute '%s'", 				\
					OBJ->ob_type->tp_name, METHOT_NAME);												\
																										\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
	}																									\
}																										\


#define SCRIPT_OBJECT_CALL_ARGS4(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2, ARG3, ARG4, GETERROR)			\
{																										\
	if(static_cast<PyObject*>(OBJ) == NULL)																\
	{																									\
		ERROR_MSG("call " #METHOT_NAME ", OBJ is NULL!\n");												\
	}																									\
	else																								\
	{																									\
		if (PyObject_HasAttrString(OBJ, METHOT_NAME))													\
		{																								\
			PyObject* pyResult = PyObject_CallMethod((OBJ), 											\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2),													\
												(ARG3),													\
												(ARG4)													\
													);													\
			if(pyResult != NULL) {																		\
				Py_DECREF(pyResult);																	\
			}																							\
			else																						\
			{																							\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
		else																							\
		{																								\
			if (GETERROR)																				\
			{																							\
				PyErr_Format(PyExc_AttributeError, "'%s' object has no attribute '%s'", 				\
					OBJ->ob_type->tp_name, METHOT_NAME);												\
																										\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
	}																									\
}																										\


#define SCRIPT_OBJECT_CALL_ARGS5(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5, GETERROR)		\
{																										\
	if(static_cast<PyObject*>(OBJ) == NULL)																\
	{																									\
		ERROR_MSG("call " #METHOT_NAME ", OBJ is NULL!\n");												\
	}																									\
	else																								\
	{																									\
		if (PyObject_HasAttrString(OBJ, METHOT_NAME))													\
		{																								\
			PyObject* pyResult = PyObject_CallMethod((OBJ), 											\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2),													\
												(ARG3),													\
												(ARG4),													\
												(ARG5)													\
													);													\
			if(pyResult != NULL) {																		\
				Py_DECREF(pyResult);																	\
			}																							\
			else																						\
			{																							\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
		else																							\
		{																								\
			if (GETERROR)																				\
			{																							\
				PyErr_Format(PyExc_AttributeError, "'%s' object has no attribute '%s'", 				\
					OBJ->ob_type->tp_name, METHOT_NAME);												\
																										\
				PyErr_PrintEx(0);																		\
			}																							\
		}																								\
	}																									\
}																										\
			
			
			
}
}
#endif
