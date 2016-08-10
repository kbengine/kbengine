/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
Base::Base(ENTITY_ID id, const ScriptDefModule* pScriptModule, 
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
pBufferedSendToCellappMessages_(NULL),
pBufferedSendToClientMessages_(NULL),
isDirty_(true),
dbInterfaceIndex_(0)
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
	SAFE_RELEASE(pBufferedSendToClientMessages_);

	if(Baseapp::getSingleton().pEntities())
		Baseapp::getSingleton().pEntities()->pGetbages()->erase(id());

	script::PyGC::decTracing("Base");
}	

//-------------------------------------------------------------------------------------
void Base::onDefDataChanged(const PropertyDescription* propertyDescription, 
		PyObject* pyData)
{
	if(initing())
		return;

	if(propertyDescription->isPersistent())
		setDirty();
	
	uint32 flags = propertyDescription->getFlags();

	if((flags & ED_FLAG_BASE_AND_CLIENT) <= 0 || clientMailbox_ == NULL)
		return;

	// ����һ����Ҫ�㲥��ģ����
	MemoryStream* mstream = MemoryStream::createPoolObject();

	propertyDescription->getDataType()->addToStream(mstream, pyData);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onUpdatePropertys);
	(*pBundle) << id();

	if(pScriptModule_->usePropertyDescrAlias())
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
	MemoryStream::reclaimPoolObject(mstream);
}

//-------------------------------------------------------------------------------------
void Base::onDestroy(bool callScript)
{
	setDirty();
	
	if(callScript)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onDestroy"));
	}

	if(this->hasDB())
	{
		onCellWriteToDBCompleted(0, -1, -1);
	}
	
	eraseEntityLog();

	// ���յ�ǰ�������˵����clientMailbox_�ض���proxy
	// ����Ϊ���ܵ�base������python������C����ʵ���й�
	if(clientMailbox_)
		static_cast<Proxy*>(this)->kick();
}

