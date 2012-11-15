#include "baseapp.hpp"
#include "base.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	
#include "network/fixed_messages.hpp"

#ifdef CODE_INLINE
//#include "base.ipp"
#endif

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

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
SCRIPT_GET_DECLARE("cell",								pyGetCellMailbox,				0,						0)	
SCRIPT_GET_DECLARE("client",							pyGetClientMailbox,				0,						0)	
SCRIPT_GET_DECLARE("databaseID",						pyGetDBID,						0,						0)	
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
creatingCell_(false),
createdSpace_(false)
{
	ENTITY_INIT_PROPERTYS(Base);

	// 创建并初始化cellData
	createCellData();
}

//-------------------------------------------------------------------------------------
Base::~Base()
{
	ENTITY_DECONSTRUCTION(Base);
	S_RELEASE(clientMailbox_);
	S_RELEASE(cellMailbox_);
	S_RELEASE(cellDataDict_);
}	

//-------------------------------------------------------------------------------------
void Base::onDefDataChanged(const PropertyDescription* propertyDescription, 
		PyObject* pyData)
{
}

//-------------------------------------------------------------------------------------
void Base::onDestroy(void)																					
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onDestroy"));

	if(this->hasDB())
	{
		onCellWriteToDBCompleted(0);

		// 擦除DB中的在线纪录
		Mercury::Bundle::SmartPoolObjectPtr bundleptr = Mercury::Bundle::createSmartPoolObj();
		(*bundleptr)->newMessage(DbmgrInterface::onEntityOffline);
		(*(*bundleptr)) << this->getDBID();


		Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
		Components::ComponentInfos* dbmgrinfos = NULL;

		if(cts.size() > 0)
			dbmgrinfos = &(*cts.begin());

		if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
		{
			ERROR_MSG("Base::onDestroy: not found dbmgr!\n");
			return;
		}

		(*bundleptr)->send(Baseapp::getSingleton().getNetworkInterface(), dbmgrinfos->pChannel);
	}
}

//-------------------------------------------------------------------------------------
bool Base::installCellDataAttr(PyObject* dictData)
{
	if(dictData != NULL)
	{
		if(cellDataDict_ != NULL)
			Py_DECREF(cellDataDict_);

		cellDataDict_ = dictData;
		Py_INCREF(cellDataDict_);
	}
	else if(cellDataDict_ == NULL)
		cellDataDict_ = PyDict_New();
	
	if(PyObject_SetAttrString(this, "cellData", cellDataDict_) == -1)
	{
		ERROR_MSG("Base::installCellDataAttr: set property cellData is error!\n");
		SCRIPT_ERROR_CHECK();
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------
void Base::createCellData(void)
{
	if(!scriptModule_->hasCell() || !installCellDataAttr())
		return;
	
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
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
			ERROR_MSG("Base::createCellData: %s PropertyDescription the dataType is NULL.\n", 
				propertyDescription->getName());	
		}
			
	}
	
	// 初始化cellEntity的位置和方向变量
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
}

//-------------------------------------------------------------------------------------
void Base::addCellDataToStream(uint32 flags, MemoryStream* s)
{
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName());
			(*s) << propertyDescription->getUType();
			propertyDescription->getDataType()->addToStream(s, pyVal);
		}
	}
}

