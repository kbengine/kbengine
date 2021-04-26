// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "entitydef/entity_component.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(EntityComponent)
//SCRIPT_METHOD_DECLARE("__reduce_ex__",				reduce_ex__,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("addTimer",						pyAddTimer,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("delTimer",						pyDelTimer,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityComponent)
SCRIPT_GET_DECLARE("ownerID",							pyGetOwnerID,			0,					0)
SCRIPT_GET_DECLARE("owner",								pyGetOwner,				0,					0)
SCRIPT_GET_DECLARE("name",								pyName,					0,					0)
SCRIPT_GET_DECLARE("className",							pyGetClassName,			0,					0)
SCRIPT_GET_DECLARE("isDestroyed",						pyIsDestroyed,			0,					0)
SCRIPT_GET_DECLARE("cell",								pyGetCellEntityCall,	0,					0)
SCRIPT_GET_DECLARE("client",							pyGetClientEntityCall,	0,					0)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(EntityComponent, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
PyObject* EntityComponent::onScriptGetAttribute(PyObject* attr)
{
	return ScriptObject::onScriptGetAttribute(attr);
}

}
