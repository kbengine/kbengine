#include "cellapp.hpp"
#include "entity.hpp"
//#include "chunk.hpp"
//#include "space.hpp"

#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	

#ifdef CODE_INLINE
//#include "entity.ipp"
#endif

namespace KBEngine{

class EntityScriptTimerHandler : public TimerHandler
{
public:
	EntityScriptTimerHandler(Entity * entity ) : pEntity_( entity ) 
	{
	}

private:
	virtual void handleTimeout(TimerHandle handle, void * pUser)
	{
		ScriptTimers* scriptTimers = &pEntity_->scriptTimers();
		int id = ScriptTimersUtil::getIDForHandle( scriptTimers, handle );
		pEntity_->onTimer(id, intptr( pUser ));
	}

	virtual void onRelease( TimerHandle handle, void * /*pUser*/ )
	{
		delete this;
	}

	Entity* pEntity_;
};


//-------------------------------------------------------------------------------------
ENTITY_METHOD_DECLARE_BEGIN(Entity)
SCRIPT_METHOD_DECLARE("addTimer",					pyAddTimer,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("delTimer",					pyDelTimer,						METH_VARARGS,				0)
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
SCRIPT_GET_DECLARE("base",							pypyGetBaseMailbox,				0,					0)
SCRIPT_GET_DECLARE("client",						pypyGetClientMailbox,			0,					0)
SCRIPT_GET_DECLARE("isDestroyed",					pypyGetIsDestroyed,				0,					0)
SCRIPT_GET_DECLARE("isWitnessed",					pypyIsWitnessed,				0,					0)
SCRIPT_GET_DECLARE("hasWitness",					pypyHasWitness,					0,					0)
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
spaceID_(0),
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
topSpeedY_(-0.1f),
pChannel_(NULL),
scriptTimers_()
{
	ENTITY_INIT_PROPERTYS(Entity);

	// 获得onTimer函数地址
//	TimerFunc_ = std::tr1::bind(&Entity::onTimer, this, _1, _2);
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	ENTITY_DECONSTRUCTION(Entity);
}	

//-------------------------------------------------------------------------------------
void Entity::test(const std::string& name)
{
	time_t t = time(NULL);
	DEBUG_MSG("Entity::test[%"PRTime"]:=========>name=%s\n", t, name.c_str());
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

void Entity::onDestroy(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onDestroy"), const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();	
	
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
		//uint32 cellDataLength = strCellData.length();
		Py_DECREF(cellData);
		/*
		// 将当前的cell部分数据打包 一起发送给base部分备份
		SocketPacket* sp = new SocketPacket(OP_ENTITY_LOSE_CELL);
		(*sp) << id_;
		(*sp) << (uint32)cellDataLength;
		if(cellDataLength > 0)
			sp->append(strCellData.c_str(), cellDataLength);
			
		baseMailbox_->post(sp);*/
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
int Entity::pySetTopSpeedY(Entity *self, PyObject *value, void *closure)
{
	self->setTopSpeedY(float(PyFloat_AsDouble(PySequence_GetItem(value, 0)))); 
	return 0; 
};

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeedY(Entity *self, void *closure)
{ 
	return PyFloat_FromDouble(self->getTopSpeedY()); 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeed(Entity *self, void *closure)
{ 
	return PyFloat_FromDouble(self->getTopSpeed()); 
}

//-------------------------------------------------------------------------------------
int Entity::pySetTopSpeed(Entity *self, PyObject *value, void *closure)
{ 
	self->setTopSpeed(float(PyFloat_AsDouble(PySequence_GetItem(value, 0)))); 
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
	uint32 utype = propertyDescription->getUType();

	// 首先创建一个需要广播的模板流
	MemoryStream* mstream = new MemoryStream();
	(*mstream) << (uint32)utype;
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
void Entity::onRemoteMethodCall(MemoryStream& s)
{
	uint32 utype = 0;
	s >> utype;
	
	DEBUG_MSG("Entity::onRemoteMethodCall: entityID %d, methodType %ld.\n", 
				id_, utype);
	
	MethodDescription* md = scriptModule_->findCellMethodDescription(utype);
	
	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>
						(md->getName().c_str()));

	if(md != NULL)
		md->call(pyFunc, md->createFromStream(&s));
	
	Py_XDECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
/*
void Entity::onCurrentChunkChanged(Chunk* oldChunk)
{
}
*/
//-------------------------------------------------------------------------------------
PyObject* Entity::pyIsReal(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	return PyBool_FromLong(entity->isReal());
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetIsDestroyed()
{
	return PyBool_FromLong(isDestroyed());
}

//-------------------------------------------------------------------------------------
void Entity::destroyEntity(void)
{
	CellApp::getSingleton().destroyEntity(id_);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDestroyEntity(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	entity->destroyEntity();
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::onWitnessed(Entity* entity, float range)
{/*
	int8 detailLevel = scriptModule_->getDetailLevel().getLevelByRange(range);
	WitnessInfo* info = new WitnessInfo(detailLevel, entity, range);
	ENTITY_ID id = entity->getID();

	DEBUG_MSG("Entity[%s:%ld]::onWitnessed:%s %ld enter detailLevel %d. range=%f.\n", getScriptModuleName(), id_, 
			entity->getScriptModuleName(), id, detailLevel, range);

#ifdef _DEBUG
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = witnessEntityDetailLevelMap_.find(id);
	if(iter != witnessEntityDetailLevelMap_.end())
		ERROR_MSG("Entity::onWitnessed: %s %ld is exist.\n", entity->getScriptModuleName(), id);
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
		ERROR_MSG("Entity[%s:%ld]::onRemoveWitness: can't not found %s %ld.\n", getScriptModuleName(), id_,
			entity->getScriptModuleName(), id);
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
			DEBUG_MSG("Entity[%s:%ld]::onUpdateWitness:%s %ld change detailLevel %d to %d. oldRange=%f, newRange=%f.\n", getScriptModuleName(), id_,
					entity->getScriptModuleName(), id, detailLevel, newDetailLevel, witnessInfo->range, range);

			witnessInfo->detailLevel = newDetailLevel;
			onEntityDetailLevelChanged(witnessInfo, detailLevel, newDetailLevel);
			witnessInfo->detailLevelLog[newDetailLevel] = true;
		}

		witnessInfo->range = range;
	}
	else
	{
		ERROR_MSG("Entity[%s:%ld]::onUpdateWitness: can't not found %s %ld.\n", getScriptModuleName(), id_,
			entity->getScriptModuleName(), id);
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
	(*sp) << (uint8)1;															// 表示向某客户端增加一个entity
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
				(*sp) << (uint32)propertyDescription->getUType();
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
		ERROR_MSG("Entity::addProximity: range(%f) <= 0.0f! entity[%s:%ld]\n", range, getScriptModuleName(), getID());
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
PyObject* Entity::pyAddProximity(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	float range = 0.0f;

	if(PyTuple_Size(args) == 1)
	{
		if(!PyArg_ParseTuple(args, "f", &range))
		{
			ERROR_MSG("Entity::addProximity: args is error!\n");
			return NULL;
		}
	}
	else
	{
		ERROR_MSG("Entity::addProximity: args require 1 args, gived %d! entity[%s:%ld]\n", PyTuple_Size(args), entity->getScriptModuleName(), entity->getID());
		return NULL;
	}

	return PyLong_FromLong(entity->addProximity(range));
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
PyObject* Entity::pyDelProximity(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	uint16 id = 0;

	if(PyTuple_Size(args) == 1)
	{
		if(!PyArg_ParseTuple(args, "I", &id))
		{
			ERROR_MSG("Entity::delProximity: args is error!\n");
			return NULL;
		}
	}
	else
	{
		ERROR_MSG("Entity::delProximity: args require 1 args, gived %d! entity[%s:%ld].\n", 
				PyTuple_Size(args), entity->getScriptModuleName(), entity->getID());
		return NULL;
	}

	entity->delProximity(id);
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
PyObject* Entity::addTimer(float interval, float repeat, int userArg)
{
	EntityScriptTimerHandler* pHandler = new EntityScriptTimerHandler(this);
	ScriptTimers * pTimers = &scriptTimers_;
	int id = ScriptTimersUtil::addTimer( &pTimers,
			interval, repeat,
			userArg, pHandler );

	if (id == 0)
	{
		PyErr_SetString(PyExc_ValueError, "Unable to add timer");
		delete pHandler;

		return NULL;
	}

	return PyLong_FromLong( id );
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddTimer(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	float interval = 0;
	float repeat = 0;
	int userArg;

	if(PyTuple_Size(args) == 3)
	{
		if(!PyArg_ParseTuple(args, "f|f|i", &interval, &repeat, &userArg))
		{
			ERROR_MSG("Entity::addTimer: args is error!\n");
			S_Return;
		}
	}
	else
	{
		ERROR_MSG("Entity::addTimer: args require 3 args, gived %d! entity[%s:%ld].\n", 
			PyTuple_Size(args), entity->getScriptModuleName(), entity->getID());

		S_Return;
	}
	
	return entity->addTimer(interval, repeat, userArg);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::delTimer(ScriptID timerID)
{
	if(!ScriptTimersUtil::delTimer(&scriptTimers_, timerID))
	{
		return PyLong_FromLong(-1);
	}

	return PyLong_FromLong(timerID);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDelTimer(PyObject* self, PyObject* args, PyObject* kwds)
{
	TIMER_ID timerID = 0;
	Entity* entity = static_cast<Entity*>(self);

	if(PyTuple_Size(args) == 1)
	{
		if(!PyArg_ParseTuple(args, "k", &timerID))
		{
			ERROR_MSG("Entity::delTimer: args is error!\n");
			return NULL;
		}
	}
	else
	{
		ERROR_MSG("Entity::delTimer: args != 1!\n");
		return NULL;
	}

	return entity->delTimer(timerID);
}

//-------------------------------------------------------------------------------------
void Entity::onTimer(ScriptID timerID, int useraAgs)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onTimer"), 
		const_cast<char*>("Ii"), timerID, useraAgs);
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddSpaceGeometryMapping(PyObject* self, PyObject* args, PyObject* kwds)
{/*
	SPACE_ID spaceID;
	char* path;
	
	if(PyTuple_Size(args) == 2)
	{
		if(!PyArg_ParseTuple(args, "k|s", &spaceID, &path))
		{
			ERROR_MSG("Entity::addSpaceGeometryMapping: args is error!\n");
			S_Return;
		}
		
		App::getSingleton().addSpaceGeometryMapping(spaceID, path);
		S_Return;
	}
	
	ERROR_MSG("Entity::addSpaceGeometryMapping: args is count != 2(spaceID, path)!\n");*/
	S_Return;
}

//-------------------------------------------------------------------------------------
int Entity::pySetPosition(Entity *self, PyObject *value, void *closure)
{
	if(!script::ScriptVector3::check(value))
		return -1;

	Position3D& pos = self->getPosition();
	script::ScriptVector3::convertPyObjectToVector3(pos, value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetPosition(Entity *self, void *closure)
{
	return new script::ScriptVector3(&self->getPosition());
}

//-------------------------------------------------------------------------------------
int Entity::pySetDirection(Entity *self, PyObject *value, void *closure)
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

	Direction3D& dir = self->getDirection();
	dir.roll	= float(PyFloat_AsDouble(PySequence_GetItem(value, 0)));
	dir.pitch	= float(PyFloat_AsDouble(PySequence_GetItem(value, 1)));
	dir.yaw		= float(PyFloat_AsDouble(PySequence_GetItem(value, 2)));
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetDirection(Entity *self, void *closure)
{
	return new script::ScriptVector3(self->getDirection().asVector3());
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
PyObject* Entity::setAoiRadius(float radius, float hyst)
{/*
	if(!hasWitness_)
	{
		ERROR_MSG("Entity::setAoiRadius:%s %ld no has witness.\n", getScriptModuleName(), id_);
		return PyLong_FromLong(0);
	}

	if(clientMailbox_ == NULL || clientMailbox_ == Py_None)
		return PyLong_FromLong(0);

	aoiRadius_ = radius;
	aoiHysteresisArea_ = hyst;

	if(aoiRadius_ + aoiHysteresisArea_ > CELL_BORDER_WIDTH)
	{
		aoiRadius_ = CELL_BORDER_WIDTH - 15.0f;
		aoiHysteresisArea_ = 15.0f;
	}
	
	// 在space中投放一个AOI陷阱
	Proximity* p = new ProximityAOI(this, aoiRadius_, aoiHysteresisArea_);
	trapMgr_.addProximity(p);
	if(currChunk_ != NULL)
		currChunk_->getSpace()->placeProximity(currChunk_, p);
*/
	return PyLong_FromLong(1);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pySetAoiRadius(PyObject* self, PyObject* args, PyObject* kwds)
{
	float radius = 0.0f, hyst = 0.0f;
	Entity* entity = static_cast<Entity*>(self);

	if(PyTuple_Size(args) == 2)
	{
		if(!PyArg_ParseTuple(args, "f|f", &radius, &hyst))
		{
			ERROR_MSG("Entity::setAoiRadius: args is error!\n");
			return NULL;
		}
	}
	else
	{
		ERROR_MSG("Entity::setAoiRadius: is error, args!=(float radius, float hyst)!\n");
		return PyLong_FromLong(0);
	}

	return entity->setAoiRadius(radius, hyst);
}

//-------------------------------------------------------------------------------------
bool Entity::navigateStep(const Position3D& destination, float velocity, float maxMoveDistance, float maxDistance, 
	bool faceMovement, float girth, PyObject* userData)
{
	DEBUG_MSG("Entity[%s:%d]:destination=(%f,%f,%f), velocity=%f, maxMoveDistance=%f, "
		"maxDistance=%f, faceMovement=%d, girth=%f, userData=%x.\n",

	getScriptModuleName(), id_, destination.x, destination.y, destination.z, velocity, maxMoveDistance, 
	maxDistance, faceMovement, girth, userData);
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyNavigateStep(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	float velocity = 0.0f, maxMoveDistance = 0.0f, maxDistance = 0.0f, girth = 0.0f;
	PyObject* userData = NULL;
	Position3D destination;
	PyObject* pyDestination = NULL;
	int faceMovement = 0;

	if(PyTuple_Size(args) == 7)
	{
		if(!PyArg_ParseTuple(args, "O|f|f|f|i|f|O", &pyDestination, &velocity, &maxMoveDistance, &maxDistance, 
			&faceMovement, &girth, &userData))
		{
			ERROR_MSG("Entity::navigateStep: args is error!\n");
			Py_RETURN_FALSE;
		}
		
		// 将坐标信息提取出来
		script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	}
	else
	{
		ERROR_MSG("Entity::navigateStep: is error, args!=(destination, velocity, maxMoveDistance, "
			"maxDistance, faceMovement, girth, userData)!\n");
		Py_RETURN_FALSE;
	}
	
	Py_INCREF(userData);
	if(entity->navigateStep(destination, velocity, maxMoveDistance, 
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
PyObject* Entity::pyMoveToPoint(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	float velocity = 0.0f;
	PyObject* userData = NULL;
	Position3D destination;
	PyObject* pyDestination = NULL;
	int faceMovement = 0;
	int moveVertically = 0;
	
	if(PyTuple_Size(args) == 5)
	{
		if(!PyArg_ParseTuple(args, "O|f|O|i|i", &pyDestination, &velocity, &userData, &faceMovement, &moveVertically))
		{
			ERROR_MSG("Entity::moveToPoint: args is error!\n");
			Py_RETURN_FALSE;
		}
		
		// 将坐标信息提取出来
		script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	}
	else
	{
		ERROR_MSG("Entity::moveToPoint: is error, args!=(destination, velocity, maxMoveDistance, "
			"maxDistance, faceMovement, girth, userData)!\n");

		Py_RETURN_FALSE;
	}
	
	Py_INCREF(userData);

	if(entity->moveToPoint(destination, velocity, userData, faceMovement > 0, moveVertically > 0)){
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
PyObject* Entity::pyStopMove(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	return PyBool_FromLong(entity->stopMove());
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
PyObject* Entity::pyEntitiesInRange(PyObject* self, PyObject* args, PyObject* kwds)
{
	/*PyObject* pyPosition = NULL;
	PyObject* pyEntityType = NULL;
	std::string entityType = "";
	float radius = 0.0f;
	Position3D pos;
	
	if(PyTuple_Size(args) == 3)
	{
		if(!PyArg_ParseTuple(args, "f|O|O", &radius, &pyEntityType, &pyPosition))
		{
			ERROR_MSG("Entity::entitiesInRange: args is error!\n");
			return 0;
		}
		
		// 将坐标信息提取出来
		script::ScriptVector3::convertPyObjectToVector3(pos, pyPosition);
		if(pyEntityType != Py_None)
		{
			entityType = PyString_AsString(pyEntityType);
		}
	}
	else
	{
		ERROR_MSG("Entity::entitiesInRange: args is error! args != 3.\n");
		return 0;
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