//-------------------------------------------------------------------------------------
void Base::addPersistentsDataToStream(uint32 flags, MemoryStream* s)
{
	std::vector<ENTITY_PROPERTY_UID> log;

	// 再将base中存储属性取出
	PyObject* pydict = PyObject_GetAttrString(this, "__dict__");

	// 先将celldata中的存储属性取出
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	if(scriptModule_->hasCell())
	{
		addPositionAndDirectionToStream(*s);
	}

	for(; iter != propertyDescrs.end(); iter++)
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

			if(PyDict_Contains(cellDataDict_, key) > 0)
			{
				PyObject* pyVal = PyDict_GetItemString(cellDataDict_, attrname);
				(*s) << propertyDescription->getUType();
				log.push_back(propertyDescription->getUType());
				propertyDescription->addPersistentToStream(s, pyVal);
				DEBUG_PERSISTENT_PROPERTY("addCellPersistentsDataToStream", attrname);
			}
			else if(PyDict_Contains(pydict, key) > 0)
			{
	    		(*s) << propertyDescription->getUType();
				log.push_back(propertyDescription->getUType());
	    		propertyDescription->addPersistentToStream(s, PyDict_GetItem(pydict, key));
				DEBUG_PERSISTENT_PROPERTY("addBasePersistentsDataToStream", attrname);
			}
			else
			{
				CRITICAL_MSG("%s::addPersistentsDataToStream: %d not found Persistent[%s].\n",
					this->getScriptName(), this->getID(), attrname);
			}

			Py_DECREF(key);
		}
	}

	Py_XDECREF(pydict);
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellDataDict(uint32 flags)
{
	PyObject* cellData = PyDict_New();

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName());
			PyDict_SetItemString(cellData, propertyDescription->getName(), pyVal);
		}
	}
	return cellData;
}

//-------------------------------------------------------------------------------------
void Base::destroyCellData(void)
{
	// cellDataDict_ 继续保留， 以供备份时使用， 这里仅仅让脚步层无法访问到即可
	// S_RELEASE(cellDataDict_);
	if(PyObject_DelAttrString(this, "cellData") == -1)
	{
		ERROR_MSG("Base::destroyCellData: del property cellData is error!\n");
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
		return false;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onDestroyCellEntityFromBaseapp);
	(*pBundle) << id_;
	(*pBundle).send(Baseapp::getSingleton().getNetworkInterface(), cellMailbox_->getChannel());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyDestroyCellEntity()
{
	if(cellMailbox_ == NULL) 
	{
		PyErr_Format(PyExc_Exception, "%s::destroyCellEntity: id:%i no cell! creatingCell=%s\n", 
			this->getScriptName(), this->getID(),
			creatingCell_ ? "true" : "false");
		PyErr_PrintEx(0);
		S_Return;
	}
	else
		destroyCellEntity();

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyDestroyEntity()
{
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_Exception, "%s::destroy: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		S_Return;																						
	}

	if(creatingCell_ || cellMailbox_ != NULL) 
	{
		PyErr_Format(PyExc_Exception, "%s::destroy: id:%i has cell! creatingCell=%s\n", 
			this->getScriptName(), this->getID(),

			creatingCell_ ? "true" : "false");
		PyErr_PrintEx(0);
	}
	else
		destroyEntity();

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetCellMailbox()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_Exception, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		S_Return;																						
	}

	EntityMailbox* mailbox = getCellMailbox();
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
		PyErr_Format(PyExc_Exception, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		S_Return;																						
	}

	return PyLong_FromUnsignedLongLong(this->getDBID()); 
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetClientMailbox()
{
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_Exception, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		S_Return;																						
	}

	EntityMailbox* mailbox = getClientMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
void Base::onCreateCellFailure(void)
{
	PyObject* pyResult = 
		PyObject_CallMethod(this, const_cast<char*>("onCreateCellFailure"), 
													const_cast<char*>(""));

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_PrintEx(0);
}

//-------------------------------------------------------------------------------------
void Base::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
	// 如果是外部通道调用则判断来源性
	if(pChannel->isExternal())
	{
		ENTITY_ID srcEntityID = pChannel->proxyID();
		if(srcEntityID <= 0 || srcEntityID != this->getID())
		{
			WARNING_MSG("Base::onRemoteMethodCall: srcEntityID:%d, thisEntityID:%d.\n", 
				srcEntityID, this->getID());

			return;
		}
	}

	if(isDestroyed())																				
	{																										
		ERROR_MSG("%s::onRemoteMethodCall: %d is destroyed!\n",											
			getScriptName(), getID());
		s.read_skip(s.opsize());
		return;																							
	}

	ENTITY_METHOD_UID utype = 0;
	s >> utype;
	
	MethodDescription* md = scriptModule_->findBaseMethodDescription(utype);
	if(md == NULL)
	{
		ERROR_MSG("Base::onRemoteMethodCall: can't found method. utype=%u, callerID:%d.\n", 
			utype, id_);

		return;
	}

	DEBUG_MSG("Base::onRemoteMethodCall: entityID:%d, methodType:%s(%u).\n", 
		id_, md->getName(), utype);

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
void Base::onGetCell(Mercury::Channel* pChannel, COMPONENT_ID componentID)
{
	creatingCell_ = false;

	// 删除cellData属性
	destroyCellData();
	
	// 回调给脚本，获得了cell
	cellMailbox_ = new EntityMailbox(scriptModule_, NULL, componentID, id_, MAILBOX_TYPE_CELL);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onGetCell"));
}

//-------------------------------------------------------------------------------------
void Base::onClientDeath()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientDeath"));
}

