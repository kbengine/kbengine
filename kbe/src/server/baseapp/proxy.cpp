#include "baseapp.hpp"
#include "proxy.hpp"
#include "profile.hpp"
#include "data_download.hpp"
#include "client_lib/client_interface.hpp"
#include "network/fixed_messages.hpp"

#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(Proxy)
SCRIPT_METHOD_DECLARE("giveClientTo",					pyGiveClientTo,					METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("streamStringToClient",			pyStreamStringToClient,			METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("streamFileToClient",				pyStreamFileToClient,			METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Proxy)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Proxy)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Proxy, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Proxy::Proxy(ENTITY_ID id, const ScriptDefModule* scriptModule):
Base(id, scriptModule, getScriptType(), true),
rndUUID_(KBEngine::genUUID64()),
addr_(Mercury::Address::NONE),
dataDownloads_()
{
	Baseapp::getSingleton().incProxicesCount();
}

//-------------------------------------------------------------------------------------
Proxy::~Proxy()
{
	Baseapp::getSingleton().decProxicesCount();
}

//-------------------------------------------------------------------------------------
void Proxy::initClientBasePropertys()
{
	if(getClientMailbox() == NULL)
		return;

	MemoryStream* s1 = MemoryStream::ObjPool().createObject();
	addClientDataToStream(s1);
	
	if(s1->wpos() > 0)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(ClientInterface::onUpdatePropertys);
		(*pBundle) << this->getID();
		(*pBundle).append(*s1);
		getClientMailbox()->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}

	MemoryStream::ObjPool().reclaimObject(s1);
}

//-------------------------------------------------------------------------------------
void Proxy::initClientCellPropertys()
{
	if(getClientMailbox() == NULL)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onUpdatePropertys);
	(*pBundle) << this->getID();

	ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

	Mercury::FixedMessages::MSGInfo* msgInfo = 
		Mercury::FixedMessages::getSingleton().isFixed("Property::spaceID");

	if(msgInfo != NULL)
	{
		spaceuid = msgInfo->msgid;
	}
	
	(*pBundle) << spaceuid << this->getSpaceID();

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	addPositionAndDirectionToStream(*s);
	(*pBundle).append(s);
	MemoryStream::ObjPool().reclaimObject(s);

	// celldata获取客户端感兴趣的数据初始化客户端 如:ALL_CLIENTS
	s = MemoryStream::ObjPool().createObject();
	addCellDataToStream(ED_FLAG_ALL_CLIENTS|ED_FLAG_CELL_PUBLIC_AND_OWN|ED_FLAG_OWN_CLIENT, s);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	getClientMailbox()->postMail((*pBundle));
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Proxy::onEntitiesEnabled(void)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEntitiesEnabled"));
}

//-------------------------------------------------------------------------------------
int32 Proxy::onLogOnAttempt(const char* addr, uint32 port, const char* password)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onLogOnAttempt"), const_cast<char*>("uku"), 
		PyUnicode_FromString(addr), 
		PyLong_FromLong(port),
		PyUnicode_FromString(password)
	);
	
	int32 ret = LOG_ON_REJECT;
	if(pyResult != NULL)
	{
		ret = PyLong_AsLong(pyResult);
		SCRIPT_ERROR_CHECK();
		Py_DECREF(pyResult);
	}
	else
		SCRIPT_ERROR_CHECK();

	return ret;
}

//-------------------------------------------------------------------------------------
void Proxy::onClientDeath(void)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	DEBUG_MSG(boost::format("%1%::onClientDeath: %2%.\n") % this->getScriptName() % this->getID());

	if(getClientMailbox() != NULL)
	{
		Py_DECREF(getClientMailbox());
		setClientMailbox(NULL);
		addr(Mercury::Address::NONE);
	}

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientDeath"));
}

//-------------------------------------------------------------------------------------
void Proxy::onClientGetCell(Mercury::Channel* pChannel)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientGetCell"));
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGiveClientTo(PyObject* pyOterProxy)
{
	// 如果为None 则设置为NULL
	Proxy* oterProxy = NULL;
	if(pyOterProxy != Py_None)
		oterProxy = static_cast<Proxy*>(pyOterProxy);
	
	giveClientTo(oterProxy);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Proxy::onGiveClientToFailure()
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onGiveClientToFailure"));
}

//-------------------------------------------------------------------------------------
void Proxy::giveClientTo(Proxy* proxy)
{
	if(clientMailbox_ == NULL || clientMailbox_->getChannel() == NULL)
	{
		char err[255];																				
		kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: no has client.\n", getScriptName());			
		PyErr_SetString(PyExc_TypeError, err);														
		PyErr_PrintEx(0);	
		onGiveClientToFailure();
		return;
	}

	Mercury::Channel* lpChannel = clientMailbox_->getChannel();

	if(proxy)
	{
		EntityMailbox* mb = proxy->getClientMailbox();
		if(mb != NULL)
		{
			ERROR_MSG(boost::format("Proxy::giveClientTo: %1%[%2%] give client to %3%[%4%], %5% have clientMailbox.") % 
					getScriptName() %
					getID() %
					proxy->getScriptName() % 
					proxy->getID() %
					proxy->getScriptName());

			onGiveClientToFailure();
			return;
		}

		if(getCellMailbox())
		{
			// 通知cell丢失客户端
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			(*pBundle).newMessage(CellappInterface::onLoseWitness);
			(*pBundle) << this->getID();
			getCellMailbox()->postMail((*pBundle));
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}

		getClientMailbox()->addr(Mercury::Address::NONE);
		Py_DECREF(getClientMailbox());
		proxy->onGiveClientTo(lpChannel);
		setClientMailbox(NULL);
		addr(Mercury::Address::NONE);
	}
}

