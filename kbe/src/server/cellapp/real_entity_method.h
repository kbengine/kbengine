// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBENGINE_REAL_ENTITY_METHOD_H
#define KBENGINE_REAL_ENTITY_METHOD_H

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

class Entity;
class PropertyDescription;

class RealEntityMethod : public script::ScriptObject
{
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(RealEntityMethod, script::ScriptObject)	
public:	
	RealEntityMethod(PropertyDescription* pComponentPropertyDescription, MethodDescription* methodDescription,
		Entity* ghostEntity);
	
	virtual ~RealEntityMethod();

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
	PropertyDescription*					pComponentPropertyDescription_;		// �Ƿ���һ������еķ���
	MethodDescription*						methodDescription_;					// �������������

	ENTITY_ID								ghostEntityID_;						// ghostEntityID_
	COMPONENT_ID							realCell_;

	std::string								scriptName_;
};

}

#endif // KBENGINE_REAL_ENTITY_METHOD_H
