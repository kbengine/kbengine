// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "entitydef/entity_component.h"

namespace KBEngine{

SCRIPT_GETSET_DECLARE_BEGIN(EntityComponent)
SCRIPT_GET_DECLARE("ownerID",						pyGetOwnerID,			0,					0)
SCRIPT_GET_DECLARE("owner",							pyGetOwner,				0,					0)
SCRIPT_GET_DECLARE("name",							pyName,					0,					0)
SCRIPT_GET_DECLARE("isDestroyed",					pyIsDestroyed,			0,					0)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(EntityComponent, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
PyObject* EntityComponent::onScriptGetAttribute(PyObject* attr)
{
	return ScriptObject::onScriptGetAttribute(attr);
}

}
