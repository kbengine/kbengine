// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBENGINE_REMOTE_ENTITY_METHOD_H
#define KBENGINE_REMOTE_ENTITY_METHOD_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
#include "datatype.h"
#include "datatypes.h"
#include "helper/debug_helper.h"
#include "network/packet.h"
#include "entitycallabstract.h"
#include "pyscript/scriptobject.h"	


namespace KBEngine{

class MethodDescription;

class RemoteEntityMethod : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(RemoteEntityMethod, script::ScriptObject)	
		
public:	
	RemoteEntityMethod(MethodDescription* methodDescription, 
						EntityCallAbstract* entityCall, PyTypeObject* pyType = NULL);
	
	virtual ~RemoteEntityMethod();

	const char* getName(void) const;

	MethodDescription* getDescription(void) const
	{ 
		return methodDescription_; 
	}

	static PyObject* tp_call(PyObject* self, 
			PyObject* args, PyObject* kwds);

	EntityCallAbstract* getEntityCall(void) const 
	{
		return pEntityCall_; 
	}
	
protected:	
	MethodDescription*		methodDescription_;					// 这个方法的描述
	EntityCallAbstract*		pEntityCall_;						// 这个方法所属的entityCall
};
}

#endif // KBENGINE_REMOTE_ENTITY_METHOD_H
