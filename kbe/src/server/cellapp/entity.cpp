/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "cellapp.h"
#include "entity.h"
#include "witness.h"	
#include "profile.h"
#include "space.h"
#include "range_trigger.h"
#include "all_clients.h"
#include "client_entity.h"
#include "controllers.h"	
#include "real_entity_method.h"
#include "entity_coordinate_node.h"
#include "proximity_controller.h"
#include "move_controller.h"	
#include "moveto_point_handler.h"	
#include "moveto_entity_handler.h"	
#include "navigate_handler.h"	
#include "rotator_handler.h"
#include "turn_controller.h"
#include "pyscript/py_gc.h"
#include "entitydef/volatileinfo.h"
#include "entitydef/entity_mailbox.h"
#include "network/channel.h"	
#include "network/bundle.h"	
#include "network/fixed_messages.h"
#include "network/network_stats.h"
#include "client_lib/client_interface.h"
#include "helper/eventhistory_stats.h"
#include "navigation/navigation.h"
#include "math/math.h"

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"

#ifndef CODE_INLINE
#include "entity.inl"
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
ENTITY_METHOD_DECLARE_BEGIN(Cellapp, Entity)
SCRIPT_METHOD_DECLARE("setAoiRadius",				pySetAoiRadius,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("getAoiRadius",				pyGetAoiRadius,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("getAoiHystArea",				pyGetAoiHystArea,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("isReal",						pyIsReal,						METH_VARARGS,				0)	
SCRIPT_METHOD_DECLARE("addProximity",				pyAddProximity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("addYawRotator",				pyAddYawRotator,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("clientEntity",				pyClientEntity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("cancelController",			pyCancelController,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("canNavigate",				pycanNavigate,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("navigatePathPoints",			pyNavigatePathPoints,			METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("navigate",					pyNavigate,						METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("getRandomPoints",			pyGetRandomPoints,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("moveToPoint",				pyMoveToPoint,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("moveToEntity",				pyMoveToEntity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("accelerate",					pyAccelerate,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("entitiesInRange",			pyEntitiesInRange,				METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("entitiesInAOI",				pyEntitiesInAOI,				METH_VARARGS,				0)
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
SCRIPT_GET_DECLARE("isOnGround",					pyGetIsOnGround,				0,							0)
SCRIPT_GET_DECLARE("spaceID",						pyGetSpaceID,					0,							0)
SCRIPT_GETSET_DECLARE("layer",						pyGetLayer,						pySetLayer,					0,		0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,				0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,				0,		0)
SCRIPT_GETSET_DECLARE("topSpeed",					pyGetTopSpeed,					pySetTopSpeed,				0,		0)
SCRIPT_GETSET_DECLARE("topSpeedY",					pyGetTopSpeedY,					pySetTopSpeedY,				0,		0)
SCRIPT_GETSET_DECLARE("controlledBy",				pyGetControlledBy,				pySetControlledBy,			0,		0)
SCRIPT_GETSET_DECLARE("volatileInfo",				pyGetVolatileinfo,				pySetVolatileinfo,			0,		0)
ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)	

Entity::BufferedScriptCallArray Entity::_scriptCallbacksBuffer;
int32 Entity::_scriptCallbacksBufferCount = 0;
int32 Entity::_scriptCallbacksBufferNum = 0;

//-------------------------------------------------------------------------------------
Entity::Entity(ENTITY_ID id, const ScriptDefModule* pScriptModule):
ScriptObject(getScriptType(), true),
ENTITY_CONSTRUCTION(Entity),
clientMailbox_(NULL),
baseMailbox_(NULL),
realCell_(0),
ghostCell_(0),
lastpos_(),
position_(),
pPyPosition_(NULL),
direction_(),
pPyDirection_(NULL),
posChangedTime_(0),
dirChangedTime_(0),
isOnGround_(false),
topSpeed_(-0.1f),
topSpeedY_(-0.1f),
witnesses_(),
witnesses_count_(0),
pWitness_(NULL),
allClients_(new AllClients(pScriptModule, id, false)),
otherClients_(new AllClients(pScriptModule, id, true)),
pEntityCoordinateNode_(NULL),
pControllers_(new Controllers(id)),
pyPositionChangedCallback_(),
pyDirectionChangedCallback_(),
layer_(0),
isDirty_(true),
pCustomVolatileinfo_(NULL)
{
	pyPositionChangedCallback_ = std::tr1::bind(&Entity::onPyPositionChanged, this);
	pyDirectionChangedCallback_ = std::tr1::bind(&Entity::onPyDirectionChanged, this);
	pPyPosition_ = new script::ScriptVector3(&position(), &pyPositionChangedCallback_);
	pPyDirection_ = new script::ScriptVector3(&direction().dir, &pyDirectionChangedCallback_);

	ENTITY_INIT_PROPERTYS(Entity);

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		pEntityCoordinateNode_ = new EntityCoordinateNode(this);
	}

	script::PyGC::incTracing("Entity");
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	ENTITY_DECONSTRUCTION(Entity);

	S_RELEASE(pCustomVolatileinfo_);

	S_RELEASE(clientMailbox_);
	S_RELEASE(baseMailbox_);
	S_RELEASE(allClients_);
	S_RELEASE(otherClients_);
	
	KBE_ASSERT(pWitness_ == NULL);

	SAFE_RELEASE(pControllers_);
	KBE_ASSERT(pEntityCoordinateNode_ == NULL);

	Py_DECREF(pPyPosition_);
	pPyPosition_ = NULL;
	
	Py_DECREF(pPyDirection_);
	pPyDirection_ = NULL;

	if(Cellapp::getSingleton().pEntities())
		Cellapp::getSingleton().pEntities()->pGetbages()->erase(id());

	script::PyGC::decTracing("Entity");
}	

//-------------------------------------------------------------------------------------
void Entity::installCoordinateNodes(CoordinateSystem* pCoordinateSystem)
{
	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
		pCoordinateSystem->insert((KBEngine::CoordinateNode*)pEntityCoordinateNode());
}

//-------------------------------------------------------------------------------------
void Entity::uninstallCoordinateNodes(CoordinateSystem* pCoordinateSystem)
{
	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		pCoordinateSystem->remove((KBEngine::CoordinateNode*)pEntityCoordinateNode());
		pEntityCoordinateNode_ = new EntityCoordinateNode(this);
	}
}

//-------------------------------------------------------------------------------------
void Entity::onCoordinateNodesDestroy(EntityCoordinateNode* pEntityCoordinateNode)
{
	if (pEntityCoordinateNode_ == pEntityCoordinateNode)
		pEntityCoordinateNode_ = NULL;
}

//-------------------------------------------------------------------------------------
void Entity::onDestroy(bool callScript)
{
	if(callScript && isReal())
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onDestroy"));
		
		// �����֪ͨ�ű��� ��ôҲ�����������ص�
		// ͨ������һ��entity��֪ͨ�ű�������Ǩ�ƻ��ߴ�����ɵ�
		if(baseMailbox_ != NULL)
		{
			setDirty();
			this->backupCellData();

			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			(*pBundle).newMessage(BaseappInterface::onLoseCell);
			(*pBundle) << id_;
			baseMailbox_->postMail(pBundle);
		}
	}

	stopMove();

	// ��������ߵ�����
	S_RELEASE(controlledBy_);

	if(pWitness_)
	{
		pWitness_->detach(this);
		Witness::reclaimPoolObject(pWitness_);
		pWitness_ = NULL;
	}

	// ��entity�ӳ������޳�
	Space* space = Spaces::findSpace(this->spaceID());
	if(space)
	{
		space->removeEntity(this);
	}
	else
	{
		WARNING_MSG(fmt::format("{}::onDestroy(): {}, not found space({})!\n", 
			this->scriptName(), this->id(), spaceID()));
	}
	
	// �ڽ���ǿ�ƹر�ʱ������ܲ�Ϊ0
	//KBE_ASSERT(spaceID() == 0);

	// ��ʱ��Ӧ�û���witnesses������ΪAOI BUG
	if (witnesses_count_ > 0)
	{
		ERROR_MSG(fmt::format("{}::onDestroy(): id={}, witnesses_count({}/{}) != 0, isReal={}, spaceID={}, position=({},{},{})\n", 
			scriptName(), id(), witnesses_count_, witnesses_.size(), isReal(), this->spaceID(), position().x, position().y, position().z));

		std::list<ENTITY_ID> witnesses_copy = witnesses_;
		std::list<ENTITY_ID>::iterator it = witnesses_copy.begin();
		for (; it != witnesses_copy.end(); ++it)
		{
			Entity *ent = Cellapp::getSingleton().findEntity((*it));

			if (ent)
			{
				bool inTargetAOI = false;

				if (ent->pWitness())
				{
					Witness::AOI_ENTITIES::iterator aoi_iter = ent->pWitness()->aoiEntities().begin();
					for (; aoi_iter != ent->pWitness()->aoiEntities().end(); ++aoi_iter)
					{
						if ((*aoi_iter)->pEntity() == this)
						{
							inTargetAOI = true;
							ent->pWitness()->_onLeaveAOI((*aoi_iter));
							break;
						}
					}
				}
				else
				{
					ent->delWitnessed(this);
				}
				
				ERROR_MSG(fmt::format("\t=>witnessed={}({}), isDestroyed={}, isReal={}, inTargetAOI={}, spaceID={}, position=({},{},{})\n", 
					ent->scriptName(), (*it), ent->isDestroyed(), ent->isReal(), inTargetAOI, ent->spaceID(), ent->position().x, ent->position().y, ent->position().z));
			}
			else
			{
				ERROR_MSG(fmt::format("\t=> witnessed={}, not found entity!\n", (*it)));
			}
			
			witnesses_count_ = 0;
			witnesses_.clear();
		}

		//KBE_ASSERT(witnesses_count_ == 0);
	}

	pPyPosition_->onLoseRef();
	pPyDirection_->onLoseRef();

	SAFE_RELEASE(pEntityCoordinateNode_);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs)
{
	uint16 currargsSize = PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);

	if(pobj->initing())
	{
		PyErr_Format(PyExc_AssertionError,
			"%s::destroy(): %d initing, reject the request!\n",
			pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return NULL;
	}
	else if (pobj->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::destroy: %d is destroyed!\n",
			pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return NULL;
	}
	else if(currargsSize > 0)
	{
		PyErr_Format(PyExc_AssertionError,
			"%s: args max require %d args, gived %d! is script(%s), id(%d)!\n",	
			__FUNCTION__, 0, currargsSize, pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return NULL;
	}

	pobj->destroyEntity();

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDestroySpace()
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::destroySpace: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::destroySpace: %d is destroyed!\n",
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}
	
	if(spaceID() == 0)
	{
		PyErr_Format(PyExc_TypeError, "%s::destroySpace: spaceID is 0.\n", scriptName());
		PyErr_PrintEx(0);
		S_Return;
	}

	destroySpace();
	S_Return;
}	

//-------------------------------------------------------------------------------------
void Entity::destroySpace()
{
	if(spaceID() == 0)
		return;

	Spaces::destroySpace(spaceID(), this->id());
}

//-------------------------------------------------------------------------------------
void Entity::onSpaceGone()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onSpaceGone"));	
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetBaseMailbox()
{ 
	EntityMailbox* mailbox = baseMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetControlledBy()
{
	EntityMailbox* mailbox = controlledBy();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
int Entity::pySetControlledBy(PyObject *value)
{
	if (isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	if (!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::controlledBy: is not real entity(%d).",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	EntityMailbox* mailbox = NULL;

	if (value != Py_None)
	{
		if (!PyObject_TypeCheck(value, EntityMailbox::getScriptType()) || !((EntityMailbox *)value)->isBase())
		{
			PyErr_Format(PyExc_AssertionError, "%s: param must be base entity mailbox!\n",
				scriptName());
			PyErr_PrintEx(0);
			return 0;
		}

		mailbox = static_cast<EntityMailbox *>(value);

		// ����������ң��Ͳ�Ҫ������
		if (!entityInWitnessed(mailbox->id()) && mailbox->id() != id())
		{
			PyErr_Format(PyExc_AssertionError, "%s: entity '%d' can't witnessed me!\n",
				scriptName(), mailbox->id());
			PyErr_PrintEx(0);
			return 0;
		}

		Entity *ent = Cellapp::getSingleton().findEntity(mailbox->id());
		if (!ent || !ent->clientMailbox())
		{
			PyErr_Format(PyExc_AssertionError, "%s: entity(%d) mailbox has no 'client' mailbox!\n",
				scriptName());
			PyErr_PrintEx(0);
			return 0;
		}
	}

	setControlledBy(mailbox);
	return 0;
}

bool Entity::setControlledBy(EntityMailbox* controllerBaseMailbox)
{
	EntityMailbox *oldMailbox = controlledBy();

	//  ����¾ɵ�mailbox��ͬһ���ˣ������κθ���
	if (oldMailbox != NULL && controllerBaseMailbox != NULL &&
		oldMailbox->id() == controllerBaseMailbox->id())
	{
		ERROR_MSG(fmt::format("Entity {0} is already a controller, don't repeat settings\n", oldMailbox->id()));
		return false;
	}

	if (oldMailbox != NULL)
	{
		// ����ɵĿ����������Լ��Ŀͻ��ˣ�
		// �Ǿ���Ҫ֪ͨ�Լ��Ŀͻ��ˣ��㲻���ٿ������Լ��ˣ�Ҳ�����㱻�����˿�����
		if (oldMailbox->id() == id())
			sendControlledByStatusMessage(oldMailbox, 1);

		// ����ɵĿ�����Ҳ���ҵĹ۲���֮һ���Ǿͱ�ʾ���Ŀͻ����ܿ����ң�
		// ���ԣ���Ҫ֪ͨ�ɵĿͻ��ˣ��㲻���ٿ���ĳ�˵�λ����
		else if (entityInWitnessed(oldMailbox->id()))
			sendControlledByStatusMessage(oldMailbox, 0);

		if (controllerBaseMailbox != NULL)
		{
			controlledBy(controllerBaseMailbox);

			// ����ǻָ����ҿ��ƣ���ô��Ҫ֪ͨ�ҵĿͻ��ˣ�û���˿�������
			if (controllerBaseMailbox->id() == id())
			{
				KBE_ASSERT(clientMailbox_);
				sendControlledByStatusMessage(controllerBaseMailbox, 0);
			}

			// ����Ǳ��˽����˿��ƣ���ôֻ��Ҫ֪ͨ�����߼��ɣ�
			//     ������Ϊ֮ǰ�Լ����Ǳ����˿����ŵģ����Բ���Ҫ����֪ͨ��
			// ���ԣ�֪ͨ���ֵĿ����ߣ��������˭
			else
			{
				sendControlledByStatusMessage(controllerBaseMailbox, 1);
			}

		}
		else  // NULL��ʾ����ϵͳ���ƣ����Բ���Ҫ֪ͨ������
		{
			controlledBy(NULL);
		}
	}
	else if (controllerBaseMailbox != NULL)
	{
		controlledBy(controllerBaseMailbox);
		
		// ��Ȼ���µĿ������ˣ�ϵͳ���ƶ���ΪҲ�ͱ���ֹͣ��
		stopMove();
		
		// ����ǻָ����ҿ��ƣ���ô��Ҫ֪ͨ�ҵĿͻ��ˣ�û���˿�������
		if (controllerBaseMailbox->id() == id())
		{
			KBE_ASSERT(clientMailbox_);
			sendControlledByStatusMessage(controllerBaseMailbox, 0);
		}

		// ����Ǳ��˽����˿��ƣ���ôֻ��Ҫ֪ͨ�����߼��ɣ�
		//     ������Ϊ֮ǰ�Լ����Ǳ����˿����ŵģ����Բ���Ҫ����֪ͨ��
		// ���ԣ�֪ͨ���ֵĿ����ߣ��������˭
		else
		{
			sendControlledByStatusMessage(controllerBaseMailbox, 1);
		}
	}

	return true;
}

void Entity::sendControlledByStatusMessage(EntityMailbox* baseMailbox, int8 isControlled)
{
	KBE_ASSERT(baseMailbox);

	Network::Channel* pChannel = NULL;

	PyObject* clientMB = PyObject_GetAttrString(baseMailbox, "client");
	if (clientMB != Py_None)
	{
		pChannel = (static_cast<EntityMailbox*>(clientMB))->getChannel();
	}

	Py_DECREF(clientMB);

	if (!pChannel)
		return;

	Network::Bundle* pSendBundle = Network::Bundle::ObjPool().createObject();
	Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();

	(*pForwardBundle).newMessage(ClientInterface::onControlEntity);
	(*pForwardBundle) << id();
	(*pForwardBundle) << isControlled;

	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(baseMailbox->id(), (*pSendBundle), (*pForwardBundle));
	pChannel->send(pSendBundle);
	Network::Bundle::ObjPool().reclaimObject(pForwardBundle);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetClientMailbox()
{ 
	EntityMailbox* mailbox = clientMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetAllClients()
{ 
	AllClients* clients = allClients();
	if(clients == NULL)
		S_Return;

	Py_INCREF(clients);
	return clients; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetOtherClients()
{ 
	AllClients* clients = otherClients();
	if(clients == NULL)
		S_Return;

	Py_INCREF(clients);
	return clients; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetIsOnGround()
{ 
	return PyBool_FromLong(isOnGround());
}

//-------------------------------------------------------------------------------------
int Entity::pySetTopSpeedY(PyObject *value)
{
	topSpeedY(float(PyFloat_AsDouble(value)) / g_kbeSrvConfig.gameUpdateHertz()); 
	return 0; 
};

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeedY()
{ 
	return PyFloat_FromDouble(topSpeedY() * g_kbeSrvConfig.gameUpdateHertz()); 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetTopSpeed()
{ 
	return PyFloat_FromDouble(topSpeed() * g_kbeSrvConfig.gameUpdateHertz()); 
}

//-------------------------------------------------------------------------------------
int Entity::pySetTopSpeed(PyObject *value)
{ 
	topSpeed(float(PyFloat_AsDouble(value)) / g_kbeSrvConfig.gameUpdateHertz()); 
	return 0; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::onScriptGetAttribute(PyObject* attr)
{
	DEBUG_OP_ATTRIBUTE("get", attr)

	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);
	char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);
		
	// �����ghost����def��������Ҫrpc���á�
	if(!isReal())
	{
		MethodDescription* pMethodDescription = const_cast<ScriptDefModule*>(pScriptModule())->findCellMethodDescription(ccattr);
		
		if(pMethodDescription)
		{
			free(ccattr);
			return new RealEntityMethod(pMethodDescription, this);
		}
	}
	else
	{
		// ���������def�־û�����������
		// ����û�кܺõļ�������������ڲ��ı仯������ʹ��һ�����еİ취���б���
		PropertyDescription* pPropertyDescription = const_cast<ScriptDefModule*>(pScriptModule())->findPersistentPropertyDescription(ccattr);
		if(pPropertyDescription && (pPropertyDescription->getFlags() & ENTITY_CELL_DATA_FLAGS) > 0)
		{
			setDirty();
		}
	}
	
	free(ccattr);
	return ScriptObject::onScriptGetAttribute(attr);
}	

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(const PropertyDescription* propertyDescription, PyObject* pyData)
{
	// �������һ��realEntity�����ڳ�ʼ�������
	if(!isReal() || initing())
		return;

	if(propertyDescription->isPersistent())
		setDirty();
	
	uint32 flags = propertyDescription->getFlags();

	// ���ȴ���һ����Ҫ�㲥��ģ����
	MemoryStream* mstream = MemoryStream::createPoolObject();

	propertyDescription->getDataType()->addToStream(mstream, pyData);

	// �ж��Ƿ���Ҫ�㲥��������cellapp, �⻹��һ��ǰ����entity����ӵ��ghostʵ��
	// ֻ����cell�߽�һ����Χ�ڵ�entity��ӵ��ghostʵ��, ��������תspaceʱҲ����ݵ���Ϊghost״̬
	if((flags & ENTITY_BROADCAST_CELL_FLAGS) > 0 && hasGhost())
	{
		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if(gm)
		{
			Network::Bundle* pForwardBundle = gm->createSendBundle(ghostCell());
			(*pForwardBundle).newMessage(CellappInterface::onUpdateGhostPropertys);
			(*pForwardBundle) << id();
			(*pForwardBundle) << propertyDescription->getUType();

			pForwardBundle->append(*mstream);

			// ��¼����¼���������������С
			g_publicCellEventHistoryStats.trackEvent(scriptName(), 
				propertyDescription->getName(), 
				pForwardBundle->currMsgLength());

			gm->pushMessage(ghostCell(), pForwardBundle);
		}
	}
	
	const Position3D& basePos = this->position(); 
	if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) > 0)
	{
		DETAIL_TYPE propertyDetailLevel = propertyDescription->getDetailLevel();

		std::list<ENTITY_ID>::iterator witer = witnesses_.begin();
		for(; witer != witnesses_.end(); ++witer)
		{
			Entity* pEntity = Cellapp::getSingleton().findEntity((*witer));
			if(pEntity == NULL || pEntity->pWitness() == NULL)
				continue;

			EntityMailbox* clientMailbox = pEntity->clientMailbox();
			if(clientMailbox == NULL)
				continue;

			Network::Channel* pChannel = clientMailbox->getChannel();
			if(pChannel == NULL)
				continue;

			// ����������Ǵ��ڵģ�����������Դ��createWitnessFromStream()
			// �����Լ���entity��δ��Ŀ��ͻ����ϴ���
			if(!pEntity->pWitness()->entityInAOI(id()))
				continue;

			const Position3D& targetPos = pEntity->position();
			Position3D lengthPos = targetPos - basePos;

			if(pScriptModule_->getDetailLevel().level[propertyDetailLevel].inLevel(lengthPos.length()))
			{
				Network::Bundle* pSendBundle = pChannel->createSendBundle();
				NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity->id(), (*pSendBundle));
				
				int ialiasID = -1;
				const Network::MessageHandler& msgHandler = pEntity->pWitness()->getAOIEntityMessageHandler(ClientInterface::onUpdatePropertys, 
					ClientInterface::onUpdatePropertysOptimized, id(), ialiasID);
				
				ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, msgHandler, aOIEntityMessage);
				
				if(ialiasID != -1)
				{
					KBE_ASSERT(msgHandler.msgID == ClientInterface::onUpdatePropertysOptimized.msgID);
					(*pSendBundle)  << (uint8)ialiasID;
				}
				else
				{
					KBE_ASSERT(msgHandler.msgID == ClientInterface::onUpdatePropertys.msgID);
					(*pSendBundle)  << id();
				}
				
				if(pScriptModule_->usePropertyDescrAlias())
					(*pSendBundle) << propertyDescription->aliasIDAsUint8();
				else
					(*pSendBundle) << propertyDescription->getUType();

				pSendBundle->append(*mstream);
				
				// ��¼����¼���������������С
				g_publicClientEventHistoryStats.trackEvent(scriptName(), 
					propertyDescription->getName(), 
					pSendBundle->currMsgLength());

				ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, msgHandler, aOIEntityMessage);

				pEntity->pWitness()->sendToClient(ClientInterface::onUpdatePropertysOptimized, pSendBundle);
			}
		}
	}

	/*
	// �ж���������Ƿ���Ҫ�㲥�������ͻ���
	if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) > 0)
	{
		int8 detailLevel = propertyDescription->getDetailLevel();
		for(int8 i=DETAIL_LEVEL_NEAR; i<=detailLevel; ++i)
		{
			std::map<ENTITY_ID, Entity*>::iterator iter = witnessEntities_[i].begin();
			for(; iter != witnessEntities_[i].end(); ++iter)
			{
				Entity* entity = iter->second;
				EntityMailbox* clientMailbox = entity->clientMailbox();
				if(clientMailbox != NULL)
				{
					Packet* sp = clientMailbox->createMail(MAIL_TYPE_UPDATE_PROPERTY);
					(*sp) << id_;
					sp->append(mstream->contents(), mstream->size());
					clientMailbox->post(sp);
				}
			}
		}

		// ��������Ѿ����¹��� ����Щ��Ϣ��ӵ������������������entity�� �����ڿ�����Զ��һ�㣬 �����������½������detaillevel
		// ʱ������½����е����Զ����µ����Ŀͻ��˿��ܲ����ʣ� ���Ǽ�¼������Եĸı䣬 �´������½�������ֻ��Ҫ�������ڼ��й��ı��
		// ���ݷ��͵����Ŀͻ��˸���
		for(int8 i=detailLevel; i<=DETAIL_LEVEL_FAR; ++i)
		{
			std::map<ENTITY_ID, Entity*>::iterator iter = witnessEntities_[i].begin();
			for(; iter != witnessEntities_[i].end(); ++iter)
			{
				Entity* entity = iter->second;
				EntityMailbox* clientMailbox = entity->clientMailbox();
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

				// ��¼����¼���������������С
				std::string event_name = this->scriptName();
				event_name += ".";
				event_name += propertyDescription->getName();
				
				g_publicClientEventHistoryStats.add(scriptName(), propertyDescription->getName(), pSendBundle->currMsgLength());
			}
		}
	}
	*/

	// �ж���������Ƿ���Ҫ�㲥���Լ��Ŀͻ���
	if((flags & ENTITY_BROADCAST_OWN_CLIENT_FLAGS) > 0 && clientMailbox_ != NULL && pWitness_)
	{
		Network::Bundle* pSendBundle = NULL;
		
		Network::Channel* pChannel = pWitness_->pChannel();
		if(!pChannel)
			pSendBundle = Network::Bundle::createPoolObject();
		else
			pSendBundle = pChannel->createSendBundle();
		
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(id(), (*pSendBundle));
		
		ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);
		(*pSendBundle) << id();

		if(pScriptModule_->usePropertyDescrAlias())
			(*pSendBundle) << propertyDescription->aliasIDAsUint8();
		else
			(*pSendBundle) << propertyDescription->getUType();

		pSendBundle->append(*mstream);
		
		// ��¼����¼���������������С
		if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) <= 0)
		{
			g_privateClientEventHistoryStats.trackEvent(scriptName(), 
				propertyDescription->getName(), 
				pSendBundle->currMsgLength());
		}

		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);

		pWitness_->sendToClient(ClientInterface::onUpdatePropertys, pSendBundle);
	}

	MemoryStream::reclaimPoolObject(mstream);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_METHOD_UID utype = 0;
	s >> utype;

	MethodDescription* pMethodDescription = pScriptModule_->findCellMethodDescription(utype);

	if(pMethodDescription == NULL)
	{
		ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: can't found method. utype={0}, methodName=unknown, callerID:{1}.\n"
			, utype, id_, this->scriptName()));

		return;
	}

	onRemoteMethodCall_(pMethodDescription, id(), s);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteCallMethodFromClient(Network::Channel* pChannel, ENTITY_ID srcEntityID, MemoryStream& s)
{
	ENTITY_METHOD_UID utype = 0;
	s >> utype;

	MethodDescription* pMethodDescription = pScriptModule_->findCellMethodDescription(utype);
	if(pMethodDescription)
	{
		if(!pMethodDescription->isExposed())
		{
			ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: {0} not is exposed, call is illegal! entityID:{1}.\n",
				pMethodDescription->getName(), this->id(), this->scriptName()));

			s.done();
			return;
		}
	}
	else
	{
		ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: can't found method. utype={0}, methodName=unknown, callerID:{1}.\n",
			utype, id_, this->scriptName()));

		return;
	}

	onRemoteMethodCall_(pMethodDescription, srcEntityID, s);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall_(MethodDescription* pMethodDescription, ENTITY_ID srcEntityID, MemoryStream& s)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	if (isDestroyed())
	{
		ERROR_MSG(fmt::format("{}::onRemoteMethodCall: {} is destroyed!\n",
			scriptName(), id()));

		s.done();
		return;
	}

	if(pMethodDescription == NULL)
	{
		ERROR_MSG(fmt::format("{1}::onRemoteMethodCall: can't found method, callerID:{0}.\n",
			id_, this->scriptName()));

		return;
	}

	if(g_debugEntity)
	{
		DEBUG_MSG(fmt::format("Entity::onRemoteMethodCall: {0}, {3}::{1}(utype={2}).\n",
			id_, pMethodDescription->getName(), pMethodDescription->getUType(), this->scriptName()));
	}

	pMethodDescription->currCallerID(srcEntityID);

	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>
						(pMethodDescription->getName()));

	if(pMethodDescription != NULL)
	{
		if(!pMethodDescription->isExposed() && pMethodDescription->getArgSize() == 0)
		{
			pMethodDescription->call(pyFunc, NULL);
		}
		else
		{
			PyObject* pyargs = pMethodDescription->createFromStream(&s);
			if(pyargs)
			{
				pMethodDescription->call(pyFunc, pyargs);
				Py_XDECREF(pyargs);
			}
			else
			{
				SCRIPT_ERROR_CHECK();
				s.done();
			}
		}
	}
	else
	{
		s.done();
	}

	Py_XDECREF(pyFunc);
	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Entity::addCellDataToStream(uint32 flags, MemoryStream* mstream, bool useAliasID)
{
	addPositionAndDirectionToStream(*mstream, useAliasID);
	PyObject* cellData = PyObject_GetAttrString(this, "__dict__");

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =
					pScriptModule_->getCellPropertyDescriptions();

	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	for(; iter != propertyDescrs.end(); ++iter)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			// DEBUG_MSG(fmt::format("Entity::addCellDataToStream: {}.\n", propertyDescription->getName()));
			PyObject* pyVal = PyDict_GetItemString(cellData, propertyDescription->getName());

			if(useAliasID && pScriptModule_->usePropertyDescrAlias())
			{
				(*mstream) << propertyDescription->aliasIDAsUint8();
			}
			else
			{
				(*mstream) << propertyDescription->getUType();
			}

			if(!propertyDescription->getDataType()->isSameType(pyVal))
			{
				ERROR_MSG(fmt::format("{}::addCellDataToStream: {}({}) not is ({})!\n", this->scriptName(), 
					propertyDescription->getName(), (pyVal ? pyVal->ob_type->tp_name : "unknown"), propertyDescription->getDataType()->getName()));
				
				PyObject* pydefval = propertyDescription->getDataType()->parseDefaultStr("");
				propertyDescription->getDataType()->addToStream(mstream, pydefval);
				Py_DECREF(pydefval);
			}
			else
			{
				propertyDescription->getDataType()->addToStream(mstream, pyVal);
			}

			if (PyErr_Occurred())
 			{	
				PyErr_PrintEx(0);
				DEBUG_MSG(fmt::format("{}::addCellDataToStream: {} error!\n", this->scriptName(),
					propertyDescription->getName()));
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
		// ����ǰ��cell�������ݴ�� һ���͸�base���ֱ���
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(BaseappInterface::onBackupEntityCellData);
		(*pBundle) << id_;
		(*pBundle) << isDirty();
		
		if(isDirty())
		{
			MemoryStream* s = MemoryStream::createPoolObject();
			addCellDataToStream(ENTITY_CELL_DATA_FLAGS, s);
			(*pBundle).append(s);
			MemoryStream::reclaimPoolObject(s);
		}
		
		baseMailbox_->postMail(pBundle);
	}
	else
	{
		WARNING_MSG(fmt::format("Entity::backupCellData(): {} {} has no base!\n", 
			this->scriptName(), this->id()));
	}

	SCRIPT_ERROR_CHECK();
	
	setDirty(false);
}

//-------------------------------------------------------------------------------------
void Entity::writeToDB(void* data, void* extra1, void* extra2)
{
	CALLBACK_ID* pCallbackID = static_cast<CALLBACK_ID*>(data);
	CALLBACK_ID callbackID = 0;

	if(pCallbackID)
		callbackID = *pCallbackID;

	int8 shouldAutoLoad = -1;
	if (extra1)
		shouldAutoLoad = *static_cast<int8*>(extra1);

	int dbInterfaceIndex = -1;

	if (extra2)
	{
		if (strlen(static_cast<char*>(extra2)) > 0)
		{
			DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(static_cast<char*>(extra2));
			if (pDBInterfaceInfo->isPure)
			{
				ERROR_MSG(fmt::format("Entity::writeToDB: dbInterface({}) is a pure database does not support Entity! "
					"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
					static_cast<char*>(extra2)));

				return;
			}

			int fdbInterfaceIndex = pDBInterfaceInfo->index;
			if (fdbInterfaceIndex >= 0)
			{
				dbInterfaceIndex = fdbInterfaceIndex;
			}
			else
			{
				ERROR_MSG(fmt::format("Entity::writeToDB: not found dbInterface({})!\n",
					static_cast<char*>(extra2)));

				return;
			}
		}
	}
	onWriteToDB();
	backupCellData();

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(BaseappInterface::onCellWriteToDBCompleted);
	(*pBundle) << this->id();
	(*pBundle) << callbackID;
	(*pBundle) << shouldAutoLoad;
	(*pBundle) << dbInterfaceIndex;

	if(this->baseMailbox())
	{
		this->baseMailbox()->postMail(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Entity::onWriteToDB()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	DEBUG_MSG(fmt::format("{}::onWriteToDB(): {}.\n", 
		this->scriptName(), this->id()));

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onWriteToDB"));
}

//-------------------------------------------------------------------------------------
bool Entity::bufferOrExeCallback(const char * funcName, PyObject * funcArgs, bool notFoundIsOK)
{
	bool canBuffer = _scriptCallbacksBufferCount > 0;

	PyObject* pyCallable = PyObject_GetAttrString(this, const_cast<char*>(funcName));

	if (pyCallable == NULL)
	{
		if (!notFoundIsOK)
		{
			ERROR_MSG(fmt::format("{}::bufferOrExeCallback({}): method({}) not found!\n",
				scriptName(), id(), funcName));
		}

		if (funcArgs)
			Py_DECREF(funcArgs);

		PyErr_Clear();
		return false;
	}

	if (canBuffer)
	{
		BufferedScriptCall* pBufferedScriptCall = new BufferedScriptCall();
		pBufferedScriptCall->entityPtr = this;
		pBufferedScriptCall->pyFuncArgs = funcArgs;
		pBufferedScriptCall->pyCallable = pyCallable;
		_scriptCallbacksBuffer.push_back(pBufferedScriptCall);
		++_scriptCallbacksBufferNum;
	}
	else
	{
		PyObject* pyResult = PyObject_CallObject(pyCallable, funcArgs);

		Py_DECREF(pyCallable);

		if (funcArgs)
			Py_DECREF(funcArgs);

		if (pyResult)
		{
			Py_DECREF(pyResult);
		}
		else
		{
			PyErr_PrintEx(0);
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Entity::bufferCallback(bool enable)
{
	if (enable)
	{
		++_scriptCallbacksBufferCount;
	}
	else
	{
		if (_scriptCallbacksBufferCount - 1 == 0)
		{
			// ��Ȼ��Ҫȡ�������ˣ���ôִ������callback������Ҫע����ִ�й����п����ֲ�����callback����
			// ��ô��Ҫ���뵽���к���
			while (_scriptCallbacksBufferNum > 0)
			{
				BufferedScriptCall* pBufferedScriptCall = (*_scriptCallbacksBuffer.begin());
				_scriptCallbacksBuffer.pop_front();
				--_scriptCallbacksBufferNum;

				PyObject* pyResult = PyObject_CallObject(pBufferedScriptCall->pyCallable, pBufferedScriptCall->pyFuncArgs);

				if (pyResult)
				{
					Py_DECREF(pyResult);
				}
				else
				{
					PyErr_PrintEx(0);
				}

				Py_DECREF(pBufferedScriptCall->pyCallable);
				if (pBufferedScriptCall->pyFuncArgs)
					Py_DECREF(pBufferedScriptCall->pyFuncArgs);

				delete pBufferedScriptCall;
			}
		}

		// �����ִ�м���������ִֹ��callback�ڼ������󻺴�callback
		--_scriptCallbacksBufferCount;
		KBE_ASSERT(_scriptCallbacksBufferCount >= 0);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyIsReal()
{
	return PyBool_FromLong(isReal());
}

//-------------------------------------------------------------------------------------
void Entity::addWitnessed(Entity* entity)
{
	if(Cellapp::getSingleton().pWitnessedTimeoutHandler())
		Cellapp::getSingleton().pWitnessedTimeoutHandler()->delWitnessed(this);

	witnesses_.push_back(entity->id());
	++witnesses_count_;

	/*
	int8 detailLevel = pScriptModule_->getDetailLevel().getLevelByRange(range);
	WitnessInfo* info = new WitnessInfo(detailLevel, entity, range);
	ENTITY_ID id = entity->id();

	DEBUG_MSG("Entity[%s:%ld]::onWitnessed:%s %ld enter detailLevel %d. range=%f.\n", scriptName(), id_, 
			entity->scriptName(), id, detailLevel, range);

#ifdef _DEBUG
	WITNESSENTITY_DETAILLEVEL_MAP::iterator iter = witnessEntityDetailLevelMap_.find(id);
	if(iter != witnessEntityDetailLevelMap_.end())
		ERROR_MSG("Entity::onWitnessed: %s %ld is exist.\n", entity->scriptName(), id);
#endif
	
	witnessEntityDetailLevelMap_[id] = info;
	witnessEntities_[detailLevel][id] = entity;
	onEntityInitDetailLevel(entity, detailLevel);
	*/

	if(witnesses_count_ == 1)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		bufferOrExeCallback(const_cast<char*>("onWitnessed"),
			Py_BuildValue(const_cast<char*>("(O)"), PyBool_FromLong(1)));
	}
}

//-------------------------------------------------------------------------------------
void Entity::delWitnessed(Entity* entity)
{
	KBE_ASSERT(witnesses_count_ > 0);

	witnesses_.remove(entity->id());
	--witnesses_count_;

	if (controlledBy_ != NULL && entity->id() == controlledBy_->id())
	{
		if (clientMailbox_ && clientMailbox_->getChannel())
			setControlledBy(baseMailbox_);
		else
			setControlledBy(NULL);

		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onLoseControlledBy"),
			const_cast<char*>("i"), entity->id());
	}

	// ��ʱִ��
	// onDelWitnessed();

	if(Cellapp::getSingleton().pWitnessedTimeoutHandler())
		Cellapp::getSingleton().pWitnessedTimeoutHandler()->addWitnessed(this);
}

//-------------------------------------------------------------------------------------
void Entity::onDelWitnessed()
{
	if(witnesses_count_ == 0)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		bufferOrExeCallback(const_cast<char*>("onWitnessed"),
			Py_BuildValue(const_cast<char*>("(O)"), PyBool_FromLong(0)));
	}
}

//-------------------------------------------------------------------------------------
bool Entity::entityInWitnessed(ENTITY_ID entityID)
{
	std::list<ENTITY_ID>::iterator it = witnesses_.begin();
	for (; it != witnesses_.end(); ++it)
	{
		if (*it == entityID)
			return true;
	}

	return false;
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
void Entity::restoreProximitys()
{
	if(this->pWitness())
		this->pWitness()->installAOITrigger();

	Controllers::CONTROLLERS_MAP& objects = pControllers_->objects();
	Controllers::CONTROLLERS_MAP::iterator iter = objects.begin();
	for(; iter != objects.end(); ++iter)
	{
		if(iter->second->type() == Controller::CONTROLLER_TYPE_PROXIMITY)
		{
			ProximityController* pProximityController = static_cast<ProximityController*>(iter->second.get());
			pProximityController->reinstall(static_cast<CoordinateNode*>(this->pEntityCoordinateNode()));
		}
	}
}

//-------------------------------------------------------------------------------------
uint32 Entity::addProximity(float range_xz, float range_y, int32 userarg)
{
	if(range_xz <= 0.0f || (CoordinateSystem::hasY && range_y <= 0.0f))
	{
		ERROR_MSG(fmt::format("Entity::addProximity: range(xz={}, y={}) <= 0.0f! entity[{}:{}]\n", 
			range_xz, range_y, scriptName(), id()));

		return 0;
	}
	
	if(this->pEntityCoordinateNode() == NULL || this->pEntityCoordinateNode()->pCoordinateSystem() == NULL)
	{
		ERROR_MSG(fmt::format("Entity::addProximity: {}({}) not in world!\n", 
			scriptName(), id()));

		return 0;
	}

	// ��space��Ͷ��һ������
	KBEShared_ptr<Controller> p( new ProximityController(this, range_xz, range_y, userarg, pControllers_->freeID()) );

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddProximity(float range_xz, float range_y, int32 userarg)
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::addProximity: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::addProximity: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	return PyLong_FromLong(addProximity(range_xz, range_y, userarg));
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyClientEntity(ENTITY_ID entityID)
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	return new ClientEntity(id(), entityID);
}

//-------------------------------------------------------------------------------------
void Entity::cancelController(uint32 id)
{
	if(this->isDestroyed())
	{
		return;
	}

	if(!pControllers_->remove(id))
	{
		ERROR_MSG(fmt::format("{}::cancel: {} not found {}.\n", 
			this->scriptName(), this->id(), id));
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyCancelController(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);
	
	if(!pobj->isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity: not is real entity(%d).", 
			pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return 0;
	}

	uint32 id = 0;
	PyObject* pyargobj = NULL;

	if(currargsSize != 1)
	{
		PyErr_Format(PyExc_AssertionError, "%s::cancel: args require 1 args(controllerID|int or \"Movement\"|str), gived %d! is script[%s].\n",								
			pobj->scriptName(), currargsSize);														
																																
		PyErr_PrintEx(0);																										
		return 0;																								
	}

	if(PyArg_ParseTuple(args, "O", &pyargobj) == -1)
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
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyargobj, NULL);
		char* s = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		
		if(strcmp(s, "Movement") == 0)
		{
			pobj->stopMove();
		}
		else
		{
			PyErr_Format(PyExc_TypeError, "%s::cancel: args not is \"Movement\"!", pobj->scriptName());
			PyErr_PrintEx(0);
			free(s);
			return 0;
		}

		free(s);

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

	// ֻҪ�������ƶ��������ķ��룬��Ӧ����stopMove()���Ա�����ַ�ʽ�Ĵ������������ϵ�����
	if ((pobj->pMoveController_ && pobj->pMoveController_->id() == id) || 
		(pobj->pTurnController_ && pobj->pTurnController_->id() == id))
	{
		pobj->stopMove();
	}
	else
	{
		pobj->cancelController(id);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::onEnterTrap(Entity* entity, float range_xz, float range_y, uint32 controllerID, int32 userarg)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onEnterTrap"), 
		Py_BuildValue(const_cast<char*>("(OffIi)"), entity, range_xz, range_y, controllerID, userarg));
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrap(Entity* entity, float range_xz, float range_y, uint32 controllerID, int32 userarg)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onLeaveTrap"), 
		Py_BuildValue(const_cast<char*>("(OffIi)"), entity, range_xz, range_y, controllerID, userarg));
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveTrapID(ENTITY_ID entityID, float range_xz, float range_y, uint32 controllerID, int32 userarg)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onLeaveTrapID"), 
		Py_BuildValue(const_cast<char*>("(kffIi)"), entityID, range_xz, range_y, controllerID, userarg));
}

//-------------------------------------------------------------------------------------
void Entity::onEnteredAoI(Entity* entity)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onEnteredAoI"),
		Py_BuildValue(const_cast<char*>("(O)"), entity));
}

//-------------------------------------------------------------------------------------
int Entity::pySetPosition(PyObject *value)
{
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return -1;																				
	}

	if(!script::ScriptVector3::check(value))
		return -1;

	Position3D pos;
	script::ScriptVector3::convertPyObjectToVector3(pos, value);
	position(pos);

	static ENTITY_PROPERTY_UID posuid = 0;
	if(posuid == 0)
	{
		posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
		Network::FixedMessages::MSGInfo* msgInfo =
					Network::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;
	}

	static PropertyDescription positionDescription(posuid, "VECTOR3", "position", ED_FLAG_ALL_CLIENTS, true, DataTypes::getDataType("VECTOR3"), false, "", 0, "", DETAIL_LEVEL_FAR);
	if(pScriptModule_->usePropertyDescrAlias() && positionDescription.aliasID() == -1)
		positionDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ);

	onDefDataChanged(&positionDescription, value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetPosition()
{
	Py_INCREF(pPyPosition_);
	return pPyPosition_;
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XZ_int(Network::Channel* pChannel, int32 x, int32 z)
{
	setPosition_XZ_float(pChannel, float(x), float(z));
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XYZ_int(Network::Channel* pChannel, int32 x, int32 y, int32 z)
{
	setPosition_XYZ_float(pChannel, float(x), float(y), float(z));
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XZ_float(Network::Channel* pChannel, float x, float z)
{
	Position3D& pos = position();
	if(almostEqual(x, pos.x) && almostEqual(z, pos.z))
		return;

	pos.x = x;
	pos.z = z;
	onPositionChanged();
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XYZ_float(Network::Channel* pChannel, float x, float y, float z)
{
	Position3D& pos = position();
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
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return -1;																				
	}

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

	if(!PyFloat_Check(pyItem))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", pyItem->ob_type->tp_name);
		PyErr_PrintEx(0);
		Py_DECREF(pyItem);
		return -1;
	}

	dir.roll(float(PyFloat_AsDouble(pyItem)));
	Py_DECREF(pyItem);

	pyItem = PySequence_GetItem(value, 1);

	if(!PyFloat_Check(pyItem))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", pyItem->ob_type->tp_name);
		PyErr_PrintEx(0);
		Py_DECREF(pyItem);
		return -1;
	}

	dir.pitch(float(PyFloat_AsDouble(pyItem)));
	Py_DECREF(pyItem);

	pyItem = PySequence_GetItem(value, 2);

	if(!PyFloat_Check(pyItem))
	{
		PyErr_Format(PyExc_TypeError, "args of direction is must a float(curr=%s).", pyItem->ob_type->tp_name);
		PyErr_PrintEx(0);
		Py_DECREF(pyItem);
		return -1;
	}

	dir.yaw(float(PyFloat_AsDouble(pyItem)));
	Py_DECREF(pyItem);

	static ENTITY_PROPERTY_UID diruid = 0;
	if(diruid == 0)
	{
		diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
		Network::FixedMessages::MSGInfo* msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)	
			diruid = msgInfo->msgid;
	}

	static PropertyDescription directionDescription(diruid, "VECTOR3", "direction", ED_FLAG_ALL_CLIENTS, true, DataTypes::getDataType("VECTOR3"), false, "", 0, "", DETAIL_LEVEL_FAR);
	if(pScriptModule_->usePropertyDescrAlias() && directionDescription.aliasID() == -1)
		directionDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW);

	onDefDataChanged(&directionDescription, value);

	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetDirection()
{
	Py_INCREF(pPyDirection_);
	return pPyDirection_;
}

//-------------------------------------------------------------------------------------
void Entity::setPositionAndDirection(const Position3D& pos, const Direction3D& dir)
{
	if(this->isDestroyed())
		return;

	position(pos);
	direction(dir);
}

//-------------------------------------------------------------------------------------
void Entity::onPyPositionChanged()
{
	if(this->isDestroyed())
		return;

	static ENTITY_PROPERTY_UID posuid = 0;
	if(posuid == 0)
	{
		posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
		Network::FixedMessages::MSGInfo* msgInfo =
					Network::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;
	}

	static PropertyDescription positionDescription(posuid, "VECTOR3", "position", ED_FLAG_ALL_CLIENTS, true, DataTypes::getDataType("VECTOR3"), false, "", 0, "", DETAIL_LEVEL_FAR);
	if(pScriptModule_->usePropertyDescrAlias() && positionDescription.aliasID() == -1)
		positionDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ);

	onDefDataChanged(&positionDescription, pPyPosition_);

	if (this->pEntityCoordinateNode())
	{
		Entity::bufferCallback(true);
		this->pEntityCoordinateNode()->update();
		Entity::bufferCallback(false);
	}

	updateLastPos();
}

//-------------------------------------------------------------------------------------
void Entity::onPositionChanged()
{
	if(this->isDestroyed())
		return;

	posChangedTime_ = g_kbetime;

	if (this->pEntityCoordinateNode())
	{
		Entity::bufferCallback(true);
		this->pEntityCoordinateNode()->update();
		Entity::bufferCallback(false);
	}

	updateLastPos();
}

//-------------------------------------------------------------------------------------
void Entity::updateLastPos()
{
	lastpos_ = this->position();
}

//-------------------------------------------------------------------------------------
void Entity::onPyDirectionChanged()
{
	if(this->isDestroyed())
		return;

	// onDirectionChanged();
	static ENTITY_PROPERTY_UID diruid = 0;
	if(diruid == 0)
	{
		diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
		Network::FixedMessages::MSGInfo* msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)	
			diruid = msgInfo->msgid;
	}

	static PropertyDescription directionDescription(diruid, "VECTOR3", "direction", ED_FLAG_ALL_CLIENTS, true, DataTypes::getDataType("VECTOR3"), false, "", 0, "", DETAIL_LEVEL_FAR);
	if(pScriptModule_->usePropertyDescrAlias() && directionDescription.aliasID() == -1)
		directionDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW);

	onDefDataChanged(&directionDescription, pPyDirection_);
}

//-------------------------------------------------------------------------------------
void Entity::onDirectionChanged()
{
	if(this->isDestroyed())
		return;

	dirChangedTime_ = g_kbetime;
}

//-------------------------------------------------------------------------------------
void Entity::setWitness(Witness* pWitness)
{
	KBE_ASSERT(this->baseMailbox() != NULL && !this->hasWitness());
	pWitness_ = pWitness;
	pWitness_->attach(this);
}

//-------------------------------------------------------------------------------------
void Entity::onGetWitnessFromBase(Network::Channel* pChannel)
{
	onGetWitness(true);
}

//-------------------------------------------------------------------------------------
void Entity::onGetWitness(bool fromBase)
{
	KBE_ASSERT(this->baseMailbox() != NULL);

	if(fromBase)
	{
		// proxy��giveClientTo���ܻ���reloginBaseapp�� ���һ��entity�Ѿ�������cell�� ��������Ȩ��
		// ����entityʱ��һ��û��clientMailbox�ġ�
		if(clientMailbox() == NULL)
		{
			PyObject* clientMB = PyObject_GetAttrString(baseMailbox(), "client");
			KBE_ASSERT(clientMB != Py_None);

			EntityMailbox* client = static_cast<EntityMailbox*>(clientMB);	
			// Py_INCREF(clientMailbox); ���ﲻ��Ҫ�������ã� ��Ϊÿ�ζ������һ���µĶ���
			clientMailbox(client);
		}

		if(pWitness_ == NULL)
		{
			setWitness(Witness::createPoolObject());
		}
		else
		{
			/*
				���°󶨣�ͨ���ǿͻ����ص�½������������һ���˺ż���
				��һ���ͻ��˵�½�Ŀͻ���, ��Entity�����ڴ��в����Ѿ�
				����witness(�������Ҳ�����Ƿ���˻�δ��鵽�ͻ��˶���)

				�������������Ȼ��Ҫ��һЩ���鱣֤�ͻ��˵���ȷ�ԣ� ���緢��enterworld
			*/
			pWitness_->onAttach(this);

			// AOI�е�ʵ��Ҳ��Ҫ���ã�����ͬ�����ͻ���
			pWitness_->resetAOIEntities();
		}
	}

	// ��ֹ�Լ���һЩ�ű��ص��б����٣�������Լ���һ������
	Py_INCREF(this);

	Space* space = Spaces::findSpace(this->spaceID());
	if(space && space->isGood())
	{
		space->onEntityAttachWitness(this);
	}

	// �������controlledByΪ�Լ���base
	controlledBy(baseMailbox());
	
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onGetWitness"));
	}
	
	// ���һ��ʵ���Ѿ���cell�������giveToClient����ô��Ҫ�����µĿͻ�������ֵ���µ��ͻ���
	if(fromBase)
	{
		Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(id(), (*pSendBundle));
		
		ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);
		MemoryStream* s1 = MemoryStream::createPoolObject();
		(*pSendBundle) << id();
		
		ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

		Network::FixedMessages::MSGInfo* msgInfo = 
			Network::FixedMessages::getSingleton().isFixed("Property::spaceID");

		if(msgInfo != NULL)
			spaceuid = msgInfo->msgid;
		
		if(pScriptModule()->usePropertyDescrAlias())
		{
			uint8 aliasID = ENTITY_BASE_PROPERTY_ALIASID_SPACEID;
			(*s1) << aliasID << this->spaceID();
		}
		else
		{
			(*s1) << spaceuid << this->spaceID();
		}

		addClientDataToStream(s1);
		(*pSendBundle).append(*s1);
		MemoryStream::reclaimPoolObject(s1);
		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);
		
		clientMailbox()->postMail(pSendBundle);
	}

	Py_DECREF(this);
}

