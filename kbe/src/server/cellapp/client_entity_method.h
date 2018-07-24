// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBENGINE_CLIENT_ENTITY_METHOD_H
#define KBENGINE_CLIENT_ENTITY_METHOD_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
// common include	
#include "entitydef/datatype.h"
#include "entitydef/datatypes.h"
#include "helper/debug_helper.h"
#include "pyscript/scriptobject.h"	
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

class ClientEntityMethod : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ClientEntityMethod, script::ScriptObject)	
public:	
	ClientEntityMethod(PropertyDescription* pComponentPropertyDescription,
		const ScriptDefModule* pScriptModule, MethodDescription* methodDescription,
		ENTITY_ID srcEntityID, ENTITY_ID clientEntityID);
	
	virtual ~ClientEntityMethod();

	const char* getName(void) const
	{ 
		return methodDescription_->getName(); 
	};

	MethodDescription* getDescription(void) const
	{ 
		return methodDescription_; 
	}

	static PyObject* tp_call(PyObject* self, 
			PyObject* args, PyObject* kwds);

	PyObject* callmethod(PyObject* args, PyObject* kwds);

protected:
	PropertyDescription*					pComponentPropertyDescription_;

	const ScriptDefModule*					pScriptModule_;						// 该entity所使用的脚本模块对象

	MethodDescription*						methodDescription_;					// 这个方法的描述

	ENTITY_ID								srcEntityID_;						// srcEntityID_

	ENTITY_ID								clientEntityID_;					// clientEntityID_
};

}

#endif // KBENGINE_CLIENT_ENTITY_METHOD_H