//-------------------------------------------------------------------------------------
void Base::onLoseCell(Mercury::Channel* pChannel, MemoryStream& s)
{
	S_RELEASE(cellMailbox_);
	
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onLoseCell"));
}

//-------------------------------------------------------------------------------------
void Base::reqBackupCellData()
{
	if(isGetingCellData_)
		return;

	EntityMailbox* mb = this->getCellMailbox();
	if(mb == NULL)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::reqBackupEntityCellData);
	(*pBundle) << this->getID();
	mb->postMail((*pBundle));

	isGetingCellData_ = true;

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Base::onBackupCellData(Mercury::Channel* pChannel, MemoryStream& s)
{
	isGetingCellData_ = false;
	std::string strCellData;
	PyObject* cellData = NULL;

	s.readBlob(strCellData);
	
	if(strCellData.size() > 0)
	{
		cellData = script::Pickler::unpickle(strCellData);
		KBE_ASSERT(cellData != NULL);
	}
	
	installCellDataAttr(cellData);
	S_RELEASE(cellData);
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
void Base::writeToDB(void* data)
{
	PyObject* pyCallback = static_cast<PyObject*>(data);

	if(isArchiveing_)
	{
		Py_DECREF(pyCallback);
		return;
	}

	isArchiveing_ = true;

	if(isDestroyed())																				
	{			
		Py_DECREF(pyCallback);
		return;																							
	}

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	// creatingCell_ 此时可能正在创建cell
	// 不过我们在此假设在cell未创建完成的时候base这个接口被调用
	// 写入数据库的是该entity的初始值， 并不影响
	if(this->getCellMailbox() == NULL) 
	{
		onCellWriteToDBCompleted(callbackID);
	}
	else
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(CellappInterface::reqWriteToDBFromBaseapp);
		(*pBundle) << this->getID();
		(*pBundle) << callbackID;
		this->getCellMailbox()->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Base::onWriteToDBCallback(ENTITY_ID eid, 
								DBID entityDBID, 
								CALLBACK_ID callbackID, 
								bool success)
{

	PyObjectPtr pyCallback;

	if(callbackID > 0)
		pyCallback = callbackMgr().take(callbackID);

	if(getDBID() <= 0)
		setDBID(entityDBID);

	if(callbackID > 0)
	{
		PyObject* pyargs = PyTuple_New(2);

		Py_INCREF(this);
		PyTuple_SET_ITEM(pyargs, 0, PyBool_FromLong((long)success));
		PyTuple_SET_ITEM(pyargs, 1, this);

		PyObject* pyRet = PyObject_CallObject(pyCallback.get(), pyargs);
		if(pyRet == NULL)
		{
			SCRIPT_ERROR_CHECK();
		}
		else
		{
			Py_DECREF(pyRet);
		}

		Py_DECREF(pyargs);
	}
}

//-------------------------------------------------------------------------------------
void Base::onCellWriteToDBCompleted(CALLBACK_ID callbackID)
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onPreArchive"));

	hasDB(true);

	onWriteToDB();

	isArchiveing_ = false;

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	addPersistentsDataToStream(ED_FLAG_ALL, s);

	Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG("Base::onCellWriteToDBCompleted: not found dbmgr!\n");
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(DbmgrInterface::writeEntity);

	(*pBundle) << this->getID();
	(*pBundle) << this->getDBID();
	(*pBundle) << this->getScriptModule()->getUType();
	(*pBundle) << callbackID;

	(*pBundle).append(*s);
	(*pBundle).send(Baseapp::getSingleton().getNetworkInterface(), dbmgrinfos->pChannel);
	MemoryStream::ObjPool().reclaimObject(s);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Base::onWriteToDB()
{
	SCRIPT_OBJECT_CALL_ARGS1(this, const_cast<char*>("onWriteToDB"), 
		const_cast<char*>("O"), cellDataDict_);
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellEntity(PyObject* pyobj)
{
	if(isDestroyed())																				
	{																										
		PyErr_Format(PyExc_Exception, "%s::createCellEntity: %d is destroyed!\n",											
			getScriptName(), getID());												
		PyErr_PrintEx(0);																					
		S_Return;																								
	}																										

	if(Baseapp::getSingleton().findEntity(getID()) == NULL)
	{
		PyErr_Format(PyExc_Exception, "%s::createCellEntity: %d not found!\n", 
			getScriptName(), getID());

		PyErr_PrintEx(0);
		S_Return;
	}

	if(creatingCell_ || this->getCellMailbox())
	{
		PyErr_Format(PyExc_Exception, "%s::createCellEntity: %d has a cell!\n", 
			getScriptName(), getID());

		PyErr_PrintEx(0);
		S_Return;
	}

	if(!PyObject_TypeCheck(pyobj, EntityMailbox::getScriptType()))
	{
		PyErr_Format(PyExc_TypeError, "create %s arg1 is not cellMailbox!", 
			this->getScriptName());

		PyErr_PrintEx(0);
		S_Return;
	}
	
	EntityMailboxAbstract* cellMailbox = static_cast<EntityMailboxAbstract*>(pyobj);
	if(cellMailbox->getType() != MAILBOX_TYPE_CELL)
	{
		PyErr_Format(PyExc_TypeError, "create %s args1 not is a direct cellMailbox!", 
			this->getScriptName());

		PyErr_PrintEx(0);
		S_Return;
	}
	
	creatingCell_ = true;
	Baseapp::getSingleton().createCellEntity(cellMailbox, this);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Base::createInNewSpace(PyObject* params)
{
	if(isDestroyed())																				
	{																										
		PyErr_Format(PyExc_Exception, "%s::createInNewSpace: %d is destroyed!\n",											
			getScriptName(), getID());												
		PyErr_PrintEx(0);																					
		S_Return;																								
	}	

	if(createdSpace_)
	{
		PyErr_Format(PyExc_Exception, "%s::createInNewSpace: %d has a space!\n", 
			getScriptName(), getID());

		PyErr_PrintEx(0);
		S_Return;
	}

	createdSpace_ = true;
	Baseapp::getSingleton().createInNewSpace(this, params);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::forwardEntityMessageToCellappFromClient(Mercury::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->proxyID() != this->getID())
	{
		WARNING_MSG("Base::forwardEntityMessageToCellappFromClient: not srcEntity(%d/%d).\n", 
			pChannel->proxyID(), this->getID());
		return;
	}

	EntityMailbox* mb = this->getCellMailbox();
	if(mb == NULL)
		return;

	// 将这个消息再打包转寄给cellapp， cellapp会对这个包中的每个消息进行判断
	// 检查是否是entity消息， 否则不合法.
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::forwardEntityMessageToCellappFromClient);
	(*pBundle) << this->getID();
	(*pBundle).append(s);
	this->getCellMailbox()->postMail((*pBundle));
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyTeleport(PyObject* baseEntityMB)
{
	if(isDestroyed())																				
	{																										
		PyErr_Format(PyExc_Exception, "%s::teleport: %d is destroyed!\n",											
			getScriptName(), getID());												
		PyErr_PrintEx(0);																					
		S_Return;																								
	}	

	if(this->getCellMailbox() == NULL)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d no has cell!\n", 
			getScriptName(), getID());

		PyErr_PrintEx(0);
		S_Return;
	}

	if(baseEntityMB == NULL)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d baseEntityMB is NULL!\n", 
			getScriptName(), getID());

		PyErr_PrintEx(0);
		S_Return;
	}

	bool isMailbox = PyObject_TypeCheck(baseEntityMB, EntityMailbox::getScriptType());
	bool isEntity = !isMailbox && (PyObject_TypeCheck(baseEntityMB, Base::getScriptType())
		|| PyObject_TypeCheck(baseEntityMB, Proxy::getScriptType()));

	if(!isMailbox && !isEntity)
	{
		PyErr_Format(PyExc_Exception, "%s::teleport: %d invalid baseEntityMB!\n", 
			getScriptName(), getID());

		PyErr_PrintEx(0);
		S_Return;
	}

	ENTITY_ID eid = 0;

	// 如果不是mailbox则是本地base
	if(isMailbox)
	{
		EntityMailbox* mb = static_cast<EntityMailbox*>(baseEntityMB);

		if(mb->getType() != MAILBOX_TYPE_BASE && mb->getType() != MAILBOX_TYPE_CELL_VIA_BASE)
		{
			PyErr_Format(PyExc_Exception, "%s::teleport: %d baseEntityMB is not baseMailbox!\n", 
				getScriptName(), getID());

			PyErr_PrintEx(0);
			S_Return;
		}

		eid = mb->getID();

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::reqTeleportOther);
		(*pBundle) << eid;

		BaseappInterface::reqTeleportOtherArgs3::staticAddToBundle((*pBundle), this->getID(), 
			this->getCellMailbox()->getComponentID(), g_componentID);

		mb->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
	else
	{
		Base* base = static_cast<Base*>(baseEntityMB);
		if(!base->isDestroyed())
		{
			base->reqTeleportOther(NULL, this->getID(), 
				this->getCellMailbox()->getComponentID(), g_componentID);
		}
		else
		{
			PyErr_Format(PyExc_Exception, "%s::teleport: %d baseEntity is destroyed!\n", 
				getScriptName(), getID());

			PyErr_PrintEx(0);
			S_Return;
		}
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::onTeleportCB(Mercury::Channel* pChannel, SPACE_ID spaceID)
{
	if(spaceID > 0)
	{
		onTeleportSuccess(spaceID);
	}
	else
	{
		onTeleportFailure();
	}
}

//-------------------------------------------------------------------------------------
void Base::onTeleportFailure()
{
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportFailure"));
}

//-------------------------------------------------------------------------------------
void Base::onTeleportSuccess(SPACE_ID spaceID)
{
	this->setSpaceID(spaceID);
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onTeleportSuccess"));
}

//-------------------------------------------------------------------------------------
void Base::reqTeleportOther(Mercury::Channel* pChannel, ENTITY_ID reqTeleportEntityID, 
							COMPONENT_ID reqTeleportEntityCellAppID, COMPONENT_ID reqTeleportEntityBaseAppID)
{
	DEBUG_MSG("Base::reqTeleportOther: reqTeleportEntityID=%d, reqTeleportEntityCellAppID=%"PRAppID".\n",
		reqTeleportEntityID, reqTeleportEntityCellAppID);

	if(this->getCellMailbox() == NULL || this->getCellMailbox()->getChannel() == NULL)
	{
		ERROR_MSG("%s::reqTeleportOther: %d, teleport is error, cellMailbox is NULL, "
			"reqTeleportEntityID, reqTeleportEntityCellAppID=%"PRAppID".\n",
			this->getScriptName(), this->getID(), reqTeleportEntityID, reqTeleportEntityCellAppID);
		return;
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(reqTeleportEntityCellAppID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("%s::reqTeleportOther: %d, teleport is error, not found cellapp, "
			"reqTeleportEntityID, reqTeleportEntityCellAppID=%"PRAppID".\n",
			this->getScriptName(), this->getID(), reqTeleportEntityID, reqTeleportEntityCellAppID);
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::teleportFromBaseapp);
	(*pBundle) << reqTeleportEntityID;

	CellappInterface::teleportFromBaseappArgs3::staticAddToBundle((*pBundle), this->getCellMailbox()->getComponentID(), 
		this->getID(), reqTeleportEntityBaseAppID);

	(*pBundle).send(Baseapp::getSingleton().getNetworkInterface(), cinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Base::onGetDBID(Mercury::Channel* pChannel, DBID dbid)
{
}

//-------------------------------------------------------------------------------------
}
