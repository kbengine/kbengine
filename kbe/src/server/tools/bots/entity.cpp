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


#include "bots.hpp"
#include "entity.hpp"
#include "clientobject.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"	
#include "network/fixed_messages.hpp"

#include "../../../server/baseapp/baseapp_interface.hpp"
#include "../../../server/cellapp/cellapp_interface.hpp"

#ifdef CODE_INLINE
//#include "entity.ipp"
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
ENTITY_METHOD_DECLARE_BEGIN(Bots, Entity)
ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

ENTITY_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GET_DECLARE("base",							pyGetBaseMailbox,				0,					0)
SCRIPT_GET_DECLARE("cell",							pyGetCellMailbox,				0,					0)
SCRIPT_GET_DECLARE("clientapp",						pyGetClientApp	,				0,					0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,		0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,		0,		0)
ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, const ScriptDefModule* scriptModule, EntityMailbox* base, EntityMailbox* cell):
ScriptObject(getScriptType(), true),
ENTITY_CONSTRUCTION(Entity),
cellMailbox_(cell),
baseMailbox_(base),
pClientApp_(NULL)
{
	ENTITY_INIT_PROPERTYS(Entity);
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	ENTITY_DECONSTRUCTION(Entity);
}	

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetBaseMailbox()
{ 
	EntityMailbox* mailbox = getBaseMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetCellMailbox()
{ 
	EntityMailbox* mailbox = getCellMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetClientApp()
{ 
	ClientObject* app = pClientApp();
	if(app == NULL)
		S_Return;

	Py_INCREF(app);
	return app; 
}

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(const PropertyDescription* propertyDescription, PyObject* pyData)
{
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_METHOD_UID utype = 0;
	s >> utype;
	
	DEBUG_MSG(boost::format("Entity::onRemoteMethodCall: entityID %1%, methodType %2%.\n") % 
				id_ % utype);
	
	MethodDescription* md = scriptModule_->findClientMethodDescription(utype);
	
	if(md == NULL)
	{
		ERROR_MSG(boost::format("Entity::onRemoteMethodCall: can't found method. utype=%1%, callerID:%2%.\n") % 
			utype % id_);

		return;
	}

	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>
						(md->getName()));

	if(md != NULL)
	{
		PyObject* pyargs = md->createFromStream(&s);
		md->call(pyFunc, pyargs);
		Py_DECREF(pyargs);
	}
	
	Py_XDECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
void Entity::writeToDB(void* data)
{
}

//-------------------------------------------------------------------------------------
int Entity::pySetPosition(PyObject *value)
{
	if(!script::ScriptVector3::check(value))
		return -1;

	script::ScriptVector3::convertPyObjectToVector3(getPosition(), value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetPosition()
{
	return new script::ScriptVector3(&getPosition());
}

//-------------------------------------------------------------------------------------
int Entity::pySetDirection(PyObject *value)
{
	if(PySequence_Check(value) <= 0)
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a sequence.");
		PyErr_PrintEx(0);
		return -1;
	}

	Py_ssize_t size = PySequence_Size(value);
	if(size != 3)
	{
		PyErr_Format(PyExc_TypeError, "len(direction) != 3. can't set.");
		PyErr_PrintEx(0);
		return -1;
	}

	Direction3D& dir = getDirection();
	PyObject* pyItem = PySequence_GetItem(value, 0);
	dir.roll	= float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);
	pyItem = PySequence_GetItem(value, 1);
	dir.pitch	= float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);
	pyItem = PySequence_GetItem(value, 2);
	dir.yaw		= float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetDirection()
{
	return new script::ScriptVector3(getDirection().asVector3());
}

//-------------------------------------------------------------------------------------
void Entity::onEntitiesEnabled(void)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEntitiesEnabled"));
}

//-------------------------------------------------------------------------------------
void Entity::onEnterWorld()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEnterWorld"));
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveWorld()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLeaveWorld"));
}

//-------------------------------------------------------------------------------------
void Entity::setPosition(Position3D& pos)
{ 
	position_ = pos; 
//	if(currChunk_ != NULL)
//		currChunk_->getSpace()->onEntityPositionChanged(this, currChunk_, position_);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs)
{
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::addCellDataToStream(uint32 flags, MemoryStream* mstream)
{
}

//-------------------------------------------------------------------------------------
}
