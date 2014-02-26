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
#include "profile.hpp"
#include "space.hpp"
#include "all_clients.hpp"
#include "controllers.hpp"	
#include "entity_range_node.hpp"
#include "proximity_controller.hpp"
#include "movetopoint_controller.hpp"	
#include "movetoentity_controller.hpp"	
#include "navigate_controller.hpp"	
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"	
#include "network/fixed_messages.hpp"
#include "client_lib/client_interface.hpp"
#include "server/eventhistory_stats.hpp"
#include "navigation/navmeshex.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"

#ifndef CODE_INLINE
#include "entity.ipp"
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
ENTITY_METHOD_DECLARE_BEGIN(Cellapp, Entity)
SCRIPT_METHOD_DECLARE("setAoiRadius",				pySetAoiRadius,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("isReal",						pyIsReal,						METH_VARARGS,				0)	
SCRIPT_METHOD_DECLARE("addProximity",				pyAddProximity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("cancel",						pyCancelController,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("canNavigate",				pycanNavigate,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("navigate",					pyNavigate,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("moveToPoint",				pyMoveToPoint,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("moveToEntity",				pyMoveToEntity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("entitiesInRange",			pyEntitiesInRange,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("entitiesInAOI",				pyEntitiesInAOI,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("raycast",					pyRaycast,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("teleport",					pyTeleport,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("destroySpace",				pyDestroySpace,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("debugAOI",					pyDebugAOI,						METH_VARARGS,				0)
ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

ENTITY_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GET_DECLARE("base",							pyGetBaseMailbox,				0,							0)
SCRIPT_GET_DECLARE("client",						pyGetClientMailbox,				0,							0)
SCRIPT_GET_DECLARE("allClients",					pyGetAllClients,				0,							0)
SCRIPT_GET_DECLARE("otherClients",					pyGetOtherClients,				0,							0)
SCRIPT_GET_DECLARE("isWitnessed",					pyIsWitnessed,					0,							0)
SCRIPT_GET_DECLARE("hasWitness",					pyHasWitness,					0,							0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,				0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,				0,		0)
SCRIPT_GETSET_DECLARE("yaw",						pyGetYaw,						pySetYaw,					0,		0)
SCRIPT_GETSET_DECLARE("roll",						pyGetRoll,						pySetRoll,					0,		0)
SCRIPT_GETSET_DECLARE("pitch",						pyGetPitch,						pySetPitch,					0,		0)
SCRIPT_GETSET_DECLARE("topSpeed",					pyGetTopSpeed,					pySetTopSpeed,				0,		0)
SCRIPT_GETSET_DECLARE("topSpeedY",					pyGetTopSpeedY,					pySetTopSpeedY,				0,		0)
SCRIPT_GETSET_DECLARE("shouldAutoBackup",			pyGetShouldAutoBackup,			pySetShouldAutoBackup,		0,		0)
ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, const ScriptDefModule* scriptModule):
ScriptObject(getScriptType(), true),
ENTITY_CONSTRUCTION(Entity),
clientMailbox_(NULL),
baseMailbox_(NULL),
posChangedTime_(0),
dirChangedTime_(0),
isReal_(true),
isOnGround_(false),
topSpeed_(-0.1f),
topSpeedY_(-0.1f),
witnesses_(),
pWitness_(NULL),
allClients_(new AllClients(scriptModule, id, false)),
otherClients_(new AllClients(scriptModule, id, true)),
pEntityRangeNode_(NULL),
pControllers_(new Controllers()),
pMoveController_(NULL)
{
	ENTITY_INIT_PROPERTYS(Entity);

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		pEntityRangeNode_ = new EntityRangeNode(this);
	}
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	ENTITY_DECONSTRUCTION(Entity);

	S_RELEASE(clientMailbox_);
	S_RELEASE(baseMailbox_);
	S_RELEASE(allClients_);
	S_RELEASE(otherClients_);
	
	if(pWitness_)
	{
		pWitness_->detach(this);
		Witness::ObjPool().reclaimObject(pWitness_);
		pWitness_ = NULL;
	}

	SAFE_RELEASE(pControllers_);
	SAFE_RELEASE(pEntityRangeNode_);
}	

//-------------------------------------------------------------------------------------
void Entity::installRangeNodes(RangeList* pRangeList)
{
	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
		pRangeList->insert((KBEngine::RangeNode*)pEntityRangeNode());
}

//-------------------------------------------------------------------------------------
void Entity::uninstallRangeNodes(RangeList* pRangeList)
{
	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
		pRangeList->remove((KBEngine::RangeNode*)pEntityRangeNode());
}

//-------------------------------------------------------------------------------------
void Entity::onDestroy(void)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

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
PyObject* Entity::__py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs)
{
	uint16 currargsSize = PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);

	if(currargsSize > 0)
	{
		PyErr_Format(PyExc_AssertionError,
						"%s: args max require %d args, gived %d! is script[%s].\n",	
			__FUNCTION__, 0, currargsSize, pobj->getScriptName());
		PyErr_PrintEx(0);
		return NULL;
	}

	pobj->destroyEntity();

	S_Return;
}																							

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDestroySpace()																		
{		
	if(getSpaceID() == 0)
	{
		PyErr_Format(PyExc_TypeError, "Entity::destroySpace: spaceID is 0.\n");
		PyErr_PrintEx(0);
		S_Return;
	}

	destroySpace();																					
	S_Return;																							
}	

//-------------------------------------------------------------------------------------
void Entity::destroySpace()
{
	if(getSpaceID() == 0)
		return;

	Spaces::destroySpace(getSpaceID(), this->getID());
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
PyObject* Entity::pyGetAllClients()
{ 
	AllClients* clients = getAllClients();
	if(clients == NULL)
		S_Return;

	Py_INCREF(clients);
	return clients; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetOtherClients()
{ 
	AllClients* clients = getOtherClients();
	if(clients == NULL)
		S_Return;

	Py_INCREF(clients);
	return clients; 
}

//-------------------------------------------------------------------------------------
int Entity::pySetTopSpeedY(PyObject *value)
{
	setTopSpeedY(float(PyFloat_AsDouble(value)) / g_kbeSrvConfig.gameUpdateHertz()); 
	return 0; 
};

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeedY()
{ 
	return PyFloat_FromDouble(getTopSpeedY() * g_kbeSrvConfig.gameUpdateHertz()); 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeed()
{ 
	return PyFloat_FromDouble(getTopSpeed() * g_kbeSrvConfig.gameUpdateHertz()); 
}

//-------------------------------------------------------------------------------------
int Entity::pySetTopSpeed(PyObject *value)
{ 
	setTopSpeed(float(PyFloat_AsDouble(value)) / g_kbeSrvConfig.gameUpdateHertz()); 
	return 0; 
}

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(const PropertyDescription* propertyDescription, PyObject* pyData)
{
	// 如果不是一个realEntity或者在初始化则不理会
	if(!isReal() || initing_)
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
	
	const Position3D& basePos = this->getPosition(); 
	if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) > 0)
	{
		DETAIL_TYPE propertyDetailLevel = propertyDescription->getDetailLevel();

		std::list<ENTITY_ID>::iterator witer = witnesses_.begin();
		for(; witer != witnesses_.end(); witer++)
		{
			Entity* pEntity = Cellapp::getSingleton().findEntity((*witer));
			if(pEntity == NULL || pEntity->pWitness() == NULL)
				continue;

			EntityMailbox* clientMailbox = pEntity->getClientMailbox();
			if(clientMailbox == NULL)
				continue;

			Mercury::Channel* pChannel = clientMailbox->getChannel();
			if(pChannel == NULL)
				continue;

			const Position3D& targetPos = pEntity->getPosition();
			Position3D lengthPos = targetPos - basePos;
			if(scriptModule_->getDetailLevel().level[propertyDetailLevel].inLevel(lengthPos.length()))
			{
				Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
				(*pForwardBundle).newMessage(ClientInterface::onUpdatePropertys);
				(*pForwardBundle) << getID();
				pForwardBundle->append(*mstream);
				
				// 记录这个事件产生的数据量大小
				g_publicClientEventHistoryStats.trackEvent(getScriptName(), 
					propertyDescription->getName(), 
					pForwardBundle->currMsgLength());

				Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity->getID(), (*pSendBundle), (*pForwardBundle));

				pEntity->pWitness()->sendToClient(ClientInterface::onUpdatePropertys, pSendBundle);
				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
			}
		}
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

				// 记录这个事件产生的数据量大小
				std::string event_name = this->getScriptName();
				event_name += ".";
				event_name += propertyDescription->getName();
				
				g_publicClientEventHistoryStats.add(getScriptName(), propertyDescription->getName(), pSendBundle->currMsgLength());
			}
		}
	}
	*/

	// 判断这个属性是否还需要广播给自己的客户端
	if((flags & ENTITY_BROADCAST_OWN_CLIENT_FLAGS) > 0 && clientMailbox_ != NULL && pWitness_)
	{
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
		(*pForwardBundle).newMessage(ClientInterface::onUpdatePropertys);
		(*pForwardBundle) << getID();
		pForwardBundle->append(*mstream);
		
		// 记录这个事件产生的数据量大小
		if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) <= 0)
		{
			g_privateClientEventHistoryStats.trackEvent(getScriptName(), 
				propertyDescription->getName(), 
				pForwardBundle->currMsgLength());
		}

		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(getID(), (*pSendBundle), (*pForwardBundle));

		pWitness_->sendToClient(ClientInterface::onUpdatePropertys, pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	}

	MemoryStream::ObjPool().reclaimObject(mstream);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	if(isDestroyed())																				
	{																										
		ERROR_MSG(boost::format("%1%::onRemoteMethodCall: %2% is destroyed!\n") %											
			getScriptName() % getID());

		s.read_skip(s.opsize());
		return;																							
	}

	ENTITY_METHOD_UID utype = 0;
	s >> utype;
	
	MethodDescription* md = scriptModule_->findCellMethodDescription(utype);
	
	DEBUG_MSG(boost::format("Entity::onRemoteMethodCall: %1%, %4%::%2%(utype=%3%).\n") % 
		id_ % (md ? md->getName() : "unknown") % utype % this->getScriptName());

	if(md == NULL)
	{
		ERROR_MSG(boost::format("Entity::onRemoteMethodCall: can't found method. utype=%1%, callerID:%2%.\n") 
			% utype % id_);

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
void Entity::addCellDataToStream(uint32 flags, MemoryStream* mstream)
{
	PyObject* cellData = PyObject_GetAttrString(this, "__dict__");

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =
					scriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellData, propertyDescription->getName());
			(*mstream) << propertyDescription->getUType();
			propertyDescription->getDataType()->addToStream(mstream, pyVal);

			if (PyErr_Occurred())
 			{	
				PyErr_PrintEx(0);
				DEBUG_MSG(boost::format("%1%::addCellDataToStream: %2% is error!\n") % this->getScriptName() % 
					propertyDescription->getName());
			}
		}
	}

	Py_XDECREF(cellData);
	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Entity::backupCellData()
{
	AUTO_SCOPED_PROFILE("backup");

	if(baseMailbox_ != NULL)
	{
		// 将当前的cell部分数据打包 一起发送给base部分备份
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onBackupEntityCellData);
		(*pBundle) << id_;

		MemoryStream* s = MemoryStream::ObjPool().createObject();
		addPositionAndDirectionToStream(*s);
		(*pBundle).append(s);
		MemoryStream::ObjPool().reclaimObject(s);

		s = MemoryStream::ObjPool().createObject();
		addCellDataToStream(ENTITY_CELL_DATA_FLAGS, s);
		(*pBundle).append(s);
		MemoryStream::ObjPool().reclaimObject(s);

		baseMailbox_->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
	else
	{
		WARNING_MSG(boost::format("Entity::backupCellData(): %1% %2% has no base!\n") % 
			this->getScriptName() % this->getID());
	}

	SCRIPT_ERROR_CHECK();
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
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	DEBUG_MSG(boost::format("%1%::onWriteToDB(): %2%.\n") % 
		this->getScriptName() % this->getID());

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onWriteToDB"));
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyIsReal()
{
	return PyBool_FromLong(isReal());
}

//-------------------------------------------------------------------------------------
void Entity::addWitnessed(Entity* entity)
{
	witnesses_.push_back(entity->getID());

	/*
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
	*/

	if(witnesses_.size() == 1)
	{
		SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onWitnessed"), 
			const_cast<char*>("O"), PyBool_FromLong(1));
	}
}

//-------------------------------------------------------------------------------------
void Entity::delWitnessed(Entity* entity)
{
	KBE_ASSERT(witnesses_.size() > 0);

	witnesses_.remove(entity->getID());

	if(witnesses_.size() == 0)
	{
		SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onWitnessed"), 
			const_cast<char*>("O"), PyBool_FromLong(0));
	}
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
uint32 Entity::addProximity(float range_xz, float range_y, int32 userarg)
{
	if(range_xz <= 0.0f || (RangeList::hasY && range_y <= 0.0f))
	{
		ERROR_MSG(boost::format("Entity::addProximity: range(xz=%1%, y=%2%) <= 0.0f! entity[%3%:%4%]\n") % 
			range_xz % range_y % getScriptName() % getID());

		return 0;
	}
	
	if(this->pEntityRangeNode() == NULL || this->pEntityRangeNode()->pRangeList() == NULL)
	{
		ERROR_MSG(boost::format("Entity::addProximity: %1% %2% not in world. pRangeList() == NULL\n") % 
			getScriptName() % getID());

		return 0;
	}

	// 在space中投放一个陷阱
	ProximityController* p = new ProximityController(this, range_xz, range_y, userarg, pControllers_->freeID());
	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddProximity(float range_xz, float range_y, int32 userarg)
{
	return PyLong_FromLong(addProximity(range_xz, range_y, userarg));
}

//-------------------------------------------------------------------------------------
void Entity::cancelController(uint32 id)
{
	if(!pControllers_->remove(id))
	{
		ERROR_MSG(boost::format("%1%::cancel: %2% not found %3%.\n") % 
			this->getScriptName() % this->getID() % id);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyCancelController(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);
	
	uint32 id = 0;
	PyObject* pystr = NULL;

	if(currargsSize != 1)
	{
		PyErr_Format(PyExc_AssertionError, "%s::cancel: args require 1 args, gived %d! is script[%s].\n",								
			pobj->getScriptName(), currargsSize);														
																																
		PyErr_PrintEx(0);																										
		return 0;																								
	}

	if(PyArg_ParseTuple(args, "I", &id) == -1 || PyArg_ParseTuple(args, "O", &pystr) == -1)
	{
		PyErr_Format(PyExc_TypeError, "%s::cancel: args is error!", pobj->getScriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(pystr && PyUnicode_Check(pystr))
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pystr, NULL);
		char* s = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		
		if(strcmp(s, "Movement") == 0)
		{
			pobj->stopMove();
		}

		free(s);

		S_Return;
	}
	else
	{
		if(!PyLong_Check(pystr))
		{
			PyErr_Format(PyExc_TypeError, "%s::cancel: args is error!", pobj->getScriptName());
			PyErr_PrintEx(0);
			return 0;
		}
	}

	pobj->cancelController(id);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::onEnterTrap(Entity* entity, float range_xz, float range_y, uint32 controllerID, int32 userarg)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS5(this, const_cast<char*>("onEnterTrap"), 
		const_cast<char*>("OffIi"), entity, range_xz, range_y, controllerID, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrap(Entity* entity, float range_xz, float range_y, uint32 controllerID, int32 userarg)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS5(this, const_cast<char*>("onLeaveTrap"), 
		const_cast<char*>("OffIi"), entity, range_xz, range_y, controllerID, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrapID(ENTITY_ID entityID, float range_xz, float range_y, uint32 controllerID, int32 userarg)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS5(this, const_cast<char*>("onLeaveTrapID"), 
		const_cast<char*>("kffIi"), entityID, range_xz, range_y, controllerID, userarg);
}

//-------------------------------------------------------------------------------------
int Entity::pySetPosition(PyObject *value)
{
	if(!script::ScriptVector3::check(value))
		return -1;

	Position3D pos;
	script::ScriptVector3::convertPyObjectToVector3(pos, value);
	setPosition(pos);

	static ENTITY_PROPERTY_UID posuid = 0;
	if(posuid == 0)
	{
		posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
		Mercury::FixedMessages::MSGInfo* msgInfo =
					Mercury::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;
	}

	static PropertyDescription positionDescription(posuid, "VECTOR3", "position", ED_FLAG_ALL_CLIENTS, false, DataTypes::getDataType("VECTOR3"), false, 0, "", DETAIL_LEVEL_FAR);
	onDefDataChanged(&positionDescription, value);
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
	setPosition_XZ_float(pChannel, float(x), float(z));
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XYZ_int(Mercury::Channel* pChannel, int32 x, int32 y, int32 z)
{
	setPosition_XYZ_float(pChannel, float(x), float(y), float(z));
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XZ_float(Mercury::Channel* pChannel, float x, float z)
{
	Position3D& pos = getPosition();
	if(almostEqual(x, pos.x) && almostEqual(z, pos.z))
		return;

	pos.x = x;
	pos.z = z;
	onPositionChanged();
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XYZ_float(Mercury::Channel* pChannel, float x, float y, float z)
{
	Position3D& pos = getPosition();
	if(almostEqual(x, pos.x) && almostEqual(y, pos.y) && almostEqual(z, pos.z))
		return;

	pos.x = x;
	pos.x = y;
	pos.z = z;
	onPositionChanged();
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

	if(!PyFloat_Check(pyItem))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", pyItem->ob_type->tp_name);
		PyErr_PrintEx(0);
		Py_DECREF(pyItem);
		return -1;
	}

	dir.roll	= float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	pyItem = PySequence_GetItem(value, 1);

	if(!PyFloat_Check(pyItem))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", pyItem->ob_type->tp_name);
		PyErr_PrintEx(0);
		Py_DECREF(pyItem);
		return -1;
	}

	dir.pitch	= float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	pyItem = PySequence_GetItem(value, 2);

	if(!PyFloat_Check(pyItem))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", pyItem->ob_type->tp_name);
		PyErr_PrintEx(0);
		Py_DECREF(pyItem);
		return -1;
	}

	dir.yaw		= float(PyFloat_AsDouble(pyItem));
	Py_DECREF(pyItem);

	static ENTITY_PROPERTY_UID diruid = 0;
	if(diruid == 0)
	{
		diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
		Mercury::FixedMessages::MSGInfo* msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)	
			diruid = msgInfo->msgid;
	}

	static PropertyDescription positionDescription(diruid, "VECTOR3", "direction", ED_FLAG_ALL_CLIENTS, false, DataTypes::getDataType("VECTOR3"), false, 0, "", DETAIL_LEVEL_FAR);
	onDefDataChanged(&positionDescription, value);

	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetDirection()
{
	return new script::ScriptVector3(getDirection().asVector3());
}

//-------------------------------------------------------------------------------------
int Entity::pySetYaw(PyObject *value)
{
	if(!PyFloat_Check(value))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", value->ob_type->tp_name);
		PyErr_PrintEx(0);
		return -1;
	}

	Direction3D& dir = getDirection();
	dir.yaw = float(PyFloat_AsDouble(value));

	PyObject* pydatas = pyGetDirection();
	pySetDirection(pydatas);
	Py_DECREF(pydatas);

	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetYaw()
{
	return PyFloat_FromDouble(getDirection().yaw);
}

//-------------------------------------------------------------------------------------
int Entity::pySetRoll(PyObject *value)
{
	if(!PyFloat_Check(value))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", value->ob_type->tp_name);
		PyErr_PrintEx(0);
		return -1;
	}

	Direction3D& dir = getDirection();
	dir.roll = float(PyFloat_AsDouble(value));

	PyObject* pydatas = pyGetDirection();
	pySetDirection(pydatas);
	Py_DECREF(pydatas);

	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetRoll()
{
	return PyFloat_FromDouble(getDirection().roll);
}

//-------------------------------------------------------------------------------------
int Entity::pySetPitch(PyObject *value)
{
	if(!PyFloat_Check(value))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", value->ob_type->tp_name);
		PyErr_PrintEx(0);
		return -1;
	}

	Direction3D& dir = getDirection();
	dir.pitch = float(PyFloat_AsDouble(value));

	PyObject* pydatas = pyGetDirection();
	pySetDirection(pydatas);
	Py_DECREF(pydatas);

	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetPitch()
{
	return PyFloat_FromDouble(getDirection().pitch);
}

//-------------------------------------------------------------------------------------
void Entity::setPositionAndDirection(const Position3D& position, const Direction3D& direction)
{
	setPosition(position);
	setDirection(direction);
}

//-------------------------------------------------------------------------------------
void Entity::onPositionChanged()
{
	posChangedTime_ = g_kbetime;
	if(this->pEntityRangeNode())
		this->pEntityRangeNode()->update();
}

//-------------------------------------------------------------------------------------
void Entity::onDirectionChanged()
{
	dirChangedTime_ = g_kbetime;
}

//-------------------------------------------------------------------------------------
void Entity::setWitness(Witness* pWitness)
{
	KBE_ASSERT(this->getBaseMailbox() != NULL && !this->hasWitness());
	pWitness_ = pWitness;
	pWitness_->attach(this);
}

//-------------------------------------------------------------------------------------
void Entity::onGetWitness(Mercury::Channel* pChannel)
{
	KBE_ASSERT(this->getBaseMailbox() != NULL);

	if(pWitness_ == NULL)
	{
		pWitness_ = Witness::ObjPool().createObject();
		pWitness_->attach(this);
	}

	Space* space = Spaces::findSpace(this->getSpaceID());
	if(space)
	{
		space->onEntityAttachWitness(this);
	}

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onGetWitness"));
}

//-------------------------------------------------------------------------------------
void Entity::onLoseWitness(Mercury::Channel* pChannel)
{
	KBE_ASSERT(this->getClientMailbox() != NULL && this->hasWitness());

	getClientMailbox()->addr(Mercury::Address::NONE);
	Py_DECREF(getClientMailbox());
	setClientMailbox(NULL);

	pWitness_->detach(this);
	Witness::ObjPool().reclaimObject(pWitness_);
	pWitness_ = NULL;

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLoseWitness"));
}

//-------------------------------------------------------------------------------------
void Entity::onResetWitness(Mercury::Channel* pChannel)
{
	INFO_MSG(boost::format("%1%::onResetWitness: %2%.\n") % 
		this->getScriptName() % this->getID());
}

//-------------------------------------------------------------------------------------
bool Entity::checkMoveForTopSpeed(const Position3D& position)
{
	Position3D movment = position - this->getPosition();
	bool move = true;
	
	// 检查移动
	if(topSpeedY_ > 0.01f && movment.y > topSpeedY_ + 0.5f)
	{
		move = false;
	}

	if(move && topSpeed_ > 0.01f)
	{
		movment.y = 0.f;
		
		if(movment.length() > topSpeed_ + 0.5f)
			move = false;
	}

	return move;
}

//-------------------------------------------------------------------------------------
void Entity::onUpdateDataFromClient(KBEngine::MemoryStream& s)
{
	Position3D pos;
	Direction3D dir;
	uint8 isOnGround = 0;
	
	s >> pos.x >> pos.y >> pos.z >> dir.yaw >> dir.pitch >> dir.roll >> isOnGround;
	isOnGround_ = isOnGround > 0;
	this->setDirection(dir);

	if(checkMoveForTopSpeed(pos))
	{
		this->setPosition(pos);
	}
	else
	{
		if(this->pWitness() == NULL)
			return;

		Position3D currpos = this->getPosition();
		Position3D movment = pos - currpos;
		float ydist = fabs(movment.y);
		movment.y = 0.f;

		DEBUG_MSG(boost::format("%1%::onUpdateDataFromClient: %2% position[(%3%,%4%,%5%) -> (%6%,%7%,%8%), (xzDist=%9%)>(topSpeed=%10%) || (yDist=%11%)>(topSpeedY=%12%)] invalid. reset client!\n") % 
			this->getScriptName() % this->getID() %
			this->getPosition().x % this->getPosition().y % this->getPosition().z %
			pos.x % pos.y % pos.z %
			movment.length() % topSpeed_ %
			ydist % topSpeedY_);
		
		// this->setPosition(currpos);

		// 通知重置
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

		(*pForwardBundle).newMessage(ClientInterface::onSetEntityPosAndDir);
		(*pForwardBundle) << getID();
		(*pForwardBundle) << currpos.x << currpos.y << currpos.z;
		(*pForwardBundle) << getDirection().yaw << getDirection().pitch << getDirection().roll;

		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(getID(), (*pSendBundle), (*pForwardBundle));
		this->pWitness()->sendToClient(ClientInterface::onSetEntityPosAndDir, pSendBundle);
		// Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	}
}

//-------------------------------------------------------------------------------------
int32 Entity::setAoiRadius(float radius, float hyst)
{
	if(pWitness_)
	{
		pWitness_->setAoiRadius(radius, hyst);
		return 1;
	}

	PyErr_Format(PyExc_AssertionError, "Entity::setAoiRadius: did not get witness.");
	PyErr_PrintEx(0);
	return -1;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pySetAoiRadius(float radius, float hyst)
{
	return PyLong_FromLong(setAoiRadius(radius, hyst));
}

//-------------------------------------------------------------------------------------
float Entity::getAoiRadius(void)const
{
	if(pWitness_)
		return pWitness_->aoiRadius();
		
	return 0.0; 
}

//-------------------------------------------------------------------------------------
float Entity::getAoiHystArea(void)const
{
	if(pWitness_)
		return pWitness_->aoiHysteresisArea();
		
	return 0.0; 
}

//-------------------------------------------------------------------------------------
bool Entity::stopMove()
{
	if(pMoveController_)
	{
		static_cast<MoveToPointController*>(pMoveController_)->destroyed();
		pMoveController_ = NULL;
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
int Entity::raycast(const Position3D& start, const Position3D& end, float* hitPos)
{
	Space* pSpace = Spaces::findSpace(getSpaceID());
	if(pSpace == NULL)
	{
		ERROR_MSG(boost::format("Entity::raycast: not found space(%1%), entityID(%2%)!\n") % 
			getSpaceID() % getID());

		return -1;
	}
	
	if(pSpace->pNavMeshHandle() == NULL)
	{
		ERROR_MSG(boost::format("Entity::raycast: space(%1%) not addSpaceGeometryMapping!\n") % 
			getSpaceID() % getID());

		return -1;
	}

	return pSpace->pNavMeshHandle()->raycast(start, end, hitPos);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyRaycast(PyObject_ptr pyStartPos, PyObject_ptr pyEndPos)
{
	if(!PySequence_Check(pyStartPos))
	{
		PyErr_Format(PyExc_TypeError, "Entity::raycast: args1(startPos) not is PySequence!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(!PySequence_Check(pyEndPos))
	{
		PyErr_Format(PyExc_TypeError, "Entity::raycast: args2(endPos) not is PySequence!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyStartPos) != 3)
	{
		PyErr_Format(PyExc_TypeError, "Entity::raycast: args1(startPos) invalid!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyEndPos) != 3)
	{
		PyErr_Format(PyExc_TypeError, "Entity::raycast: args2(endPos) invalid!");
		PyErr_PrintEx(0);
		return 0;
	}

	Position3D startPos;
	Position3D endPos;
	float hitPos[3];

	script::ScriptVector3::convertPyObjectToVector3(startPos, pyStartPos);
	script::ScriptVector3::convertPyObjectToVector3(endPos, pyEndPos);
	if(raycast(startPos, endPos, hitPos) <= 0)
	{
		S_Return;
	}

	PyObject* pyHitpos = PyTuple_New(3);
	PyTuple_SetItem(pyHitpos, 0, ::PyFloat_FromDouble(hitPos[0]));
	PyTuple_SetItem(pyHitpos, 1, ::PyFloat_FromDouble(hitPos[1]));
	PyTuple_SetItem(pyHitpos, 2, ::PyFloat_FromDouble(hitPos[2]));
	return pyHitpos;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pycanNavigate()
{
	if(canNavigate())
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
bool Entity::canNavigate()
{
	if(getSpaceID() <= 0)
		return false;

	Space* pSpace = Spaces::findSpace(getSpaceID());
	if(pSpace == NULL)
		return false;

	if(pSpace->pNavMeshHandle() == NULL)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
uint32 Entity::navigate(const Position3D& destination, float velocity, float range, float maxMoveDistance, float maxDistance, 
	bool faceMovement, float girth, PyObject* userData)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	NavigateController* p = new NavigateController(this, destination, velocity, 
		range, faceMovement, maxMoveDistance, maxDistance, girth, userData, pControllers_->freeID());

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;

	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyNavigate(PyObject_ptr pyDestination, float velocity, float range, float maxMoveDistance, float maxDistance,
								 int8 faceMovement, float girth, PyObject_ptr userData)
{

	Position3D destination;

	if(!PySequence_Check(pyDestination))
	{
		PyErr_Format(PyExc_TypeError, "Entity::navigate: args1(position) not is PySequence!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyDestination) != 3)
	{
		PyErr_Format(PyExc_TypeError, "Entity::navigate: args1(position) invalid!");
		PyErr_PrintEx(0);
		return 0;
	}

	// 将坐标信息提取出来
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	Py_INCREF(userData);

	return PyLong_FromLong(navigate(destination, velocity, range, maxMoveDistance, 
		maxDistance, faceMovement > 0, girth, userData));
}

//-------------------------------------------------------------------------------------
uint32 Entity::moveToPoint(const Position3D& destination, float velocity, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	MoveToPointController* p = new MoveToPointController(this, destination, velocity, 
		0.0f, faceMovement, moveVertically, userData, pControllers_->freeID());

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;

	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToPoint(PyObject_ptr pyDestination, float velocity, PyObject_ptr userData,
								 int32 faceMovement, int32 moveVertically)
{
	Position3D destination;

	if(!PySequence_Check(pyDestination))
	{
		PyErr_Format(PyExc_TypeError, "Entity::moveToPoint: args1(position) not is PySequence!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyDestination) != 3)
	{
		PyErr_Format(PyExc_TypeError, "Entity::moveToPoint: args1(position) invalid!");
		PyErr_PrintEx(0);
		return 0;
	}

	// 将坐标信息提取出来
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);
	Py_INCREF(userData);

	return PyLong_FromLong(moveToPoint(destination, velocity, userData, faceMovement > 0, moveVertically > 0));
}

//-------------------------------------------------------------------------------------
uint32 Entity::moveToEntity(ENTITY_ID targetID, float velocity, float range, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	MoveToEntityController* p = new MoveToEntityController(this, targetID, velocity, range,
		faceMovement, moveVertically, userData, pControllers_->freeID());

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToEntity(ENTITY_ID targetID, float velocity, float range, PyObject_ptr userData,
								 int32 faceMovement, int32 moveVertically)
{
	Py_INCREF(userData);
	return PyLong_FromLong(moveToEntity(targetID, velocity, range, userData, faceMovement > 0, moveVertically > 0));
}

//-------------------------------------------------------------------------------------
void Entity::onMove(uint32 controllerId, PyObject* userarg)
{
	SCOPED_PROFILE(ONMOVE_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS2(this, const_cast<char*>("onMove"), 
		const_cast<char*>("IO"), controllerId, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::onMoveOver(uint32 controllerId, PyObject* userarg)
{
	pMoveController_ = NULL;
	SCRIPT_OBJECT_CALL_ARGS2(this, const_cast<char*>("onMoveOver"), 
		const_cast<char*>("IO"), controllerId, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::onMoveFailure(uint32 controllerId, PyObject* userarg)
{
	pMoveController_ = NULL;
	SCRIPT_OBJECT_CALL_ARGS2(this, const_cast<char*>("onMoveFailure"), 
		const_cast<char*>("IO"), controllerId, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::debugAOI()
{
	if(pWitness_ == NULL)
	{
		ERROR_MSG(boost::format("%1%::debugAOI: %2% has no witness!\n") % getScriptName() % this->getID());
		return;
	}
	
	INFO_MSG(boost::format("%1%::debugAOI: %2% size=%3%\n") % getScriptName() % this->getID() % 
		pWitness_->aoiEntities().size());

	Witness::AOI_ENTITIES::iterator iter = pWitness_->aoiEntities().begin();
	for(; iter != pWitness_->aoiEntities().end(); iter++)
	{
		Entity* pEntity = (*iter)->pEntity();
		Position3D epos;
		float dist = 0.0f;

		if(pEntity)
		{
			epos = pEntity->getPosition();
			Vector3 distvec = epos - this->getPosition();
			dist = KBEVec3Length(&distvec);
		}

		INFO_MSG(boost::format("%8%::debugAOI: %1% %2%(%3%), position(%4%.%5%.%6%), dist=%7%\n") % 
			this->getID() % 
			(pEntity != NULL ? pEntity->getScriptName() : "unknown") % 
			(*iter)->id() % 
			epos.x % epos.y % epos.z %
			dist % 
			this->getScriptName());
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDebugAOI()
{
	debugAOI();
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyEntitiesInAOI()
{
	PyObject* pyList = PyList_New(pWitness_->aoiEntities().size());

	Witness::AOI_ENTITIES::iterator iter = pWitness_->aoiEntities().begin();
	int i = 0;
	for(; iter != pWitness_->aoiEntities().end(); iter++)
	{
		Entity* pEntity = (*iter)->pEntity();

		if(pEntity)
		{
			Py_INCREF(pEntity);
			PyList_SET_ITEM(pyList, i++, pEntity);
		}
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyEntitiesInRange(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);
	PyObject* pyPosition = NULL, *pyEntityType = NULL;
	float radius = 0.f;

	if(currargsSize == 1)
	{
		if(PyArg_ParseTuple(args, "f", &radius) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else if(currargsSize == 2)
	{
		if(PyArg_ParseTuple(args, "fO", &radius, &pyEntityType) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args is error!");
			PyErr_PrintEx(0);
			return 0;
		}

		if(pyEntityType != Py_None && !PyUnicode_Check(pyEntityType))
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args(entityType) is error!");
			PyErr_PrintEx(0);
			return 0;
		}

	}
	else if(currargsSize == 3)
	{
		if(PyArg_ParseTuple(args, "fOO", &radius, &pyEntityType, &pyPosition) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args is error!");
			PyErr_PrintEx(0);
			return 0;
		}
		
		if(pyEntityType != Py_None && !PyUnicode_Check(pyEntityType))
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args(entityType) is error!");
			PyErr_PrintEx(0);
			return 0;
		}

		if(!PySequence_Check(pyPosition) || PySequence_Size(pyPosition) < 3)
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args(position) is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* pEntityType = NULL;
	Position3D orginpos;
	
	// 将坐标信息提取出来
	if(pyPosition)
	{
		script::ScriptVector3::convertPyObjectToVector3(orginpos, pyPosition);
	}
	else
	{
		orginpos = pobj->getPosition();
	}

	if(pyEntityType && pyEntityType != Py_None)
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyEntityType, NULL);
		pEntityType = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
	}
	
	int entityUType = -1;
	
	if(pEntityType)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(pEntityType);
		if(sm == NULL)
		{
			free(pEntityType);
			return PyList_New(0);
		}

		free(pEntityType);
		entityUType = sm->getUType();
	}
	
	std::vector<Entity*> findentities;

	// 用户总是期望在entity附近搜寻， 因此我们从身边搜索
	EntityRangeNode::entitiesInRange(findentities,  pobj->pEntityRangeNode(), orginpos, radius, entityUType);

	PyObject* pyList = PyList_New(findentities.size());

	std::vector<Entity*>::iterator iter = findentities.begin();
	int i = 0;
	for(; iter != findentities.end(); iter++)
	{
		Entity* pEntity = (*iter);

		Py_INCREF(pEntity);
		PyList_SET_ITEM(pyList, i++, pEntity);
	}

	return pyList;
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
	DEBUG_MSG(boost::format("%1%::teleportFromBaseapp: %2%, targetEntityID=%3%, cell=%4%, sourceBaseAppID=%5%.\n") % 
		this->getScriptName() % this->getID() % targetEntityID % cellAppID % sourceBaseAppID);
	
	SPACE_ID lastSpaceID = this->getSpaceID();

	// 如果不在一个cell上
	if(cellAppID != g_componentID)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cellAppID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG(boost::format("%1%::teleportFromBaseapp: %2%, teleport is error, not found cellapp, targetEntityID, cellAppID=%3%.\n") %
				this->getScriptName() % this->getID() % targetEntityID % cellAppID);

			_sendBaseTeleportResult(this->getID(), sourceBaseAppID, 0, lastSpaceID);
			return;
		}
	}
	else
	{
		Entity* entity = Cellapp::getSingleton().findEntity(targetEntityID);
		if(entity == NULL || entity->isDestroyed())
		{
			ERROR_MSG(boost::format("%1%::teleportFromBaseapp: %2%, can't found targetEntity(%3%).\n") %
				this->getScriptName() % this->getID() % targetEntityID);

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
				ERROR_MSG(boost::format("%1%::teleportFromBaseapp: %2%, can't found space(%3%).\n") %
					this->getScriptName() % this->getID() % spaceID);

				_sendBaseTeleportResult(this->getID(), sourceBaseAppID, 0, lastSpaceID);
				return;
			}
			
			Space* currspace = Spaces::findSpace(this->getSpaceID());
			currspace->removeEntity(this);
			space->addEntityAndEnterWorld(this);
			_sendBaseTeleportResult(this->getID(), sourceBaseAppID, spaceID, lastSpaceID);
		}
		else
		{
			WARNING_MSG(boost::format("%1%::teleportFromBaseapp: %2% targetSpace(%3%) == currSpaceID(%4%).\n") %
				this->getScriptName() % this->getID() % spaceID % this->getSpaceID());

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
		return 0;
	}

	if(!PySequence_Check(pydirection) || PySequence_Size(pydirection) != 3)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d direction not is Sequence!\n", getScriptName(), getID());
		PyErr_PrintEx(0);
		return 0;
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
				space->addEntityAndEnterWorld(this);
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
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleport"));
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportFailure()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportFailure"));
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportSuccess(PyObject* nearbyEntity, SPACE_ID lastSpaceID)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onTeleportSuccess"), 
		const_cast<char*>("O"), nearbyEntity);
}

//-------------------------------------------------------------------------------------
void Entity::onEnteredCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEnteredCell"));
}

//-------------------------------------------------------------------------------------
void Entity::onEnteringCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEnteringCell"));
}

//-------------------------------------------------------------------------------------
void Entity::onLeavingCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLeavingCell"));
}

//-------------------------------------------------------------------------------------
void Entity::onLeftCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLeftCell"));
}

//-------------------------------------------------------------------------------------
void Entity::onRestore()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onRestore"));
}

//-------------------------------------------------------------------------------------
int Entity::pySetShouldAutoBackup(PyObject *value)
{
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	if(!PyLong_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d set shouldAutoBackup value is not int!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;	
	}

	shouldAutoBackup_ = (int8)PyLong_AsLong(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetShouldAutoBackup()
{
	return PyLong_FromLong(shouldAutoBackup_);
}

//-------------------------------------------------------------------------------------
bool Entity::_reload(bool fullReload)
{
	allClients_->setScriptModule(scriptModule_);
	return true;
}

//-------------------------------------------------------------------------------------
}
