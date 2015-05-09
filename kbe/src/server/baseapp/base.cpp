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

#include "baseapp.h"
#include "base.h"
#include "profile.h"
#include "base_messages_forward_handler.h"
#include "pyscript/py_gc.h"
#include "entitydef/entity_mailbox.h"
#include "network/channel.h"	
#include "network/fixed_messages.h"
#include "client_lib/client_interface.h"

#ifndef CODE_INLINE
#include "base.inl"
#endif

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"

namespace KBEngine{

ENTITY_METHOD_DECLARE_BEGIN(Baseapp, Base)
SCRIPT_METHOD_DECLARE("createCellEntity",				createCellEntity,				METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("createInNewSpace",				createInNewSpace,				METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("destroyCellEntity",				pyDestroyCellEntity,			METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("teleport",						pyTeleport,						METH_VARARGS,			0)
ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Base)
SCRIPT_MEMBER_DECLARE_END()

ENTITY_GETSET_DECLARE_BEGIN(Base)
SCRIPT_GET_DECLARE("cell",								pyGetCellMailbox,				0,								0)	
SCRIPT_GET_DECLARE("client",							pyGetClientMailbox,				0,								0)	
SCRIPT_GET_DECLARE("databaseID",						pyGetDBID,						0,								0)	
SCRIPT_GETSET_DECLARE("shouldAutoBackup",				pyGetShouldAutoBackup,			pySetShouldAutoBackup,			0,		0)
SCRIPT_GETSET_DECLARE("shouldAutoArchive",				pyGetShouldAutoArchive,			pySetShouldAutoArchive,			0,		0)
ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Base, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Base::Base(ENTITY_ID id, const ScriptDefModule* scriptModule, 
		   PyTypeObject* pyType, bool isInitialised):
ScriptObject(pyType, isInitialised),
ENTITY_CONSTRUCTION(Base),
clientMailbox_(NULL),
cellMailbox_(NULL),
cellDataDict_(NULL),
hasDB_(false),
DBID_(0),
isGetingCellData_(false),
isArchiveing_(false),
shouldAutoArchive_(1),
shouldAutoBackup_(1),
creatingCell_(false),
createdSpace_(false),
inRestore_(false),
pBufferedSendToCellappMessages_(NULL)
{
	script::PyGC::incTracing("Base");
	ENTITY_INIT_PROPERTYS(Base);

	// ��������ʼ��cellData
	createCellData();
}

//-------------------------------------------------------------------------------------
Base::~Base()
{
	ENTITY_DECONSTRUCTION(Base);
	S_RELEASE(clientMailbox_);
	S_RELEASE(cellMailbox_);
	S_RELEASE(cellDataDict_);
	SAFE_RELEASE(pBufferedSendToCellappMessages_);

	if(Baseapp::getSingleton().pEntities())
		Baseapp::getSingleton().pEntities()->pGetbages()->erase(id());

	script::PyGC::decTracing("Base");
}	

//-------------------------------------------------------------------------------------
void Base::onDefDataChanged(const PropertyDescription* propertyDescription, 
		PyObject* pyData)
{
	if(initing_)
		return;

	uint32 flags = propertyDescription->getFlags();

	if((flags & ED_FLAG_BASE_AND_CLIENT) <= 0 || clientMailbox_ == NULL)
		return;

	// ����һ����Ҫ�㲥��ģ����
	MemoryStream* mstream = MemoryStream::ObjPool().createObject();

	propertyDescription->getDataType()->addToStream(mstream, pyData);

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onUpdatePropertys);
	(*pBundle) << id();

	if(scriptModule_->usePropertyDescrAlias())
		(*pBundle) << propertyDescription->aliasIDAsUint8();
	else
		(*pBundle) << propertyDescription->getUType();

	pBundle->append(*mstream);
	
	g_privateClientEventHistoryStats.trackEvent(scriptName(), 
		propertyDescription->getName(), 
		pBundle->currMsgLength());

	// ���յ�ǰ�������˵����clientMailbox_�ض���proxy
	// ����Ϊ���ܵ�base������python������C����ʵ���й�
	static_cast<Proxy*>(this)->sendToClient(ClientInterface::onUpdatePropertys, pBundle);
	MemoryStream::ObjPool().reclaimObject(mstream);
}

//-------------------------------------------------------------------------------------
void Base::onDestroy(bool callScript)																					
{
	if(callScript)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onDestroy"));
	}

	if(this->hasDB())
	{
		onCellWriteToDBCompleted(0, -1);
	}
	
	eraseEntityLog();
}

//-------------------------------------------------------------------------------------
void Base::eraseEntityLog()
{
	// ����û��ʹ��hasDB()�������ж�
	// �û�����destroy( writeToDB = False ), ��������ᵼ��hasDBΪfalse�� �������
	// ��Ҫ�ж�dbid�Ƿ����0�� �������0��Ӧ��Ҫȥ�������ߵȼ�¼���.
	if(this->dbid() > 0)
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(DbmgrInterface::onEntityOffline);
		(*pBundle) << this->dbid();
		(*pBundle) << this->scriptModule()->getUType();

		Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
		Components::ComponentInfos* dbmgrinfos = NULL;

		if(cts.size() > 0)
			dbmgrinfos = &(*cts.begin());

		if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
		{
			ERROR_MSG("Base::onDestroy: not found dbmgr!\n");
			Network::Bundle::ObjPool().reclaimObject(pBundle);
			return;
		}

		dbmgrinfos->pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
bool Base::installCellDataAttr(PyObject* dictData, bool installpy)
{
	if(dictData != NULL)
	{
		if(cellDataDict_ != dictData)
		{
			if(cellDataDict_ != NULL)
				Py_DECREF(cellDataDict_);

			cellDataDict_ = dictData;
			Py_INCREF(cellDataDict_);
		}
	}

	if(installpy)
	{
		if(cellDataDict_ == NULL)
		{
			cellDataDict_ = PyDict_New();
		}

		if(PyObject_SetAttrString(this, "cellData", cellDataDict_) == -1)
		{
			ERROR_MSG("Base::installCellDataAttr: set property cellData is error!\n");
			SCRIPT_ERROR_CHECK();
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Base::createCellData(void)
{
	if(!scriptModule_->hasCell() || !installCellDataAttr())
	{
		if(scriptModule_->getCellPropertyDescriptions().size() > 0)
		{
			if(!scriptModule_->hasCell())
			{
				WARNING_MSG(fmt::format("{}::createCellData: do not create cellData, cannot find the cellapp script({})!\n", 
					scriptModule_->getName(), scriptModule_->getName()));
			}
		}

		return;
	}
	
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); ++iter)
	{
		PropertyDescription* propertyDescription = iter->second;
		DataType* dataType = propertyDescription->getDataType();
		
		if(dataType)
		{
			PyObject* pyObj = propertyDescription->newDefaultVal();
			PyDict_SetItemString(cellDataDict_, propertyDescription->getName(), pyObj);
			Py_DECREF(pyObj);
		}
		else
		{
			ERROR_MSG(fmt::format("{}::createCellData: {} PropertyDescription the dataType is NULL.\n",
				this->scriptName(), propertyDescription->getName()));	
		}
		
		SCRIPT_ERROR_CHECK();
	}
	
	// ��ʼ��cellEntity��λ�úͷ������
	PyObject* position = PyTuple_New(3);
	PyTuple_SET_ITEM(position, 0, PyFloat_FromDouble(0.0));
	PyTuple_SET_ITEM(position, 1, PyFloat_FromDouble(0.0));
	PyTuple_SET_ITEM(position, 2, PyFloat_FromDouble(0.0));
	
	PyObject* direction = PyTuple_New(3);
	PyTuple_SET_ITEM(direction, 0, PyFloat_FromDouble(0.0));
	PyTuple_SET_ITEM(direction, 1, PyFloat_FromDouble(0.0));
	PyTuple_SET_ITEM(direction, 2, PyFloat_FromDouble(0.0));
	
	PyDict_SetItemString(cellDataDict_, "position", position);
	PyDict_SetItemString(cellDataDict_, "direction", direction);

	Py_DECREF(position);
	Py_DECREF(direction);

	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Base::addCellDataToStream(uint32 flags, MemoryStream* s, bool useAliasID)
{
	addPositionAndDirectionToStream(*s, useAliasID);

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	for(; iter != propertyDescrs.end(); ++iter)
	{
		PropertyDescription* propertyDescription = iter->second;
		if(flags == 0 || (flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName());

			if(useAliasID && scriptModule_->usePropertyDescrAlias())
			{
				(*s) << propertyDescription->aliasIDAsUint8();
			}
			else
			{
				(*s) << propertyDescription->getUType();
			}

			if(!propertyDescription->getDataType()->isSameType(pyVal))
			{
				ERROR_MSG(fmt::format("{}::addCellDataToStream: {}({}) not is ({})!\n", this->scriptName(), 
					propertyDescription->getName(), (pyVal ? pyVal->ob_type->tp_name : "unknown"), propertyDescription->getDataType()->getName()));
				
				PyObject* pydefval = propertyDescription->getDataType()->parseDefaultStr("");
				propertyDescription->getDataType()->addToStream(s, pydefval);
				Py_DECREF(pydefval);
			}
			else
			{
				propertyDescription->getDataType()->addToStream(s, pyVal);
			}

			if (PyErr_Occurred())
 			{	
				PyErr_PrintEx(0);
				DEBUG_MSG(fmt::format("{}::addCellDataToStream: {} is error!\n", this->scriptName(),
					propertyDescription->getName()));
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Base::addPersistentsDataToStream(uint32 flags, MemoryStream* s)
{
	std::vector<ENTITY_PROPERTY_UID> log;

	// �ٽ�base�д洢����ȡ��
	PyObject* pydict = PyObject_GetAttrString(this, "__dict__");

	// �Ƚ�celldata�еĴ洢����ȡ��
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	if(scriptModule_->hasCell())
	{
		addPositionAndDirectionToStream(*s);
	}

	for(; iter != propertyDescrs.end(); ++iter)
	{
		PropertyDescription* propertyDescription = iter->second;
		std::vector<ENTITY_PROPERTY_UID>::const_iterator finditer = 
			std::find(log.begin(), log.end(), propertyDescription->getUType());

		if(finditer != log.end())
			continue;

		const char* attrname = propertyDescription->getName();
		if(propertyDescription->isPersistent() && (flags & propertyDescription->getFlags()) > 0)
		{
			PyObject *key = PyUnicode_FromString(attrname);

			if(cellDataDict_ != NULL && PyDict_Contains(cellDataDict_, key) > 0)
			{
				PyObject* pyVal = PyDict_GetItemString(cellDataDict_, attrname);
				if(!propertyDescription->getDataType()->isSameType(pyVal))
				{
					CRITICAL_MSG(fmt::format("{}::addPersistentsDataToStream: {} persistent[{}] type(curr_py: {} != {}) is error.\n",
						this->scriptName(), this->id(), attrname, (pyVal ? pyVal->ob_type->tp_name : "unknown"), propertyDescription->getDataType()->getName()));
				}
				else
				{
					(*s) << propertyDescription->getUType();
					log.push_back(propertyDescription->getUType());
					propertyDescription->addPersistentToStream(s, pyVal);
					DEBUG_PERSISTENT_PROPERTY("addCellPersistentsDataToStream", attrname);
				}
			}
			else if(PyDict_Contains(pydict, key) > 0)
			{
				PyObject* pyVal = PyDict_GetItem(pydict, key);
				if(!propertyDescription->getDataType()->isSameType(pyVal))
				{
					CRITICAL_MSG(fmt::format("{}::addPersistentsDataToStream: {} persistent[{}] type(curr_py: {} != {}) is error.\n",
						this->scriptName(), this->id(), attrname, (pyVal ? pyVal->ob_type->tp_name : "unknown"), propertyDescription->getDataType()->getName()));
				}
				else
				{
	    			(*s) << propertyDescription->getUType();
					log.push_back(propertyDescription->getUType());
	    			propertyDescription->addPersistentToStream(s, pyVal);
					DEBUG_PERSISTENT_PROPERTY("addBasePersistentsDataToStream", attrname);
				}
			}
			else
			{
				CRITICAL_MSG(fmt::format("{}::addPersistentsDataToStream: {} not found Persistent[{}].\n",
					this->scriptName(), this->id(), attrname));
			}

			Py_DECREF(key);
		}

		SCRIPT_ERROR_CHECK();
	}

	Py_XDECREF(pydict);
	SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellDataDict(uint32 flags)
{
	PyObject* cellData = PyDict_New();

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); ++iter)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName());
			PyDict_SetItemString(cellData, propertyDescription->getName(), pyVal);
			Py_DECREF(pyVal);
			SCRIPT_ERROR_CHECK();
		}
	}

	return cellData;
}

//-------------------------------------------------------------------------------------
void Base::sendToCellapp(Network::Bundle* pBundle)
{
	KBE_ASSERT(cellMailbox_ != NULL);

	if(pBufferedSendToCellappMessages_ && pBufferedSendToCellappMessages_->isStop())
	{
		pBufferedSendToCellappMessages_->pushMessages(pBundle);
		return;
	}

	sendToCellapp(cellMailbox_->getChannel(), pBundle);
}

//-------------------------------------------------------------------------------------
void Base::sendToCellapp(Network::Channel* pChannel, Network::Bundle* pBundle)
{
	KBE_ASSERT(pChannel != NULL && pBundle != NULL);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Base::destroyCellData(void)
{
	// cellDataDict_ ���������� �Թ�����ʱʹ�ã� ��������ýŲ����޷����ʵ�����
	// S_RELEASE(cellDataDict_);
	if(PyObject_DelAttrString(this, "cellData") == -1)
	{
		ERROR_MSG(fmt::format("{}::destroyCellData: delete cellData is error!\n", this->scriptName()));
		SCRIPT_ERROR_CHECK();
	}
}

//-------------------------------------------------------------------------------------
bool Base::destroyCellEntity(void)
{
	if(isDestroyed())	
	{
		return false;																					
	}

	if(cellMailbox_  == NULL || cellMailbox_->getChannel() == NULL)
	{
		isArchiveing_ = false;
		return false;
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onDestroyCellEntityFromBaseapp);
	(*pBundle) << id_;
	sendToCellapp(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyDestroyCellEntity()
{
	if(cellMailbox_ == NULL) 
	{
		PyErr_Format(PyExc_AssertionError, "%s::destroyCellEntity: id:%i no cell! creatingCell=%s\n", 
			this->scriptName(), this->id(),
			creatingCell_ ? "true" : "false");
		PyErr_PrintEx(0);
		return 0;
	}
	else
		destroyCellEntity();

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Base::__py_pyDestroyEntity(PyObject* self, PyObject* args, PyObject * kwargs)
{
	Base* pobj = static_cast<Base*>(self);

	if(pobj->initing())
	{
		PyErr_Format(PyExc_AssertionError,
			"%s::destroy(): is initing, reject the request!\n",	
			pobj->scriptName());
		PyErr_PrintEx(0);
		return NULL;
	}

	static char * keywords[] =
	{
		const_cast<char *> ("deleteFromDB"),
		const_cast<char *> ("writeToDB"),
		NULL
	};

	if(pobj->isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s::destroy: %d is destroyed!\n",
			pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return NULL;
	}

	if(pobj->creatingCell() || pobj->cellMailbox() != NULL) 
	{
		PyErr_Format(PyExc_Exception, "%s::destroy: id:%i has cell! creatingCell=%s\n", 
			pobj->scriptName(), pobj->id(),

			pobj->creatingCell() ? "true" : "false");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyDeleteFromDB = NULL;
	PyObject* pyWriteToDB = NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OO", 
		keywords, &pyDeleteFromDB, &pyWriteToDB))
	{
		PyErr_Format(PyExc_AssertionError, "%s::destroy: %d ParseTupleAndKeywords(deleteFromDB, &writeToDB) error!\n",
			pobj->scriptName(), pobj->id());
		PyErr_PrintEx(0);
		return NULL;
	}

	bool deleteFromDB = (pyDeleteFromDB != NULL) ? 
		(PyObject_IsTrue(pyDeleteFromDB) ? true : false) : false; 

	bool writeToDB = (pyWriteToDB != NULL) ? 
		(PyObject_IsTrue(pyWriteToDB) ? true : false) : pobj->hasDB();

	if(deleteFromDB || writeToDB)
	{
		// �п����Ѿ�������writeToDB����δ����д���dbid
		// ���������Ҫ���ظ��û�һ������ �û����Լ��������������
		if(pobj->hasDB() && pobj->dbid() == 0)
		{
			PyErr_Format(PyExc_AssertionError, "%s::destroy: id:%i has db, current dbid is 0. "
				"please wait for dbmgr to processing!\n", 
				pobj->scriptName(), pobj->id());
			PyErr_PrintEx(0);
			return NULL;
		}
	}

	pobj->onDestroyEntity(deleteFromDB, writeToDB);
	pobj->destroyEntity();

	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::onDestroyEntity(bool deleteFromDB, bool writeToDB)
{
	if(deleteFromDB && hasDB())
	{
		Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
		Components::ComponentInfos* dbmgrinfos = NULL;

		if(cts.size() > 0)
			dbmgrinfos = &(*cts.begin());

		if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
		{
			ERROR_MSG(fmt::format("{}::onDestroyEntity({}): writeToDB not found dbmgr!\n", this->scriptName(), this->id()));
			return;
		}

		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(DbmgrInterface::removeEntity);

		(*pBundle) << g_componentID;
		(*pBundle) << this->id();
		(*pBundle) << this->dbid();
		(*pBundle) << this->scriptModule()->getUType();
		dbmgrinfos->pChannel->send(pBundle);

		this->hasDB(false);
		return;
	}

	if(writeToDB)
	{
		// �����ΪĬ�ϻᴦ��
		// this->writeToDB(NULL);
	}
	else
	{
		this->hasDB(false);
	}

	shouldAutoArchive_ = 0;
	shouldAutoBackup_ = 0;
}

//-------------------------------------------------------------------------------------
PyObject* Base::onScriptGetAttribute(PyObject* attr)
{
	DEBUG_OP_ATTRIBUTE("get", attr)
	return ScriptObject::onScriptGetAttribute(attr);
}	

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetCellMailbox()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																					
	}

	EntityMailbox* mailbox = cellMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetDBID()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																					
	}

	return PyLong_FromUnsignedLongLong(this->dbid()); 
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetClientMailbox()
{
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	EntityMailbox* mailbox = clientMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
int Base::pySetShouldAutoArchive(PyObject *value)
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
		PyErr_Format(PyExc_AssertionError, "%s: %d set shouldAutoArchive value is not int!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;	
	}

	shouldAutoArchive_ = (int8)PyLong_AsLong(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetShouldAutoArchive()
{
	return PyLong_FromLong(shouldAutoArchive_);
}

//-------------------------------------------------------------------------------------
int Base::pySetShouldAutoBackup(PyObject *value)
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
		PyErr_Format(PyExc_AssertionError, "%s: %d set shouldAutoBackup value is not int!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;	
	}

	shouldAutoBackup_ = (int8)PyLong_AsLong(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetShouldAutoBackup()
{
	return PyLong_FromLong(shouldAutoBackup_);
}

//-------------------------------------------------------------------------------------
void Base::onCreateCellFailure(void)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	creatingCell_ = false;
	isGetingCellData_ = false;

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onCreateCellFailure"));
}

//-------------------------------------------------------------------------------------
void Base::onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	if(isDestroyed())																				
	{																										
		ERROR_MSG(fmt::format("{}::onRemoteMethodCall: {} is destroyed!\n",											
			scriptName(), id()));

		s.done();
		return;																							
	}

	ENTITY_METHOD_UID utype = 0;
	s >> utype;
	
	MethodDescription* md = scriptModule_->findBaseMethodDescription(utype);
	if(md == NULL)
	{
		ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: can't found method. utype={0}, callerID:{1}.\n", 
			utype, id_, this->scriptName()));
		
		s.done();
		return;
	}

	// ������ⲿͨ���������ж���Դ��
	if (pChannel->isExternal())
	{
		ENTITY_ID srcEntityID = pChannel->proxyID();
		if (srcEntityID <= 0 || srcEntityID != this->id())
		{
			WARNING_MSG(fmt::format("{2}::onRemoteMethodCall({3}): srcEntityID:{0} != thisEntityID:{1}.\n",
				srcEntityID, this->id(), this->scriptName(), md->getName()));

			s.done();
			return;
		}

		if(!md->isExposed())
		{
			ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: {0} not is exposed, call is illegal! srcEntityID:{1}.\n",
				md->getName(), srcEntityID, this->scriptName()));

			s.done();
			return;
		}
	}

	if(g_debugEntity)
	{
		DEBUG_MSG(fmt::format("{3}::onRemoteMethodCall: {0}, {3}::{1}(utype={2}).\n", 
			id_, (md ? md->getName() : "unknown"), utype, this->scriptName()));
	}

	md->currCallerID(this->id());
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
				Py_XDECREF(pyargs);
			}
			else
			{
				SCRIPT_ERROR_CHECK();
			}
		}
	}
	
