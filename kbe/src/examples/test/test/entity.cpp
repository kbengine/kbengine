//#include "app.hpp"
#include "entity.hpp"
//#include "chunk.hpp"
//#include "space.hpp"
namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(Entity)
SCRIPT_METHOD_DECLARE("__reduce_ex__",				__reduce_ex__,					METH_VARARGS,				0)
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
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GET_DECLARE(	"id",							pyGetID,						0,					0)
SCRIPT_GET_DECLARE("spaceID",						pyGetSpaceID,					0,					0)
//SCRIPT_GET_DECLARE("base",							pyGetBaseMailbox,				0,					0)
//SCRIPT_GET_DECLARE("client",						pyGetClientMailbox,				0,					0)
SCRIPT_GET_DECLARE("isDestroyed",					pyGetIsDestroyed,				0,					0)
SCRIPT_GET_DECLARE("isWitnessed",					pyIsWitnessed,					0,					0)
SCRIPT_GET_DECLARE("hasWitness",					pyHasWitness,					0,					0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,		0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,		0,		0)
SCRIPT_GETSET_DECLARE("topSpeed",					pyGetTopSpeed,					pySetTopSpeed,		0,		0)
SCRIPT_GETSET_DECLARE("topSpeedY",					pyGetTopSpeedY,					pySetTopSpeedY,		0,		0)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, ScriptModule* scriptModule):
ScriptObject(getScriptType(), true),
m_id_(id),
m_scriptModule_(scriptModule),
m_lpPropertyDescrs_(NULL),
m_spaceID_(0),
//m_baseMailbox_(NULL),
//m_clientMailbox_(NULL),
//m_currChunk_(NULL),
m_isReal_(true),
m_isDestroyed_(false),
m_aoiRadius_(0.0f),
m_aoiHysteresisArea_(0.0f),
m_hasWitness_(false),
m_isWitnessed_(false),
m_topSpeed_(-0.1f),
m_topSpeedY_(-0.1f)
{
	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = m_scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		DataType* dataType = propertyDescription->getDataType();
		
		if(dataType)
		{
			MemoryStream* ms = propertyDescription->getDefaultVal();
			PyObject* pyVal = dataType->createObject(ms);
			PyObject_SetAttrString(this, propertyDescription->getName().c_str(), pyVal);
			Py_DECREF(pyVal);
			if(ms)
				ms->rpos(0);
		}
		else
			ERROR_MSG("Entity::Entity: %s PropertyDescription the dataType is NULL.\n", propertyDescription->getName().c_str());		
	}

	m_lpPropertyDescrs_ = &propertyDescrs;

	// 获得onTimer函数地址
//	m_TimerFunc_ = std::tr1::bind(&Entity::onTimer, this, _1, _2);
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	DEBUG_MSG("Entity::~Entity(): %s %ld\n", getScriptModuleName(), m_id_);
	m_scriptModule_ = NULL;
}	

//-------------------------------------------------------------------------------------
void Entity::destroy()
{
	m_isDestroyed_ = true;
	onDestroy();
//	S_RELEASE(m_clientMailbox_);
//	S_RELEASE(m_baseMailbox_);
	Py_DECREF(this);
}

