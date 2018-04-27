// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBENGINE_CLIENTS_REMOTE_ENTITY_METHOD_H
#define KBENGINE_CLIENTS_REMOTE_ENTITY_METHOD_H

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

class ClientsRemoteEntityMethod : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ClientsRemoteEntityMethod, script::ScriptObject)	
public:	
	ClientsRemoteEntityMethod(PropertyDescription* pComponentPropertyDescription, 
		const ScriptDefModule* pScriptModule, 
		MethodDescription* methodDescription,
		bool otherClients,
		ENTITY_ID id);
	
	virtual ~ClientsRemoteEntityMethod();

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
	PropertyDescription*	pComponentPropertyDescription_;

	const ScriptDefModule*	pScriptModule_;			// 该entity所使用的脚本模块对象
	MethodDescription*		methodDescription_;		// 这个方法的描述

	bool					otherClients_;			// 是否只是其他客户端， 不包括自己

	ENTITY_ID				id_;					// entityID
};

}

#endif // KBENGINE_CLIENTS_REMOTE_ENTITY_METHOD_H
