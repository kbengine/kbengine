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
//#include "chunk.hpp"
#include "space.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	

#include "../../server/baseapp/baseapp_interface.hpp"

#ifdef CODE_INLINE
//#include "entity.ipp"
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
ENTITY_METHOD_DECLARE_BEGIN(Entity)
SCRIPT_METHOD_DECLARE("addSpaceGeometryMapping",	pyAddSpaceGeometryMapping,		METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("setAoiRadius",				pySetAoiRadius,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("isReal",						pyIsReal,						METH_VARARGS,				0)	
SCRIPT_METHOD_DECLARE("addProximity",				pyAddProximity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("delProximity",				pyDelProximity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("destroy",					pyDestroyEntity,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("navigateStep",				pyNavigateStep,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("moveToPoint",				pyMoveToPoint,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("stopMove",					pyStopMove,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("entitiesInRange",			pyEntitiesInRange,				METH_VARARGS,				0)
ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

ENTITY_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GET_DECLARE("base",							pyGetBaseMailbox,				0,					0)
SCRIPT_GET_DECLARE("client",						pyGetClientMailbox,				0,					0)
SCRIPT_GET_DECLARE("isDestroyed",					pyGetIsDestroyed,				0,					0)
SCRIPT_GET_DECLARE("isWitnessed",					pyIsWitnessed,					0,					0)
SCRIPT_GET_DECLARE("hasWitness",					pyHasWitness,					0,					0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,		0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,		0,		0)
SCRIPT_GETSET_DECLARE("topSpeed",					pyGetTopSpeed,					pySetTopSpeed,		0,		0)
SCRIPT_GETSET_DECLARE("topSpeedY",					pyGetTopSpeedY,					pySetTopSpeedY,		0,		0)
ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, ScriptModule* scriptModule):
ScriptObject(getScriptType(), true),
ENTITY_CONSTRUCTION(Entity),
clientMailbox_(NULL),
baseMailbox_(NULL),
//currChunk_(NULL),
isReal_(true),
isDestroyed_(false),
aoiRadius_(0.0f),
aoiHysteresisArea_(0.0f),
isWitnessed_(false),
hasWitness_(false),
topSpeed_(-0.1f),
topSpeedY_(-0.1f)
{
	ENTITY_INIT_PROPERTYS(Entity);
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	ENTITY_DECONSTRUCTION(Entity);
}	

//-------------------------------------------------------------------------------------
void Entity::destroy()
{
	isDestroyed_ = true;
	onDestroy();
	S_RELEASE(clientMailbox_);
	S_RELEASE(baseMailbox_);
	Py_DECREF(this);
}

//-------------------------------------------------------------------------------------
void Entity::onDestroy(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onDestroy"), const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();	
	
	this->backupCellData();

	if(baseMailbox_ != NULL)
	{
		Mercury::Bundle bundle;
		bundle.newMessage(BaseappInterface::onLoseCell);
		bundle << id_;
		baseMailbox_->postMail(bundle);
	}
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
	setTopSpeedY(float(PyFloat_AsDouble(PySequence_GetItem(value, 0)))); 
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
	setTopSpeed(float(PyFloat_AsDouble(PySequence_GetItem(value, 0)))); 
	return 0; 
}

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(const PropertyDescription* propertyDescription, PyObject* pyData)
{
	/*
	// 如果不是一个realEntity则不理会
	if(!isReal())
		return;
	
	const uint32& flags = propertyDescription->getFlags();
	ENTITY_PROPERTY_UID utype = propertyDescription->getUType();

	// 首先创建一个需要广播的模板流
	MemoryStream* mstream = new MemoryStream();
	(*mstream) << utype;
	propertyDescription->getDataType()->addToStream(mstream, pyData);

	// 判断是否需要广播给其他的cellapp, 这个还需一个前提是entity必须拥有ghost实体
	// 只有在cell边界一定范围内的entity才拥有ghost实体
	if((flags & ENTITY_BROADCAST_CELL_FLAGS) > 0)
	{
	}
	
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

	// 判断这个属性是否还需要广播给自己的客户端
	if((flags & ENTITY_BROADCAST_OWN_CLIENT_FLAGS) > 0 && clientMailbox_ != NULL)
	{
		SocketPacket* sp = clientMailbox_->createMail(MAIL_TYPE_UPDATE_PROPERTY);
		(*sp) << id_;
		sp->append(bytestream->contents(), bytestream->size());
		clientMailbox_->post(sp);
	}

	SAFE_RELEASE(bytestream);
	*/
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
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
						(md->getName().c_str()));

	if(md != NULL)
	{
		PyObject* pyargs = md->createFromStream(&s);
		md->call(pyFunc, pyargs);
		Py_DECREF(pyargs);
	}
	
	Py_XDECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
void Entity::backupCellData()
{
	if(baseMailbox_ != NULL)
	{
		PyObject* cellData = getCellDataByFlags(ENTITY_CELL_DATA_FLAGS);
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
		
		std::string strCellData = script::Pickler::pickle(cellData);
		uint32 cellDataLength = strCellData.length();
		Py_DECREF(cellData);
	
		// 将当前的cell部分数据打包 一起发送给base部分备份
		Mercury::Bundle bundle;
		bundle.newMessage(BaseappInterface::onBackupEntityCellData);
		bundle << id_;
		bundle << cellDataLength;
		if(cellDataLength > 0)
			bundle.append(strCellData.c_str(), cellDataLength);
			
		baseMailbox_->postMail(bundle);
	}
	else
	{
		WARNING_MSG("Entity::backupCellData(): %s %i has no base!\n", this->getScriptName(), this->getID());
	}
}

//-------------------------------------------------------------------------------------
void Entity::writeToDB()
{
	onWriteToDB();
	backupCellData();
}

//-------------------------------------------------------------------------------------
void Entity::onWriteToDB()
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onWriteToDB"), const_cast<char*>(""));

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
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
PyObject* Entity::pyGetIsDestroyed()
{
	return PyBool_FromLong(isDestroyed());
}

//-------------------------------------------------------------------------------------
void Entity::destroyEntity(void)
{
	Cellapp::getSingleton().destroyEntity(id_);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDestroyEntity()
{
	destroyEntity();
	S_Return;
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
		PyObject* pyResult = PyObject_CallMethod(this, "onWitnessed", "O", PyBool_FromLong(1));
		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			PyErr_Clear();
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::onRemoveWitness(Entity* entity)
{
	ENTITY_ID id = entity->getID();
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = witnessEntityDetailLevelMap_.find(id);
	if(iter != witnessEntityDetailLevelMap_.end())
	{
		witnessEntities_[iter->second->detailLevel].erase(id);
		witnessEntityDetailLevelMap_.erase(iter);
	}
	else
	{
		ERROR_MSG("Entity[%s:%ld]::onRemoveWitness: can't not found %s %ld.\n", getScriptName(), id_,
			entity->getScriptName(), id);
	}
	
	if(witnessEntityDetailLevelMap_.size() <= 0)
	{
		isWitnessed_ = false;
		PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onWitnessed"), const_cast<char*>("O"), PyBool_FromLong(0));
		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			PyErr_Clear();
	}
}

//-------------------------------------------------------------------------------------
void Entity::onUpdateWitness(Entity* entity, float range)
{/*
	ENTITY_ID id = entity->getID();
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = witnessEntityDetailLevelMap_.find(id);
	if(iter != witnessEntityDetailLevelMap_.end())
	{
		WitnessInfo* witnessInfo = iter->second;
		int8 detailLevel = witnessInfo->detailLevel;
		int8 newDetailLevel = scriptModule_->getDetailLevel().getLevelByRange(detailLevel, range);

		// 详情级别改变了
		if(detailLevel != newDetailLevel)
		{
			DEBUG_MSG("Entity[%s:%ld]::onUpdateWitness:%s %ld change detailLevel %d to %d. oldRange=%f, newRange=%f.\n", getScriptName(), id_,
					entity->getScriptName(), id, detailLevel, newDetailLevel, witnessInfo->range, range);

			witnessInfo->detailLevel = newDetailLevel;
			onEntityDetailLevelChanged(witnessInfo, detailLevel, newDetailLevel);
			witnessInfo->detailLevelLog[newDetailLevel] = true;
		}

		witnessInfo->range = range;
	}
	else
	{
		ERROR_MSG("Entity[%s:%ld]::onUpdateWitness: can't not found %s %ld.\n", getScriptName(), id_,
			entity->getScriptName(), id);
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::onViewEntity(Entity* entity)
{
	// 将这个entity加入视野范围Entity列表
	viewEntities_[entity->getID()] = entity;
}

//-------------------------------------------------------------------------------------
void Entity::onLoseViewEntity(Entity* entity)
{/*
	ENTITY_ID eid = entity->getID();
	// 将这个entit从视野范围Entity列表删除
	std::map<ENTITY_ID, Entity*>::iterator iter = viewEntities_.find(eid);
	if(iter != viewEntities_.end())
		viewEntities_.erase(iter);
	
	// 从本entity的客户端中删除他
	if(entity->getScriptModule()->hasClient() && clientMailbox_ != NULL)
	{
		SocketPacket* sp = clientMailbox_->createMail(MAIL_TYPE_LOST_VIEW_ENTITY);
		(*sp) << eid;
		clientMailbox_->post(sp);
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::onEntityInitDetailLevel(Entity* entity, int8 detailLevel)
{
	// 自身没有客户端部分则无需做相关操作
	if(!scriptModule_->hasClient())
		return;
	/*
	EntityMailbox* clientMailbox = entity->getClientMailbox();
	ENTITY_ID eid = entity->getID();
	SocketPacket* sp = new SocketPacket(OP_ENTITY_ENTER_WORLD);

	// 将这个entity的数据写入包中
	(*sp) << eid;
	bool isAdd = true;
	(*sp) << isAdd;															// 表示向某客户端增加一个entity
	(*sp) << id_;
	(*sp) << this->ob_type->tp_name;
	(*sp) << position_.x << position_.y << position_.z;
	(*sp) << direction_.roll << direction_.pitch << direction_.yaw;
	
	// 如果是非常远的级别， 那么我们只发送一个entity基础信息到客户端
	if(detailLevel == DETAIL_LEVEL_UNKNOW)
	{
		clientMailbox->post(sp);
		return;
	}
	
	while(detailLevel <= DETAIL_LEVEL_FAR)
		getCellDataByDetailLevel(detailLevel++, sp);
	
	clientMailbox->post(sp);*/
}

//-------------------------------------------------------------------------------------
void Entity::onEntityDetailLevelChanged(const WitnessInfo* witnessInfo, int8 oldDetailLevel, int8 newDetailLevel)
{/*
	// 自身没有客户端部分则无需做相关操作
	if(!scriptModule_->hasClient())
		return;
	
	// 如果是非常远的级别或者是由近级别改变到远级别， 那么我们忽略下面的操作
	if(newDetailLevel == DETAIL_LEVEL_UNKNOW || oldDetailLevel <= newDetailLevel)
		return;
	
	Entity* entity = witnessInfo->entity;
	EntityMailbox* clientMailbox = entity->getClientMailbox();
	SocketPacket* sp = NULL;

	// 如果没有初始化过属性则初始化所有属性到客户端， 否则只发送期间有过改变的属性
	if(!witnessInfo->detailLevelLog[newDetailLevel])
	{
		sp = clientMailbox->createMail(MAIL_TYPE_UPDATE_PROPERTYS);
		
		// 将这个entity的数据写入包中
		(*sp) << id_;
		getCellDataByDetailLevel(newDetailLevel, sp);
		clientMailbox->post(sp);
	}
	else
	{
		std::vector<uint32>& cddlog = witnessInfo->changeDefDataLogs[newDetailLevel];

		// 如果有属性被改变才进行属性更新
		if(cddlog.size() > 0)
		{
			sp = clientMailbox->createMail(MAIL_TYPE_UPDATE_PROPERTYS);
			std::vector<uint32>::iterator piter = cddlog.begin();
			for(;piter != cddlog.end(); piter++)
			{
				PropertyDescription* propertyDescription = scriptModule_->findCellPropertyDescription((*piter));
				(*sp) << propertyDescription->getUType();
				PyObject* pyVal = PyObject_GetAttrString(this, propertyDescription->getName().c_str());
				propertyDescription->getDataType()->addToStream(sp, pyVal);
				Py_DECREF(pyVal);
			}

			clientMailbox->post(sp);
			cddlog.clear();
		}
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::onGetWitness(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onGetWitness"), 
										const_cast<char*>(""));
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();

	hasWitness_ = true;
}

//-------------------------------------------------------------------------------------
void Entity::onLoseWitness(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onLoseWitness"), 
						const_cast<char*>(""));
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();

	hasWitness_ = false;
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
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onEnterTrap"), 
					const_cast<char*>("Ofi"), entity, range, controllerID);
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrap(Entity* entity, float range, int controllerID)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onLeaveTrap"), 
			const_cast<char*>("Ofi"), entity, range, controllerID);
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrapID(ENTITY_ID entityID, float range, int controllerID)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onLeaveTrapID"), 
	const_cast<char*>("kfi"), entityID, range, controllerID);
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
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
	dir.roll	= float(PyFloat_AsDouble(PySequence_GetItem(value, 0)));
	dir.pitch	= float(PyFloat_AsDouble(PySequence_GetItem(value, 1)));
	dir.yaw		= float(PyFloat_AsDouble(PySequence_GetItem(value, 2)));
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
int32 Entity::setAoiRadius(float radius, float hyst)
{
	if(!hasWitness_)
	{
		ERROR_MSG("Entity::setAoiRadius:%s %ld no has witness.\n", getScriptName(), id_);
		return 0;
	}

	if(clientMailbox_ == NULL || clientMailbox_ == Py_None)
		return 0;

	aoiRadius_ = radius;
	aoiHysteresisArea_ = hyst;

	if(aoiRadius_ + aoiHysteresisArea_ > CELL_BORDER_WIDTH)
	{
		aoiRadius_ = CELL_BORDER_WIDTH - 15.0f;
		aoiHysteresisArea_ = 15.0f;
	}
	
/*	// 在space中投放一个AOI陷阱
	Proximity* p = new ProximityAOI(this, aoiRadius_, aoiHysteresisArea_);
	trapMgr_.addProximity(p);
	if(currChunk_ != NULL)
		currChunk_->getSpace()->placeProximity(currChunk_, p);
*/
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
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onMove"), 
		const_cast<char*>("O"), userData);
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
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
		ScriptModule* sm = ExtendScriptModuleMgr::findScriptModule(entityType.c_str());
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
}
