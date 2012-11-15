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


#include "cellapp.hpp"
#include "entity.hpp"
#include "witness.hpp"	
#include "space.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"	
#include "network/fixed_messages.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"

#ifdef CODE_INLINE
//#include "entity.ipp"
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
ENTITY_METHOD_DECLARE_BEGIN(Cellapp, Entity)
SCRIPT_METHOD_DECLARE("addSpaceGeometryMapping",	pyAddSpaceGeometryMapping,		METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("setAoiRadius",				pySetAoiRadius,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("isReal",						pyIsReal,						METH_VARARGS,				0)	
SCRIPT_METHOD_DECLARE("addProximity",				pyAddProximity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("delProximity",				pyDelProximity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("navigateStep",				pyNavigateStep,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("moveToPoint",				pyMoveToPoint,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("stopMove",					pyStopMove,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("entitiesInRange",			pyEntitiesInRange,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("teleport",					pyTeleport,						METH_VARARGS,				0)
ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

ENTITY_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GET_DECLARE("base",							pyGetBaseMailbox,				0,					0)
SCRIPT_GET_DECLARE("client",						pyGetClientMailbox,				0,					0)
SCRIPT_GET_DECLARE("isWitnessed",					pyIsWitnessed,					0,					0)
SCRIPT_GET_DECLARE("hasWitness",					pyHasWitness,					0,					0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,		0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,		0,		0)
SCRIPT_GETSET_DECLARE("topSpeed",					pyGetTopSpeed,					pySetTopSpeed,		0,		0)
SCRIPT_GETSET_DECLARE("topSpeedY",					pyGetTopSpeedY,					pySetTopSpeedY,		0,		0)
ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, const ScriptDefModule* scriptModule):
ScriptObject(getScriptType(), true),
ENTITY_CONSTRUCTION(Entity),
clientMailbox_(NULL),
baseMailbox_(NULL),
isReal_(true),
aoiRadius_(0.0f),
aoiHysteresisArea_(0.0f),
isWitnessed_(false),
topSpeed_(-0.1f),
topSpeedY_(-0.1f),
pWitness_(NULL)
{
	ENTITY_INIT_PROPERTYS(Entity);
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	ENTITY_DECONSTRUCTION(Entity);
	S_RELEASE(clientMailbox_);
	S_RELEASE(baseMailbox_);
	Witness::ObjPool().reclaimObject(pWitness_);
}	

#define CALL_ENTITY_METHOD
//-------------------------------------------------------------------------------------
void Entity::onDestroy(void)
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onDestroy"));

	if(baseMailbox_ != NULL)
	{
		this->backupCellData();

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onLoseCell);
		(*pBundle) << id_;
		baseMailbox_->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}

	// 将entity从场景中剔除
	Space* space = Spaces::findSpace(this->getSpaceID());
	if(space)
	{
		space->removeEntity(this);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDestroyEntity()																		
{																										
	destroyEntity();																					
	S_Return;																							
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
PyObject* Entity::pyGetClientMailbox()
{ 
	EntityMailbox* mailbox = getClientMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
int Entity::pySetTopSpeedY(PyObject *value)
{
	PyObject* pyItem = PySequence_GetItem(value, 0);
	setTopSpeedY(float(PyFloat_AsDouble(pyItem))); 
	Py_DECREF(pyItem);
	return 0; 
};

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeedY()
{ 
	return PyFloat_FromDouble(getTopSpeedY()); 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeed()
{ 
	return PyFloat_FromDouble(getTopSpeed()); 
}

//-------------------------------------------------------------------------------------
int Entity::pySetTopSpeed(PyObject *value)
{ 
	PyObject* pyItem = PySequence_GetItem(value, 0);
	setTopSpeed(float(PyFloat_AsDouble(pyItem))); 
	Py_DECREF(pyItem);
	return 0; 
}

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(const PropertyDescription* propertyDescription, PyObject* pyData)
{
	// 如果不是一个realEntity则不理会
	if(!isReal() && initing_)
		return;

	const uint32& flags = propertyDescription->getFlags();
	ENTITY_PROPERTY_UID utype = propertyDescription->getUType();

	// 首先创建一个需要广播的模板流
	MemoryStream* mstream = MemoryStream::ObjPool().createObject();
	(*mstream) << utype;
	propertyDescription->getDataType()->addToStream(mstream, pyData);

	// 判断是否需要广播给其他的cellapp, 这个还需一个前提是entity必须拥有ghost实体
	// 只有在cell边界一定范围内的entity才拥有ghost实体
	if((flags & ENTITY_BROADCAST_CELL_FLAGS) > 0)
	{
	}
	
	/*
	// 判断这个属性是否还需要广播给其他客户端
	if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) > 0)
	{
		int8 detailLevel = propertyDescription->getDetailLevel();
		for(int8 i=DETAIL_LEVEL_NEAR; i<=detailLevel; i++)
		{
			std::map<ENTITY_ID, Entity*>::iterator iter = witnessEntities_[i].begin();
			for(; iter != witnessEntities_[i].end(); iter++)
			{
				Entity* entity = iter->second;
				EntityMailbox* clientMailbox = entity->getClientMailbox();
				if(clientMailbox != NULL)
				{
					Packet* sp = clientMailbox->createMail(MAIL_TYPE_UPDATE_PROPERTY);
					(*sp) << id_;
					sp->append(mstream->contents(), mstream->size());
					clientMailbox->post(sp);
				}
			}
		}

		// 这个属性已经更新过， 将这些信息添加到曾经进入过这个级别的entity， 但现在可能走远了一点， 在他回来重新进入这个detaillevel
		// 时如果重新将所有的属性都更新到他的客户端可能不合适， 我们记录这个属性的改变， 下次他重新进入我们只需要将所有期间有过改变的
		// 数据发送到他的客户端更新
		for(int8 i=detailLevel; i<=DETAIL_LEVEL_FAR; i++)
		{
			std::map<ENTITY_ID, Entity*>::iterator iter = witnessEntities_[i].begin();
			for(; iter != witnessEntities_[i].end(); iter++)
			{
				Entity* entity = iter->second;
				EntityMailbox* clientMailbox = entity->getClientMailbox();
				if(clientMailbox != NULL)
				{
					WitnessInfo* witnessInfo = witnessEntityDetailLevelMap_.find(iter->first)->second;
					if(witnessInfo->detailLevelLog[detailLevel])
					{
						std::vector<uint32>& cddlog = witnessInfo->changeDefDataLogs[detailLevel];
						std::vector<uint32>::iterator fiter = std::find(cddlog.begin(), cddlog.end(), utype);
						if(fiter == cddlog.end())
							witnessInfo->changeDefDataLogs[detailLevel].push_back(utype);
					}
				}
			}
		}
	}
	*/

	// 判断这个属性是否还需要广播给自己的客户端
	if((flags & ENTITY_BROADCAST_OWN_CLIENT_FLAGS) > 0 && clientMailbox_ != NULL)
	{
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
		(*pForwardBundle).newMessage(ClientInterface::onUpdatePropertys);
		(*pForwardBundle) << getID();
		pForwardBundle->append(*mstream);
		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(this->getID(), (*pSendBundle), (*pForwardBundle));
		clientMailbox_->postMail(*pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	}

	MemoryStream::ObjPool().reclaimObject(mstream);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
	if(isDestroyed())																				
	{																										
		ERROR_MSG("%s::onRemoteMethodCall: %d is destroyed!\n",											
			getScriptName(), getID());
		s.read_skip(s.opsize());
		return;																							
	}

	ENTITY_METHOD_UID utype = 0;
	s >> utype;
	
	DEBUG_MSG("Entity::onRemoteMethodCall: entityID %d, methodType %d.\n", 
				id_, utype);
	
	MethodDescription* md = scriptModule_->findCellMethodDescription(utype);
	
	if(md == NULL)
	{
		ERROR_MSG("Entity::onRemoteMethodCall: can't found method. utype=%u, callerID:%d.\n", utype, id_);
		return;
	}

	md->currCallerID(this->getID());

	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>
						(md->getName()));

	if(md != NULL)
	{
		PyObject* pyargs = md->createFromStream(&s);
		if(pyargs)
		{
			md->call(pyFunc, pyargs);
			Py_XDECREF(pyargs);
		}
		else
		{
			SCRIPT_ERROR_CHECK();
		}
	}
	
	Py_XDECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
void Entity::backupCellData()
{
	if(baseMailbox_ != NULL)
	{
		PyObject* cellData = addCellDataToStream(ENTITY_CELL_DATA_FLAGS);
		// 将entity位置和方向变量也设置进去
		PyObject* pyPosition = PyTuple_New(3);
		PyTuple_SET_ITEM(pyPosition, 0, PyFloat_FromDouble(position_.x));
		PyTuple_SET_ITEM(pyPosition, 1, PyFloat_FromDouble(position_.y));
		PyTuple_SET_ITEM(pyPosition, 2, PyFloat_FromDouble(position_.z));
		
		PyObject* pyDirection = PyTuple_New(3);
		PyTuple_SET_ITEM(pyDirection, 0, PyFloat_FromDouble(direction_.roll));
		PyTuple_SET_ITEM(pyDirection, 1, PyFloat_FromDouble(direction_.pitch));
		PyTuple_SET_ITEM(pyDirection, 2, PyFloat_FromDouble(direction_.yaw));
		
		PyDict_SetItemString(cellData, const_cast<char*>("position"), pyPosition);
		PyDict_SetItemString(cellData, const_cast<char*>("direction"), pyDirection);

		Py_DECREF(pyPosition);
		Py_DECREF(pyDirection);

		std::string strCellData = script::Pickler::pickle(cellData);
		ArraySize cellDataLength = strCellData.length();
		Py_DECREF(cellData);
	
		// 将当前的cell部分数据打包 一起发送给base部分备份
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onBackupEntityCellData);
		(*pBundle) << id_;
		(*pBundle) << cellDataLength;
		if(cellDataLength > 0)
			(*pBundle).append(strCellData.c_str(), cellDataLength);
			
		baseMailbox_->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
	else
	{
		WARNING_MSG("Entity::backupCellData(): %s %i has no base!\n", this->getScriptName(), this->getID());
	}
}

//-------------------------------------------------------------------------------------
void Entity::writeToDB(void* data)
{
	CALLBACK_ID* pCallbackID = static_cast<CALLBACK_ID*>(data);
	CALLBACK_ID callbackID = 0;

	if(pCallbackID)
		callbackID = *pCallbackID;

	onWriteToDB();
	backupCellData();

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onCellWriteToDBCompleted);
	(*pBundle) << this->getID();
	(*pBundle) << callbackID;
	if(this->getBaseMailbox())
	{
		this->getBaseMailbox()->postMail((*pBundle));
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Entity::onWriteToDB()
{
	DEBUG_MSG("%s::onWriteToDB(): %d.\n", this->getScriptName(), this->getID());

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onWriteToDB"));
}

//-------------------------------------------------------------------------------------
/*
void Entity::onCurrentChunkChanged(Chunk* oldChunk)
{
}
*/
//-------------------------------------------------------------------------------------
PyObject* Entity::pyIsReal()
{
	return PyBool_FromLong(isReal());
}

//-------------------------------------------------------------------------------------
void Entity::onWitnessed(Entity* entity, float range)
{/*
	int8 detailLevel = scriptModule_->getDetailLevel().getLevelByRange(range);
	WitnessInfo* info = new WitnessInfo(detailLevel, entity, range);
	ENTITY_ID id = entity->getID();

	DEBUG_MSG("Entity[%s:%ld]::onWitnessed:%s %ld enter detailLevel %d. range=%f.\n", getScriptName(), id_, 
			entity->getScriptName(), id, detailLevel, range);

#ifdef _DEBUG
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = witnessEntityDetailLevelMap_.find(id);
	if(iter != witnessEntityDetailLevelMap_.end())
		ERROR_MSG("Entity::onWitnessed: %s %ld is exist.\n", entity->getScriptName(), id);
#endif
	
	witnessEntityDetailLevelMap_[id] = info;
	witnessEntities_[detailLevel][id] = entity;
	onEntityInitDetailLevel(entity, detailLevel);
	
	if(!isWitnessed_)
	{
		isWitnessed_ = true; 
		SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onWitnessed"), const_cast<char*>("O"), PyBool_FromLong(1));
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::onRemoveWitness(Entity* entity)
{
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyIsWitnessed()
{
	return PyBool_FromLong(isWitnessed());
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyHasWitness()
{
	return PyBool_FromLong(hasWitness());
}

//-------------------------------------------------------------------------------------
uint16 Entity::addProximity(float range)
{
	if(range <= 0.0f)
	{
		ERROR_MSG("Entity::addProximity: range(%f) <= 0.0f! entity[%s:%ld]\n", range, getScriptName(), getID());
		return 0;
	}

	// 不允许范围大于cell边界
	if(range > CELL_BORDER_WIDTH)
		range = CELL_BORDER_WIDTH;
	/*
	// 在space中投放一个陷阱
	Proximity* p = new Proximity(this, range, 0.0f);
	trapMgr_.addProximity(p);
	if(currChunk_ != NULL)
		currChunk_->getSpace()->placeProximity(currChunk_, p);
	return p->getID();*/return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddProximity(float range)
{
	return PyLong_FromLong(addProximity(range));
}

//-------------------------------------------------------------------------------------
void Entity::delProximity(uint16 id)
{
//	if(!trapMgr_.delProximity(id))
//		ERROR_MSG("Entity::delProximity: not found proximity %ld.\n", id);

	// 从chunk中清除这个陷阱
	//currChunk_->clearProximity(p);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDelProximity(uint16 id)
{
	delProximity(id);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::onEnterTrap(Entity* entity, float range, int controllerID)
{
	SCRIPT_OBJECT_CALL_ARGS3(this, const_cast<char*>("onEnterTrap"), 
		const_cast<char*>("Ofi"), entity, range, controllerID);
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrap(Entity* entity, float range, int controllerID)
{
	SCRIPT_OBJECT_CALL_ARGS3(this, const_cast<char*>("onLeaveTrap"), 
		const_cast<char*>("Ofi"), entity, range, controllerID);
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrapID(ENTITY_ID entityID, float range, int controllerID)
{
	SCRIPT_OBJECT_CALL_ARGS3(this, const_cast<char*>("onLeaveTrapID"), 
		const_cast<char*>("kfi"), entityID, range, controllerID);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddSpaceGeometryMapping(SPACE_ID spaceID, const_charptr path)
{
	//App::getSingleton().addSpaceGeometryMapping(spaceID, path);
	S_Return;
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
void Entity::setPosition_XZ_int(Mercury::Channel* pChannel, int32 x, int32 z)
{
	getPosition().x = float(x);
	getPosition().z = float(z);
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XYZ_int(Mercury::Channel* pChannel, int32 x, int32 y, int32 z)
{
	getPosition().x = float(x);
	getPosition().y = float(y);
	getPosition().z = float(z);
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XZ_float(Mercury::Channel* pChannel, float x, float z)
{
	getPosition().x = x;
	getPosition().z = z;
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XYZ_float(Mercury::Channel* pChannel, float x, float y, float z)
{
	getPosition().x = x;
	getPosition().y = y;
	getPosition().z = z;
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
	dir.roll	= float(PyFloat_AsDouble(PySequence_GetItem(value, 0)));
	Py_DECREF(pyItem);
	pyItem = PySequence_GetItem(value, 1);
	dir.pitch	= float(PyFloat_AsDouble(PySequence_GetItem(value, 1)));
	Py_DECREF(pyItem);
	pyItem = PySequence_GetItem(value, 2);
	dir.yaw		= float(PyFloat_AsDouble(PySequence_GetItem(value, 2)));
	Py_DECREF(pyItem);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetDirection()
{
	return new script::ScriptVector3(getDirection().asVector3());
}

//-------------------------------------------------------------------------------------
void Entity::setPosition(Position3D& pos)
{ 
	position_ = pos; 
//	if(currChunk_ != NULL)
//		currChunk_->getSpace()->onEntityPositionChanged(this, currChunk_, position_);
}

//-------------------------------------------------------------------------------------
void Entity::setPositionAndDirection(Position3D& position, Direction3D& direction)
{
	position_ = position;
	direction_ = direction;
//	if(currChunk_ != NULL)
//		currChunk_->getSpace()->onEntityPositionChanged(this, currChunk_, position_);
}

//-------------------------------------------------------------------------------------
void Entity::onGetWitness(Mercury::Channel* pChannel)
{
	KBE_ASSERT(this->getBaseMailbox() != NULL && !this->hasWitness());
	PyObject* clientMailbox = PyObject_GetAttrString(this->getBaseMailbox(), "client");
	KBE_ASSERT(clientMailbox != Py_None);

	EntityMailbox* client = static_cast<EntityMailbox*>(clientMailbox);	
	// Py_INCREF(clientMailbox); 这里不需要增加引用， 因为每次都会产生一个新的对象
	setClientMailbox(client);
	
	pWitness_ = Witness::ObjPool().createObject();

	Space* space = Spaces::findSpace(this->getSpaceID());
	if(space)
	{
		space->onEntityAttachWitness(this);
	}

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onGetWitness"));
}

//-------------------------------------------------------------------------------------
void Entity::onLoseWitness(Mercury::Channel* pChannel)
{
	KBE_ASSERT(this->getClientMailbox() != NULL && this->hasWitness());
	getClientMailbox()->addr(Mercury::Address::NONE);
	Py_DECREF(getClientMailbox());
	setClientMailbox(NULL);
	Witness::ObjPool().reclaimObject(pWitness_);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLoseWitness"));
}

//-------------------------------------------------------------------------------------
int32 Entity::setAoiRadius(float radius, float hyst)
{
	aoiRadius_ = radius;
	aoiHysteresisArea_ = hyst;

	if(aoiRadius_ + aoiHysteresisArea_ > CELL_BORDER_WIDTH)
	{
		aoiRadius_ = CELL_BORDER_WIDTH - 15.0f;
		aoiHysteresisArea_ = 15.0f;
	}
	
	return 1;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pySetAoiRadius(float radius, float hyst)
{
	return PyLong_FromLong(setAoiRadius(radius, hyst));
}

//-------------------------------------------------------------------------------------
bool Entity::navigateStep(const Position3D& destination, float velocity, float maxMoveDistance, float maxDistance, 
	bool faceMovement, float girth, PyObject* userData)
{
	DEBUG_MSG("Entity[%s:%d]:destination=(%f,%f,%f), velocity=%f, maxMoveDistance=%f, "
		"maxDistance=%f, faceMovement=%d, girth=%f, userData=%x.\n",

	getScriptName(), id_, destination.x, destination.y, destination.z, velocity, maxMoveDistance, 
	maxDistance, faceMovement, girth, userData);
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyNavigateStep(PyObject_ptr pyDestination, float velocity, float maxMoveDistance, float maxDistance,
								 int8 faceMovement, float girth, PyObject_ptr userData)
{
	Position3D destination;
	// 将坐标信息提取出来
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	
	Py_INCREF(userData);
	if(navigateStep(destination, velocity, maxMoveDistance, 
		maxDistance, faceMovement > 0, girth, userData))
	{
		Py_RETURN_TRUE;
	}
	
	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
bool Entity::moveToPoint(const Position3D& destination, float velocity, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
//	EntityMoveControllerMgr::addMovement(id_, new EntityMoveToPointController(this, destination, velocity, userData, faceMovement, moveVertically));
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToPoint(PyObject_ptr pyDestination, float velocity, PyObject_ptr userData,
								 int32 faceMovement, int32 moveVertically)
{
	Position3D destination;

	// 将坐标信息提取出来
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	Py_INCREF(userData);

	if(moveToPoint(destination, velocity, userData, faceMovement > 0, moveVertically > 0)){
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
bool Entity::stopMove()
{
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyStopMove()
{
	return PyBool_FromLong(stopMove());
}

//-------------------------------------------------------------------------------------
void Entity::onMove(PyObject* userData)
{
	SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onMove"), const_cast<char*>("O"), userData);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyEntitiesInRange(float radius, PyObject_ptr pyEntityType, PyObject_ptr pyPosition)
{
	/*
	std::string entityType = "";
	Position3D pos;
	
	// 将坐标信息提取出来
	script::ScriptVector3::convertPyObjectToVector3(pos, pyPosition);
	if(pyEntityType != Py_None)
	{
		entityType = PyString_AsString(pyEntityType);
	}
	
	int i = 0, entityUType = -1;
	std::vector<std::pair<Entity*, float>> viewEntities;
	Entity* entity = static_cast<Entity*>(self);

	if(entityType.size() > 0)
	{
		ScriptDefModule* sm = ExtendScriptModuleMgr::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG("Entity::entitiesInRange: args entityType[%s] not found.\n", entityType.c_str());
			return 0;
		}

		entityUType = sm->getUType();
	}

	// 查询所有范围内的entity
	Chunk* currChunk = entity->getAtChunk();
	if(currChunk != NULL && radius > 0.0f)
		currChunk->getSpace()->getRangeEntities(currChunk, pos, radius, &viewEntities, entityUType);
	
	// 将结果返回给脚本
	PyObject* pyList = PyList_New(viewEntities.size());
	std::vector<std::pair<Entity*, float>>::iterator iter = viewEntities.begin();
	while(iter != viewEntities.end())
	{
		Entity* e = iter->first;
		Py_INCREF(e);
		PyList_SET_ITEM(pyList, i++, e);
		iter++;
	}

	return pyList;*/return 0;
}

//-------------------------------------------------------------------------------------
void Entity::_sendBaseTeleportResult(ENTITY_ID sourceEntityID, COMPONENT_ID sourceBaseAppID, SPACE_ID spaceID, SPACE_ID lastSpaceID)
{
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(sourceBaseAppID);
	if(cinfos != NULL && cinfos->pChannel != NULL)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onTeleportCB);
		(*pBundle) << sourceEntityID;
		BaseappInterface::onTeleportCBArgs1::staticAddToBundle((*pBundle), spaceID);
		(*pBundle).send(Cellapp::getSingleton().getNetworkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Entity::teleportFromBaseapp(Mercury::Channel* pChannel, COMPONENT_ID cellAppID, ENTITY_ID targetEntityID, COMPONENT_ID sourceBaseAppID)
{
	DEBUG_MSG("%s::teleportFromBaseapp: %d, targetEntityID=%d, cell=%"PRAppID", sourceBaseAppID=%"PRAppID".\n", 
		this->getScriptName(), this->getID(), targetEntityID, cellAppID, sourceBaseAppID);
	
	SPACE_ID lastSpaceID = this->getSpaceID();

	// 如果不在一个cell上
	if(cellAppID != g_componentID)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cellAppID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG("%s::teleportFromBaseapp: %d, teleport is error, not found cellapp, targetEntityID, cellAppID=%"PRAppID".\n",
				this->getScriptName(), this->getID(), targetEntityID, cellAppID);
			_sendBaseTeleportResult(this->getID(), sourceBaseAppID, 0, lastSpaceID);
			return;
		}
	}
	else
	{
		Entity* entity = Cellapp::getSingleton().findEntity(targetEntityID);
		if(entity == NULL || entity->isDestroyed())
		{
			ERROR_MSG("%s::teleportFromBaseapp: %d, can't found targetEntity(%d).\n", 
				this->getScriptName(), this->getID(), targetEntityID);

			_sendBaseTeleportResult(this->getID(), sourceBaseAppID, 0, lastSpaceID);
			return;
		}
		
		// 找到space
		SPACE_ID spaceID = entity->getSpaceID();

		// 如果是不同space跳转
		if(spaceID != this->getSpaceID())
		{
			Space* space = Spaces::findSpace(spaceID);
			if(space == NULL)
			{
				ERROR_MSG("%s::teleportFromBaseapp: %d, can't found space(%u).\n", 
					this->getScriptName(), this->getID(), spaceID);

				_sendBaseTeleportResult(this->getID(), sourceBaseAppID, 0, lastSpaceID);
				return;
			}
			
			Space* currspace = Spaces::findSpace(this->getSpaceID());
			currspace->removeEntity(this);
			space->addEntity(this);
			_sendBaseTeleportResult(this->getID(), sourceBaseAppID, spaceID, lastSpaceID);
		}
		else
		{
			WARNING_MSG("%s::teleportFromBaseapp: %d targetSpace(%u) == currSpaceID.\n", 
				this->getScriptName(), this->getID(), spaceID, this->getSpaceID());

			_sendBaseTeleportResult(this->getID(), sourceBaseAppID, spaceID, lastSpaceID);
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyTeleport(PyObject* nearbyMBRef, PyObject* pyposition, PyObject* pydirection)
{
	if(!PySequence_Check(pyposition) || PySequence_Size(pyposition) != 3)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d position not is Sequence!\n", getScriptName(), getID());
		PyErr_PrintEx(0);
		S_Return;
	}

	if(!PySequence_Check(pydirection) || PySequence_Size(pydirection) != 3)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d direction not is Sequence!\n", getScriptName(), getID());
		PyErr_PrintEx(0);
		S_Return;
	}

	Position3D pos;
	Direction3D dir;

	PyObject* pyitem = PySequence_GetItem(pyposition, 0);
	pos.x = (float)PyFloat_AsDouble(pyitem);
	Py_DECREF(pyitem);

	pyitem = PySequence_GetItem(pyposition, 1);
	pos.y = (float)PyFloat_AsDouble(pyitem);
	Py_DECREF(pyitem);

	pyitem = PySequence_GetItem(pyposition, 2);
	pos.z = (float)PyFloat_AsDouble(pyitem);
	Py_DECREF(pyitem);

	pyitem = PySequence_GetItem(pydirection, 0);
	dir.roll = (float)PyFloat_AsDouble(pyitem);
	Py_DECREF(pyitem);

	pyitem = PySequence_GetItem(pydirection, 1);
	dir.pitch = (float)PyFloat_AsDouble(pyitem);
	Py_DECREF(pyitem);

	pyitem = PySequence_GetItem(pydirection, 2);
	dir.yaw = (float)PyFloat_AsDouble(pyitem);
	Py_DECREF(pyitem);
	
	teleport(nearbyMBRef, pos, dir);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::teleport(PyObject_ptr nearbyMBRef, Position3D& pos, Direction3D& dir)
{
	SPACE_ID lastSpaceID = this->getSpaceID();

	// 如果为None则是entity自己想在本space上跳转到某位置
	if(nearbyMBRef == Py_None)
	{
		this->setPositionAndDirection(pos, dir);
	}
	else
	{
		//EntityMailbox* mb = NULL;
		SPACE_ID spaceID = 0;
		
		// 如果是entity则一定是在本cellapp上， 可以直接进行操作
		if(PyObject_TypeCheck(nearbyMBRef, Entity::getScriptType()))
		{
			Entity* entity = static_cast<Entity*>(nearbyMBRef);
			spaceID = entity->getSpaceID();
			if(spaceID == this->getSpaceID())
			{
				this->setPositionAndDirection(pos, dir);
				onTeleportSuccess(nearbyMBRef, lastSpaceID);
			}
			else
			{
				this->setPositionAndDirection(pos, dir);
				Space* currspace = Spaces::findSpace(this->getSpaceID());
				Space* space = Spaces::findSpace(spaceID);
				currspace->removeEntity(this);
				space->addEntity(this);
				onTeleportSuccess(nearbyMBRef, lastSpaceID);
			}
		}
		else
		{
			if(PyObject_TypeCheck(nearbyMBRef, EntityMailbox::getScriptType()))
			{
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Entity::onTeleport()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleport"));
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportFailure()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportFailure"));
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportSuccess(PyObject* nearbyEntity, SPACE_ID lastSpaceID)
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportSuccess"));
}

//-------------------------------------------------------------------------------------
void Entity::onEnteredCell()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEnteredCell"));
}

//-------------------------------------------------------------------------------------
void Entity::onEnteringCell()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEnteringCell"));
}


//-------------------------------------------------------------------------------------
void Entity::onLeavingCell()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLeavingCell"));
}

//-------------------------------------------------------------------------------------
void Entity::onLeftCell()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLeftCell"));
}

//-------------------------------------------------------------------------------------
}