	Py_XDECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
void Base::onGetCell(Network::Channel* pChannel, COMPONENT_ID componentID)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	creatingCell_ = false;

	// ɾ��cellData����
	destroyCellData();
	
	// �ص����ű��������cell
	if(cellMailbox_ == NULL)
		cellMailbox_ = new EntityMailbox(scriptModule_, NULL, componentID, id_, MAILBOX_TYPE_CELL);

	if(!inRestore_)
		SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onGetCell"));
}

//-------------------------------------------------------------------------------------
void Base::onClientDeath()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientDeath"));
}

//-------------------------------------------------------------------------------------
void Base::onLoseCell(Network::Channel* pChannel, MemoryStream& s)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	S_RELEASE(cellMailbox_);

	isArchiveing_ = false;
	isGetingCellData_ = false;

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLoseCell"));
}

//-------------------------------------------------------------------------------------
void Base::onRestore()
{
	if(!inRestore_)
		return;

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onRestore"));
	inRestore_ = false;
	isArchiveing_ = false;
}

//-------------------------------------------------------------------------------------
void Base::reqBackupCellData()
{
	if(isGetingCellData_)
		return;

	EntityMailbox* mb = this->cellMailbox();
	if(mb == NULL)
		return;

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::reqBackupEntityCellData);
	(*pBundle) << this->id();
	sendToCellapp(pBundle);

	isGetingCellData_ = true;
}