void Entity::onDestroy(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, "onDestroy", "");
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();	
	/*
	if(m_baseMailbox_ != NULL)
	{
		PyObject* cellData = getCellDataByFlags(ENTITY_CELL_DATA_FLAGS);
		// 将entity位置和方向变量也设置进去
		PyObject* pyPosition = PyTuple_New(3);
		PyTuple_SET_ITEM(pyPosition, 0, PyFloat_FromDouble(m_position_.x));
		PyTuple_SET_ITEM(pyPosition, 1, PyFloat_FromDouble(m_position_.y));
		PyTuple_SET_ITEM(pyPosition, 2, PyFloat_FromDouble(m_position_.z));
		
		PyObject* pyDirection = PyTuple_New(3);
		PyTuple_SET_ITEM(pyDirection, 0, PyFloat_FromDouble(m_direction_.roll));
		PyTuple_SET_ITEM(pyDirection, 1, PyFloat_FromDouble(m_direction_.pitch));
		PyTuple_SET_ITEM(pyDirection, 2, PyFloat_FromDouble(m_direction_.yaw));
		
		PyDict_SetItemString(cellData, "position", pyPosition);
		PyDict_SetItemString(cellData, "direction", pyDirection);
		
		std::string strCellData = script::Pickler::pickle(cellData);
		uint32 cellDataLength = strCellData.length();
		Py_DECREF(cellData);
		
		// 将当前的cell部分数据打包 一起发送给base部分备份
		SocketPacket* sp = new SocketPacket(OP_ENTITY_LOSE_CELL);
		(*sp) << m_id_;
		(*sp) << (uint32)cellDataLength;
		if(cellDataLength > 0)
			sp->append(strCellData.c_str(), cellDataLength);
			
		m_baseMailbox_->post(sp);
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::initializeScript()
{
	// 调用脚本的__init__初始化脚本
	PyObject* pyResult = PyObject_CallMethod(this, "__init__", "");
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Entity::createNamespace(PyObject* dictData)
{
	if(dictData == NULL)
		return;

	if(!PyDict_Check(dictData)){
		ERROR_MSG("Entity::createNamespace: createEntity[%s:%ld] args is not a dict.\n", getScriptModuleName(), m_id_);
		return;
	}
	
	PyObject *key, *value;
	int pos = 0;

	while(PyDict_Next(dictData, &pos, &key, &value))
	{
	    PyObject_SetAttr(this, key, value);
	}

	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::getCellDataByFlags(uint32 flags)
{
	PyObject* cellData = PyDict_New();
	PyObject* pydict = PyObject_GetAttrString(this, "__dict__");

	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = m_scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(pydict, propertyDescription->getName().c_str());
			PyDict_SetItemString(cellData, propertyDescription->getName().c_str(), pyVal);
		}
	}

	Py_XDECREF(pydict);
	return cellData;
}

//-------------------------------------------------------------------------------------
void Entity::getCellDataByDetailLevel(const int8& detailLevel, MemoryStream* mstream)
{
	PyObject* cellData = PyObject_GetAttrString(this, "__dict__");

	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = m_scriptModule_->getCellPropertyDescriptionsByDetailLevel(detailLevel);
	ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		PyObject* pyVal = PyDict_GetItemString(cellData, propertyDescription->getName().c_str());
		(*mstream) << (uint32)propertyDescription->getUType();
		propertyDescription->getDataType()->addToStream(mstream, pyVal);
	}

	Py_XDECREF(cellData);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__reduce_ex__(PyObject* self, PyObject* protocol)
{
	Entity* entity = static_cast<Entity*>(self);
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Mailbox");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	PyObject* args1 = PyTuple_New(4);
	PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(entity->getID()));
//	PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLong(App::getSingleton().getComponentID()));
	PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(entity->getScriptModule()->getUType()));
//	PyTuple_SET_ITEM(args1, 3, PyLong_FromUnsignedLong(MAILBOX_TYPE_BASE));
	PyTuple_SET_ITEM(args, 1, args1);

	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}
	return args;
}
/*
//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetBaseMailbox(Entity *self, void *closure)
{ 
	EntityMailbox* mailbox = self->getBaseMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetClientMailbox(Entity *self, void *closure)
{ 
	EntityMailbox* mailbox = self->getClientMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}
*/
//-------------------------------------------------------------------------------------
int Entity::onScriptSetAttribute(const Py_UNICODE* attr, PyObject* value)
{
	char cattr[255];
	wcstombs(cattr, attr, 255);

	if(m_lpPropertyDescrs_)
	{
		ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = m_lpPropertyDescrs_->find(cattr);
		if(iter != m_lpPropertyDescrs_->end())
		{
			PropertyDescription* propertyDescription = iter->second;
			DataType* dataType = propertyDescription->getDataType();
			
			if(!dataType->isSameType(value)){
				return 0;
			}
			else
			{
				int result = propertyDescription->onSetValue(this, value);
				
				// 如果def属性数据有改变， 那么可能需要广播
				if(result != -1)
					onDefDataChanged(propertyDescription, value);
				
				return result;
			}
		}
	}
	return ScriptObject::onScriptSetAttribute(attr, value);
}

//-------------------------------------------------------------------------------------
PyObject * Entity::onScriptGetAttribute(const Py_UNICODE* attr)
{
	return ScriptObject::onScriptGetAttribute(attr);
}

