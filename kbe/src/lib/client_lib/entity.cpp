// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "clientapp.h"
#include "entity.h"
#include "config.h"
#include "clientobjectbase.h"
#include "moveto_point_handler.h"	
#include "entitydef/entity_call.h"
#include "entitydef/entity_component.h"
#include "network/channel.h"	
#include "network/bundle.h"	
#include "network/fixed_messages.h"
#include "pyscript/py_gc.h"

#include "../../../server/baseapp/baseapp_interface.h"
#include "../../../server/cellapp/cellapp_interface.h"

#ifndef CODE_INLINE
#include "entity.inl"
#endif

namespace KBEngine{
namespace client
{

//-------------------------------------------------------------------------------------
CLIENT_ENTITY_METHOD_DECLARE_BEGIN(ClientApp, Entity)
SCRIPT_METHOD_DECLARE("moveToPoint",				pyMoveToPoint,					METH_VARARGS,					0)
SCRIPT_METHOD_DECLARE("cancelController",			pyCancelController,				METH_VARARGS,					0)
SCRIPT_METHOD_DECLARE("isPlayer",					pyIsPlayer,						METH_VARARGS,					0)
SCRIPT_METHOD_DECLARE("addTimer",					pyAddTimer,						METH_VARARGS,					0)	
SCRIPT_METHOD_DECLARE("delTimer",					pyDelTimer,						METH_VARARGS,					0)	
SCRIPT_METHOD_DECLARE("getComponent",				pyGetComponent,					METH_VARARGS | METH_KEYWORDS,	0)
CLIENT_ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

CLIENT_ENTITY_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GET_DECLARE("base",							pyGetBaseEntityCall,			0,					0)
SCRIPT_GET_DECLARE("cell",							pyGetCellEntityCall,			0,					0)
SCRIPT_GET_DECLARE("clientapp",						pyGetClientApp,					0,					0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,		0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,		0,		0)
SCRIPT_GETSET_DECLARE("velocity",					pyGetMoveSpeed,					pySetMoveSpeed,		0,		0)
CLIENT_ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, const ScriptDefModule* pScriptModule, EntityCall* base, EntityCall* cell):
ScriptObject(getScriptType(), true),
ENTITY_CONSTRUCTION(Entity),
cellEntityCall_(cell),
baseEntityCall_(base),
position_(),
serverPosition_(),
direction_(),
clientPos_(FLT_MAX, FLT_MAX, FLT_MAX),
clientDir_(FLT_MAX, FLT_MAX, FLT_MAX),
pClientApp_(NULL),
aspect_(id),
velocity_(3.0f),
enterworld_(false),
isOnGround_(true),
pMoveHandlerID_(0),
inited_(false),
isControlled_(false)
{
	ENTITY_INIT_PROPERTYS(Entity);
	script::PyGC::incTracing("Entity");
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	stopMove();

	enterworld_ = false;
	ENTITY_DECONSTRUCTION(Entity);
	S_RELEASE(cellEntityCall_);
	S_RELEASE(baseEntityCall_);

	script::PyGC::decTracing("Entity");
	
	if(pClientApp_->pEntities())
		pClientApp_->pEntities()->pGetbages()->erase(id());

	Py_DECREF(pClientApp_);
}	

//-------------------------------------------------------------------------------------
void Entity::pClientApp(ClientObjectBase* p)
{ 
	if(p)
		Py_INCREF(p);
	else
		Py_DECREF(pClientApp_);

	pClientApp_ = p; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetBaseEntityCall()
{ 
	EntityCall* entityCall = baseEntityCall();
	if(entityCall == NULL)
		S_Return;

	Py_INCREF(entityCall);
	return entityCall; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetCellEntityCall()
{ 
	EntityCall* entityCall = cellEntityCall();
	if(entityCall == NULL)
		S_Return;

	Py_INCREF(entityCall);
	return entityCall; 
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
void Entity::onInitializeScript()
{

}

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(EntityComponent* pEntityComponent, const PropertyDescription* propertyDescription, PyObject* pyData)
{
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_METHOD_UID utype = 0;
	
	MethodDescription* pMethodDescription = NULL;
	ScriptDefModule* pScriptModule = pScriptModule_;
	PropertyDescription* pComponentPropertyDescription = NULL;

	if (pScriptModule->usePropertyDescrAlias())
	{
		uint8 componentPropertyAliasID;
		s >> componentPropertyAliasID;

		if (componentPropertyAliasID > 0)
		{
			pComponentPropertyDescription = pScriptModule_->findAliasPropertyDescription(componentPropertyAliasID);
		}
	}
	else
	{
		ENTITY_PROPERTY_UID componentPropertyUID = 0;
		if (componentPropertyUID > 0)
		{
			pComponentPropertyDescription = pScriptModule_->findClientPropertyDescription(componentPropertyUID);
		}
	}

	PyObject* pyCallObject = this;

	if (pComponentPropertyDescription)
	{
		DataType* pDataType = pComponentPropertyDescription->getDataType();
		KBE_ASSERT(pDataType->type() == DATA_TYPE_ENTITY_COMPONENT);

		pScriptModule = static_cast<EntityComponentType*>(pDataType)->pScriptDefModule();

		pyCallObject = PyObject_GetAttrString(this, const_cast<char*>
			(pComponentPropertyDescription->getName()));
	}

	if(pScriptModule->useMethodDescrAlias())
	{
		ENTITY_DEF_ALIASID aliasID;
		s >> aliasID;
		pMethodDescription = pScriptModule->findAliasMethodDescription(aliasID);
		utype = aliasID;
	}
	else
	{
		s >> utype;
		pMethodDescription = pScriptModule->findClientMethodDescription(utype);
	}

	if(pMethodDescription == NULL)
	{
		ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: can't found method. utype={0}, methodName=unknown, callerID:{1}.\n", 
			utype, id_, this->scriptName()));

		if (pyCallObject != static_cast<PyObject*>(this))
			Py_DECREF(pyCallObject);

		return;
	}

	if(g_debugEntity)
	{
		DEBUG_MSG(fmt::format("{3}::onRemoteMethodCall: {0}, {3}::{4}{1}(utype={2}).\n", 
			id_, (pMethodDescription ? pMethodDescription->getName() : "unknown"), utype, this->scriptName(),
			(pComponentPropertyDescription ? (std::string(pScriptModule->getName()) + "::") : "")));
	}

	PyObject* pyFunc = PyObject_GetAttrString(pyCallObject, const_cast<char*>
						(pMethodDescription->getName()));

	if(pMethodDescription != NULL)
	{
		if(pMethodDescription->getArgSize() == 0)
		{
			pMethodDescription->call(pyFunc, NULL);
		}
		else
		{
			PyObject* pyargs = pMethodDescription->createFromStream(&s);
			if(pyargs)
			{
				pMethodDescription->call(pyFunc, pyargs);
				Py_DECREF(pyargs);
			}
			else
			{
				SCRIPT_ERROR_CHECK();
			}
		}
	}
	
	Py_XDECREF(pyFunc);

	if (pyCallObject != static_cast<PyObject*>(this))
		Py_DECREF(pyCallObject);

	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Entity::onUpdatePropertys(MemoryStream& s)
{
	// 由于在属性更新过程中可能产生新的组件属性，需要在此设置
	EntityDef::context().currClientappID = pClientApp_->appID();
	EntityDef::context().currEntityID = id();
	EntityDef::context().currComponentType = CLIENT_TYPE;

	ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
	ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
	ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

	if(!pScriptModule_->usePropertyDescrAlias())
	{
		Network::FixedMessages::MSGInfo* msgInfo =
					Network::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;

		msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)
			diruid = msgInfo->msgid;

		msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::spaceID");
		if(msgInfo != NULL)
			spaceuid = msgInfo->msgid;
	}
	else
	{
		posuid = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;
		diruid = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;
		spaceuid = ENTITY_BASE_PROPERTY_ALIASID_SPACEID;
	}

	while(s.length() > 0)
	{
		ENTITY_PROPERTY_UID uid;
		ENTITY_PROPERTY_UID child_uid;

		uint8 aliasID = 0;
		uint8 child_aliasID = 0;
		PropertyDescription* pPropertyDescription = NULL;
		PyObject* setToObj = this;

		if(pScriptModule_->usePropertyDescrAlias())
		{
			// 父属性ID
			s >> aliasID;
			s >> child_aliasID;
			uid = aliasID;
			child_uid = child_aliasID;
		}
		else
		{
			// 父属性ID
			s >> uid;
			s >> child_uid;
		}

		// 如果是位置或者朝向信息则
		if (uid == 0)
		{
			if (child_uid == posuid)
			{
				Position3D pos;

#ifdef CLIENT_NO_FLOAT		
				int32 x, y, z;
				s >> x >> y >> z;

				pos.x = (float)x;
				pos.y = (float)y;
				pos.z = (float)z;
#else
				s >> pos.x >> pos.y >> pos.z;
#endif
				position(pos);
				clientPos(pos);
				continue;
			}
			else if (child_uid == diruid)
			{
				Direction3D dir;

#ifdef CLIENT_NO_FLOAT		
				int32 x, y, z;
				s >> x >> y >> z;

				dir.roll((float)x);
				dir.pitch((float)y);
				dir.yaw((float)z);
#else
				float yaw, pitch, roll;
				s >> roll >> pitch >> yaw;
				dir.yaw(yaw);
				dir.pitch(pitch);
				dir.roll(roll);
#endif

				direction(dir);
				clientDir(dir);
				continue;
			}
			else if (child_uid == spaceuid)
			{
				SPACE_ID ispaceID;
				s >> ispaceID;
				spaceID(ispaceID);
				continue;
			}

			if (pScriptModule_->usePropertyDescrAlias())
				pPropertyDescription = pScriptModule()->findAliasPropertyDescription(child_aliasID);
			else
				pPropertyDescription = pScriptModule()->findClientPropertyDescription(child_uid);
		}
		else
		{
			// 先得到父属性找到属性名称
			if (pScriptModule_->usePropertyDescrAlias())
				pPropertyDescription = pScriptModule()->findAliasPropertyDescription(aliasID);
			else
				pPropertyDescription = pScriptModule()->findClientPropertyDescription(uid);

			if (pPropertyDescription == NULL)
			{
				ERROR_MSG(fmt::format("{}::onUpdatePropertys: not found EntityComponentProperty(uid={}, aliasID={})!\n", pScriptModule_->getName(), uid, aliasID));
				return;
			}

			// 然后得到组件属性，再从其中找到子属性
			EntityComponent* pEntityComponent = static_cast<EntityComponent*>(PyObject_GetAttrString(this, pPropertyDescription->getName()));
			if (!pEntityComponent)
			{
				SCRIPT_ERROR_CHECK();
			}
			else
			{
				pPropertyDescription = pEntityComponent->getProperty(child_uid);
				setToObj = pEntityComponent;
			}
		}

		if(pPropertyDescription == NULL)
		{
			ERROR_MSG(fmt::format("Entity::onUpdatePropertys: not found uid={}, aliasID={}, child_uid={}\n", uid, aliasID, child_uid));

			if (setToObj != static_cast<PyObject*>(this))
				Py_DECREF(setToObj);

			return;
		}

		PyObject* pyobj = pPropertyDescription->createFromStream(&s);

		PyObject* pyOld = PyObject_GetAttrString(setToObj, pPropertyDescription->getName());
		if (!pyOld)
		{
			SCRIPT_ERROR_CHECK();
			pyOld = Py_None;
			Py_INCREF(pyOld);
		}

		PyObject_SetAttrString(setToObj, pPropertyDescription->getName(), pyobj);

		bool willCallScript = pPropertyDescription->hasBase() ? inited_ : enterworld_;
		if (willCallScript)
		{
			std::string setname = "set_";
			setname += pPropertyDescription->getName();

			SCRIPT_OBJECT_CALL_ARGS1(setToObj, const_cast<char*>(setname.c_str()),
				const_cast<char*>("O"), pyOld, false);
		}

		Py_DECREF(pyobj);
		Py_DECREF(pyOld);

		if(setToObj != static_cast<PyObject*>(this))
			Py_DECREF(setToObj);

		SCRIPT_ERROR_CHECK();
	}
}

//-------------------------------------------------------------------------------------
void Entity::writeToDB(void* data, void* extra1, void* extra2)
{
}

//-------------------------------------------------------------------------------------
int Entity::pySetPosition(PyObject *value)
{
	if(!script::ScriptVector3::check(value))
		return -1;

	script::ScriptVector3::convertPyObjectToVector3(position(), value);
	onPositionChanged();
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetPosition()
{
	return new script::ScriptVector3(&position(), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onPositionChanged()
{
	if(isPlayer())
		return;

	EventData_PositionChanged eventdata;
	eventdata.x = position_.x;
	eventdata.y = position_.y;
	eventdata.z = position_.z;
	eventdata.speed = velocity_;
	
	eventdata.entityID = id();

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

	Direction3D& dir = direction();
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
	return new script::ScriptVector3(&direction().dir, NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onDirectionChanged()
{
	if(isPlayer())
		return;

	EventData_DirectionChanged eventdata;
	eventdata.yaw = direction_.yaw();
	eventdata.pitch = direction_.pitch();
	eventdata.roll = direction_.roll();
	eventdata.entityID = id();

	pClientApp_->fireEvent(&eventdata);
}

//-------------------------------------------------------------------------------------
int Entity::pySetMoveSpeed(PyObject *value)
{
	moveSpeed((float)PyFloat_AsDouble(value));
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
	eventdata.entityID = id();

	pClientApp_->fireEvent(&eventdata);
}

//-------------------------------------------------------------------------------------
void Entity::onEnterWorld()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	enterworld_ = true;

	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onEnterWorld"), GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveWorld()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	enterworld_ = false;
	spaceID(0);

	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onLeaveWorld"), GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::onEnterSpace()
{
	this->stopMove();
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onEnterSpace"), GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveSpace()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	spaceID(0);

	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onLeaveSpace"), GETERR));

	this->stopMove();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs)
{
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::addCellDataToStream(COMPONENT_TYPE sendTo, uint32 flags, MemoryStream* mstream, bool useAliasID)
{
}

//-------------------------------------------------------------------------------------
void Entity::onBecomePlayer()
{
	std::string moduleName = "Player";
	moduleName += this->pScriptModule_->getName();

	PyObject* pyModule = 
		PyImport_ImportModule(const_cast<char*>(this->pScriptModule_->getName()));

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
			// 不在强制需要实现Player**类
			PyErr_Clear();
		}
		else
		{
			PyObject_SetAttrString(static_cast<PyObject*>(this), "__class__", pyClass);
			SCRIPT_ERROR_CHECK();
		}

		S_RELEASE(pyModule);
	}

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onBecomePlayer"), GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::onBecomeNonPlayer()
{
	if(!enterworld_)
		return;
	
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onBecomeNonPlayer"), GETERR));

	PyObject_SetAttrString(static_cast<PyObject*>(this), "__class__", (PyObject*)this->pScriptModule_->getScriptType());
	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
bool Entity::stopMove()
{
	if(pMoveHandlerID_ > 0)
	{
		pClientApp_->scriptCallbacks().delCallback(pMoveHandlerID_);
		pMoveHandlerID_ = 0;
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
uint32 Entity::moveToPoint(const Position3D& destination, float velocity, float distance, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
	stopMove();

	int hertz = 0;
	if(g_componentType == BOTS_TYPE)
		hertz = g_kbeSrvConfig.gameUpdateHertz();
	else
		hertz = Config::getSingleton().gameUpdateHertz();

	velocity = velocity / hertz;

	pMoveHandlerID_ = pClientApp_->scriptCallbacks().addCallback(0.0f, 0.1f, new MoveToPointHandler(pClientApp_->scriptCallbacks(), this, 0, destination, velocity, 
		distance, faceMovement, moveVertically, userData));

	return pMoveHandlerID_;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToPoint(PyObject_ptr pyDestination, float velocity, float distance, PyObject_ptr userData,
								 int32 faceMovement, int32 moveVertically)
{
	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::moveToPoint: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	Position3D destination;

	if(!PySequence_Check(pyDestination))
	{
		PyErr_Format(PyExc_TypeError, "%s::moveToPoint: args1(position) not is PySequence!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyDestination) != 3)
	{
		PyErr_Format(PyExc_TypeError, "%s::moveToPoint: args1(position) invalid!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	// 将坐标信息提取出来
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	Py_INCREF(userData);

	return PyLong_FromLong(moveToPoint(destination, velocity, distance, userData, faceMovement > 0, moveVertically > 0));
}

//-------------------------------------------------------------------------------------
void Entity::onMove(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	AUTO_SCOPED_PROFILE("onMove");
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS2(pyTempObj, const_cast<char*>("onMove"),
		const_cast<char*>("IO"), controllerId, userarg, GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::onMoveOver(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	stopMove();

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS2(pyTempObj, const_cast<char*>("onMoveOver"),
		const_cast<char*>("IO"), controllerId, userarg, GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::onMoveFailure(uint32 controllerId, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	stopMove();

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS2(pyTempObj, const_cast<char*>("onMoveFailure"),
		const_cast<char*>("IO"), controllerId, userarg, GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::cancelController(uint32 id)
{
	if(this->isDestroyed())
	{
		return;
	}

	// 暂时只有回调, 主要是因为用在了移动中，当前可能不是非常合适
	if(id == (uint32)pMoveHandlerID_)
		this->stopMove();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyCancelController(PyObject* self, PyObject* args)
{
	uint16 currargsSize = (uint16)PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);

	uint32 id = 0;
	PyObject* pyargobj = NULL;

	if(currargsSize != 1)
	{
		PyErr_Format(PyExc_AssertionError, "%s::cancel: args require 1 args(controllerID|int or \"Movement\"|str), gived %d! is script[%s].\n",								
			pobj->scriptName(), currargsSize);														
																																
		PyErr_PrintEx(0);																										
		return 0;																								
	}

	if(!PyArg_ParseTuple(args, "O", &pyargobj))
	{
		PyErr_Format(PyExc_TypeError, "%s::cancel: args(controllerID|int or \"Movement\"|str) error!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}
	
	if(pyargobj == NULL)
	{
		PyErr_Format(PyExc_TypeError, "%s::cancel: args(controllerID|int or \"Movement\"|str) error!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(PyUnicode_Check(pyargobj))
	{
		if (strcmp(PyUnicode_AsUTF8AndSize(pyargobj, NULL), "Movement") == 0)
		{
			pobj->stopMove();
		}
		else
		{
			PyErr_Format(PyExc_TypeError, "%s::cancel: args not is \"Movement\"!", pobj->scriptName());
			PyErr_PrintEx(0);
			return 0;
		}

		S_Return;
	}
	else
	{
		if(!PyLong_Check(pyargobj))
		{
			PyErr_Format(PyExc_TypeError, "%s::cancel: args(controllerID|int) error!", pobj->scriptName());
			PyErr_PrintEx(0);
			return 0;
		}

		id = PyLong_AsLong(pyargobj);
	}

	pobj->cancelController(id);
	S_Return;
}

//-------------------------------------------------------------------------------------
bool Entity::isPlayer()
{
	return id() == pClientApp_->entityID();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyIsPlayer()
{
	if (isPlayer())
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
void Entity::callPropertysSetMethods()
{
	ScriptDefModule::PROPERTYDESCRIPTION_MAP &clientProperties = pScriptModule_->getClientPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::iterator iter = clientProperties.begin();
	for (; iter != clientProperties.end(); ++iter)
	{
		bool willCallScript = false;
		PyObject* pyOld = PyObject_GetAttrString(this, iter->first.c_str());

		if (iter->second->hasBase())
		{
			if (inited_ && !enterworld_)
			{
				willCallScript = true;
			}
		}
		else
		{
			if (enterworld_)
			{
				willCallScript = true;
			}
		}

		if (willCallScript)
		{
			std::string setname = "set_";
			setname += iter->second->getName();

			SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>(setname.c_str()),
				const_cast<char*>("O"), pyOld, false);
		}

		Py_DECREF(pyOld);
		SCRIPT_ERROR_CHECK();
	}
}

//-------------------------------------------------------------------------------------
void Entity::onTimer(ScriptID timerID, int useraAgs)
{
	SCOPED_PROFILE(ONTIMER_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS2(pyTempObj, const_cast<char*>("onTimer"),
		const_cast<char*>("Ii"), timerID, useraAgs, GETERR));
}

//-------------------------------------------------------------------------------------
void Entity::onControlled(bool p_controlled)
{
    isControlled_ = p_controlled;

    PyObject *pyval = p_controlled ? Py_True : Py_False;

	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS1(pyTempObj, const_cast<char*>("onControlled"),
		const_cast<char*>("O"), pyval, GETERR));
}

//-------------------------------------------------------------------------------------
}
}


