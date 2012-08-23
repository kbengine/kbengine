#include "baseapp.hpp"
#include "base.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	

#ifdef CODE_INLINE
//#include "base.ipp"
#endif

#include "../../server/cellapp/cellapp_interface.hpp"

namespace KBEngine{

ENTITY_METHOD_DECLARE_BEGIN(Base)
SCRIPT_METHOD_DECLARE("createCellEntity",				createCellEntity,				METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("createInNewSpace",				createInNewSpace,				METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("destroyCellEntity",				pyDestroyCellEntity,			METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("destroy",						pyDestroyBase,					METH_VARARGS,			0)
ENTITY_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Base)
SCRIPT_MEMBER_DECLARE_END()

ENTITY_GETSET_DECLARE_BEGIN(Base)
SCRIPT_GET_DECLARE("cell",								pyGetCellMailbox,				0,						0)	
SCRIPT_GET_DECLARE("client",							pyGetClientMailbox,				0,						0)	
ENTITY_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Base, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Base::Base(ENTITY_ID id, ScriptModule* scriptModule, PyTypeObject* pyType, bool isInitialised):
ScriptObject(pyType, isInitialised),
ENTITY_CONSTRUCTION(Base),
clientMailbox_(NULL),
cellMailbox_(NULL),
cellDataDict_(NULL),
hasDB_(false),
isGetingCellData_(false),
isArchiveing_(false),
creatingCell_(false)
{
	ENTITY_INIT_PROPERTYS(Base);

	// 创建并初始化cellData
	createCellData();
}

//-------------------------------------------------------------------------------------
Base::~Base()
{
	ENTITY_DECONSTRUCTION(Base);
}	

//-------------------------------------------------------------------------------------
void Base::onDefDataChanged(const PropertyDescription* propertyDescription, 
		PyObject* pyData)
{
}

//-------------------------------------------------------------------------------------
void Base::destroy()
{
	S_RELEASE(clientMailbox_);
	S_RELEASE(cellMailbox_);
	S_RELEASE(cellDataDict_);
	Py_DECREF(this);
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
	if(!installCellDataAttr())
		return;
	
	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		DataType* dataType = propertyDescription->getDataType();
		
		if(dataType)
		{
			MemoryStream* ms = propertyDescription->getDefaultVal();
			PyDict_SetItemString(cellDataDict_, propertyDescription->getName().c_str(), dataType->createObject(ms));
			if(ms)
				ms->rpos(0);
		}
		else
		{
			ERROR_MSG("Base::createCellData: %s PropertyDescription the dataType is NULL.\n", 
				propertyDescription->getName().c_str());	
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
}

//-------------------------------------------------------------------------------------
void Base::addCellDataToStream(uint32 flags, MemoryStream* s)
{
	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName().c_str());
			(*s) << propertyDescription->getUType();
			propertyDescription->getDataType()->addToStream(s, pyVal);
		}
	}
}

//-------------------------------------------------------------------------------------
void Base::addPersistentsDataToStream(uint32 flags, MemoryStream* s)
{
	std::vector<ENTITY_PROPERTY_UID> log;

	// 先将celldata中的存储属性取出
	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		std::vector<ENTITY_PROPERTY_UID>::const_iterator finditer = std::find(log.begin(), log.end(), propertyDescription->getUType());
		if(finditer != log.end() && propertyDescription->isPersistent() && (flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName().c_str());
			(*s) << propertyDescription->getUType();
			log.push_back(propertyDescription->getUType());
			propertyDescription->getDataType()->addToStream(s, pyVal);
			DEBUG_PERSISTENT_PROPERTY("addCellPersistentsDataToStream", propertyDescription->getName().c_str());
		}
	}

	// 再将base中存储属性取出
	PyObject* pydict = PyObject_GetAttrString(this, "__dict__");
		
	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs1 = getScriptModule()->getBasePropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter1 = propertyDescrs1.begin();
	for(; iter1 != propertyDescrs1.end(); iter1++)
	{
		PropertyDescription* propertyDescription = iter1->second;
		PyObject *key = PyUnicode_FromString(propertyDescription->getName().c_str());
			
		std::vector<ENTITY_PROPERTY_UID>::const_iterator finditer = std::find(log.begin(), log.end(), propertyDescription->getUType());
		if(finditer != log.end() && propertyDescription->isPersistent() && PyDict_Contains(pydict, key) > 0)
	    {
	    	(*s) << propertyDescription->getUType();
			log.push_back(propertyDescription->getUType());
	    	propertyDescription->getDataType()->addToStream(s, PyDict_GetItem(pydict, key));
			DEBUG_PERSISTENT_PROPERTY("addBasePersistentsDataToStream", propertyDescription->getName().c_str());
	    }

		Py_DECREF(key);
	}

	Py_XDECREF(pydict);
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellDataDict(uint32 flags)
{
	PyObject* cellData = PyDict_New();

	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName().c_str());
			PyDict_SetItemString(cellData, propertyDescription->getName().c_str(), pyVal);
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
void Base::getClientPropertys(MemoryStream* s)
{
	// 获得base部分关联的客户端属性数据
	PyObject* pydict = PyObject_GetAttrString(this, "__dict__");
		
	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = getScriptModule()->getClientPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		PyObject *key = PyUnicode_FromString(propertyDescription->getName().c_str());
			
	    if(PyDict_Contains(pydict, key) > 0)
	    {
	    	(*s) << propertyDescription->getUType();
	    	propertyDescription->getDataType()->addToStream(s, PyDict_GetItem(pydict, key));
	    }

		Py_DECREF(key);
	}

	Py_XDECREF(pydict);
}