//-------------------------------------------------------------------------------------
int Entity::onScriptDelAttribute(const Py_UNICODE* attr)
{
	char cattr[255];
	wcstombs(cattr, attr, 255);

	if(m_lpPropertyDescrs_)
	{

		ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = m_lpPropertyDescrs_->find(cattr);
		if(iter != m_lpPropertyDescrs_->end())
		{
			char err[255];
			sprintf((char*)&err, "property[%s] is in [%s] def. can't to del.", cattr, getScriptModuleName());
			PyErr_SetString(PyExc_TypeError, err);			
			PyErr_PrintEx(0);
			return 0;
		}
	}

	if(m_scriptModule_->findCellMethodDescription(cattr) != NULL)
	{
		char err[255];
		sprintf((char*)&err, "method[%s] is in [%s] def. can't to del.", cattr, getScriptModuleName());
		PyErr_SetString(PyExc_TypeError, err);			
		PyErr_PrintEx(0);
		return 0;
	}

	return ScriptObject::onScriptDelAttribute(attr);
}

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(PropertyDescription* propertyDescription, PyObject* pyData)
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
			std::map<ENTITY_ID, Entity*>::iterator iter = m_witnessEntities_[i].begin();
			for(; iter != m_witnessEntities_[i].end(); iter++)
			{
				Entity* entity = iter->second;
				EntityMailbox* clientMailbox = entity->getClientMailbox();
				if(clientMailbox != NULL)
				{
					Packet* sp = clientMailbox->createMail(MAIL_TYPE_UPDATE_PROPERTY);
					(*sp) << m_id_;
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
			std::map<ENTITY_ID, Entity*>::iterator iter = m_witnessEntities_[i].begin();
			for(; iter != m_witnessEntities_[i].end(); iter++)
			{
				Entity* entity = iter->second;
				EntityMailbox* clientMailbox = entity->getClientMailbox();
				if(clientMailbox != NULL)
				{
					WitnessInfo* witnessInfo = m_witnessEntityDetailLevelMap_.find(iter->first)->second;
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
	if((flags & ENTITY_BROADCAST_OWN_CLIENT_FLAGS) > 0 && m_clientMailbox_ != NULL)
	{
		SocketPacket* sp = m_clientMailbox_->createMail(MAIL_TYPE_UPDATE_PROPERTY);
		(*sp) << m_id_;
		sp->append(bytestream->contents(), bytestream->size());
		m_clientMailbox_->post(sp);
	}

	SAFE_RELEASE(bytestream);
	*/
}

//-------------------------------------------------------------------------------------
/*
void Entity::onRemoteMethodCall(SocketPacket& recvPacket)
{
	uint32 utype = 0;
	recvPacket >> (uint32)utype;
	DEBUG_MSG("Entity::onRemoteMethodCall: entityID %d, methodType %ld.\n", m_id_, utype);
	MethodDescription* md = m_scriptModule_->findCellMethodDescription(utype);
	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>(md->getName().c_str()));
	if(md != NULL)
		md->call(pyFunc, md->createFromStream(&recvPacket));
	
	Py_XDECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
void Entity::onReceiveMail(MAIL_TYPE& mailType, SocketPacket& recvPacket)
{
	DEBUG_MSG("Entity::onReceiveMail: entityID %d, mailType %ld.\n", m_id_, mailType);
	switch(mailType)
	{
		case MAIL_TYPE_REMOTE_CALL:
			onRemoteMethodCall(recvPacket);
			break;
		case MAIL_TYPE_SEEK:
		{
			Position3D pos;
			float verticalRange;
			float moveSpeed;
			recvPacket >> pos.x >> pos.y >> pos.z;
			recvPacket >> moveSpeed;
			recvPacket >> verticalRange;
			Py_INCREF(Py_None);
			moveToPoint(pos, moveSpeed, Py_None, true, true);
			break;
		}
	};
}

//-------------------------------------------------------------------------------------
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
PyObject* Entity::pyGetIsDestroyed(Entity *self, void *closure)
{
	return PyBool_FromLong(self->isDestroyed());
}

//-------------------------------------------------------------------------------------
void Entity::destroyEntity(void)
{
//	App::getSingleton().destroyEntity(m_id_);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDestroyEntity(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	entity->destroyEntity();
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::onWitnessed(Entity* entity, const float& range)
{/*
	int8 detailLevel = m_scriptModule_->getDetailLevel().getLevelByRange(range);
	WitnessInfo* info = new WitnessInfo(detailLevel, entity, range);
	ENTITY_ID id = entity->getID();

	DEBUG_MSG("Entity[%s:%ld]::onWitnessed:%s %ld enter detailLevel %d. range=%f.\n", getScriptModuleName(), m_id_, 
			entity->getScriptModuleName(), id, detailLevel, range);

#ifdef _DEBUG
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = m_witnessEntityDetailLevelMap_.find(id);
	if(iter != m_witnessEntityDetailLevelMap_.end())
		ERROR_MSG("Entity::onWitnessed: %s %ld is exist.\n", entity->getScriptModuleName(), id);
#endif
	
	m_witnessEntityDetailLevelMap_[id] = info;
	m_witnessEntities_[detailLevel][id] = entity;
	onEntityInitDetailLevel(entity, detailLevel);
	
	if(!m_isWitnessed_)
	{
		m_isWitnessed_ = true; 
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
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = m_witnessEntityDetailLevelMap_.find(id);
	if(iter != m_witnessEntityDetailLevelMap_.end())
	{
		m_witnessEntities_[iter->second->detailLevel].erase(id);
		m_witnessEntityDetailLevelMap_.erase(iter);
	}
	else
	{
		ERROR_MSG("Entity[%s:%ld]::onRemoveWitness: can't not found %s %ld.\n", getScriptModuleName(), m_id_,
			entity->getScriptModuleName(), id);
	}
	
	if(m_witnessEntityDetailLevelMap_.size() <= 0)
	{
		m_isWitnessed_ = false;
		PyObject* pyResult = PyObject_CallMethod(this, "onWitnessed", "O", PyBool_FromLong(0));
		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			PyErr_Clear();
	}
}

//-------------------------------------------------------------------------------------
void Entity::onUpdateWitness(Entity* entity, const float& range)
{/*
	ENTITY_ID id = entity->getID();
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = m_witnessEntityDetailLevelMap_.find(id);
	if(iter != m_witnessEntityDetailLevelMap_.end())
	{
		WitnessInfo* witnessInfo = iter->second;
		int8 detailLevel = witnessInfo->detailLevel;
		int8 newDetailLevel = m_scriptModule_->getDetailLevel().getLevelByRange(detailLevel, range);

		// 详情级别改变了
		if(detailLevel != newDetailLevel)
		{
			DEBUG_MSG("Entity[%s:%ld]::onUpdateWitness:%s %ld change detailLevel %d to %d. oldRange=%f, newRange=%f.\n", getScriptModuleName(), m_id_,
					entity->getScriptModuleName(), id, detailLevel, newDetailLevel, witnessInfo->range, range);

			witnessInfo->detailLevel = newDetailLevel;
			onEntityDetailLevelChanged(witnessInfo, detailLevel, newDetailLevel);
			witnessInfo->detailLevelLog[newDetailLevel] = true;
		}

		witnessInfo->range = range;
	}
	else
	{
		ERROR_MSG("Entity[%s:%ld]::onUpdateWitness: can't not found %s %ld.\n", getScriptModuleName(), m_id_,
			entity->getScriptModuleName(), id);
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::onViewEntity(Entity* entity)
{
	// 将这个entity加入视野范围Entity列表
	m_viewEntities_[entity->getID()] = entity;
}

//-------------------------------------------------------------------------------------
void Entity::onLoseViewEntity(Entity* entity)
{/*
	ENTITY_ID eid = entity->getID();
	// 将这个entit从视野范围Entity列表删除
	std::map<ENTITY_ID, Entity*>::iterator iter = m_viewEntities_.find(eid);
	if(iter != m_viewEntities_.end())
		m_viewEntities_.erase(iter);
	
	// 从本entity的客户端中删除他
	if(entity->getScriptModule()->hasClient() && m_clientMailbox_ != NULL)
	{
		SocketPacket* sp = m_clientMailbox_->createMail(MAIL_TYPE_LOST_VIEW_ENTITY);
		(*sp) << eid;
		m_clientMailbox_->post(sp);
	}*/
}

//-------------------------------------------------------------------------------------
void Entity::onEntityInitDetailLevel(Entity* entity, int8 detailLevel)
{
	// 自身没有客户端部分则无需做相关操作
	if(!m_scriptModule_->hasClient())
		return;
	/*
	EntityMailbox* clientMailbox = entity->getClientMailbox();
	ENTITY_ID eid = entity->getID();
	SocketPacket* sp = new SocketPacket(OP_ENTITY_ENTER_WORLD);

	// 将这个entity的数据写入包中
	(*sp) << eid;
	(*sp) << (uint8)1;															// 表示向某客户端增加一个entity
	(*sp) << m_id_;
	(*sp) << this->ob_type->tp_name;
	(*sp) << m_position_.x << m_position_.y << m_position_.z;
	(*sp) << m_direction_.roll << m_direction_.pitch << m_direction_.yaw;
	
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
void Entity::onEntityDetailLevelChanged(WitnessInfo* witnessInfo, const int8& oldDetailLevel, const int8& newDetailLevel)
{/*
	// 自身没有客户端部分则无需做相关操作
	if(!m_scriptModule_->hasClient())
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
		(*sp) << m_id_;
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
				PropertyDescription* propertyDescription = m_scriptModule_->findCellPropertyDescription((*piter));
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
	PyObject* pyResult = PyObject_CallMethod(this, "onGetWitness", "");
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();

	m_hasWitness_ = true;
}

//-------------------------------------------------------------------------------------
void Entity::onLoseWitness(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, "onLoseWitness", "");
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();

	m_hasWitness_ = false;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyIsWitnessed(Entity *self, void *closure)
{
	return PyBool_FromLong(self->isWitnessed());
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyHasWitness(Entity *self, void *closure)
{
	return PyBool_FromLong(self->hasWitness());
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
	m_trapMgr_.addProximity(p);
	if(m_currChunk_ != NULL)
		m_currChunk_->getSpace()->placeProximity(m_currChunk_, p);
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
void Entity::delProximity(const uint16& id)
{
//	if(!m_trapMgr_.delProximity(id))
//		ERROR_MSG("Entity::delProximity: not found proximity %ld.\n", id);

	// 从chunk中清除这个陷阱
	//m_currChunk_->clearProximity(p);
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
void Entity::onEnterTrap(Entity* entity, const float& range, const int& controllerID)
{
	PyObject* pyResult = PyObject_CallMethod(this, "onEnterTrap", "Ofi", entity, range, controllerID);
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrap(Entity* entity, const float& range, const int& controllerID)
{
	PyObject* pyResult = PyObject_CallMethod(this, "onLeaveTrap", "Ofi", entity, range, controllerID);
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrapID(const ENTITY_ID& entityID, const float& range, const int& controllerID)
{
	PyObject* pyResult = PyObject_CallMethod(this, "onLeaveTrapID", "kfi", entityID, range, controllerID);
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::addTimer(uint32 startTrrigerIntervalTime, uint32 loopTrrigerIntervalTime, PyObject* args)
{return 0;
//	return PyLong_FromLong(m_timers_.addTimer(&m_TimerFunc_, startTrrigerIntervalTime, loopTrrigerIntervalTime, new TimerArgsPyObject(args)));
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddTimer(PyObject* self, PyObject* args, PyObject* kwds)
{
	Entity* entity = static_cast<Entity*>(self);
	uint32 startTrrigerIntervalTime = 0;
	uint32 loopTrrigerIntervalTime = 0;
	PyObject* timerArgs;

	if(PyTuple_Size(args) == 3)
	{
		if(!PyArg_ParseTuple(args, "k|k|O", &startTrrigerIntervalTime, &loopTrrigerIntervalTime, &timerArgs))
		{
			ERROR_MSG("Entity::addTimer: args is error!\n");
			return NULL;
		}
	}
	else
	{
		ERROR_MSG("Entity::addTimer: args require 3 args, gived %d! entity[%s:%ld].\n", 
			PyTuple_Size(args), entity->getScriptModuleName(), entity->getID());
		return NULL;
	}
	
	Py_INCREF(timerArgs);
	return entity->addTimer(startTrrigerIntervalTime, loopTrrigerIntervalTime, timerArgs);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::delTimer(TIMER_ID timerID)
{return 0;
//	return PyInt_FromLong(m_timers_.delTimer(timerID));
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
/*
void Entity::onTimer(const TIMER_ID& timerID, TimerArgsBase* args)
{
	PyObject* pyResult = PyObject_CallMethod(this, "onTimer", "IO", timerID, static_cast<TimerArgsPyObject*>(args)->getArgs());
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}*/

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
	m_position_ = pos; 
//	if(m_currChunk_ != NULL)
//		m_currChunk_->getSpace()->onEntityPositionChanged(this, m_currChunk_, m_position_);
}

//-------------------------------------------------------------------------------------
void Entity::setPositionAndDirection(Position3D& position, Direction3D& direction)
{
	m_position_ = position;
	m_direction_ = direction;
//	if(m_currChunk_ != NULL)
//		m_currChunk_->getSpace()->onEntityPositionChanged(this, m_currChunk_, m_position_);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::setAoiRadius(float radius, float hyst)
{/*
	if(!m_hasWitness_)
	{
		ERROR_MSG("Entity::setAoiRadius:%s %ld no has witness.\n", getScriptModuleName(), m_id_);
		return PyLong_FromLong(0);
	}

	if(m_clientMailbox_ == NULL || m_clientMailbox_ == Py_None)
		return PyLong_FromLong(0);

	m_aoiRadius_ = radius;
	m_aoiHysteresisArea_ = hyst;

	if(m_aoiRadius_ + m_aoiHysteresisArea_ > CELL_BORDER_WIDTH)
	{
		m_aoiRadius_ = CELL_BORDER_WIDTH - 15.0f;
		m_aoiHysteresisArea_ = 15.0f;
	}
	
	// 在space中投放一个AOI陷阱
	Proximity* p = new ProximityAOI(this, m_aoiRadius_, m_aoiHysteresisArea_);
	m_trapMgr_.addProximity(p);
	if(m_currChunk_ != NULL)
		m_currChunk_->getSpace()->placeProximity(m_currChunk_, p);
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
bool Entity::navigateStep(Position3D& destination, float& velocity, float& maxMoveDistance, float& maxDistance, 
	bool faceMovement, float& girth, PyObject* userData)
{
	DEBUG_MSG("Entity[%s:%d]:destination=(%f,%f,%f), velocity=%f, maxMoveDistance=%f, maxDistance=%f, faceMovement=%d, girth=%f, userData=%x.\n",
	getScriptModuleName(), m_id_, destination.x, destination.y, destination.z, velocity, maxMoveDistance, maxDistance, faceMovement, girth, userData);
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
		if(!PyArg_ParseTuple(args, "O|f|f|f|i|f|O", &pyDestination, &velocity, &maxMoveDistance, &maxDistance, &faceMovement, &girth, &userData))
		{
			ERROR_MSG("Entity::navigateStep: args is error!\n");
			Py_RETURN_FALSE;
		}
		
		// 将坐标信息提取出来
		script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	}
	else
	{
		ERROR_MSG("Entity::navigateStep: is error, args!=(destination, velocity, maxMoveDistance, maxDistance, faceMovement, girth, userData)!\n");
		Py_RETURN_FALSE;
	}
	
	Py_INCREF(userData);
	if(entity->navigateStep(destination, velocity, maxMoveDistance, maxDistance, faceMovement, girth, userData)){
		Py_RETURN_TRUE;
	}
	
	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
bool Entity::moveToPoint(Position3D& destination, float& velocity, PyObject* userData, bool faceMovement, bool moveVertically)
{
//	EntityMoveControllerMgr::addMovement(m_id_, new EntityMoveToPointController(this, destination, velocity, userData, faceMovement, moveVertically));
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
		ERROR_MSG("Entity::moveToPoint: is error, args!=(destination, velocity, maxMoveDistance, maxDistance, faceMovement, girth, userData)!\n");
		Py_RETURN_FALSE;
	}
	
	Py_INCREF(userData);
	if(entity->moveToPoint(destination, velocity, userData, faceMovement, moveVertically)){
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
	PyObject* pyResult = PyObject_CallMethod(this, "onMove", "O", userData);
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyEntitiesInRange(PyObject* self, PyObject* args, PyObject* kwds)
{
	PyObject* pyPosition = NULL;
	PyObject* pyEntityType = NULL;
	std::string entityType = "";
	float radius = 0.0f;
	Position3D pos;
	/*
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