//-------------------------------------------------------------------------------------
void Base::onBackupCellData(Network::Channel* pChannel, MemoryStream& s)
{
	isGetingCellData_ = false;

	PyObject* cellData = createCellDataFromStream(&s);
	installCellDataAttr(cellData);
	Py_DECREF(cellData);
}

//-------------------------------------------------------------------------------------
void Base::writeBackupData(MemoryStream* s)
{
	onBackup();
}

//-------------------------------------------------------------------------------------
void Base::onBackup()
{
	reqBackupCellData();
}

//-------------------------------------------------------------------------------------
void Base::writeToDB(void* data, void* extra)
{
	PyObject* pyCallback = NULL;
	int8 shouldAutoLoad = -1;

	// data ���п��ܻ�NULL�ģ� ���綨ʱ�浵�ǲ���Ҫ�ص�������
	if(data != NULL)
		pyCallback = static_cast<PyObject*>(data);

	if(extra != NULL)
		shouldAutoLoad = (*static_cast<int*>(extra)) ? 1 : 0;

	if(isArchiveing_)
	{
		// __py_pyWriteToDBû����������
		//if(pyCallback != NULL)
		//	Py_DECREF(pyCallback);

		WARNING_MSG(fmt::format("{}::writeToDB(): is archiveing! entityid={}, dbid={}.\n", 
			this->scriptName(), this->id(), this->dbid()));

		return;
	}

	isArchiveing_ = true;

	if(isDestroyed())																				
	{	
		// __py_pyWriteToDBû����������
		//if(pyCallback != NULL)
		//	Py_DECREF(pyCallback);

		ERROR_MSG(fmt::format("{}::writeToDB(): is destroyed! entityid={}, dbid={}.\n", 
			this->scriptName(), this->id(), this->dbid()));

		return;																							
	}

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	// creatingCell_ ��ʱ�������ڴ���cell
	// ���������ڴ˼�����cellδ������ɵ�ʱ��base����ӿڱ�����
	// д�����ݿ���Ǹ�entity�ĳ�ʼֵ�� ����Ӱ��
	if(this->cellMailbox() == NULL) 
	{
		onCellWriteToDBCompleted(callbackID, shouldAutoLoad);
	}
	else
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(CellappInterface::reqWriteToDBFromBaseapp);
		(*pBundle) << this->id();
		(*pBundle) << callbackID;
		(*pBundle) << shouldAutoLoad;
		sendToCellapp(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Base::onWriteToDBCallback(ENTITY_ID eid, 
								DBID entityDBID, 
								CALLBACK_ID callbackID, 
								int8 shouldAutoLoad,
								bool success)
{
	isArchiveing_ = false;

	PyObjectPtr pyCallback;

	if(callbackID > 0)
		pyCallback = callbackMgr().take(callbackID);

	if(dbid() <= 0)
		dbid(entityDBID);

	if(callbackID > 0)
	{
		PyObject* pyargs = PyTuple_New(2);

		Py_INCREF(this);
		PyTuple_SET_ITEM(pyargs, 0, PyBool_FromLong((long)success));
		PyTuple_SET_ITEM(pyargs, 1, this);
		
		if(pyCallback != NULL)
		{
			PyObject* pyRet = PyObject_CallObject(pyCallback.get(), pyargs);
			if(pyRet == NULL)
			{
				SCRIPT_ERROR_CHECK();
			}
			else
			{
				Py_DECREF(pyRet);
			}
		}
		else
		{
			ERROR_MSG(fmt::format("{}::onWriteToDBCallback: can't found callback:{}.\n",
				this->scriptName(), callbackID));
		}

		Py_DECREF(pyargs);
	}
}

//-------------------------------------------------------------------------------------
void Base::onCellWriteToDBCompleted(CALLBACK_ID callbackID, int8 shouldAutoLoad)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onPreArchive"));

	hasDB(true);
	
	onWriteToDB();
	
	// ��������ݿ����Ѿ����ڸ�entity������Ӧ�ò��ε���д��������ݼ�ʱ��������
	if(this->DBID_ > 0)
		isArchiveing_ = false;

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(fmt::format("{}::onCellWriteToDBCompleted({}): not found dbmgr!\n", 
			this->scriptName(), this->id()));
		return;
	}

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	addPersistentsDataToStream(ED_FLAG_ALL, s);

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(DbmgrInterface::writeEntity);

	(*pBundle) << g_componentID;
	(*pBundle) << this->id();
	(*pBundle) << this->dbid();
	(*pBundle) << this->scriptModule()->getUType();
	(*pBundle) << callbackID;
	(*pBundle) << shouldAutoLoad;

	// ��¼��¼��ַ
	if(this->dbid() == 0)
	{
		uint32 ip = 0;
		uint16 port = 0;
		
		if(this->clientMailbox())
		{
			ip = this->clientMailbox()->addr().ip;
			port = this->clientMailbox()->addr().port;
		}

		(*pBundle) << ip;
		(*pBundle) << port;
	}

	(*pBundle).append(*s);

	dbmgrinfos->pChannel->send(pBundle);
	MemoryStream::ObjPool().reclaimObject(s);
}

