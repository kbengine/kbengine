#include "baseapp.hpp"
#include "base.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "network/channel.hpp"	

#ifdef CODE_INLINE
//#include "base.ipp"
#endif


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
cellDataDict_(NULL)
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
	}
	else if(cellDataDict_ == NULL)
		cellDataDict_ = PyDict_New();
	
	if(PyObject_SetAttrString(this, "cellData", cellDataDict_) == -1)
	{
		ERROR_MSG("Base::installCellDataAttr: set property cellData is error!");
		SCRIPT_ERROR_CHECK();
		S_RELEASE(cellDataDict_);
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
	ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		DataType* dataType = propertyDescription->getDataType();
		
		if(dataType)
		{
			//ByteStream* bs = propertyDescription->getDefaultVal();
			//PyDict_SetItemString(cellDataDict_, propertyDescription->getName().c_str(), dataType->createObject(bs));
			//if(bs)
			//	bs->rpos(0);
		}
		else
			ERROR_MSG("Base::createCellData: %s PropertyDescription the dataType is NULL.", propertyDescription->getName().c_str());		
			
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
void Base::getCellDataByFlags(uint32 flags, MemoryStream* s)
{
	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();
	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		if((flags & propertyDescription->getFlags()) > 0)
		{
			PyObject* pyVal = PyDict_GetItemString(cellDataDict_, propertyDescription->getName().c_str());
			(*s) << (uint32)propertyDescription->getUType();
			propertyDescription->getDataType()->addToStream(s, pyVal);
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellDataDictByFlags(uint32 flags)
{
	PyObject* cellData = PyDict_New();

	ScriptModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule_->getCellPropertyDescriptions();
	ScriptModule::PROPERTYDESCRIPTION_MAP::iterator iter = propertyDescrs.begin();
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
	S_RELEASE(cellDataDict_);
	if(PyObject_DelAttrString(this, "cellData") == -1)
	{
		ERROR_MSG("Base::destroyCellData: del property cellData is error!");
		SCRIPT_ERROR_CHECK();
	}
}

//-------------------------------------------------------------------------------------
bool Base::destroyCellEntity(void)
{
	if(cellMailbox_ == NULL) 
		return false;
	
//	SocketPacket* sp = new SocketPacket(OP_ENTITY_DESTROY_CELL_ENTITY, 8);
//	(*sp) << id_;
//	cellMailbox_->post(sp);
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyDestroyCellEntity()
{
	destroyCellEntity();
	S_Return;
}

//-------------------------------------------------------------------------------------
void Base::destroyBase(void)
{
	//Baseapp::getSingleton().destroyBase(id_);
}

//-------------------------------------------------------------------------------------
PyObject* Base::pyDestroyBase()
{
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
void Base::onGetCell(Mercury::Channel* handler, COMPONENT_ID componentID)
{
	// 删除cellData属性
	destroyCellData();
	
	// 回调给脚本，获得了cell
	cellMailbox_ = new EntityMailbox(handler, scriptModule_, componentID, id_, MAILBOX_TYPE_CELL);
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onGetCell"), 
																	const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Base::onClientGetCell()
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onClientGetCell"), 
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
void Base::onLoseCell(PyObject* cellData)
{
	S_RELEASE(cellMailbox_);
	installCellDataAttr(cellData);
	
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onLoseCell"), 
																		const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
void Base::onWriteToDB(PyObject* cellData)
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onWriteToDB"), const_cast<char*>("O"), cellData);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
}

//-------------------------------------------------------------------------------------
PyObject* Base::createCellEntity(PyObject* pyobj)
{
	if(!PyObject_TypeCheck(pyobj, EntityMailbox::getScriptType()))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 is not cellMailbox!");
		PyErr_PrintEx(0);
		S_Return;
	}

	EntityMailboxAbstract* cellMailbox = static_cast<EntityMailboxAbstract*>(pyobj);
	if(cellMailbox->getType() != MAILBOX_TYPE_CELL)
	{
		PyErr_SetString(PyExc_TypeError, "args not is a direct cellMailbox!");
		PyErr_PrintEx(0);
		S_Return;
	}
	
	//Baseapp::getSingleton().createCellEntity(cellMailbox, base);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Base::createInNewSpace(PyObject* params)
{
	//Baseapp::getSingleton().createInNewSpace(base, params);
	S_Return;
}

//-------------------------------------------------------------------------------------
/*
void Base::onRemoteMethodCall(SocketPacket& recvPacket)
{
	uint32 utype = 0;
	recvPacket >> (uint32)utype;
	DEBUG_MSG("Base::onRemoteMethodCall: entityID %d, methodType %ld.\n", id_, utype);
	MethodDescription* md = scriptModule_->findBaseMethodDescription(utype);
	PyObject* pyFunc = PyObject_GetAttrString(this, const_cast<char*>(md->getName().c_str()));
	if(md != NULL)
		md->call(pyFunc, md->createFromStream(&recvPacket));
	
	Py_XDECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
void Base::onReceiveMail(MAIL_TYPE& mailType, SocketPacket& recvPacket)
{
	DEBUG_MSG("Base::onReceiveMail: entityID %d, mailType %ld.\n", id_, mailType);
	switch(mailType)
	{
		case MAIL_TYPE_REMOTE_CALL:
			onRemoteMethodCall(recvPacket);
			break;
	};
}
*/
//-------------------------------------------------------------------------------------
}