//-------------------------------------------------------------------------------------
bool Base::destroyCellEntity(void)
{
	Mercury::Bundle bundle;
	bundle.newMessage(CellappInterface::onDestroyCellEntityFromBaseapp);
	bundle << id_;
	bundle.send(Baseapp::getSingleton().getNetworkInterface(), cellMailbox_->getChannel());
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyDestroyCellEntity()
{
	if(cellMailbox_ == NULL) 
	{
		PyErr_Format(PyExc_Exception, "%s::destroyCellEntity: id:%i no cell! creatingCell=%s\n", this->getScriptName(), this->getID(),
			creatingCell_ ? "true" : "false");
		return false;
	}
	else
		destroyCellEntity();

	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::destroyBase(void)
{
	Baseapp::getSingleton().destroyEntity(id_);
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyDestroyBase()
{
	if(creatingCell_ || cellMailbox_ != NULL) 
	{
		PyErr_Format(PyExc_Exception, "%s::destroyBase: id:%i has cell! creatingCell=%s\n", this->getScriptName(), this->getID(),
			creatingCell_ ? "true" : "false");
	}
	else
		destroyBase();

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetCellMailbox()
{ 
	EntityMailbox* mailbox = getCellMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyGetClientMailbox()
{ 
	EntityMailbox* mailbox = getClientMailbox();
	if(mailbox == NULL)
		S_Return;

	Py_INCREF(mailbox);
	return mailbox; 
}

//-------------------------------------------------------------------------------------
void Base::onCreateCellFailure(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onCreateCellFailure"), 
																		const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Base::onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID srcEntityID = pChannel->proxyID();
	if(srcEntityID <= 0 || srcEntityID != this->getID())
	{
		WARNING_MSG("Base::onRemoteMethodCall: srcEntityID:%d, thisEntityID:%d.\n", srcEntityID, this->getID());
		return;
	}

	ENTITY_METHOD_UID utype = 0;
	s >> utype;
	
	MethodDescription* md = scriptModule_->findBaseMethodDescription(utype);
	if(md == NULL)
	{
		ERROR_MSG("Base::onRemoteMethodCall: can't found method. utype=%u, callerID:%d.\n", utype, id_);
		return;
	}

	DEBUG_MSG("Base::onRemoteMethodCall: entityID:%d, methodType:%s(%u).\n", 
		id_, md->getName().c_str(), utype);

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
void Base::onGetCell(Mercury::Channel* pChannel, COMPONENT_ID componentID)
{
	creatingCell_ = false;

	// 删除cellData属性
	destroyCellData();
	
	// 回调给脚本，获得了cell
	cellMailbox_ = new EntityMailbox(scriptModule_, NULL, componentID, id_, MAILBOX_TYPE_CELL);
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onGetCell"), 
																	const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Base::onClientDeath()
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onClientDeath"), 
																		const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Base::onLoseCell(Mercury::Channel* pChannel, MemoryStream& s)
{
	S_RELEASE(cellMailbox_);

	// 通知脚本
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onLoseCell"), 
																		const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Base::reqBackupCellData()
{
	if(isGetingCellData_)
		return;

	EntityMailbox* mb = this->getCellMailbox();
	if(mb == NULL)
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(CellappInterface::reqBackupEntityCellData);
	bundle << this->getID();
	mb->postMail(bundle);

	isGetingCellData_ = true;
}

//-------------------------------------------------------------------------------------
void Base::onBackupCellData(Mercury::Channel* pChannel, MemoryStream& s)
{
	isGetingCellData_ = false;
	std::string strCellData;
	uint32 cellDataLength;
	PyObject* cellData = NULL;

	s >> cellDataLength;
	
	if(cellDataLength > 0)
	{
		strCellData.assign((char*)(s.data() + s.rpos()), cellDataLength);
		s.read_skip(cellDataLength);
		
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
void Base::writeToDB()
{
	if(isArchiveing_)
		return;

	isArchiveing_ = true;

	if(this->getCellMailbox() == NULL)
	{
		onCellWriteToDBComplete();
	}
	else
	{
		Mercury::Bundle bundle;
		bundle.newMessage(CellappInterface::reqWriteToDBFromBaseapp);
		bundle << this->getID();
		this->getCellMailbox()->postMail(bundle);
	}
}

//-------------------------------------------------------------------------------------
void Base::onCellWriteToDBComplete()
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onPreArchive"), const_cast<char*>(""));

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();

	hasDB(true);

	onWriteToDB();

	isArchiveing_ = false;

	MemoryStream s;
	addPersistentsDataToStream(ED_FLAG_ALL, &s);
}

//-------------------------------------------------------------------------------------
void Base::onWriteToDB()
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onWriteToDB"), const_cast<char*>("O"), cellDataDict_);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellEntity(PyObject* pyobj)
{
	if(!PyObject_IsInstance(pyobj, (PyObject*)EntityMailbox::getScriptType()))
	{
		PyErr_Format(PyExc_TypeError, "create %s arg1 is not cellMailbox!", this->getScriptName());
		S_Return;
	}
	
	EntityMailboxAbstract* cellMailbox = static_cast<EntityMailboxAbstract*>(pyobj);
	if(cellMailbox->getType() != MAILBOX_TYPE_CELL)
	{
		PyErr_Format(PyExc_TypeError, "create %s args1 not is a direct cellMailbox!", this->getScriptName());
		S_Return;
	}
	
	creatingCell_ = true;
	Baseapp::getSingleton().createCellEntity(cellMailbox, this);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Base::createInNewSpace(PyObject* params)
{
	Baseapp::getSingleton().createInNewSpace(this, params);
	S_Return;
}

//-------------------------------------------------------------------------------------
}