//-------------------------------------------------------------------------------------
void Base::onWriteToDB()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onWriteToDB"), 
		const_cast<char*>("O"), cellDataDict_);
}

//-------------------------------------------------------------------------------------
void Base::onCellAppDeath()
{
	isArchiveing_ = false;
	isGetingCellData_ = false;
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellEntity(PyObject* pyobj)
{
	if(isDestroyed())																				
	{																										
		PyErr_Format(PyExc_AssertionError, "%s::createCellEntity: %d is destroyed!\n",											
			scriptName(), id());												
		PyErr_PrintEx(0);																					
		return 0;																						
	}																										

	if(Baseapp::getSingleton().findEntity(id()) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::createCellEntity: %d not found!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);
		return 0;
	}

	if(creatingCell_ || this->cellMailbox())
	{
		PyErr_Format(PyExc_AssertionError, "%s::createCellEntity: %d has a cell!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);
		return 0;
	}

	if(!PyObject_TypeCheck(pyobj, EntityMailbox::getScriptType()))
	{
		PyErr_Format(PyExc_TypeError, "create %s arg1 is not cellMailbox!", 
			this->scriptName());

		PyErr_PrintEx(0);
		return 0;
	}
	
	EntityMailboxAbstract* cellMailbox = static_cast<EntityMailboxAbstract*>(pyobj);
	if(cellMailbox->type() != MAILBOX_TYPE_CELL)
	{
		PyErr_Format(PyExc_TypeError, "create %s args1 not is a direct cellMailbox!", 
			this->scriptName());

		PyErr_PrintEx(0);
		return 0;
	}
	
	creatingCell_ = true;
	Baseapp::getSingleton().createCellEntity(cellMailbox, this);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::restoreCell(EntityMailboxAbstract* cellMailbox)
{
	if(creatingCell_ || inRestore_) return;

	creatingCell_ = true;
	inRestore_ = true;

	if(createdSpace_)
	{
		Baseapp::getSingleton().restoreSpaceInCell(this);
	}
	else
	{
		Baseapp::getSingleton().createCellEntity(cellMailbox, this);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Base::createInNewSpace(PyObject* params)
{
	if(isDestroyed())																				
	{																										
		PyErr_Format(PyExc_AssertionError, "%s::createInNewSpace: %d is destroyed!\n",											
			scriptName(), id());												
		PyErr_PrintEx(0);																					
		return 0;																						
	}	

	if(createdSpace_)
	{
		PyErr_Format(PyExc_AssertionError, "%s::createInNewSpace: %d has a space!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);
		return 0;
	}

	createdSpace_ = true;
	Baseapp::getSingleton().createInNewSpace(this, params);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::forwardEntityMessageToCellappFromClient(Network::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->proxyID() != this->id())
	{
		WARNING_MSG(fmt::format("{2}::forwardEntityMessageToCellappFromClient: not srcEntity({0}/{1}).\n",
			pChannel->proxyID(), this->id(), this->scriptName()));

		return;
	}

	EntityMailbox* mb = this->cellMailbox();
	if(mb == NULL)
		return;

	// �������Ϣ�ٴ��ת�ĸ�cellapp�� cellapp���������е�ÿ����Ϣ�����ж�
	// ����Ƿ���entity��Ϣ�� ���򲻺Ϸ�.
	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::forwardEntityMessageToCellappFromClient);
	(*pBundle) << this->id();
	(*pBundle).append(s);
	sendToCellapp(pBundle);
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyTeleport(PyObject* baseEntityMB)
{
	if(isDestroyed())																				
	{																										
		PyErr_Format(PyExc_AssertionError, "%s::teleport: %d is destroyed!\n",											
			scriptName(), id());												
		PyErr_PrintEx(0);																					
		return 0;																					
	}	

	if(this->cellMailbox() == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::teleport: %d no has cell!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);
		return 0;
	}

	if(baseEntityMB == NULL)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d baseEntityMB is NULL!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);
		return 0;
	}

	bool isMailbox = PyObject_TypeCheck(baseEntityMB, EntityMailbox::getScriptType());
	bool isEntity = !isMailbox && (PyObject_TypeCheck(baseEntityMB, Base::getScriptType())
		|| PyObject_TypeCheck(baseEntityMB, Proxy::getScriptType()));

	if(!isMailbox && !isEntity)
	{
		PyErr_Format(PyExc_AssertionError, "%s::teleport: %d invalid baseEntityMB!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);
		return 0;
	}

	ENTITY_ID eid = 0;

	// �������mailbox���Ǳ���base
	if(isMailbox)
	{
		EntityMailbox* mb = static_cast<EntityMailbox*>(baseEntityMB);

		if(mb->type() != MAILBOX_TYPE_BASE && mb->type() != MAILBOX_TYPE_CELL_VIA_BASE)
		{
			PyErr_Format(PyExc_AssertionError, "%s::teleport: %d baseEntityMB is not baseMailbox!\n", 
				scriptName(), id());

			PyErr_PrintEx(0);
			return 0;
		}

		eid = mb->id();

		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::reqTeleportOther);
		(*pBundle) << eid;

		BaseappInterface::reqTeleportOtherArgs3::staticAddToBundle((*pBundle), this->id(), 
			this->cellMailbox()->componentID(), g_componentID);

		mb->postMail(pBundle);
	}
	else
	{
		Base* base = static_cast<Base*>(baseEntityMB);
		if(!base->isDestroyed())
		{
			base->reqTeleportOther(NULL, this->id(), 
				this->cellMailbox()->componentID(), g_componentID);
		}
		else
		{
			PyErr_Format(PyExc_AssertionError, "%s::teleport: %d baseEntity is destroyed!\n", 
				scriptName(), id());

			PyErr_PrintEx(0);
			return 0;
		}
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::onTeleportCB(Network::Channel* pChannel, SPACE_ID spaceID, bool fromCellTeleport)
{
	if(spaceID > 0)
	{
		if(!fromCellTeleport)
			onTeleportSuccess(spaceID);
		else
			this->spaceID(spaceID);
	}
	else
	{
		onTeleportFailure();
	}
}

//-------------------------------------------------------------------------------------
void Base::onTeleportFailure()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportFailure"));
}

//-------------------------------------------------------------------------------------
void Base::onTeleportSuccess(SPACE_ID spaceID)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	this->spaceID(spaceID);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportSuccess"));
}

//-------------------------------------------------------------------------------------
void Base::reqTeleportOther(Network::Channel* pChannel, ENTITY_ID reqTeleportEntityID, 
							COMPONENT_ID reqTeleportEntityCellAppID, COMPONENT_ID reqTeleportEntityBaseAppID)
{
	DEBUG_MSG(fmt::format("{2}::reqTeleportOther: reqTeleportEntityID={0}, reqTeleportEntityCellAppID={1}.\n",
		reqTeleportEntityID, reqTeleportEntityCellAppID, this->scriptName()));

	if(this->cellMailbox() == NULL || this->cellMailbox()->getChannel() == NULL)
	{
		ERROR_MSG(fmt::format("{}::reqTeleportOther: {}, teleport is error, cellMailbox is NULL, "
			"reqTeleportEntityID={}, reqTeleportEntityCellAppID={}.\n",
			this->scriptName(), this->id(), reqTeleportEntityID, reqTeleportEntityCellAppID));

		return;
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(reqTeleportEntityCellAppID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(fmt::format("{}::reqTeleportOther: {}, teleport is error, not found cellapp, "
			"reqTeleportEntityID={}, reqTeleportEntityCellAppID={}.\n",
			this->scriptName(), this->id(), reqTeleportEntityID, reqTeleportEntityCellAppID));

		return;
	}

	if(pBufferedSendToCellappMessages_)
	{
		ERROR_MSG(fmt::format("{}::reqTeleportOther: {}, teleport is error, is transfer, "
			"reqTeleportEntityID={}, reqTeleportEntityCellAppID={}.\n",
			this->scriptName(), this->id(), reqTeleportEntityID, reqTeleportEntityCellAppID));

		return;
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::teleportFromBaseapp);
	(*pBundle) << reqTeleportEntityID;

	CellappInterface::teleportFromBaseappArgs3::staticAddToBundle((*pBundle), this->cellMailbox()->componentID(), 
		this->id(), reqTeleportEntityBaseAppID);
	
	sendToCellapp(cinfos->pChannel, pBundle);
}

//-------------------------------------------------------------------------------------
void Base::onMigrationCellappStart(Network::Channel* pChannel, COMPONENT_ID cellappID)
{
	DEBUG_MSG(fmt::format("{}::onTeleportCellappStart: {}, targetCellappID={}\n",											
		scriptName(), id(), cellappID));

	// cell���ֿ�ʼ��cellappǨ���ˣ� ��ʱbaseapp����cellapp�İ���Ӧ�û���
	// ��onTeleportCellappEnd������ʱ������İ�����cell

	if(pBufferedSendToCellappMessages_ == NULL)
		pBufferedSendToCellappMessages_ = new BaseMessagesForwardHandler(this);

	pBufferedSendToCellappMessages_->stopForward();
}

//-------------------------------------------------------------------------------------
void Base::onMigrationCellappEnd(Network::Channel* pChannel, COMPONENT_ID cellappID)
{
	DEBUG_MSG(fmt::format("{}::onTeleportCellappEnd: {}, targetCellappID={}\n",											
		scriptName(), id(), cellappID));

	// �ı�cell��ָ���µ�cellapp
	this->cellMailbox()->componentID(cellappID);

	KBE_ASSERT(pBufferedSendToCellappMessages_);
	pBufferedSendToCellappMessages_->startForward();
}

//-------------------------------------------------------------------------------------
void Base::onBufferedForwardToCellappMessagesOver()
{
	SAFE_RELEASE(pBufferedSendToCellappMessages_);
}

//-------------------------------------------------------------------------------------
void Base::onGetDBID(Network::Channel* pChannel, DBID dbid)
{
}

//-------------------------------------------------------------------------------------
bool Base::_reload(bool fullReload)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