//-------------------------------------------------------------------------------------
void Entity::onLoseWitness(Network::Channel* pChannel)
{
	//INFO_MSG(fmt::format("{}::onLoseWitness: {}.\n", 
	//	this->scriptName(), this->id()));

	KBE_ASSERT(this->clientMailbox() != NULL && this->hasWitness());

	clientMailbox()->addr(Network::Address::NONE);
	Py_DECREF(clientMailbox());
	clientMailbox(NULL);

	pWitness_->detach(this);
	Witness::reclaimPoolObject(pWitness_);
	pWitness_ = NULL;

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLoseWitness"));
}

//-------------------------------------------------------------------------------------
int Entity::pySetLayer(PyObject *value)
{
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	if(!PyLong_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d set layer value is not int!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;	
	}

	layer_ = (int8)PyLong_AsLong(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetLayer()
{
	return PyLong_FromLong(layer_);
}

//-------------------------------------------------------------------------------------
int Entity::pySetVolatileinfo(PyObject *value)
{
	if (isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}


	if (!PySequence_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d set volatileInfo value is not tuple!\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if (PySequence_Size(value) != 4)
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d set volatileInfo value is not tuple(position, yaw, pitch, roll)!\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if (pCustomVolatileinfo_ == NULL)
		pCustomVolatileinfo_ = new VolatileInfo();

	PyObject* pyPos = PySequence_GetItem(value, 0);
	PyObject* pyYaw = PySequence_GetItem(value, 1);
	PyObject* pyPitch = PySequence_GetItem(value, 2);
	PyObject* pyRoll = PySequence_GetItem(value, 3);

	pCustomVolatileinfo_->position(float(PyFloat_AsDouble(pyPos)));
	pCustomVolatileinfo_->yaw(float(PyFloat_AsDouble(pyYaw)));
	pCustomVolatileinfo_->pitch(float(PyFloat_AsDouble(pyPitch)));
	pCustomVolatileinfo_->roll(float(PyFloat_AsDouble(pyRoll)));

	Py_DECREF(pyPos);
	Py_DECREF(pyYaw);
	Py_DECREF(pyPitch);
	Py_DECREF(pyRoll);

	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetVolatileinfo()
{
	if (pCustomVolatileinfo_ == NULL)
		pCustomVolatileinfo_ = new VolatileInfo();

	Py_INCREF(pCustomVolatileinfo_);
	return pCustomVolatileinfo_;
}

//-------------------------------------------------------------------------------------
bool Entity::checkMoveForTopSpeed(const Position3D& position)
{
	Position3D movment = position - this->position();
	bool move = true;
	
	// ����ƶ�
	if(topSpeedY_ > 0.01f && movment.y > topSpeedY_)
	{
		move = false;
	}

	if(move && topSpeed_ > 0.01f)
	{
		movment.y = 0.f;
		
		if(movment.length() > topSpeed_)
			move = false;
	}

	return move;
}

//-------------------------------------------------------------------------------------
void Entity::onUpdateDataFromClient(KBEngine::MemoryStream& s)
{
	if(spaceID_ == 0)
	{
		s.done();
		return;
	}

	Position3D pos;
	Direction3D dir;
	uint8 isOnGround = 0;
	float yaw, pitch, roll;
	SPACE_ID currSpace;

	s >> pos.x >> pos.y >> pos.z >> roll >> pitch >> yaw >> isOnGround >> currSpace;
	isOnGround_ = isOnGround > 0;

	if(spaceID_ != currSpace)
	{
		s.done();
		return;
	}

	dir.yaw(yaw);
	dir.pitch(pitch);
	dir.roll(roll);
	this->direction(dir);

	if(checkMoveForTopSpeed(pos))
	{
		this->position(pos);
	}
	else
	{
		if (this->pWitness() == NULL && this->controlledBy_ == NULL)
			return;

		Position3D currpos = this->position();
		Position3D movment = pos - currpos;
		float ydist = fabs(movment.y);
		movment.y = 0.f;

		DEBUG_MSG(fmt::format("{}::onUpdateDataFromClient: {} position[({},{},{}) -> ({},{},{}), (xzDist={})>(topSpeed={}) || (yDist={})>(topSpeedY={})] invalid. reset client!\n", 
			this->scriptName(), this->id(),
			this->position().x, this->position().y, this->position().z,
			pos.x, pos.y, pos.z,
			movment.length(), topSpeed_,
			ydist, topSpeedY_));
		
		// this->position(currpos);

		// ������Ѿ������ƣ���ô�����ݵ���Դ���ǿ����ߵĿͻ��ˣ�
		// ���ԣ�������Ҫ������֪ͨ��Դ�ͻ��ˣ������������Լ��Ŀͻ��ˡ�
		Witness* pW = NULL;
		KBEngine::ENTITY_ID targetID = 0;

		if (controlledBy_ != NULL)
		{
			targetID = controlledBy_->id();
			Entity* entity = Cellapp::getSingleton().findEntity(targetID);
			
			if(entity->isReal())
				pW = entity->pWitness();
		}
		else
		{
			targetID = id();
			
			if(isReal())
				pW = this->pWitness();
		}
		
		// �ڿ����teleportʱ�����������ghost����ĳ��״̬��witness��ʱ����ΪNone
		if(pW)
		{
			// ֪ͨ����
			Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
			NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(targetID, (*pSendBundle));
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);

			(*pSendBundle) << id();
			(*pSendBundle) << currpos.x << currpos.y << currpos.z;
			(*pSendBundle) << direction().roll() << direction().pitch() << direction().yaw();

			ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);
			pW->sendToClient(ClientInterface::onSetEntityPosAndDir, pSendBundle);
		}
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

	PyErr_Format(PyExc_AssertionError, "%s::setAoiRadius: did not get witness.", scriptName());
	PyErr_PrintEx(0);
	return -1;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pySetAoiRadius(float radius, float hyst)
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::setAoiRadius: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	return PyLong_FromLong(setAoiRadius(radius, hyst));
}

//-------------------------------------------------------------------------------------
float Entity::getAoiRadius(void) const
{
	if(pWitness_)
		return pWitness_->aoiRadius();
		
	return 0.0; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetAoiRadius()
{
	return PyFloat_FromDouble(getAoiRadius());
}

//-------------------------------------------------------------------------------------
float Entity::getAoiHystArea(void) const
{
	if(pWitness_)
		return pWitness_->aoiHysteresisArea();
		
	return 0.0; 
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetAoiHystArea()
{
	return PyFloat_FromDouble(getAoiHystArea());
}

//-------------------------------------------------------------------------------------
bool Entity::stopMove()
{
	bool done = false;
	
	if(pMoveController_)
	{
		cancelController(pMoveController_->id());
		pMoveController_->destroy();
		pMoveController_.reset();
		done = true;
	}

	if(pTurnController_)
	{
		cancelController(pTurnController_->id());
		pTurnController_->destroy();
		pTurnController_.reset();
		done = true;
	}

	return done;
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
	if(spaceID() <= 0)
		return false;

	Space* pSpace = Spaces::findSpace(spaceID());
	if(pSpace == NULL || !pSpace->isGood())
		return false;

	if(pSpace->pNavHandle() == NULL)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool Entity::navigatePathPoints( std::vector<Position3D>& outPaths, const Position3D& destination, float maxSearchDistance, int8 layer )
{
	Space* pSpace = Spaces::findSpace(spaceID());
	if(pSpace == NULL || !pSpace->isGood())
	{
		ERROR_MSG(fmt::format("Entity::navigatePathPoints(): not found space({}), entityID({})!\n",
			spaceID(), id()));

		return false;
	}

	NavigationHandlePtr pNavHandle = pSpace->pNavHandle();

	if(!pNavHandle)
	{
		WARNING_MSG(fmt::format("Entity::navigatePathPoints(): space({}), entityID({}), not found navhandle!\n",
			spaceID(), id()));

		return false;
	}

	if (pNavHandle->findStraightPath(layer, position_, destination, outPaths) < 0)
	{
		return false;
	}

	std::vector<Position3D>::iterator iter = outPaths.begin();
	while(iter != outPaths.end())
	{
		Vector3 movement = (*iter) - position_;
		if(KBEVec3Length(&movement) <= 0.00001f)
		{
			iter++;
			continue;
		}

		break;
	}

	// ��һ��������ǵ�ǰλ�ã���˿��Թ��˵�
	if (iter != outPaths.begin())
	{
		outPaths.erase(outPaths.begin(), iter);
	}

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyNavigatePathPoints(PyObject_ptr pyDestination, float maxSearchDistance, int8 layer)
{
	Position3D destination;

	if(!PySequence_Check(pyDestination))
	{
		PyErr_Format(PyExc_TypeError, "%s::navigatePathPoints: args1(position) not is PySequence!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyDestination) != 3)
	{
		PyErr_Format(PyExc_TypeError, "%s::navigatePathPoints: args1(position) invalid!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	// ��������Ϣ��ȡ����
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);

	std::vector<Position3D> outPaths;
	navigatePathPoints(outPaths, destination, maxSearchDistance, layer);
	
	PyObject* pyList = PyList_New(outPaths.size());

	int i = 0;
	std::vector<Position3D>::iterator iter = outPaths.begin();
	for(; iter != outPaths.end(); ++iter)
	{
		script::ScriptVector3 *pos = new script::ScriptVector3(*iter);
		Py_INCREF(pos);
		PyList_SET_ITEM(pyList, i++, pos);
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
uint32 Entity::navigate(const Position3D& destination, float velocity, float distance, float maxMoveDistance, float maxSearchDistance,
	bool faceMovement, int8 layer, PyObject* userData)
{
	VECTOR_POS3D_PTR paths_ptr( new std::vector<Position3D>() );
	navigatePathPoints(*paths_ptr, destination, maxSearchDistance, layer);
	if (paths_ptr->size() <= 0)
	{
		return 0;
	}

	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	KBEShared_ptr<Controller> p(new MoveController(this, NULL));
	
	new NavigateHandler(p, destination, velocity, 
		distance, faceMovement, maxMoveDistance, paths_ptr, userData);

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyNavigate(PyObject_ptr pyDestination, float velocity, float distance, float maxMoveDistance, float maxDistance,
								 int8 faceMovement, int8 layer, PyObject_ptr userData)
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::navigate: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::navigate: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	Position3D destination;

	if(!PySequence_Check(pyDestination))
	{
		PyErr_Format(PyExc_TypeError, "%s::navigate: args1(position) not is PySequence!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyDestination) != 3)
	{
		PyErr_Format(PyExc_TypeError, "%s::navigate: args1(position) invalid!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	// ��������Ϣ��ȡ����
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);

	return PyLong_FromLong(navigate(destination, velocity, distance, maxMoveDistance, 
		maxDistance, faceMovement > 0, layer, userData));
}

//-------------------------------------------------------------------------------------
bool Entity::getRandomPoints(std::vector<Position3D>& outPoints, const Position3D& centerPos,
	float maxRadius, uint32 maxPoints, int8 layer)
{
	Space* pSpace = Spaces::findSpace(spaceID());
	if(pSpace == NULL || !pSpace->isGood())
	{
		ERROR_MSG(fmt::format("Entity::getRandomPoints(): not found space({}), entityID({})!\n",
			spaceID(), id()));

		return false;
	}

	NavigationHandlePtr pNavHandle = pSpace->pNavHandle();

	if(!pNavHandle)
	{
		WARNING_MSG(fmt::format("Entity::getRandomPoints(): space({}), entityID({}), not found navhandle!\n",
			spaceID(), id()));
		return false;
	}

	return pNavHandle->findRandomPointAroundCircle(layer, centerPos, outPoints, maxPoints, maxRadius) > 0;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyGetRandomPoints(PyObject_ptr pyCenterPos, float maxRadius, uint32 maxPoints, int8 layer)
{
	Position3D centerPos;

	if (!PySequence_Check(pyCenterPos))
	{
		PyErr_Format(PyExc_TypeError, "%s::getRandomPoints: args1(position) not is PySequence!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if (PySequence_Size(pyCenterPos) != 3)
	{
		PyErr_Format(PyExc_TypeError, "%s::getRandomPoints: args1(position) invalid!", scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	// ��������Ϣ��ȡ����
	script::ScriptVector3::convertPyObjectToVector3(centerPos, pyCenterPos);

	std::vector<Position3D> outPoints;
	getRandomPoints(outPoints, centerPos, maxRadius, maxPoints, layer);
	
	PyObject* pyList = PyList_New(outPoints.size());

	int i = 0;
	std::vector<Position3D>::iterator iter = outPoints.begin();
	for (; iter != outPoints.end(); ++iter)
	{
		script::ScriptVector3 *pos = new script::ScriptVector3(*iter);
		Py_INCREF(pos);
		PyList_SET_ITEM(pyList, i++, pos);
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
uint32 Entity::moveToPoint(const Position3D& destination, float velocity, float distance, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	KBEShared_ptr<Controller> p(new MoveController(this, NULL));

	new MoveToPointHandler(p, layer(), destination, velocity, 
		distance, faceMovement, moveVertically, userData);

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToPoint(PyObject_ptr pyDestination, float velocity, float distance, PyObject_ptr userData,
								 int32 faceMovement, int32 moveVertically)
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::moveToPoint: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

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

	// ��������Ϣ��ȡ����
	script::ScriptVector3::convertPyObjectToVector3(destination, pyDestination);

	return PyLong_FromLong(moveToPoint(destination, velocity, distance, userData, faceMovement > 0, moveVertically > 0));
}

//-------------------------------------------------------------------------------------
uint32 Entity::moveToEntity(ENTITY_ID targetID, float velocity, float distance, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	KBEShared_ptr<Controller> p(new MoveController(this, NULL));

	new MoveToEntityHandler(p, targetID, velocity, distance,
		faceMovement, moveVertically, userData);

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToEntity(ENTITY_ID targetID, float velocity, float distance, PyObject_ptr userData,
								 int32 faceMovement, int32 moveVertically)
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::moveToEntity: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::moveToEntity: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	return PyLong_FromLong(moveToEntity(targetID, velocity, distance, userData, faceMovement > 0, moveVertically > 0));
}

//-------------------------------------------------------------------------------------
void Entity::onMove(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	SCOPED_PROFILE(ONMOVE_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onMove"),
		Py_BuildValue(const_cast<char*>("(IO)"), controllerId, userarg));

	setDirty();
}

//-------------------------------------------------------------------------------------
void Entity::onMoveOver(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	if(pMoveController_ == NULL)
		return;
	
	pMoveController_->destroy();
	pMoveController_.reset();

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onMoveOver"),
		Py_BuildValue(const_cast<char*>("(IO)"), controllerId, userarg));
	
	setDirty();
}

//-------------------------------------------------------------------------------------
void Entity::onMoveFailure(uint32 controllerId, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	if(pMoveController_ == NULL)
		return;
	
	pMoveController_->destroy();
	pMoveController_.reset();

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onMoveFailure"),
		Py_BuildValue(const_cast<char*>("(IO)"), controllerId, userarg));
	
	setDirty();
}

//-------------------------------------------------------------------------------------
float Entity::accelerate(const char* type, float acceleration)
{
	acceleration = acceleration / g_kbeSrvConfig.gameUpdateHertz();

	if (strcmp(type, "Movement") == 0)
	{
		MoveController* pMoveController = static_cast<MoveController*>(pMoveController_.get());
		if (pMoveController != NULL)
		{
			float velocity = pMoveController->velocity() + acceleration;
			pMoveController->velocity(velocity);
			return velocity * g_kbeSrvConfig.gameUpdateHertz();
		}
	}
	else if (strcmp(type, "Turn") == 0)
	{
		TurnController* pTurnController = static_cast<TurnController*>(pTurnController_.get());
		if (pTurnController != NULL)
		{
			float velocity = pTurnController->velocity() + acceleration;
			pTurnController->velocity(velocity);
			return velocity * g_kbeSrvConfig.gameUpdateHertz();
		}
	}
	else
	{
		PyErr_Format(PyExc_AssertionError, "%s::accelerate: %d type error! only support[\"Movement\",\"Turn\"]\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0.f;
	}

	return 0.f;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAccelerate(const_charptr type, float acceleration)
{
	if (!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::accelerate: not is real entity(%d).",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if (this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::accelerate: %d is destroyed!\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	return PyFloat_FromDouble(accelerate(type, acceleration));
}

//-------------------------------------------------------------------------------------
uint32 Entity::addYawRotator(float yaw, float velocity, PyObject* userData)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	KBEShared_ptr<Controller> p(new TurnController(this, NULL));

	Direction3D dir;
	dir.yaw(yaw);

	new RotatorHandler(p, dir, velocity,
		userData);

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);

	pTurnController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyAddYawRotator(float yaw, float velocity, PyObject* userData)
{
	if (!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::addYawRotator: not is real entity(%d).",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if (this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::addYawRotator: %d is destroyed!\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	return PyLong_FromLong(addYawRotator(yaw, velocity, userData));
}

//-------------------------------------------------------------------------------------
void Entity::onTurn(uint32 controllerId, PyObject* userarg)
{
	if (this->isDestroyed())
		return;

	pTurnController_.reset();

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onTurn"),
		Py_BuildValue(const_cast<char*>("(IO)"), controllerId, userarg));

	setDirty();
}

//-------------------------------------------------------------------------------------
void Entity::debugAOI()
{
	if(pWitness_ == NULL)
	{
		Cellapp::getSingleton().getScript().pyPrint(fmt::format("{}::debugAOI: {} has no witness!", scriptName(), this->id()));
		return;
	}
	
	int pending = 0;
	Witness::AOI_ENTITIES::iterator iter = pWitness_->aoiEntities().begin();
	for (; iter != pWitness_->aoiEntities().end(); ++iter)
	{
		Entity* pEntity = (*iter)->pEntity();

		if(pEntity)
		{
			if(((*iter)->flags() & ENTITYREF_FLAG_ENTER_CLIENT_PENDING) > 0)
				pending++;
		}
	}

	Cellapp::getSingleton().getScript().pyPrint(fmt::format("{}::debugAOI: {} size={}, Seen={}, Pending={}, aoiRadius={}, aoiHyst={}", scriptName(), this->id(), 
		pWitness_->aoiEntitiesMap().size(), pWitness_->aoiEntitiesMap().size() - pending, pending, pWitness_->aoiRadius(), pWitness_->aoiHysteresisArea()));

	iter = pWitness_->aoiEntities().begin();
	for(; iter != pWitness_->aoiEntities().end(); ++iter)
	{
		Entity* pEntity = (*iter)->pEntity();
		Position3D epos;
		float dist = 0.0f;

		if(pEntity)
		{
			epos = pEntity->position();
			Vector3 distvec = epos - this->position();
			dist = KBEVec3Length(&distvec);
		}

		Cellapp::getSingleton().getScript().pyPrint(fmt::format("{7}::debugAOI: {0} {1}({2}), position({3}.{4}.{5}), dist={6}, Seen={8}", 
			this->id(), 
			(pEntity != NULL ? pEntity->scriptName() : "unknown"),
			(*iter)->id(),
			epos.x, epos.y, epos.z,
			dist,
			this->scriptName(), (((*iter)->flags() & ENTITYREF_FLAG_ENTER_CLIENT_PENDING) ? "false" : "true")));
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyDebugAOI()
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::debugAOI: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::debugAOI: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	debugAOI();
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyEntitiesInAOI()
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::entitiesInAOI: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::entitiesInAOI: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	if (!pWitness_)
	{
		PyErr_Format(PyExc_AssertionError, "%s::entitiesInAOI: %d has no witness!\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	int calcSize = 0;
	Witness::AOI_ENTITIES::iterator iter = pWitness_->aoiEntities().begin();
	
	for(; iter != pWitness_->aoiEntities().end(); ++iter)
	{
		Entity* pEntity = (*iter)->pEntity();

		if(pEntity)
		{
			++calcSize;
		}
	}
	
	PyObject* pyList = PyList_New(calcSize);
	
	iter = pWitness_->aoiEntities().begin();
		
	int i = 0;
	for(; iter != pWitness_->aoiEntities().end(); ++iter)
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

	if(!pobj->isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::entitiesInAOI: not is real entity(%d).", 
			pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return 0;
	}

	PyObject* pyPosition = NULL, *pyEntityType = NULL;
	float radius = 0.f;

	if(pobj->isDestroyed())
	{
		PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: entity is destroyed!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(currargsSize == 1)
	{
		if(PyArg_ParseTuple(args, "f", &radius) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else if(currargsSize == 2)
	{
		if(PyArg_ParseTuple(args, "fO", &radius, &pyEntityType) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args error!");
			PyErr_PrintEx(0);
			return 0;
		}

		if(pyEntityType && pyEntityType != Py_None && !PyUnicode_Check(pyEntityType))
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args(entityType) error!");
			PyErr_PrintEx(0);
			return 0;
		}

	}
	else if(currargsSize == 3)
	{
		if(PyArg_ParseTuple(args, "fOO", &radius, &pyEntityType, &pyPosition) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args error!");
			PyErr_PrintEx(0);
			return 0;
		}
		
		if(pyEntityType && pyEntityType != Py_None && !PyUnicode_Check(pyEntityType))
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args(entityType) error!");
			PyErr_PrintEx(0);
			return 0;
		}

		if (pyPosition != Py_None && (!PySequence_Check(pyPosition) || PySequence_Size(pyPosition) < 3))
		{
			PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args(position) error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "Entity::entitiesInRange: args error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* pEntityType = NULL;
	Position3D originpos;
	
	// ��������Ϣ��ȡ����
	if (pyPosition && pyPosition != Py_None)
	{
		script::ScriptVector3::convertPyObjectToVector3(originpos, pyPosition);
	}
	else
	{
		originpos = pobj->position();
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

	// �û�����������entity������Ѱ�� ������Ǵ��������
	EntityCoordinateNode::entitiesInRange(findentities,  pobj->pEntityCoordinateNode(), originpos, radius, entityUType);

	PyObject* pyList = PyList_New(findentities.size());

	std::vector<Entity*>::iterator iter = findentities.begin();
	int i = 0;
	for(; iter != findentities.end(); ++iter)
	{
		Entity* pEntity = (*iter);

		Py_INCREF(pEntity);
		PyList_SET_ITEM(pyList, i++, pEntity);
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
void Entity::_sendBaseTeleportResult(ENTITY_ID sourceEntityID, COMPONENT_ID sourceBaseAppID, SPACE_ID spaceID, SPACE_ID lastSpaceID, bool fromCellTeleport)
{
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(sourceBaseAppID);
	if(cinfos != NULL && cinfos->pChannel != NULL)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(BaseappInterface::onTeleportCB);
		(*pBundle) << sourceEntityID;
		BaseappInterface::onTeleportCBArgs2::staticAddToBundle((*pBundle), spaceID, fromCellTeleport);
		cinfos->pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Entity::teleportFromBaseapp(Network::Channel* pChannel, COMPONENT_ID cellAppID, ENTITY_ID targetEntityID, COMPONENT_ID sourceBaseAppID)
{
	DEBUG_MSG(fmt::format("{}::teleportFromBaseapp: {}, targetEntityID={}, cell={}, sourceBaseAppID={}.\n", 
		this->scriptName(), this->id(), targetEntityID, cellAppID, sourceBaseAppID));

	SPACE_ID lastSpaceID = this->spaceID();

	if (!isReal())
	{
		ERROR_MSG(fmt::format("{}::teleportFromBaseapp: not is real entity({}), sourceBaseAppID={}.\n",
			this->scriptName(), this->id(), sourceBaseAppID));

		_sendBaseTeleportResult(this->id(), sourceBaseAppID, 0, lastSpaceID, false);
		return;
	}

	if(hasFlags(ENTITY_FLAGS_TELEPORT_START))
	{
		ERROR_MSG(fmt::format("{}::teleportFromBaseapp: In transit! entity={}, sourceBaseAppID={}.\n",
			this->scriptName(), this->id(), sourceBaseAppID));

		_sendBaseTeleportResult(this->id(), sourceBaseAppID, 0, lastSpaceID, false);
		return;
	}
	
	// �������һ��cell��
	if(cellAppID != g_componentID)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cellAppID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG(fmt::format("{}::teleportFromBaseapp: {}, teleport error, not found cellapp, targetEntityID, cellAppID={}.\n",
				this->scriptName(), this->id(), targetEntityID, cellAppID));

			_sendBaseTeleportResult(this->id(), sourceBaseAppID, 0, lastSpaceID, false);
			return;
		}

		// Ŀ��cell���ǵ�ǰ�� �������ڿ��Խ�entityǨ��Ŀ�ĵ���
	}
	else
	{
		Entity* entity = Cellapp::getSingleton().findEntity(targetEntityID);
		if(entity == NULL || entity->isDestroyed())
		{
			ERROR_MSG(fmt::format("{}::teleportFromBaseapp: {}, can't found targetEntity({}).\n",
				this->scriptName(), this->id(), targetEntityID));

			_sendBaseTeleportResult(this->id(), sourceBaseAppID, 0, lastSpaceID, false);
			return;
		}
		
		// �ҵ�space
		SPACE_ID spaceID = entity->spaceID();

		// ����ǲ�ͬspace��ת
		if(spaceID != this->spaceID())
		{
			Space* space = Spaces::findSpace(spaceID);
			if(space == NULL || !space->isGood())
			{
				ERROR_MSG(fmt::format("{}::teleportFromBaseapp: {}, can't found space({}).\n",
					this->scriptName(), this->id(), spaceID));

				_sendBaseTeleportResult(this->id(), sourceBaseAppID, 0, lastSpaceID, false);
				return;
			}
			
			Space* currspace = Spaces::findSpace(this->spaceID());
			currspace->removeEntity(this);
			space->addEntityAndEnterWorld(this);
			_sendBaseTeleportResult(this->id(), sourceBaseAppID, spaceID, lastSpaceID, false);
		}
		else
		{
			WARNING_MSG(fmt::format("{}::teleportFromBaseapp: {} targetSpace({}) == currSpaceID({}).\n",
				this->scriptName(), this->id(), spaceID, this->spaceID()));

			_sendBaseTeleportResult(this->id(), sourceBaseAppID, spaceID, lastSpaceID, false);
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyTeleport(PyObject* nearbyMBRef, PyObject* pyposition, PyObject* pydirection)
{
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::teleport: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::teleport: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	Space* currspace = Spaces::findSpace(this->spaceID());
	if(currspace == NULL || !currspace->isGood())
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d, current space has been destroyed!\n", scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}
	
	if(!PySequence_Check(pyposition) || PySequence_Size(pyposition) != 3)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d position not is Sequence!\n", scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(!PySequence_Check(pydirection) || PySequence_Size(pydirection) != 3)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d direction not is Sequence!\n", scriptName(), id());
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
	dir.roll((float)PyFloat_AsDouble(pyitem));
	Py_DECREF(pyitem);

	pyitem = PySequence_GetItem(pydirection, 1);
	dir.pitch((float)PyFloat_AsDouble(pyitem));
	Py_DECREF(pyitem);

	pyitem = PySequence_GetItem(pydirection, 2);
	dir.yaw((float)PyFloat_AsDouble(pyitem));
	Py_DECREF(pyitem);
	
	teleport(nearbyMBRef, pos, dir);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Entity::teleportRefEntity(Entity* entity, Position3D& pos, Direction3D& dir)
{
	if(entity == NULL)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d nearbyEntityRef is null!\n", scriptName(), id());
		PyErr_PrintEx(0);

		onTeleportFailure();
		return;
	}
	
	SPACE_ID lastSpaceID = this->spaceID();
	
	/* ��ʹentity�Ѿ����٣� ���ڴ�δ�ͷ�ʱspaceIDӦ������ȷ�ģ� �������ۿ����ҵ�space
	if(entity->isDestroyed())
	{
		ERROR_MSG("Entity::teleport: nearbyMBRef is destroyed!\n");
		onTeleportFailure();
		return;
	}
	*/

	/* ��ʹ��ghost�� ��space�϶����ڵ�ǰcell�ϣ� ֱ�Ӳ���Ӧ�ò���������
	if(!entity->isReal())
	{
		ERROR_MSG("Entity::teleport: nearbyMBRef is ghost!\n");
		onTeleportFailure();
		return;
	}
	*/

	SPACE_ID spaceID = entity->spaceID();

	// �������ͬspace��Ϊ������ת
	if(spaceID == this->spaceID())
	{
		teleportLocal(entity, pos, dir);
	}
	else
	{
		// ����Ϊ��ǰcellapp�ϵ�space�� ��ô����Ҳ�ܹ�ֱ��ִ�в���
		Space* currspace = Spaces::findSpace(this->spaceID());
		Space* space = Spaces::findSpace(spaceID);

		// ���Ҫ��ת��space�����ڻ������õ�entity�����space�Ĵ��������Ѿ����٣� ��ô��Ӧ������תʧ��
		if(space == NULL || !space->isGood() || entity->isDestroyed())
		{
			if (entity->isDestroyed())
			{
				PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyEntityRef has been destroyed!\n", scriptName(), id());
				PyErr_PrintEx(0);
			}
			else
			{
				PyErr_Format(PyExc_Exception, "%s::teleport: %d, not found space(%d)!\n", scriptName(), id() % spaceID);
				PyErr_PrintEx(0);
			}

			onTeleportFailure();
			return;
		}

		currspace->removeEntity(this);
		this->setPositionAndDirection(pos, dir);
		space->addEntityAndEnterWorld(this);

		onTeleportSuccess(entity, lastSpaceID);
	}
}

//-------------------------------------------------------------------------------------
void Entity::teleportRefMailbox(EntityMailbox* nearbyMBRef, Position3D& pos, Direction3D& dir)
{
	if(hasFlags(ENTITY_FLAGS_TELEPORT_START))
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d, In transit!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);

		onTeleportFailure();
	}
	
	if (!nearbyMBRef->isCellReal())
	{
		char buf[1024];
		nearbyMBRef->c_str(buf, 1024);

		PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyRef error, not is cellMailbox! curr=%s\n", 
			scriptName(), id(), buf);

		PyErr_PrintEx(0);

		onTeleportFailure();
		return;
	}

	// ������entity��base���֣� �����Ǳ����̼���Ĵ��ͣ���ô��ز�������������ִ��
	// ����ǿ�cellapp�Ĵ��ͣ� ��ô���ǿ���������entityΪghost���������л�entity����Ŀ��cellapp
	// ����ڼ���base����Ϣ���͹����� entity��ghost�����ܹ�ת��real��ȥ�� ��˴���֮ǰ����Ҫ��base
	// ��һЩ���ã����ͳɹ���������base�Ĺ�ϵbase�ڱ��ı��ϵ����Ȼ��0.1���ʱ���յ�����������ghost��
	// ���һֱ�а���һֱˢ��ʱ��ֱ��û���κΰ���Ҫ�㲥���ҳ�ʱ0.1��֮��İ��Ż�ֱ�ӷ���real��, �������ĺô��Ǵ��Ͳ�����Ҫ�ǳ���������base���
	// ���͹��������κδ���Ҳ����Ӱ�쵽base���֣�base���ֵİ�Ҳ�ܹ�������������real��
	if(this->baseMailbox() != NULL)
	{
		// �����base����, ���ǻ���Ҫ����һ�±��ݹ��ܡ�
		// ����ghost���ܻ�addCellDataToStreamһ���������������ڴ���ʧ��ʱ�����ø�ʵ��
		// ������ﲻ��Ҫ���б���
		// this->backupCellData();
		
		Network::Channel* pBaseChannel = baseMailbox()->getChannel();
		if(pBaseChannel)
		{
			// ͬʱ��Ҫ֪ͨbase�ݴ淢��cellapp����Ϣ����Ϊ���������ת�ɹ���Ҫ�л�cellMailboxӳ���ϵ���µ�cellapp
			// Ϊ�˱������л���һ˲����Ϣ����������(�ɵ�cellapp��ϢҲ��ת���µ�cellapp��)�� �����Ҫ�ڴ���ǰ����
			// �ݴ棬 ���ͳɹ���֪ͨ�ɵ�cellapp����entity֮��ͬʱ֪ͨbaseapp�ı�ӳ���ϵ��
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			(*pBundle).newMessage(BaseappInterface::onMigrationCellappStart);
			(*pBundle) << id();
			(*pBundle) << g_componentID;
			(*pBundle) << nearbyMBRef->componentID();
			pBaseChannel->send(pBundle);
		}
		else
		{
			PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyRef error, not found baseapp!\n", scriptName(), id());
			PyErr_PrintEx(0);
			onTeleportFailure();
			return;
		}
	}

	onTeleportRefMailbox(nearbyMBRef, pos, dir);
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportRefMailbox(EntityMailbox* nearbyMBRef, Position3D& pos, Direction3D& dir)
{
	// ������Ҫ��entity�������Ŀ��cellapp
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(CellappInterface::reqTeleportToCellApp);
	(*pBundle) << id();
	(*pBundle) << nearbyMBRef->id();
	(*pBundle) << spaceID();
	(*pBundle) << pScriptModule()->getUType();
	(*pBundle) << pos.x << pos.y << pos.z;
	(*pBundle) << dir.roll() << dir.pitch() << dir.yaw();
	(*pBundle) << g_componentID;

	MemoryStream* s = MemoryStream::createPoolObject();
	changeToGhost(nearbyMBRef->componentID(), *s);

	(*pBundle).append(s);
	MemoryStream::reclaimPoolObject(s);

	// ��ʱ���������entity, ���Ǳ߳ɹ�����֮���ٻ�������
	// ���ڼ����Ϣ����ͨ��ghostת����real
	// ���δ����ȷ�����ȥ����Դӵ�ǰcell�����ָ�entity.
	// Cellapp::getSingleton().destroyEntity(id(), false);

	nearbyMBRef->postMail(pBundle);

	// ���л���entity��ֹͣ�ƶ��� �������ʧ��������Ը������л������ݽ��лָ�
	stopMove();
}

//-------------------------------------------------------------------------------------
void Entity::teleportLocal(PyObject_ptr nearbyMBRef, Position3D& pos, Direction3D& dir)
{
	// ������ת��δ����Ҫ����space���ָ�Ϊ��cell������� ��ǰֱ�Ӳ���
	SPACE_ID lastSpaceID = this->spaceID();

	// ��Ҫ��CoordinateSystem��ɾ��entity�ڵ�
	Space* currspace = Spaces::findSpace(this->spaceID());
	this->uninstallCoordinateNodes(currspace->pCoordinateSystem());

	// ��ʱ�����Ŷ�ranglist
	this->setPositionAndDirection(pos, dir);

	if(this->pWitness())
	{
		// ֪ͨλ��ǿ�Ƹı�
		Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(id(), (*pSendBundle));
		
		ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);
		(*pSendBundle) << id();
		(*pSendBundle) << pos.x << pos.y << pos.z;
		(*pSendBundle) << direction().roll() << direction().pitch() << direction().yaw();

		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);
		this->pWitness()->sendToClient(ClientInterface::onSetEntityPosAndDir, pSendBundle);
	}

	currspace->addEntityToNode(this);

	std::list<ENTITY_ID>::iterator witer = witnesses_.begin();
	for (; witer != witnesses_.end(); ++witer)
	{
		Entity* pEntity = Cellapp::getSingleton().findEntity((*witer));
		if (pEntity == NULL || pEntity->pWitness() == NULL)
			continue;

		EntityMailbox* clientMailbox = pEntity->clientMailbox();
		if (clientMailbox == NULL)
			continue;

		Network::Channel* pChannel = clientMailbox->getChannel();
		if (pChannel == NULL)
			continue;

		// ����������Ǵ��ڵģ�����������Դ��createWitnessFromStream()
		// �����Լ���entity��δ��Ŀ��ͻ����ϴ���
		if (!pEntity->pWitness()->entityInAOI(id()))
			continue;

		// ֪ͨλ��ǿ�Ƹı�
		Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity->id(), (*pSendBundle));
		
		ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);
		(*pSendBundle) << id();
		(*pSendBundle) << pos.x << pos.y << pos.z;
		(*pSendBundle) << direction().roll() << direction().pitch() << direction().yaw();

		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);
		pEntity->pWitness()->sendToClient(ClientInterface::onSetEntityPosAndDir, pSendBundle);
	}

	onTeleportSuccess(nearbyMBRef, lastSpaceID);
}

//-------------------------------------------------------------------------------------
void Entity::teleport(PyObject_ptr nearbyMBRef, Position3D& pos, Direction3D& dir)
{
	/*
		1: �κ���ʽ��teleport������Ϊ��˲���ƶ��ģ���ͻ�ƿռ����ƽ��뵽�κοռ䣩�� �������ڵ�ǰλ��ֻ�ƶ���0.1��, �����������ǰentity
			�պ���ĳ��trap�У� teleport��ǰ�ƶ�0.1�׵���û�г�trap�� ��Ϊ����˲���ƶ�����������Ŀǰ��Ϊ
			entity�����뿪trap���Ҵ�����ػص�, Ȼ��˲ʱ����������һ���㣬 ��ô��Ϊ�õ�Ҳ���ڵ�ǰtrap�������ֻ��׳�����trap�ص�.

		2: ����ǵ�ǰspace����ת�����������ƶ�����

		3: �������ת������space��, �����Ǹ�spaceҲ�ڵ�ǰcellapp�ϵ����ʱ�� ����ִ����ת����(��Ϊ����Ҫ�����κ�������ϵ��ά���� ֱ���л��ͺ���)�� 
		
		4: ���Ҫ��ת��Ŀ��space����һ��cellapp�ϣ�
			4.1: ��ǰentityû��base���֣� ������ά��base���ֵĹ�ϵ�� ���ǻ���Ҫ�����������������תʧ�ܣ� ��ô��ʱӦ�÷�����תʧ�ܻص����Ҽ���
			���������ڵ�ǰspace�ϡ�
		
			4.2: ��ǰentity��base���֣� ��ô������Ҫ�ı�base��ӳ���cell����(������δ��ʽ�л���ϵʱbaseapp�������ʹ�cell����Ϣ��Ӧ�ò�����ʧ)�� Ϊ�˰�ȫ������Ҫ��һЩ����
	*/

	Py_INCREF(this);

	// ���ΪNone����entity�Լ����ڱ�space����ת��ĳλ��
	if(nearbyMBRef == Py_None)
	{
		// ֱ��ִ�в���
		teleportLocal(nearbyMBRef, pos, dir);
	}
	else
	{
		//EntityMailbox* mb = NULL;

		// �����entity��һ�����ڱ�cellapp�ϣ� ����ֱ�ӽ��в���
		if(PyObject_TypeCheck(nearbyMBRef, Entity::getScriptType()))
		{
			teleportRefEntity(static_cast<Entity*>(nearbyMBRef), pos, dir);
		}
		else
		{
			// �����mailbox, �ȼ�鱾cell���Ƿ��ܹ�ͨ�����mailbox��ID�ҵ�entity
			// ������ҵ���Ҳ���ڱ�cellapp�Ͽ�ֱ�ӽ��в���
			if(PyObject_TypeCheck(nearbyMBRef, EntityMailbox::getScriptType()))
			{
				EntityMailbox* mb = static_cast<EntityMailbox*>(nearbyMBRef);
				Entity* entity = Cellapp::getSingleton().findEntity(mb->id());
				
				if(entity)
				{
					teleportRefEntity(entity, pos, dir);
				}
				else
				{
					teleportRefMailbox(mb, pos, dir);
				}
			}
			else
			{
				// �������entity�� Ҳ����mailboxͬʱҲ����None? �ǿ϶����������
				PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyRef error!\n", scriptName(), id());
				PyErr_PrintEx(0);

				onTeleportFailure();
			}
		}
	}
	
	Py_DECREF(this);
}

//-------------------------------------------------------------------------------------
void Entity::onTeleport()
{
	// �����������base.teleport��ת֮ǰ�����ã� cell.teleport�ǲ��ᱻ���õġ�
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onTeleport"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportFailure()
{
	ERROR_MSG(fmt::format("{}::onTeleportFailure(): entityID={}\n", 
		this->scriptName(), id()));

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onTeleportFailure"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportSuccess(PyObject* nearbyEntity, SPACE_ID lastSpaceID)
{
	EntityMailbox* mb = this->baseMailbox();
	if(mb)
	{
		_sendBaseTeleportResult(this->id(), mb->componentID(), this->spaceID(), lastSpaceID, true);
	}

	// ���������trap�ȴ���������������ӽ�ȥ
	restoreProximitys();

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onTeleportSuccess"),
		Py_BuildValue(const_cast<char*>("(O)"), nearbyEntity));
}

//-------------------------------------------------------------------------------------
void Entity::onEnterSpace(Space* pSpace)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onEnterSpace"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveSpace(Space* pSpace)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onLeaveSpace"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onEnteredCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onEnteredCell"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onEnteringCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onEnteringCell"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onLeavingCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onLeavingCell"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onLeftCell()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onLeftCell"), NULL);
}

//-------------------------------------------------------------------------------------
void Entity::onRestore()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onRestore"), NULL);
	removeFlags(ENTITY_FLAGS_INITING);
}

//-------------------------------------------------------------------------------------
bool Entity::_reload(bool fullReload)
{
	allClients_->setScriptModule(pScriptModule_);
	return true;
}

//-------------------------------------------------------------------------------------
void Entity::onUpdateGhostPropertys(KBEngine::MemoryStream& s)
{
	ENTITY_PROPERTY_UID utype;
	s >> utype;

	PropertyDescription* pPropertyDescription = pScriptModule()->findCellPropertyDescription(utype);
	if(pPropertyDescription == NULL)
	{
		ERROR_MSG(fmt::format("{}::onUpdateGhostPropertys: not found propertyID({}), entityID({})\n", 
			scriptName(), utype, id()));

		s.done();
		return;
	}

	DEBUG_MSG(fmt::format("{}::onUpdateGhostPropertys: property({}), entityID({})\n", 
		scriptName(), pPropertyDescription->getName(), id()));

	PyObject* pyVal = pPropertyDescription->createFromStream(&s);
	if(pyVal == NULL)
	{
		ERROR_MSG(fmt::format("{}::onUpdateGhostPropertys: entityID={}, create({}) error!\n", 
			scriptName(), id(), pPropertyDescription->getName()));

		s.done();
		return;
	}

	PyObject_SetAttrString(static_cast<PyObject*>(this),
				pPropertyDescription->getName(), pyVal);

	Py_DECREF(pyVal);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteRealMethodCall(KBEngine::MemoryStream& s)
{
	ENTITY_METHOD_UID utype;
	s >> utype;

	MethodDescription* pMethodDescription = pScriptModule()->findCellMethodDescription(utype);
	if(pMethodDescription == NULL)
	{
		ERROR_MSG(fmt::format("{}::onRemoteRealMethodCall: not found propertyID({}), entityID({})\n", 
			scriptName(), utype, id()));

		s.done();
		return;
	}

	onRemoteMethodCall_(pMethodDescription, id(), s);
}

//-------------------------------------------------------------------------------------
void Entity::onUpdateGhostVolatileData(KBEngine::MemoryStream& s)
{
	DEBUG_MSG(fmt::format("{}::onUpdateGhostVolatileData: entityID({})\n", 
		scriptName(), id()));
}

//-------------------------------------------------------------------------------------
void Entity::changeToGhost(COMPONENT_ID realCell, KBEngine::MemoryStream& s)
{
	// һ��entityҪת��Ϊghost
	// ������Ҫ���������realCell
	// ������def������ӽ���
	// ���л�controller��ֹͣ���е�controller(timer, navigate, trap,...)
	// ж��witness�� �������л�
	KBE_ASSERT(isReal() == true && "Entity::changeToGhost(): not is real.\n");
	KBE_ASSERT(realCell_ != g_componentID);

	realCell_ = realCell;
	ghostCell_ = 0;
	
	GhostManager* gm = Cellapp::getSingleton().pGhostManager();
	if(gm)
	{
		gm->addRoute(id(), realCell_);
	}

	DEBUG_MSG(fmt::format("{}::changeToGhost(): {}, realCell={}, spaceID={}, position=({},{},{}).\n", 
		scriptName(), id(), realCell_, spaceID_, position().x, position().y, position().z));
	
	// �������ǰ��
	addToStream(s);

	//witnesses_.clear();
	//witnesses_count_ = 0;

	if(pControllers_)
	{
		pControllers_->clear();
		SAFE_RELEASE(pControllers_);
	}

	if(pWitness())
	{
		pWitness()->clear(this);
		Witness::reclaimPoolObject(pWitness_);
		pWitness_ = NULL;
	}

	scriptTimers_.cancelAll();
	pyCallbackMgr_.finalise();
}

//-------------------------------------------------------------------------------------
void Entity::changeToReal(COMPONENT_ID ghostCell, KBEngine::MemoryStream& s)
{
	// һ��entityҪת��Ϊreal
	// ������Ҫ���������ghostCell
	// ������def������ӽ���
	// �����л�controller���ָ����е�controller(timer, navigate, trap,...)
	// �����л���װwitness
	KBE_ASSERT(isReal() == false && "Entity::changeToReal(): not is ghost.\n");

	ghostCell_ = ghostCell;
	realCell_ = 0;

	DEBUG_MSG(fmt::format("{}::changeToReal(): {}, ghostCell={}, spaceID={}, position=({},{},{}).\n",
		scriptName(), id(), ghostCell_, spaceID_, position().x, position().y, position().z));

	createFromStream(s);
}

//-------------------------------------------------------------------------------------
void Entity::addToStream(KBEngine::MemoryStream& s)
{
	COMPONENT_ID baseMailboxComponentID = 0;
	if(baseMailbox_)
	{
		baseMailboxComponentID = baseMailbox_->componentID();
	}

	bool hasCustomVolatileinfo = (pCustomVolatileinfo_ != NULL);
	ENTITY_ID controlledByID = (controlledBy_ != NULL ? controlledBy_->id() : 0);
		
	s << pScriptModule_->getUType() << spaceID_ << isDestroyed_ << 
		isOnGround_ << topSpeed_ << topSpeedY_ << 
		layer_ << baseMailboxComponentID << hasCustomVolatileinfo << controlledByID;

	if (pCustomVolatileinfo_)
		pCustomVolatileinfo_->addToStream(s);

	addCellDataToStream(ENTITY_CELL_DATA_FLAGS, &s);
	
	addMovementHandlerToStream(s);
	addControllersToStream(s);
	addWitnessToStream(s);
	addTimersToStream(s);

	pyCallbackMgr_.addToStream(s);
}

//-------------------------------------------------------------------------------------
void Entity::createFromStream(KBEngine::MemoryStream& s)
{
	ENTITY_SCRIPT_UID scriptUType;
	COMPONENT_ID baseMailboxComponentID;
	bool hasCustomVolatileinfo;
	ENTITY_ID controlledByID;

	s >> scriptUType >> spaceID_ >> isDestroyed_ >> isOnGround_ >> topSpeed_ >> 
		topSpeedY_ >> layer_ >> baseMailboxComponentID >> hasCustomVolatileinfo >> controlledByID;

	if (hasCustomVolatileinfo)
	{
		if (!pCustomVolatileinfo_)
			pCustomVolatileinfo_ = new VolatileInfo();

		pCustomVolatileinfo_->createFromStream(s);
	}

	// ��ʱǿ������Ϊ���ڵ��棬�޷��ж����Ƿ��ڵ��棬��ɫ��Ҫ�ͻ����ϱ��Ƿ��ڵ���
	// ������˵�NPC�����ƶ����Ƿ��ڵ������ж���
	isOnGround_ = false;

	this->pScriptModule_ = EntityDef::findScriptModule(scriptUType);

	KBE_ASSERT(this->pScriptModule_);

	// ����entity��baseMailbox
	if(baseMailboxComponentID > 0)
		baseMailbox(new EntityMailbox(pScriptModule(), NULL, baseMailboxComponentID, id_, MAILBOX_TYPE_BASE));

	// �������ǰ�Ŀ�������ϵͳ���Լ��Ŀͻ��ˣ����������
	// ����������ͻ����ڿ��ƣ����Իָ����ƹ�ϵ������޷��ָ���������
	if (controlledByID == id())
		controlledBy(baseMailbox());
	else if (controlledByID == 0)
		controlledBy(NULL);
	else
	{
		Entity* controllerEntity = Cellapp::getSingleton().findEntity(controlledByID);
		if (controllerEntity && spaceID() == controllerEntity->spaceID() && \
			controllerEntity->clientMailbox())
			controlledBy(controllerEntity->baseMailbox());
		else
			setControlledBy(baseMailbox());
	}

	PyObject* cellData = createCellDataFromStream(&s);
	createNamespace(cellData);
	Py_XDECREF(cellData);

	removeFlags(ENTITY_FLAGS_INITING);
	
	createMovementHandlerFromStream(s);
	createControllersFromStream(s);
	createWitnessFromStream(s);
	createTimersFromStream(s);

	pyCallbackMgr_.createFromStream(s);
	setDirty();
}

//-------------------------------------------------------------------------------------
void Entity::addControllersToStream(KBEngine::MemoryStream& s)
{
	if(pControllers_)
	{
		s << true;

		// �����������ƶ���ص�Controllers
		stopMove();

		pControllers_->addToStream(s);
	}
	else
	{
		s << false;
	}
}

//-------------------------------------------------------------------------------------
void Entity::createControllersFromStream(KBEngine::MemoryStream& s)
{
	bool hasControllers;
	s >> hasControllers;

	if(hasControllers)
	{
		if(pControllers_ == NULL)
			pControllers_ = new Controllers(id());

		pControllers_->createFromStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Entity::addWitnessToStream(KBEngine::MemoryStream& s)
{
	uint32 size = witnesses_count_;
	s << size;

	std::list<ENTITY_ID>::iterator iter = witnesses_.begin();
	for(; iter != witnesses_.end(); ++iter)
	{
		s << (*iter);
	}

	if(pWitness())
	{
		s << true;
		pWitness()->addToStream(s);
	}
	else
	{
		s << false;
	}
}

//-------------------------------------------------------------------------------------
void Entity::createWitnessFromStream(KBEngine::MemoryStream& s)
{
	uint32 size;
	s >> size;

	if (witnesses_count_ > 0)
	{
		WARNING_MSG(fmt::format("{}::createWitnessFromStream: witnesses_count({}/{}) != 0! entityID={}, isReal={}\n",
			scriptName(), witnesses_.size(), witnesses_count_, id(), isReal()));

		/*
		std::list<ENTITY_ID>::iterator it = witnesses_.begin();
		for (; it != witnesses_.end(); ++it)
		{
			Entity *ent = Cellapp::getSingleton().findEntity((*it));

			if (ent)
			{
				bool inTargetAOI = false;

				if (ent->pWitness())
				{
					Witness::AOI_ENTITIES::iterator aoi_iter = ent->pWitness()->aoiEntities().begin();
					for (; aoi_iter != ent->pWitness()->aoiEntities().end(); ++aoi_iter)
					{
						if ((*aoi_iter)->pEntity() == this)
						{
							inTargetAOI = true;
							break;
						}
					}
				}

				ERROR_MSG(fmt::format("\t=>witnessed={}({}), isDestroyed={}, isReal={}, inTargetAOI={}, spaceID={}, position=({},{},{})\n",
					ent->scriptName(), (*it), ent->isDestroyed(), ent->isReal(), inTargetAOI, ent->spaceID(), ent->position().x, ent->position().y, ent->position().z));
			}
			else
			{
				ERROR_MSG(fmt::format("\t=> witnessed={}, not found entity!\n", (*it)));
			}
		}

		KBE_ASSERT(witnesses_count_ == 0);
		*/
		
		for (uint32 i = 0; i < size; ++i)
		{
			ENTITY_ID entityID;
			s >> entityID;
		}
	}
	else
	{
		for (uint32 i = 0; i < size; ++i)
		{
			ENTITY_ID entityID;
			s >> entityID;

			Entity* pEntity = Cellapp::getSingleton().findEntity(entityID);
			if (pEntity == NULL || pEntity->spaceID() != spaceID())
				continue;

			witnesses_.push_back(entityID);
			++witnesses_count_;
		}
	}

	bool hasWitness;
	s >> hasWitness;

	if(hasWitness)
	{
		PyObject* clientMB = PyObject_GetAttrString(baseMailbox(), "client");
		KBE_ASSERT(clientMB != Py_None);

		EntityMailbox* client = static_cast<EntityMailbox*>(clientMB);	
		clientMailbox(client);

		// ��Ҫʹ��setWitness����Ϊ��ʱ����Ҫ��onAttach���̣��ͻ��˲���Ҫ����enterworld��
		// setWitness(Witness::createPoolObject());
		pWitness_ = Witness::createPoolObject();
		pWitness_->pEntity(this);
		pWitness_->createFromStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Entity::addMovementHandlerToStream(KBEngine::MemoryStream& s)
{
	if(pMoveController_)
	{
		s << true;
		pMoveController_->addToStream(s);
	}
	else
	{
		s << false;
	}
	
	if(pTurnController_)
	{
		s << true;
		pTurnController_->addToStream(s);
	}
	else
	{
		s << false;
	}	
}

//-------------------------------------------------------------------------------------
void Entity::createMovementHandlerFromStream(KBEngine::MemoryStream& s)
{
	bool hasMoveHandler;
	s >> hasMoveHandler;

	if(hasMoveHandler)
	{
		stopMove();

		pMoveController_ = KBEShared_ptr<Controller>(new MoveController(this));
		pMoveController_->createFromStream(s);
		pControllers_->add(pMoveController_);
	}
	
	bool hasTurnHandler;
	s >> hasTurnHandler;

	if(hasTurnHandler)
	{
		if(!hasMoveHandler)
			stopMove();
		
		pTurnController_ = KBEShared_ptr<Controller>(new TurnController(this));
		pTurnController_->createFromStream(s);
		pControllers_->add(pTurnController_);
	}	
}

//-------------------------------------------------------------------------------------
void Entity::onTimer(ScriptID timerID, int useraAgs)
{
	SCOPED_PROFILE(ONTIMER_PROFILE);

	bufferOrExeCallback(const_cast<char*>("onTimer"),
		Py_BuildValue(const_cast<char*>("(Ii)"), timerID, useraAgs));
}

//-------------------------------------------------------------------------------------
void Entity::addTimersToStream(KBEngine::MemoryStream& s)
{
	ScriptTimers::Map& map = scriptTimers_.map();
	uint32 size = map.size();
	s << size;

	ScriptTimers::Map::const_iterator iter = map.begin();
	while (iter != map.end())
	{
		// timerID
		s << iter->first;

		uint32 time;
		uint32 interval;
		void* pUser;

		Cellapp::getSingleton().timers().getTimerInfo(iter->second, time, interval, pUser);
		int32 userData = int32(uintptr(pUser));
		s << time << interval << userData;
		++iter;
	}
}

//-------------------------------------------------------------------------------------
void Entity::createTimersFromStream(KBEngine::MemoryStream& s)
{
	uint32 size;
	s >> size;

	for(uint32 i=0; i<size; ++i)
	{
		ScriptID tid;
		uint32 time;
		uint32 interval;
		int32 userData = 0;

		s >> tid >> time >> interval >> userData;

		EntityScriptTimerHandler* pEntityScriptTimerHandler = new EntityScriptTimerHandler(this);

		TimerHandle timerHandle = Cellapp::getSingleton().timers().add(
				time, interval,
				pEntityScriptTimerHandler, (void *)(intptr_t)userData);
		
		scriptTimers_.directAddTimer(tid, timerHandle);
	}
}

//-------------------------------------------------------------------------------------
}