//-------------------------------------------------------------------------------------
void Base::eraseEntityLog()
{
	// ����û��ʹ��hasDB()�������ж�
	// �û�����destroy( writeToDB = False ), ��������ᵼ��hasDBΪfalse�� �������
	// ��Ҫ�ж�dbid�Ƿ����0�� �������0��Ӧ��Ҫȥ�������ߵȼ�¼���.
	if(this->dbid() > 0)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(DbmgrInterface::onEntityOffline);
		(*pBundle) << this->dbid();
		(*pBundle) << this->pScriptModule()->getUType();
		(*pBundle) << dbInterfaceIndex();

		Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
		Components::ComponentInfos* dbmgrinfos = NULL;

		if(cts.size() > 0)
			dbmgrinfos = &(*cts.begin());

		if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
		{
			ERROR_MSG("Base::onDestroy: not found dbmgr!\n");
			Network::Bundle::reclaimPoolObject(pBundle);
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
			ERROR_MSG("Base::installCellDataAttr: set property cellData error!\n");
			SCRIPT_ERROR_CHECK();
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Base::createCellData(void)
{
	if(!pScriptModule_->hasCell() || !installCellDataAttr())
	{
		if(pScriptModule_->getCellPropertyDescriptions().size() > 0)
		{
			if(!pScriptModule_->hasCell())
			{
				WARNING_MSG(fmt::format("{}::createCellData: do not create cellData, cannot find the cellapp script({})!\n", 
					pScriptModule_->getName(), pScriptModule_->getName()));
			}
		}

		return;
	}
	
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = pScriptModule_->getCellPropertyDescriptions();
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

	if (!cellDataDict_)
		return;

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = pScriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	for(; iter != propertyDescrs.end(); ++iter)
	{
		PropertyDescription* propertyDescription = iter->second;
		if(flags == 0 || (flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName());

			if(useAliasID && pScriptModule_->usePropertyDescrAlias())
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
				DEBUG_MSG(fmt::format("{}::addCellDataToStream: {} error!\n", this->scriptName(),
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
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = pScriptModule_->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	if(pScriptModule_->hasCell())
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
					CRITICAL_MSG(fmt::format("{}::addPersistentsDataToStream: {} persistent({}) type(curr_py: {} != {}) error.\n",
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
					CRITICAL_MSG(fmt::format("{}::addPersistentsDataToStream: {} persistent({}) type(curr_py: {} != {}) error.\n",
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
				WARNING_MSG(fmt::format("{}::addPersistentsDataToStream: {} not found Persistent({}), use default values!\n",
					this->scriptName(), this->id(), attrname));

				(*s) << propertyDescription->getUType();
				log.push_back(propertyDescription->getUType());
				propertyDescription->addPersistentToStream(s, NULL);
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

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = pScriptModule_->getCellPropertyDescriptions();
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
		ERROR_MSG(fmt::format("{}::destroyCellData: delete cellData error!\n", this->scriptName()));
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

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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
			"%s::destroy(): %d initing, reject the request!\n",	
			pobj->scriptName(), pobj->id());
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

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(DbmgrInterface::removeEntity);
		
		(*pBundle) << this->dbInterfaceIndex();
		(*pBundle) << g_componentID;
		(*pBundle) << this->id();
		(*pBundle) << this->dbid();
		(*pBundle) << this->pScriptModule()->getUType();
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
		
	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);
	char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);
	
	// ���������def�־û�����������
	// ����û�кܺõļ�������������ڲ��ı仯������ʹ��һ�����еİ취���б���
	PropertyDescription* pPropertyDescription = const_cast<ScriptDefModule*>(pScriptModule())->findPersistentPropertyDescription(ccattr);
	if(pPropertyDescription && (pPropertyDescription->getFlags() & ENTITY_BASE_DATA_FLAGS) > 0)
	{
		setDirty();
	}

	free(ccattr);
	return ScriptObject::onScriptGetAttribute(attr);
}	

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetCellMailbox()
{ 
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
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
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
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
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
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
	
	MethodDescription* pMethodDescription = pScriptModule_->findBaseMethodDescription(utype);
	if(pMethodDescription == NULL)
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
				srcEntityID, this->id(), this->scriptName(), pMethodDescription->getName()));

			s.done();
			return;
		}

		if(!pMethodDescription->isExposed())
		{
			ERROR_MSG(fmt::format("{2}::onRemoteMethodCall: {0} not is exposed, call is illegal! srcEntityID:{1}.\n",
				pMethodDescription->getName(), srcEntityID, this->scriptName()));

			s.done();
			return;
		}
	}

	if(g_debugEntity)
	{
		DEBUG_MSG(fmt::format("{3}::onRemoteMethodCall: {0}, {3}::{1}(utype={2}).\n", 
			id_, (pMethodDescription ? pMethodDescription->getName() : "unknown"), utype, this->scriptName()));
	}

	pMethodDescription->currCallerID(this->id());
	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>
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
		cellMailbox_ = new EntityMailbox(pScriptModule_, NULL, componentID, id_, MAILBOX_TYPE_CELL);

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
	createdSpace_ = false;
	
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
	removeFlags(ENTITY_FLAGS_INITING);
}

//-------------------------------------------------------------------------------------
void Base::reqBackupCellData()
{
	if(isGetingCellData_)
		return;

	EntityMailbox* mb = this->cellMailbox();
	if(mb == NULL)
		return;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(CellappInterface::reqBackupEntityCellData);
	(*pBundle) << this->id();
	sendToCellapp(pBundle);

	isGetingCellData_ = true;
}

//-------------------------------------------------------------------------------------
void Base::onBackupCellData(Network::Channel* pChannel, MemoryStream& s)
{
	isGetingCellData_ = false;

	bool isDirty = false;
	s >> isDirty;
	
	if(isDirty)
	{		
		PyObject* cellData = createCellDataFromStream(&s);
		installCellDataAttr(cellData);
		Py_DECREF(cellData);
		setDirty();
	}
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
void Base::writeToDB(void* data, void* extra1, void* extra2)
{
	PyObject* pyCallback = NULL;
	int8 shouldAutoLoad = dbid() <= 0 ? 0 : -1;

	// data ���п��ܻ�NULL�ģ� ���綨ʱ�浵�ǲ���Ҫ�ص�������
	if(data != NULL)
		pyCallback = static_cast<PyObject*>(data);

	if(extra1 != NULL && (*static_cast<int*>(extra1)) != -1)
		shouldAutoLoad = (*static_cast<int*>(extra1)) > 0 ? 1 : 0;

	if (extra2)
	{
		if (strlen(static_cast<char*>(extra2)) > 0)
		{
			DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(static_cast<char*>(extra2));
			if (pDBInterfaceInfo->isPure)
			{
				ERROR_MSG(fmt::format("Base::writeToDB: dbInterface({}) is a pure database does not support Entity! "
					"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
					static_cast<char*>(extra2)));

				return;
			}

			int dbInterfaceIndex = pDBInterfaceInfo->index;
			if (dbInterfaceIndex >= 0)
			{
				dbInterfaceIndex_ = dbInterfaceIndex;
			}
			else
			{
				ERROR_MSG(fmt::format("Base::writeToDB: not found dbInterface({})!\n",
					static_cast<char*>(extra2)));

				return;
			}
		}
	}

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
		onCellWriteToDBCompleted(callbackID, shouldAutoLoad, -1);
	}
	else
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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
								uint16 dbInterfaceIndex,
								CALLBACK_ID callbackID, 
								int8 shouldAutoLoad,
								bool success)
{
	isArchiveing_ = false;

	PyObjectPtr pyCallback;

	if(callbackID > 0)
		pyCallback = callbackMgr().take(callbackID);

	if(dbid() <= 0)
		dbid(dbInterfaceIndex, entityDBID);

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
void Base::onCellWriteToDBCompleted(CALLBACK_ID callbackID, int8 shouldAutoLoad, int dbInterfaceIndex)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onPreArchive"));

	if (dbInterfaceIndex >= 0)
		dbInterfaceIndex_ = dbInterfaceIndex;

	hasDB(true);
	
	onWriteToDB();
	
	// ��������ݿ����Ѿ����ڸ�entity������Ӧ�ò��ε���д��������ݼ�ʱ��������
	if(this->DBID_ > 0)
		isArchiveing_ = false;
	else
		setDirty();
	
	// �������û�иı���ô����Ҫ�־û�
	if(!isDirty())
		return;
	
	setDirty(false);
	
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
	
	MemoryStream* s = MemoryStream::createPoolObject();
	addPersistentsDataToStream(ED_FLAG_ALL, s);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::writeEntity);

	(*pBundle) << g_componentID;
	(*pBundle) << this->id();
	(*pBundle) << this->dbid();
	(*pBundle) << this->dbInterfaceIndex();
	(*pBundle) << this->pScriptModule()->getUType();
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
	MemoryStream::reclaimPoolObject(s);
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
PyObject* Base::createInNewSpace(PyObject* args)
{
	if(isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s::createInNewSpace: %d is destroyed!\n",
			scriptName(), id());
		PyErr_PrintEx(0);
		return 0;
	}	

	if(createdSpace_ || this->cellMailbox() != NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::createInNewSpace: %d in space!\n", 
			scriptName(), id());

		PyErr_PrintEx(0);
		return 0;
	}

	createdSpace_ = true;
	Baseapp::getSingleton().createInNewSpace(this, args);
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
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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
		ERROR_MSG(fmt::format("{}::reqTeleportOther: {}, teleport error, cellMailbox is NULL, "
			"reqTeleportEntityID={}, reqTeleportEntityCellAppID={}.\n",
			this->scriptName(), this->id(), reqTeleportEntityID, reqTeleportEntityCellAppID));

		return;
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(reqTeleportEntityCellAppID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG(fmt::format("{}::reqTeleportOther: {}, teleport error, not found cellapp, "
			"reqTeleportEntityID={}, reqTeleportEntityCellAppID={}.\n",
			this->scriptName(), this->id(), reqTeleportEntityID, reqTeleportEntityCellAppID));

		return;
	}

	if(pBufferedSendToCellappMessages_ || pBufferedSendToClientMessages_)
	{
		ERROR_MSG(fmt::format("{}::reqTeleportOther: {}, teleport error, is transfer, "
			"reqTeleportEntityID={}, reqTeleportEntityCellAppID={}.\n",
			this->scriptName(), this->id(), reqTeleportEntityID, reqTeleportEntityCellAppID));

		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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
		pBufferedSendToCellappMessages_ = new BaseMessagesForwardCellappHandler(this);

	pBufferedSendToCellappMessages_->stopForward();

	addFlags(ENTITY_FLAGS_TELEPORT_START);
}

//-------------------------------------------------------------------------------------
void Base::onMigrationCellappArrived(Network::Channel* pChannel, COMPONENT_ID cellappID)
{
	DEBUG_MSG(fmt::format("{}::onTeleportCellappArrived: {}, targetCellappID={}\n",
		scriptName(), id(), cellappID));
	
	// �����ʱʵ�廹û�б�����ΪENTITY_FLAGS_TELEPORT_START,  ˵��onMigrationCellappArrived��������
	// onMigrationCellappStart����(ĳЩѹ�����µ�����»ᵼ��ʵ��������תʱ����cell1��ת��cell2����
	// ��תǰ�������İ����cell2��enterSpace��������)����˷����������ʱ��Ҫ��cell2�İ��Ȼ���
	// ��cell1�İ������ִ�������ִ��cell2�İ�
	if (!hasFlags(ENTITY_FLAGS_TELEPORT_START))
	{
		if(pBufferedSendToClientMessages_ == NULL)
			pBufferedSendToClientMessages_ = new BaseMessagesForwardClientHandler(this, cellappID);
		
		pBufferedSendToClientMessages_->stopForward();
	}

	// ����onMigrationCellappEndû��ִ�й��������õļ�ֵ
	// ĳЩ��������¿���onMigrationCellappArrived������������
	if (!hasFlags(ENTITY_FLAGS_TELEPORT_END))
	{
		addFlags(ENTITY_FLAGS_TELEPORT_ARRIVED);
	}
	else
	{
		DEBUG_MSG(fmt::format("{}::onTeleportCellappArrived: reset flags! {}, targetCellappID={}\n",
			scriptName(), id(), cellappID));

		// ����״̬�£�pBufferedSendToClientMessages_һ��ΪNULL
		KBE_ASSERT(pBufferedSendToClientMessages_ == NULL);

		removeFlags(ENTITY_FLAGS_TELEPORT_START);
		removeFlags(ENTITY_FLAGS_TELEPORT_END);
	}

}

//-------------------------------------------------------------------------------------
void Base::onMigrationCellappEnd(Network::Channel* pChannel, COMPONENT_ID cellappID)
{
	DEBUG_MSG(fmt::format("{}::onTeleportCellappEnd: {}, targetCellappID={}\n",
		scriptName(), id(), cellappID));

	// �ı�cell��ָ���µ�cellapp
	this->cellMailbox()->componentID(cellappID);

	// ĳЩ��������¿���onMigrationCellappArrived������onMigrationCellappEnd��������ʱ�������ñ��
	// �ȴ�onMigrationCellappEnd������������
	if (!hasFlags(ENTITY_FLAGS_TELEPORT_ARRIVED))
	{
		// ����״̬�£�pBufferedSendToClientMessages_һ��ΪNULL
		KBE_ASSERT(pBufferedSendToClientMessages_ == NULL);
		addFlags(ENTITY_FLAGS_TELEPORT_END);
	}
	else
	{
		removeFlags(ENTITY_FLAGS_TELEPORT_START);
		removeFlags(ENTITY_FLAGS_TELEPORT_ARRIVED);

		DEBUG_MSG(fmt::format("{}::onTeleportCellappEnd: reset flags! {}, targetCellappID={}\n",
			scriptName(), id(), cellappID));
	}

	KBE_ASSERT(pBufferedSendToCellappMessages_);
	pBufferedSendToCellappMessages_->startForward();
	
	if(pBufferedSendToClientMessages_)
		pBufferedSendToClientMessages_->startForward();
}

//-------------------------------------------------------------------------------------
void Base::onBufferedForwardToCellappMessagesOver()
{
	SAFE_RELEASE(pBufferedSendToCellappMessages_);
}

//-------------------------------------------------------------------------------------
void Base::onBufferedForwardToClientMessagesOver()
{
	SAFE_RELEASE(pBufferedSendToClientMessages_);
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
