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


#include "clientapp.hpp"
#include "entity.hpp"
#include "clientobjectbase.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"	
#include "network/fixed_messages.hpp"

#include "../../../server/baseapp/baseapp_interface.hpp"
#include "../../../server/cellapp/cellapp_interface.hpp"

#ifndef CODE_INLINE
#include "entity.ipp"
#endif

namespace KBEngine{
namespace client
{

//-------------------------------------------------------------------------------------
CLIENT_ENTITY_METHOD_DECLARE_BEGIN(ClientApp, Entity)
CLIENT_ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

CLIENT_ENTITY_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GET_DECLARE("base",							pyGetBaseMailbox,				0,					0)
SCRIPT_GET_DECLARE("cell",							pyGetCellMailbox,				0,					0)
SCRIPT_GET_DECLARE("clientapp",						pyGetClientApp	,				0,					0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,		0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,		0,		0)
SCRIPT_GETSET_DECLARE("velocity",					pyGetMoveSpeed,					pySetMoveSpeed,		0,		0)
CLIENT_ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, const ScriptDefModule* scriptModule, EntityMailbox* base, EntityMailbox* cell):
ScriptObject(getScriptType(), true),
ENTITY_CONSTRUCTION(Entity),
cellMailbox_(cell),
baseMailbox_(base),
position_(),
serverPosition_(),
direction_(),
pClientApp_(NULL),
aspect_(id),
velocity_(3.0f),
enterword_(false),
isOnGound_(true)
{
	ENTITY_INIT_PROPERTYS(Entity);
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	enterword_ = false;
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
	ClientObjectBase* app = pClientApp();
	if(app == NULL)
		S_Return;

	Py_INCREF(app);
	return app; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::onScriptGetAttribute(PyObject* attr)
{
	DEBUG_OP_ATTRIBUTE("get", attr)
	return ScriptObject::onScriptGetAttribute(attr);
}	

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(const PropertyDescription* propertyDescription, PyObject* pyData)
{
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_METHOD_UID utype = 0;
	
	MethodDescription* md = NULL;
	
	if(scriptModule_->useMethodDescrAlias())
	{
		ENTITY_DEF_ALIASID aliasID;
		s >> aliasID;
		md = scriptModule_->findAliasMethodDescription(aliasID);
		utype = aliasID;
	}
	else
	{
		s >> utype;
		md = scriptModule_->findClientMethodDescription(utype);
	}

	if(md == NULL)
	{
		ERROR_MSG(boost::format("Entity::onRemoteMethodCall: can't found method. utype=%1%, callerID:%2%.\n") % 
			utype % id_);

		return;
	}

	if(g_debugEntity)
	{
		DEBUG_MSG(boost::format("Entity::onRemoteMethodCall: entityID %1%, methodType %2%.\n") % 
				id_ % utype);
	}

	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>
						(md->getName()));

	if(md != NULL)
	{
		if(md->getArgSize() == 0)
		{
			md->call(pyFunc, NULL);
		}
		else
		{
			PyObject* pyargs = md->createFromStream(&s);
			if(pyargs)
			{
				md->call(pyFunc, pyargs);
				Py_DECREF(pyargs);
			}
			else
			{
				SCRIPT_ERROR_CHECK();
			}
		}
	}
	
	Py_XDECREF(pyFunc);
	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Entity::onUpdatePropertys(MemoryStream& s)
{
	ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
	ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
	ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

	if(!scriptModule_->usePropertyDescrAlias())
	{
		Mercury::FixedMessages::MSGInfo* msgInfo =
					Mercury::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;

		msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)
			diruid = msgInfo->msgid;

		msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::spaceID");
		if(msgInfo != NULL)
			spaceuid = msgInfo->msgid;
	}
	else
	{
		posuid = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;
		diruid = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;
		spaceuid = ENTITY_BASE_PROPERTY_ALIASID_SPACEID;
	}

	while(s.opsize() > 0)
	{
		ENTITY_PROPERTY_UID uid;
		uint8 aliasID = 0;

		if(scriptModule_->usePropertyDescrAlias())
		{
			s >> aliasID;
			uid = aliasID;
		}
		else
		{
			s >> uid;
		}

		// 如果是位置或者朝向信息则
		if(uid == posuid)
		{
			Position3D pos;
			ArraySize size;

#ifdef CLIENT_NO_FLOAT		
			int32 x, y, z;
			s >> size >> x >> y >> z;

			pos.x = (float)x;
			pos.y = (float)y;
			pos.z = (float)z;
#else
			s >> size >> pos.x >> pos.y >> pos.z;
#endif
			setPosition(pos);
			continue;
		}
		else if(uid == diruid)
		{
			Direction3D dir;
			ArraySize size;

#ifdef CLIENT_NO_FLOAT		
			int32 x, y, z;
			s >> size >> x >> y >> z;

			dir.roll((float)x);
			dir.pitch((float)y);
			dir.yaw((float)z);
#else
			float yaw, pitch, roll;
			s >> size >> roll >> pitch >> yaw;
			dir.yaw(yaw);
			dir.pitch(pitch);
			dir.roll(roll);
#endif

			setDirection(dir);
			continue;
		}
		else if(uid == spaceuid)
		{
			SPACE_ID spaceID;
			s >> spaceID;
			setSpaceID(spaceID);
			continue;
		}

		PropertyDescription* pPropertyDescription = NULL;
		
		if(scriptModule_->usePropertyDescrAlias())
			pPropertyDescription = getScriptModule()->findAliasPropertyDescription(aliasID);
		else
			pPropertyDescription = getScriptModule()->findClientPropertyDescription(uid);

		if(pPropertyDescription == NULL)
		{
			ERROR_MSG(boost::format("Entity::onUpdatePropertys: not found %1%\n") % uid);
			return;
		}

		PyObject* pyobj = pPropertyDescription->createFromStream(&s);

		PyObject* pyOld = PyObject_GetAttrString(this, pPropertyDescription->getName());
		PyObject_SetAttrString(this, pPropertyDescription->getName(), pyobj);

		std::string setname = "set_";
		setname += pPropertyDescription->getName();

		SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>(setname.c_str()), 
		const_cast<char*>("O"), pyOld);

		Py_DECREF(pyobj);
		Py_DECREF(pyOld);
		SCRIPT_ERROR_CHECK();
	}
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
	onPositionChanged();
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetPosition()
{
	return new script::ScriptVector3(&getPosition(), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onPositionChanged()
{
	if(pClientApp_->entityID() == this->getID())
		return;

	EventData_PositionChanged eventdata;
	eventdata.x = position_.x;
	eventdata.y = position_.y;
	eventdata.z = position_.z;
	eventdata.speed = velocity_;
	
	eventdata.entityID = getID();

	pClientApp_->fireEvent(&eventdata);
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
	dir.roll(float(PyFloat_AsDouble(pyItem)));
	Py_DECREF(pyItem);
	pyItem = PySequence_GetItem(value, 1);
	dir.pitch(float(PyFloat_AsDouble(pyItem)));
	Py_DECREF(pyItem);
	pyItem = PySequence_GetItem(value, 2);
	dir.yaw(float(PyFloat_AsDouble(pyItem)));
	Py_DECREF(pyItem);

	onDirectionChanged();
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetDirection()
{
	return new script::ScriptVector3(&getDirection().dir, NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onDirectionChanged()
{
	if(pClientApp_->entityID() == this->getID())
		return;

	EventData_DirectionChanged eventdata;
	eventdata.yaw = direction_.yaw();
	eventdata.pitch = direction_.pitch();
	eventdata.roll = direction_.roll();
	eventdata.entityID = getID();

	pClientApp_->fireEvent(&eventdata);
}

//-------------------------------------------------------------------------------------
int Entity::pySetMoveSpeed(PyObject *value)
{
	setMoveSpeed((float)PyFloat_AsDouble(value));
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetMoveSpeed()
{
	return PyFloat_FromDouble(velocity_);
}

//-------------------------------------------------------------------------------------
void Entity::onMoveSpeedChanged()
{
	EventData_MoveSpeedChanged eventdata;
	eventdata.speed = velocity_;
	eventdata.entityID = getID();

	pClientApp_->fireEvent(&eventdata);
}

//-------------------------------------------------------------------------------------
void Entity::onEnterWorld()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	enterword_ = true;
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEnterWorld"));
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveWorld()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	enterword_ = false;
	setSpaceID(0);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLeaveWorld"));
}

//-------------------------------------------------------------------------------------
void Entity::onEnterSpace()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEnterSpace"));
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveSpace()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	setSpaceID(0);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLeaveSpace"));
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs)
{
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::addCellDataToStream(uint32 flags, MemoryStream* mstream, bool useAliasID)
{
}

//-------------------------------------------------------------------------------------
void Entity::onBecomePlayer()
{
	std::string moduleName = "Player";
	moduleName += this->scriptModule_->getName();

	PyObject* pyModule = 
		PyImport_ImportModule(const_cast<char*>(this->scriptModule_->getName()));

	if(pyModule == NULL)
	{
		SCRIPT_ERROR_CHECK();
	}
	else
	{
		PyObject* pyClass = 
			PyObject_GetAttrString(pyModule, const_cast<char *>(moduleName.c_str()));

		if(pyClass == NULL)
		{
			SCRIPT_ERROR_CHECK();
			ERROR_MSG(boost::format("%1%::onBecomePlayer(): please implement %2%.\n") % this->scriptModule_->getName() % moduleName);
		}
		else
		{
			PyObject_SetAttrString(static_cast<PyObject*>(this), "__class__", pyClass);
			SCRIPT_ERROR_CHECK();
		}

		S_RELEASE(pyModule);
	}

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onBecomePlayer"));
}

//-------------------------------------------------------------------------------------
void Entity::onBecomeNonPlayer()
{
	PyObject_SetAttrString(static_cast<PyObject*>(this), "__class__", (PyObject*)this->scriptModule_->getScriptType());
	SCRIPT_ERROR_CHECK();

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onBecomeNonPlayer"));
}

//-------------------------------------------------------------------------------------
}
}