//-------------------------------------------------------------------------------------
void Proxy::onGiveClientTo(Mercury::Channel* lpChannel)
{
	setClientMailbox(new EntityMailbox(this->scriptModule_, 
		&lpChannel->addr(), 0, id_, MAILBOX_TYPE_CLIENT));

	addr(lpChannel->addr());
	Baseapp::getSingleton().createClientProxies(this);

	if(getCellMailbox())
	{
		// 通知cell获得客户端
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(CellappInterface::onGetWitness);
		getCellMailbox()->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::__py_pyStreamFileToClient(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Proxy* pobj = static_cast<Proxy*>(self);

	if(pobj->getClientMailbox() == NULL)
	{
		PyErr_Format(PyExc_AssertionError,
						"Proxy::streamStringToClient: has no client.\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(currargsSize > 3 || currargsSize == 0)
	{
		PyErr_Format(PyExc_AssertionError,
						"Proxy::streamFileToClient: args max require 3, gived %d! is script[%s].\n",
			currargsSize, pobj->getScriptName());
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyResourceName = NULL;
	PyObject* pyDesc = NULL;
	int16 id = -1;

	if(currargsSize == 1)
	{
		if(PyArg_ParseTuple(args, "O", &pyResourceName) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: args is error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 2)
	{
		if(PyArg_ParseTuple(args, "O|O", &pyResourceName, &pyDesc) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: args is error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 3)
	{
		if(PyArg_ParseTuple(args, "O|O|H", &pyResourceName, &pyDesc, &id) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: args is error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}

	char* pDescr = NULL;

	if(pDescr != NULL)
	{
		wchar_t* PyUnicode_AsWideCharStringRet1 = PyUnicode_AsWideCharString(pyDesc, NULL);
		pDescr = strutil::wchar2char(PyUnicode_AsWideCharStringRet1);
		PyMem_Free(PyUnicode_AsWideCharStringRet1);
	}

	if(pDescr && strlen(pDescr) > 255)
	{
		PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: the descr-size(%d > 255)!", 
			strlen(pDescr));

		PyErr_PrintEx(0);
		free(pDescr);
		return NULL;
	}

	int16 rid = pobj->streamFileToClient(pyResourceName, 
							(pDescr == NULL ? "" : pDescr),  
							id);

	if(pDescr)
		free(pDescr);

	return PyLong_FromLong(rid);
}

//-------------------------------------------------------------------------------------
int16 Proxy::streamFileToClient(PyObjectPtr objptr, 
	const std::string& descr, int16 id)
{
	DataDownload* pDataDownload = DataDownloadFactory::create(
		DataDownloadFactory::DATA_DOWNLOAD_STREAM_FILE, objptr, descr, id);

	pDataDownload->entityID(this->getID());
	return dataDownloads_.pushDownload(pDataDownload);
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::__py_pyStreamStringToClient(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Proxy* pobj = static_cast<Proxy*>(self);

	if(pobj->getClientMailbox() == NULL)
	{
		PyErr_Format(PyExc_AssertionError,
						"Proxy::streamStringToClient: has no client.\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(currargsSize > 3 || currargsSize == 0)
	{
		PyErr_Format(PyExc_AssertionError,
						"Proxy::streamStringToClient: args max require 3, gived %d! is script[%s].\n",
			currargsSize, pobj->getScriptName());
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyData = NULL;
	PyObject* pyDesc = NULL;
	int16 id = -1;

	if(currargsSize == 1)
	{
		if(PyArg_ParseTuple(args, "O", &pyData) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamStringToClient: args is error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 2)
	{
		if(PyArg_ParseTuple(args, "O|O", &pyData, &pyDesc) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamStringToClient: args is error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 3)
	{
		if(PyArg_ParseTuple(args, "O|O|H", &pyData, &pyDesc, &id) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamStringToClient: args is error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}

	char* pDescr = NULL;

	if(pDescr != NULL)
	{
		wchar_t* PyUnicode_AsWideCharStringRet1 = PyUnicode_AsWideCharString(pyDesc, NULL);
		pDescr = strutil::wchar2char(PyUnicode_AsWideCharStringRet1);
		PyMem_Free(PyUnicode_AsWideCharStringRet1);
	}

	if(pDescr && strlen(pDescr) > 255)
	{
		PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: the descr-size(%d > 255)!", 
			strlen(pDescr));

		PyErr_PrintEx(0);
		free(pDescr);
		return NULL;
	}

	int16 rid = pobj->streamStringToClient(pyData, 
						(pDescr == NULL ? "" : pDescr),  
						id);

	if(pDescr)
		free(pDescr);

	return PyLong_FromLong(rid);
}

//-------------------------------------------------------------------------------------
int16 Proxy::streamStringToClient(PyObjectPtr objptr, 
	const std::string& descr, int16 id)
{
	DataDownload* pDataDownload = DataDownloadFactory::create(
		DataDownloadFactory::DATA_DOWNLOAD_STREAM_STRING, objptr, descr, id);

	pDataDownload->entityID(this->getID());
	return dataDownloads_.pushDownload(pDataDownload);
}

//-------------------------------------------------------------------------------------
}
