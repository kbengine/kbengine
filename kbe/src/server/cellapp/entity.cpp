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
#include "range_trigger.hpp"
#include "all_clients.hpp"
#include "client_entity.hpp"
#include "controllers.hpp"	
#include "real_entity_method.hpp"
#include "entity_coordinate_node.hpp"
#include "proximity_controller.hpp"
#include "move_controller.hpp"	
#include "moveto_point_handler.hpp"	
#include "moveto_entity_handler.hpp"	
#include "navigate_handler.hpp"	
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"	
#include "network/fixed_messages.hpp"
#include "client_lib/client_interface.hpp"
#include "helper/eventhistory_stats.hpp"
#include "navigation/navigation.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"

#ifndef CODE_INLINE
#include "entity.ipp"
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
ENTITY_METHOD_DECLARE_BEGIN(Cellapp, Entity)
SCRIPT_METHOD_DECLARE("setAoiRadius",				pySetAoiRadius,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("isReal",						pyIsReal,						METH_VARARGS,				0)	
SCRIPT_METHOD_DECLARE("addProximity",				pyAddProximity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("clientEntity",				pyClientEntity,					METH_VARARGS,				0)
SCRIPT_METHOD_DECLARE("cancelController",			pyCancelController,				METH_VARARGS,				0)
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
SCRIPT_GET_DECLARE("isOnGround",					pyGetIsOnGround,				0,							0)
SCRIPT_GET_DECLARE("spaceID",						pyGetSpaceID,					0,							0)
SCRIPT_GETSET_DECLARE("layer",						pyGetLayer,						pySetLayer,					0,		0)
SCRIPT_GETSET_DECLARE("position",					pyGetPosition,					pySetPosition,				0,		0)
SCRIPT_GETSET_DECLARE("direction",					pyGetDirection,					pySetDirection,				0,		0)
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
allClients_(new AllClients(scriptModule, id, false)),
otherClients_(new AllClients(scriptModule, id, true)),
pEntityCoordinateNode_(NULL),
pControllers_(new Controllers(id)),
pMoveController_(NULL),
pyPositionChangedCallback_(),
pyDirectionChangedCallback_(),
layer_(0)
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
}

//-------------------------------------------------------------------------------------
Entity::~Entity()
{
	ENTITY_DECONSTRUCTION(Entity);

	S_RELEASE(clientMailbox_);
	S_RELEASE(baseMailbox_);
	S_RELEASE(allClients_);
	S_RELEASE(otherClients_);
	
	KBE_ASSERT(pWitness_ == NULL);

	SAFE_RELEASE(pControllers_);
	SAFE_RELEASE(pEntityCoordinateNode_);

	Py_DECREF(pPyPosition_);
	pPyPosition_ = NULL;
	
	Py_DECREF(pPyDirection_);
	pPyDirection_ = NULL;
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
void Entity::onDestroy(bool callScript)
{
	if(callScript)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onDestroy"));
		
		// �����֪ͨ�ű��� ��ôҲ�����������ص�
		// ͨ������һ��entity��֪ͨ�ű�������Ǩ�ƻ��ߴ�����ɵ�
		if(baseMailbox_ != NULL)
		{
			this->backupCellData();

			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			(*pBundle).newMessage(BaseappInterface::onLoseCell);
			(*pBundle) << id_;
			baseMailbox_->postMail((*pBundle));
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}
	}

	stopMove();

	if(pWitness_)
	{
		pWitness_->detach(this);
		Witness::ObjPool().reclaimObject(pWitness_);
		pWitness_ = NULL;
	}

	// ��entity�ӳ������޳�
	Space* space = Spaces::findSpace(this->spaceID());
	if(space)
	{
		space->removeEntity(this);
	}

	pPyPosition_->onLoseRef();
	pPyDirection_->onLoseRef();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs)
{
	uint16 currargsSize = PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);

	if(pobj->initing())
	{
		PyErr_Format(PyExc_AssertionError,
			"Entity::destroy(): %s is in initing, reject the request!\n",	
			pobj->scriptName());
		PyErr_PrintEx(0);
		return NULL;
	}

	if(currargsSize > 0)
	{
		PyErr_Format(PyExc_AssertionError,
						"%s: args max require %d args, gived %d! is script[%s].\n",	
			__FUNCTION__, 0, currargsSize, pobj->scriptName());
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

	if(this->isDestroyed())
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
PyObject* Entity::pyGetBaseMailbox()
{ 
	EntityMailbox* mailbox = baseMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
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

	// �����ghost����def��������Ҫrpc���á�
	if(!isReal())
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		PyMem_Free(PyUnicode_AsWideCharStringRet0);

		MethodDescription* md = const_cast<ScriptDefModule*>(scriptModule())->findCellMethodDescription(ccattr);
		free(ccattr);

		if(md != NULL)
		{
			return new RealEntityMethod(md, this);
		}
	}

	return ScriptObject::onScriptGetAttribute(attr);
}	

//-------------------------------------------------------------------------------------
void Entity::onDefDataChanged(const PropertyDescription* propertyDescription, PyObject* pyData)
{
	// �������һ��realEntity�����ڳ�ʼ�������
	if(!isReal() || initing_)
		return;

	uint32 flags = propertyDescription->getFlags();

	// ���ȴ���һ����Ҫ�㲥��ģ����
	MemoryStream* mstream = MemoryStream::ObjPool().createObject();

	propertyDescription->getDataType()->addToStream(mstream, pyData);

	// �ж��Ƿ���Ҫ�㲥��������cellapp, �⻹��һ��ǰ����entity����ӵ��ghostʵ��
	// ֻ����cell�߽�һ����Χ�ڵ�entity��ӵ��ghostʵ��, ��������תspaceʱҲ����ݵ���Ϊghost״̬
	if((flags & ENTITY_BROADCAST_CELL_FLAGS) > 0 && hasGhost())
	{
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
		(*pForwardBundle).newMessage(CellappInterface::onUpdateGhostPropertys);
		(*pForwardBundle) << id();
		(*pForwardBundle) << propertyDescription->getUType();

		pForwardBundle->append(*mstream);

		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if(gm)
		{
			// ��¼����¼���������������С
			g_publicCellEventHistoryStats.trackEvent(scriptName(), 
				propertyDescription->getName(), 
				pForwardBundle->currMsgLength());

			gm->pushMessage(ghostCell(), pForwardBundle);
		}
		else
		{
			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
		}
	}
	
	const Position3D& basePos = this->position(); 
	if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) > 0)
	{
		DETAIL_TYPE propertyDetailLevel = propertyDescription->getDetailLevel();

		std::list<ENTITY_ID>::iterator witer = witnesses_.begin();
		for(; witer != witnesses_.end(); witer++)
		{
			Entity* pEntity = Cellapp::getSingleton().findEntity((*witer));
			if(pEntity == NULL || pEntity->pWitness() == NULL)
				continue;

			EntityMailbox* clientMailbox = pEntity->clientMailbox();
			if(clientMailbox == NULL)
				continue;

			Mercury::Channel* pChannel = clientMailbox->getChannel();
			if(pChannel == NULL)
				continue;

			if(!pEntity->pWitness()->entityInAOI(id()))
				continue;

			const Position3D& targetPos = pEntity->position();
			Position3D lengthPos = targetPos - basePos;

			if(scriptModule_->getDetailLevel().level[propertyDetailLevel].inLevel(lengthPos.length()))
			{
				Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

				pEntity->pWitness()->addSmartAOIEntityMessageToBundle(pForwardBundle, ClientInterface::onUpdatePropertys, 
					ClientInterface::onUpdatePropertysOptimized, id());

				if(scriptModule_->usePropertyDescrAlias())
					(*pForwardBundle) << propertyDescription->aliasIDAsUint8();
				else
					(*pForwardBundle) << propertyDescription->getUType();

				pForwardBundle->append(*mstream);
				
				// ��¼����¼���������������С
				g_publicClientEventHistoryStats.trackEvent(scriptName(), 
					propertyDescription->getName(), 
					pForwardBundle->currMsgLength());

				Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity->id(), (*pSendBundle), (*pForwardBundle));

				pEntity->pWitness()->sendToClient(ClientInterface::onUpdatePropertysOptimized, pSendBundle);
				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
			}
		}
	}

	/*
	// �ж���������Ƿ���Ҫ�㲥�������ͻ���
	if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) > 0)
	{
		int8 detailLevel = propertyDescription->getDetailLevel();
		for(int8 i=DETAIL_LEVEL_NEAR; i<=detailLevel; i++)
		{
			std::map<ENTITY_ID, Entity*>::iterator iter = witnessEntities_[i].begin();
			for(; iter != witnessEntities_[i].end(); iter++)
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
		for(int8 i=detailLevel; i<=DETAIL_LEVEL_FAR; i++)
		{
			std::map<ENTITY_ID, Entity*>::iterator iter = witnessEntities_[i].begin();
			for(; iter != witnessEntities_[i].end(); iter++)
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
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
		(*pForwardBundle).newMessage(ClientInterface::onUpdatePropertys);
		(*pForwardBundle) << id();

		if(scriptModule_->usePropertyDescrAlias())
			(*pForwardBundle) << propertyDescription->aliasIDAsUint8();
		else
			(*pForwardBundle) << propertyDescription->getUType();

		pForwardBundle->append(*mstream);
		
		// ��¼����¼���������������С
		if((flags & ENTITY_BROADCAST_OTHER_CLIENT_FLAGS) <= 0)
		{
			g_privateClientEventHistoryStats.trackEvent(scriptName(), 
				propertyDescription->getName(), 
				pForwardBundle->currMsgLength());
		}

		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(id(), (*pSendBundle), (*pForwardBundle));

		pWitness_->sendToClient(ClientInterface::onUpdatePropertys, pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	}

	MemoryStream::ObjPool().reclaimObject(mstream);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_METHOD_UID utype = 0;
	s >> utype;

	MethodDescription* md = scriptModule_->findCellMethodDescription(utype);

	if(md == NULL)
	{
		ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: can't found method. utype={0}, callerID:{1}.\n"
			, utype, id_, this->scriptName()));

		return;
	}

	onRemoteMethodCall_(md, s);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteCallMethodFromClient(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_METHOD_UID utype = 0;
	s >> utype;

	MethodDescription* md = scriptModule_->findCellMethodDescription(utype);
	if(md)
	{
		if(!md->isExposed())
		{
			ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: {0} not is exposed, call is illegal! entityID:{1}.\n",
				md->getName(), this->id(), this->scriptName()));

			s.opfini();
			return;
		}
	}
	else
	{
		ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: can't found method. utype={0}, callerID:{1}.\n",
			utype, id_, this->scriptName()));

		return;
	}

	onRemoteMethodCall_(md, s);
}

//-------------------------------------------------------------------------------------
void Entity::onRemoteMethodCall_(MethodDescription* md, MemoryStream& s)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	if (isDestroyed())
	{
		ERROR_MSG(fmt::format("{}::onRemoteMethodCall: {} is destroyed!\n",
			scriptName(), id()));

		s.opfini();
		return;
	}

	if(md == NULL)
	{
		ERROR_MSG(fmt::format("{1}::onRemoteMethodCall: can't found method, callerID:{0}.\n",
			id_, this->scriptName()));

		return;
	}

	if(g_debugEntity)
	{
		DEBUG_MSG(fmt::format("Entity::onRemoteMethodCall: {0}, {3}::{1}(utype={2}).\n",
			id_, md->getName(), md->getUType(), this->scriptName()));
	}

	md->currCallerID(this->id());

	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>
						(md->getName()));

	if(md != NULL)
	{
		if(!md->isExposed() && md->getArgSize() == 0)
		{
			md->call(pyFunc, NULL);
		}
		else
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
				s.opfini();
			}
		}
	}
	else
	{
		s.opfini();
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
					scriptModule_->getCellPropertyDescriptions();

	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			// DEBUG_MSG(fmt::format("Entity::addCellDataToStream: {}.\n", propertyDescription->getName()));
			PyObject* pyVal = PyDict_GetItemString(cellData, propertyDescription->getName());

			if(useAliasID && scriptModule_->usePropertyDescrAlias())
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
					propertyDescription->getName(), pyVal->ob_type->tp_name, propertyDescription->getDataType()->getName()));
				
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
				DEBUG_MSG(fmt::format("{}::addCellDataToStream: {} is error!\n", this->scriptName(),
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
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onBackupEntityCellData);
		(*pBundle) << id_;

		MemoryStream* s = MemoryStream::ObjPool().createObject();
		addCellDataToStream(ENTITY_CELL_DATA_FLAGS, s);
		(*pBundle).append(s);
		MemoryStream::ObjPool().reclaimObject(s);

		baseMailbox_->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
	else
	{
		WARNING_MSG(fmt::format("Entity::backupCellData(): {} {} has no base!\n", 
			this->scriptName(), this->id()));
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
	(*pBundle) << this->id();
	(*pBundle) << callbackID;
	if(this->baseMailbox())
	{
		this->baseMailbox()->postMail((*pBundle));
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
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
PyObject* Entity::pyIsReal()
{
	return PyBool_FromLong(isReal());
}

//-------------------------------------------------------------------------------------
void Entity::addWitnessed(Entity* entity)
{
	Cellapp::getSingleton().pWitnessedTimeoutHandler()->delWitnessed(this);

	witnesses_.push_back(entity->id());
	++witnesses_count_;

	/*
	int8 detailLevel = scriptModule_->getDetailLevel().getLevelByRange(range);
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
		SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onWitnessed"), 
			const_cast<char*>("O"), PyBool_FromLong(1));
	}
}

//-------------------------------------------------------------------------------------
void Entity::delWitnessed(Entity* entity)
{
	KBE_ASSERT(witnesses_count_ > 0);

	witnesses_.remove(entity->id());
	--witnesses_count_;

	// ��ʱִ��
	// onDelWitnessed();
	Cellapp::getSingleton().pWitnessedTimeoutHandler()->addWitnessed(this);
}

//-------------------------------------------------------------------------------------
void Entity::onDelWitnessed()
{
	if(witnesses_count_ == 0)
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
void Entity::restoreProximitys()
{
	if(this->pWitness() && this->pWitness()->pAOITrigger())
	{
		((RangeTrigger*)(this->pWitness()->pAOITrigger()))->reinstall(static_cast<CoordinateNode*>(this->pEntityCoordinateNode()));
	}

	Controllers::CONTROLLERS_MAP& objects = pControllers_->objects();
	Controllers::CONTROLLERS_MAP::iterator iter = objects.begin();
	for(; iter != objects.end(); iter++)
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
	ProximityController* p = new ProximityController(this, range_xz, range_y, userarg, pControllers_->freeID());
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
		PyErr_Format(PyExc_TypeError, "%s::cancel: args(controllerID|int or \"Movement\"|str) is error!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}
	
	if(pyargobj == NULL)
	{
		PyErr_Format(PyExc_TypeError, "%s::cancel: args(controllerID|int or \"Movement\"|str) is error!", pobj->scriptName());
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
			PyErr_Format(PyExc_TypeError, "%s::cancel: args(controllerID|int) is error!", pobj->scriptName());
			PyErr_PrintEx(0);
			return 0;
		}

		id = PyLong_AsLong(pyargobj);
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
		Mercury::FixedMessages::MSGInfo* msgInfo =
					Mercury::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;
	}

	static PropertyDescription positionDescription(posuid, "VECTOR3", "position", ED_FLAG_ALL_CLIENTS, false, DataTypes::getDataType("VECTOR3"), false, 0, "", DETAIL_LEVEL_FAR);
	if(scriptModule_->usePropertyDescrAlias() && positionDescription.aliasID() == -1)
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
	Position3D& pos = position();
	if(almostEqual(x, pos.x) && almostEqual(z, pos.z))
		return;

	pos.x = x;
	pos.z = z;
	onPositionChanged();
}

//-------------------------------------------------------------------------------------
void Entity::setPosition_XYZ_float(Mercury::Channel* pChannel, float x, float y, float z)
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
		Mercury::FixedMessages::MSGInfo* msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)	
			diruid = msgInfo->msgid;
	}

	static PropertyDescription directionDescription(diruid, "VECTOR3", "direction", ED_FLAG_ALL_CLIENTS, false, DataTypes::getDataType("VECTOR3"), false, 0, "", DETAIL_LEVEL_FAR);
	if(scriptModule_->usePropertyDescrAlias() && directionDescription.aliasID() == -1)
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
		Mercury::FixedMessages::MSGInfo* msgInfo =
					Mercury::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;
	}

	static PropertyDescription positionDescription(posuid, "VECTOR3", "position", ED_FLAG_ALL_CLIENTS, false, DataTypes::getDataType("VECTOR3"), false, 0, "", DETAIL_LEVEL_FAR);
	if(scriptModule_->usePropertyDescrAlias() && positionDescription.aliasID() == -1)
		positionDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ);

	onDefDataChanged(&positionDescription, pPyPosition_);

	if(this->pEntityCoordinateNode())
		this->pEntityCoordinateNode()->update();

	updateLastPos();
}

//-------------------------------------------------------------------------------------
void Entity::onPositionChanged()
{
	if(this->isDestroyed())
		return;

	posChangedTime_ = g_kbetime;
	if(this->pEntityCoordinateNode())
		this->pEntityCoordinateNode()->update();

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
		Mercury::FixedMessages::MSGInfo* msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)	
			diruid = msgInfo->msgid;
	}

	static PropertyDescription directionDescription(diruid, "VECTOR3", "direction", ED_FLAG_ALL_CLIENTS, false, DataTypes::getDataType("VECTOR3"), false, 0, "", DETAIL_LEVEL_FAR);
	if(scriptModule_->usePropertyDescrAlias() && directionDescription.aliasID() == -1)
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
void Entity::onGetWitnessFromBase(Mercury::Channel* pChannel)
{
	onGetWitness(true);
}

//-------------------------------------------------------------------------------------
void Entity::onGetWitness(bool fromBase)
{
	KBE_ASSERT(this->baseMailbox() != NULL);

	if(fromBase)
	{
		// proxy��giveClientTo���ܻ���reloginGateway�� ���һ��entity�Ѿ�������cell�� ��������Ȩ��
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
			setWitness(Witness::ObjPool().createObject());
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

	Space* space = Spaces::findSpace(this->spaceID());
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
	//INFO_MSG(fmt::format("{}::onLoseWitness: {}.\n", 
	//	this->scriptName(), this->id()));

	KBE_ASSERT(this->clientMailbox() != NULL && this->hasWitness());

	clientMailbox()->addr(Mercury::Address::NONE);
	Py_DECREF(clientMailbox());
	clientMailbox(NULL);

	pWitness_->detach(this);
	Witness::ObjPool().reclaimObject(pWitness_);
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
bool Entity::checkMoveForTopSpeed(const Position3D& position)
{
	Position3D movment = position - this->position();
	bool move = true;
	
	// ����ƶ�
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
	if(spaceID_ == 0)
	{
		s.opfini();
		return;
	}

	Position3D pos;
	Direction3D dir;
	uint8 isOnGround = 0;
	float yaw, pitch, roll;
	SPACE_ID currspace;

	s >> pos.x >> pos.y >> pos.z >> yaw >> pitch >> roll >> isOnGround >> currspace;
	isOnGround_ = isOnGround > 0;

	if(spaceID_ != currspace)
	{
		s.opfini();
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
		if(this->pWitness() == NULL)
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

		// ֪ͨ����
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

		(*pForwardBundle).newMessage(ClientInterface::onSetEntityPosAndDir);
		(*pForwardBundle) << id();
		(*pForwardBundle) << currpos.x << currpos.y << currpos.z;
		(*pForwardBundle) << direction().yaw() << direction().pitch() << direction().roll();

		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(id(), (*pSendBundle), (*pForwardBundle));
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
		static_cast<MoveController*>(pMoveController_)->destroy();
		pMoveController_ = NULL;
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
int Entity::raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPos)
{
	Space* pSpace = Spaces::findSpace(spaceID());
	if(pSpace == NULL)
	{
		ERROR_MSG(fmt::format("Entity::raycast: not found space({}), entityID({})!\n", 
			spaceID(), id()));

		return -1;
	}
	
	if(pSpace->pNavHandle() == NULL)
	{
		ERROR_MSG(fmt::format("Entity::raycast: space({}) not addSpaceGeometryMapping!\n", 
			spaceID(), id()));

		return -1;
	}

	return pSpace->pNavHandle()->raycast(layer, start, end, hitPos);
}

//-------------------------------------------------------------------------------------
PyObject* Entity::__py_pyRaycast(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Entity* pobj = static_cast<Entity*>(self);

	if(!pobj->isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::raycast: not is real entity(%d).", 
			pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return 0;
	}

	int layer = pobj->layer();
	PyObject* pyStartPos = NULL;
	PyObject* pyEndPos = NULL;

	if(pobj->isDestroyed())
	{
		PyErr_Format(PyExc_TypeError, "%s::raycast: entity is destroyed!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(currargsSize == 2)
	{
		if(PyArg_ParseTuple(args, "OO", &pyStartPos, &pyEndPos) == -1)
		{
			PyErr_Format(PyExc_TypeError, "%s::raycast: args is error!", pobj->scriptName());
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else if(currargsSize == 3)
	{
		if(PyArg_ParseTuple(args, "OOi", &pyStartPos, &pyEndPos, &layer) == -1)
		{
			PyErr_Format(PyExc_TypeError, "%s::raycast: args is error!", pobj->scriptName());
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "%s::raycast: args is error!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(!PySequence_Check(pyStartPos))
	{
		PyErr_Format(PyExc_TypeError, "%s::raycast: args1(startPos) not is PySequence!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(!PySequence_Check(pyEndPos))
	{
		PyErr_Format(PyExc_TypeError, "%s::raycast: args2(endPos) not is PySequence!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyStartPos) != 3)
	{
		PyErr_Format(PyExc_TypeError, "%s::raycast: args1(startPos) invalid!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyEndPos) != 3)
	{
		PyErr_Format(PyExc_TypeError, "%s::raycast: args2(endPos) invalid!", pobj->scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	Position3D startPos;
	Position3D endPos;
	std::vector<Position3D> hitPosVec;
	//float hitPos[3];

	script::ScriptVector3::convertPyObjectToVector3(startPos, pyStartPos);
	script::ScriptVector3::convertPyObjectToVector3(endPos, pyEndPos);
	if(pobj->raycast(layer, startPos, endPos, hitPosVec) <= 0)
	{
		S_Return;
	}

	int idx = 0;
	PyObject* pyHitpos = PyTuple_New(hitPosVec.size());
	for(std::vector<Position3D>::iterator iter = hitPosVec.begin(); iter != hitPosVec.end(); iter++)
	{
		PyObject* pyHitposItem = PyTuple_New(3);
		PyTuple_SetItem(pyHitposItem, 0, ::PyFloat_FromDouble((*iter).x));
		PyTuple_SetItem(pyHitposItem, 1, ::PyFloat_FromDouble((*iter).y));
		PyTuple_SetItem(pyHitposItem, 2, ::PyFloat_FromDouble((*iter).z));

		PyTuple_SetItem(pyHitpos, idx++, pyHitposItem);
	}

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
	if(spaceID() <= 0)
		return false;

	Space* pSpace = Spaces::findSpace(spaceID());
	if(pSpace == NULL)
		return false;

	if(pSpace->pNavHandle() == NULL)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
uint32 Entity::navigate(const Position3D& destination, float velocity, float range, float maxMoveDistance, float maxDistance, 
	bool faceMovement, float girth, PyObject* userData)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	MoveController* p = new MoveController(this, NULL, pControllers_->freeID());
	
	new NavigateHandler(p, destination, velocity, 
		range, faceMovement, maxMoveDistance, maxDistance, girth, userData);

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyNavigate(PyObject_ptr pyDestination, float velocity, float range, float maxMoveDistance, float maxDistance,
								 int8 faceMovement, float layer, PyObject_ptr userData)
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
	Py_INCREF(userData);

	return PyLong_FromLong(navigate(destination, velocity, range, maxMoveDistance, 
		maxDistance, faceMovement > 0, layer, userData));
}

//-------------------------------------------------------------------------------------
uint32 Entity::moveToPoint(const Position3D& destination, float velocity, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	MoveController* p = new MoveController(this, NULL, pControllers_->freeID());

	new MoveToPointHandler(p, layer(), destination, velocity, 
		0.0f, faceMovement, moveVertically, userData);

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToPoint(PyObject_ptr pyDestination, float velocity, PyObject_ptr userData,
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
	Py_INCREF(userData);

	return PyLong_FromLong(moveToPoint(destination, velocity, userData, faceMovement > 0, moveVertically > 0));
}

//-------------------------------------------------------------------------------------
uint32 Entity::moveToEntity(ENTITY_ID targetID, float velocity, float range, PyObject* userData, 
						 bool faceMovement, bool moveVertically)
{
	stopMove();

	velocity = velocity / g_kbeSrvConfig.gameUpdateHertz();

	MoveController* p = new MoveController(this, NULL, pControllers_->freeID());

	new MoveToEntityHandler(p, targetID, velocity, range,
		faceMovement, moveVertically, userData);

	bool ret = pControllers_->add(p);
	KBE_ASSERT(ret);
	
	pMoveController_ = p;
	return p->id();
}

//-------------------------------------------------------------------------------------
PyObject* Entity::pyMoveToEntity(ENTITY_ID targetID, float velocity, float range, PyObject_ptr userData,
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

	Py_INCREF(userData);
	return PyLong_FromLong(moveToEntity(targetID, velocity, range, userData, faceMovement > 0, moveVertically > 0));
}

//-------------------------------------------------------------------------------------
void Entity::onMove(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	SCOPED_PROFILE(ONMOVE_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS2(this, const_cast<char*>("onMove"), 
		const_cast<char*>("IO"), controllerId, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::onMoveOver(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	pMoveController_ = NULL;
	SCRIPT_OBJECT_CALL_ARGS2(this, const_cast<char*>("onMoveOver"), 
		const_cast<char*>("IO"), controllerId, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::onMoveFailure(uint32 controllerId, PyObject* userarg)
{
	if(this->isDestroyed())
		return;

	pMoveController_ = NULL;
	SCRIPT_OBJECT_CALL_ARGS2(this, const_cast<char*>("onMoveFailure"), 
		const_cast<char*>("IO"), controllerId, userarg);
}

//-------------------------------------------------------------------------------------
void Entity::debugAOI()
{
	if(pWitness_ == NULL)
	{
		ERROR_MSG(fmt::format("{}::debugAOI: {} has no witness!\n", scriptName(), this->id()));
		return;
	}
	
	INFO_MSG(fmt::format("{}::debugAOI: {} size={}\n", scriptName(), this->id(), 
		pWitness_->aoiEntities().size()));

	EntityRef::AOI_ENTITIES::iterator iter = pWitness_->aoiEntities().begin();
	for(; iter != pWitness_->aoiEntities().end(); iter++)
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

		INFO_MSG(fmt::format("{7}::debugAOI: {0} {1}({2}), position({3}.{4}.{5}), dist={6}\n", 
			this->id(), 
			(pEntity != NULL ? pEntity->scriptName() : "unknown"),
			(*iter)->id(),
			epos.x, epos.y, epos.z,
			dist,
			this->scriptName()));
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

	if(this->isDestroyed())
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

	PyObject* pyList = PyList_New(pWitness_->aoiEntities().size());

	EntityRef::AOI_ENTITIES::iterator iter = pWitness_->aoiEntities().begin();
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
	Position3D originpos;
	
	// ��������Ϣ��ȡ����
	if(pyPosition)
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
	for(; iter != findentities.end(); iter++)
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
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onTeleportCB);
		(*pBundle) << sourceEntityID;
		BaseappInterface::onTeleportCBArgs2::staticAddToBundle((*pBundle), spaceID, fromCellTeleport);
		(*pBundle).send(Cellapp::getSingleton().networkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Entity::teleportFromBaseapp(Mercury::Channel* pChannel, COMPONENT_ID cellAppID, ENTITY_ID targetEntityID, COMPONENT_ID sourceBaseAppID)
{
	DEBUG_MSG(fmt::format("{}::teleportFromBaseapp: {}, targetEntityID={}, cell={}, sourceBaseAppID={}.\n", 
		this->scriptName(), this->id(), targetEntityID, cellAppID, sourceBaseAppID));
	
	SPACE_ID lastSpaceID = this->spaceID();

	// �������һ��cell��
	if(cellAppID != g_componentID)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cellAppID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG(fmt::format("{}::teleportFromBaseapp: {}, teleport is error, not found cellapp, targetEntityID, cellAppID={}.\n",
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
			if(space == NULL)
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
		if(space == NULL || (space->creatorID() == entity->id() && entity->isDestroyed()))
		{
			if(space != NULL)
			{
				PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyEntityRef is spaceEntity, spaceEntity is destroyed!\n", scriptName(), id());
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
	if(nearbyMBRef->isBase())
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyRef is error, not is cellMailbox!\n", scriptName(), id());
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
		this->backupCellData();
		
		Mercury::Channel* pBaseChannel = baseMailbox()->getChannel();
		if(pBaseChannel)
		{
			// ͬʱ��Ҫ֪ͨbase�ݴ淢��cellapp����Ϣ����Ϊ���������ת�ɹ���Ҫ�л�cellMailboxӳ���ϵ���µ�cellapp
			// Ϊ�˱������л���һ˲����Ϣ����������(�ɵ�cellapp��ϢҲ��ת���µ�cellapp��)�� �����Ҫ�ڴ���ǰ����
			// �ݴ棬 ���ͳɹ���֪ͨ�ɵ�cellapp����entity֮��ͬʱ֪ͨbaseapp�ı�ӳ���ϵ��
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			(*pBundle).newMessage(BaseappInterface::onMigrationCellappStart);
			(*pBundle) << id();
			(*pBundle) << g_componentID;
			(*pBundle).send(Cellapp::getSingleton().networkInterface(), pBaseChannel);
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}
		else
		{
			PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyRef is error, not found baseapp!\n", scriptName(), id());
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
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::reqTeleportToTheCellApp);
	(*pBundle) << id();
	(*pBundle) << nearbyMBRef->id();
	(*pBundle) << spaceID();
	(*pBundle) << scriptModule()->getUType();
	(*pBundle) << pos.x << pos.y << pos.z;
	(*pBundle) << dir.roll() << pos.pitch() << dir.yaw();

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	changeToGhost(nearbyMBRef->componentID(), *s);

	(*s) << g_componentID;
	(*pBundle).append(s);
	MemoryStream::ObjPool().reclaimObject(s);

	// ��ʱ���������entity, ���Ǳ߳ɹ�����֮���ٻ�������
	// ���ڼ����Ϣ����ͨ��ghostת����real
	// ���δ����ȷ�����ȥ����Դӵ�ǰcell�����ָ�entity.
	// Cellapp::getSingleton().destroyEntity(id(), false);

	nearbyMBRef->postMail((*pBundle));
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

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

	currspace->addEntityToNode(this);

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
				PyErr_Format(PyExc_Exception, "%s::teleport: %d, nearbyRef is error!\n", scriptName(), id());
				PyErr_PrintEx(0);

				onTeleportFailure();
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Entity::onTeleport()
{
	// �����������base.teleport��ת֮ǰ�����ã� cell.teleport�ǲ��ᱻ���õġ�
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleport"));
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportFailure()
{
	ERROR_MSG(fmt::format("{}::onTeleportFailure(): entityID={}\n", 
		this->scriptName(), id()));

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportFailure"));
}

//-------------------------------------------------------------------------------------
void Entity::onTeleportSuccess(PyObject* nearbyEntity, SPACE_ID lastSpaceID)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	
	EntityMailbox* mb = this->baseMailbox();
	if(mb)
	{
		_sendBaseTeleportResult(this->id(), mb->componentID(), this->spaceID(), lastSpaceID, true);
	}

	// ���������trap�ȴ���������������ӽ�ȥ
	restoreProximitys();

	SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onTeleportSuccess"), 
		const_cast<char*>("O"), nearbyEntity);
}

//-------------------------------------------------------------------------------------
void Entity::onEnterSpace(Space* pSpace)
{
}

//-------------------------------------------------------------------------------------
void Entity::onLeaveSpace(Space* pSpace)
{
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
	if(!isReal())
	{
		PyErr_Format(PyExc_AssertionError, "%s::shouldAutoBackup: not is real entity(%d).", 
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s::shouldAutoBackup: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	if(!PyLong_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s::shouldAutoBackup: %d set shouldAutoBackup value is not int!\n",		
			scriptName(), id());		
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
void Entity::onUpdateGhostPropertys(KBEngine::MemoryStream& s)
{
	ENTITY_PROPERTY_UID utype;
	s >> utype;

	PropertyDescription* pPropertyDescription = scriptModule()->findCellPropertyDescription(utype);
	if(pPropertyDescription == NULL)
	{
		ERROR_MSG(fmt::format("{}::onUpdateGhostPropertys: not found propertyID({}), entityID({})\n", 
			scriptName(), utype, id()));

		s.opfini();
		return;
	}

	DEBUG_MSG(fmt::format("{}::onUpdateGhostPropertys: property({}), entityID({})\n", 
		scriptName(), pPropertyDescription->getName(), id()));

	PyObject* pyVal = pPropertyDescription->createFromStream(&s);
	if(pyVal == NULL)
	{
		ERROR_MSG(fmt::format("{}::onUpdateGhostPropertys: entityID={}, create({}) is error!\n", 
			scriptName(), id(), pPropertyDescription->getName()));

		s.opfini();
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

	MethodDescription* pMethodDescription = scriptModule()->findCellMethodDescription(utype);
	if(pMethodDescription == NULL)
	{
		ERROR_MSG(fmt::format("{}::onRemoteRealMethodCall: not found propertyID({}), entityID({})\n", 
			scriptName(), utype, id()));

		s.opfini();
		return;
	}

	onRemoteMethodCall_(pMethodDescription, s);
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

	realCell_ = realCell;
	ghostCell_ = 0;
	
	GhostManager* gm = Cellapp::getSingleton().pGhostManager();
	if(gm)
	{
		gm->addRoute(id(), realCell_);
	}

	DEBUG_MSG(fmt::format("{}::changeToGhost(): {}, realCell={}.\n", 
		scriptName(), id(), realCell));
	
	// �������ǰ��
	addToStream(s);

	witnesses_.clear();

	if(pControllers_)
	{
		pControllers_->clear();
		SAFE_RELEASE(pControllers_);
	}

	if(pWitness())
	{
		pWitness()->clear(this);
		Witness::ObjPool().reclaimObject(pWitness_);
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

	DEBUG_MSG(fmt::format("{}::changeToReal(): {}, ghostCell={}.\n", 
		scriptName(), id(), ghostCell_));

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

	s << scriptModule_->getUType() << spaceID_ << isDestroyed_ << 
		isOnGround_ << topSpeed_ << topSpeedY_ << shouldAutoBackup_ << 
		layer_ << baseMailboxComponentID;

	addCellDataToStream(ENTITY_CELL_DATA_FLAGS, &s);
	
	addMoveHandlerToStream(s);
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

	s >> scriptUType >> spaceID_ >> isDestroyed_ >> isOnGround_ >> topSpeed_ >> 
		topSpeedY_ >> shouldAutoBackup_ >> layer_ >> baseMailboxComponentID;

	this->scriptModule_ = EntityDef::findScriptModule(scriptUType);

	// ����entity��baseMailbox
	if(baseMailboxComponentID > 0)
		baseMailbox(new EntityMailbox(scriptModule(), NULL, baseMailboxComponentID, id_, MAILBOX_TYPE_BASE));

	PyObject* cellData = createCellDataFromStream(&s);
	createNamespace(cellData);
	Py_XDECREF(cellData);
	
	createMoveHandlerFromStream(s);
	createControllersFromStream(s);
	createWitnessFromStream(s);
	createTimersFromStream(s);

	pyCallbackMgr_.createFromStream(s);
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
	for(; iter != witnesses_.end(); iter++)
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

	KBE_ASSERT(witnesses_count_ == 0);
	witnesses_count_ = size;

	for(uint32 i=0; i<size; i++)
	{
		ENTITY_ID entityID;
		s >> entityID;

		if(Cellapp::getSingleton().findEntity(entityID) == NULL)
			continue;

		witnesses_.push_back(entityID);
	}

	bool hasWitness;
	s >> hasWitness;

	if(hasWitness)
	{
		PyObject* clientMB = PyObject_GetAttrString(baseMailbox(), "client");
		KBE_ASSERT(clientMB != Py_None);

		EntityMailbox* client = static_cast<EntityMailbox*>(clientMB);	
		clientMailbox(client);

		setWitness(Witness::ObjPool().createObject());
		pWitness_->createFromStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Entity::addMoveHandlerToStream(KBEngine::MemoryStream& s)
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
}

//-------------------------------------------------------------------------------------
void Entity::createMoveHandlerFromStream(KBEngine::MemoryStream& s)
{
	bool hasMoveHandler;
	s >> hasMoveHandler;

	if(hasMoveHandler)
	{
		stopMove();

		pMoveController_ = new MoveController(this);
		pControllers_->objects()[pMoveController_->id()].reset(pMoveController_);
		pMoveController_->createFromStream(s);
	}
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
		int32 userData = 0;
		int* pUserData = &userData;

		Cellapp::getSingleton().timers().getTimerInfo(iter->second, time, interval, (void *&)pUserData);

		s << time << interval << userData;
		++iter;
	}
}

//-------------------------------------------------------------------------------------
void Entity::createTimersFromStream(KBEngine::MemoryStream& s)
{
	uint32 size;
	s >> size;

	for(uint32 i=0; i<size; i++)
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
