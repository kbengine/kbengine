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

#include "baseapp.hpp"
#include "proxy.hpp"
#include "proxy_sender.hpp"
#include "profile.hpp"
#include "data_download.hpp"
#include "client_lib/client_interface.hpp"
#include "network/fixed_messages.hpp"
#include "network/channel.hpp"

#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

#ifndef CODE_INLINE
#include "proxy.ipp"
#endif

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(Proxy)
SCRIPT_METHOD_DECLARE("giveClientTo",					pyGiveClientTo,					METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("getClientType",					pyGetClientType,				METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("streamStringToClient",			pyStreamStringToClient,			METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("streamFileToClient",				pyStreamFileToClient,			METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Proxy)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Proxy)
SCRIPT_GET_DECLARE("roundTripTime",						pyGetRoundTripTime,				0,						0)	
SCRIPT_GET_DECLARE("timeSinceHeardFromClient",			pyGetTimeSinceHeardFromClient,	0,						0)	
SCRIPT_GET_DECLARE("clientAddr",						pyClientAddr,					0,						0)	
SCRIPT_GET_DECLARE("hasClient",							pyHasClient,					0,						0)	
SCRIPT_GET_DECLARE("entitiesEnabled",					pyGetEntitiesEnabled,			0,						0)	
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Proxy, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Proxy::Proxy(ENTITY_ID id, const ScriptDefModule* scriptModule):
Base(id, scriptModule, getScriptType(), true),
rndUUID_(KBEngine::genUUID64()),
addr_(Mercury::Address::NONE),
dataDownloads_(),
entitiesEnabled_(false),
bandwidthPerSecond_(0),
encryptionKey(),
pProxySender_(NULL),
clientComponentType_(UNKNOWN_CLIENT_COMPONENT_TYPE)
{
	Baseapp::getSingleton().incProxicesCount();

	pProxySender_ = new ProxySender(this);
}

//-------------------------------------------------------------------------------------
Proxy::~Proxy()
{
	Baseapp::getSingleton().decProxicesCount();

	// 如果被销毁频道仍然存活则将其关闭
	Mercury::Channel* pChannel = Baseapp::getSingleton().getNetworkInterface().findChannel(addr_);
	if(pChannel && !pChannel->isDead())
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(ClientInterface::onKicked);
		ClientInterface::onKickedArgs1::staticAddToBundle(*pBundle, SERVER_ERR_PROXY_DESTROYED);
		//pBundle->send(Baseapp::getSingleton().getNetworkInterface(), pChannel);
		//Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		this->sendToClient(ClientInterface::onKicked, pBundle);
		this->sendToClient();
		pChannel->condemn();
	}

	SAFE_RELEASE(pProxySender_);
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
		sendToClient(ClientInterface::onUpdatePropertys, pBundle);
		//getClientMailbox()->postMail((*pBundle));
		//Mercury::Bundle::ObjPool().reclaimObject(pBundle);
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
	//getClientMailbox()->postMail((*pBundle));
	//Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	sendToClient(ClientInterface::onUpdatePropertys, pBundle);
}

//-------------------------------------------------------------------------------------
void Proxy::onEntitiesEnabled(void)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	entitiesEnabled_ = true;
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
	if(getClientMailbox() == NULL)
	{
		ERROR_MSG(boost::format("%1%::onClientDeath: %2%, channel is null!\n") % 
			this->getScriptName() % this->getID());

		return;
	}

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	DEBUG_MSG(boost::format("%1%::onClientDeath: %2%.\n") % 
		this->getScriptName() % this->getID());

	Py_DECREF(getClientMailbox());
	setClientMailbox(NULL);
	addr(Mercury::Address::NONE);

	entitiesEnabled_ = false;
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientDeath"));
}

//-------------------------------------------------------------------------------------
void Proxy::onClientGetCell(Mercury::Channel* pChannel)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientGetCell"));
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetClientType()
{
	return PyLong_FromLong((long)getClientType());
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGiveClientTo(PyObject* pyOterProxy)
{
	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;
	}

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
	if(isDestroyed())
	{
		char err[255];																				
		kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: %d is destroyed.", 
			getScriptName(), getID());			

		PyErr_SetString(PyExc_TypeError, err);														
		PyErr_PrintEx(0);	
		onGiveClientToFailure();
		return;
	}

	if(clientMailbox_ == NULL || clientMailbox_->getChannel() == NULL)
	{
		char err[255];																				
		kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: no has client.", getScriptName());			
		PyErr_SetString(PyExc_TypeError, err);														
		PyErr_PrintEx(0);	
		onGiveClientToFailure();
		return;
	}

	Mercury::Channel* lpChannel = clientMailbox_->getChannel();

	if(proxy)
	{
		if(proxy->isDestroyed())
		{
			char err[255];																				
			kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: target(%d) is destroyed.", 
				getScriptName(), proxy->getID());			

			PyErr_SetString(PyExc_TypeError, err);														
			PyErr_PrintEx(0);	
			onGiveClientToFailure();
			return;
		}

		EntityMailbox* mb = proxy->getClientMailbox();
		if(mb != NULL)
		{
			ERROR_MSG(boost::format("Proxy::giveClientTo: %1%[%2%] give client to %3%[%4%], %5% have clientMailbox.\n") % 
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
			(*pBundle).newMessage(CellappInterface::onResetWitness);
			(*pBundle) << this->getID();
			getCellMailbox()->postMail((*pBundle));
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}

		entitiesEnabled_ = false;
		getClientMailbox()->addr(Mercury::Address::NONE);
		Py_DECREF(getClientMailbox());
		proxy->onGiveClientTo(lpChannel);
		setClientMailbox(NULL);
		addr(Mercury::Address::NONE);
		
		if(proxy->getClientMailbox() != NULL)
		{
			// 通知client销毁当前entity
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			(*pBundle).newMessage(ClientInterface::onEntityDestroyed);
			(*pBundle) << this->getID();
			proxy->getClientMailbox()->postMail((*pBundle));
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}
	}
}

//-------------------------------------------------------------------------------------
void Proxy::onGiveClientTo(Mercury::Channel* lpChannel)
{
	setClientMailbox(new EntityMailbox(this->scriptModule_, 
		&lpChannel->addr(), 0, id_, MAILBOX_TYPE_CLIENT));

	addr(lpChannel->addr());
	Baseapp::getSingleton().createClientProxies(this);

	/*
	如果有cell则已经绑定了witness， 在此我们不需要再次绑定。
	if(getCellMailbox())
	{
		// 通知cell获得客户端
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(CellappInterface::onGetWitness);
		(*pBundle) << this->getID();
		getCellMailbox()->postMail((*pBundle));
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
	*/
}

//-------------------------------------------------------------------------------------
double Proxy::getRoundTripTime()const
{
	if(getClientMailbox() == NULL || getClientMailbox()->getChannel() == NULL || 
		getClientMailbox()->getChannel()->endpoint() == NULL)
		return 0.0;

	return double(getClientMailbox()->getChannel()->endpoint()->getRTT()) / 1000000.0;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetRoundTripTime()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	return PyFloat_FromDouble(this->getRoundTripTime()); 
}

//-------------------------------------------------------------------------------------
double Proxy::getTimeSinceHeardFromClient()const
{
	if(getClientMailbox() == NULL || getClientMailbox()->getChannel() == NULL || 
		getClientMailbox()->getChannel()->endpoint() == NULL)
		return DBL_MAX;

	return double(timestamp() - getClientMailbox()->getChannel()->lastReceivedTime()) / stampsPerSecondD();
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetTimeSinceHeardFromClient()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;																					
	}
	
	return PyFloat_FromDouble(this->getTimeSinceHeardFromClient()); 
}

//-------------------------------------------------------------------------------------
bool Proxy::hasClient()const
{
	if(getClientMailbox() == NULL || getClientMailbox()->getChannel() == NULL || 
		getClientMailbox()->getChannel()->endpoint() == NULL)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyHasClient()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	if(this->hasClient())
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyClientAddr()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	PyObject* pyobj = PyTuple_New(2);

	if(getClientMailbox() == NULL || getClientMailbox()->getChannel() == NULL || 
		getClientMailbox()->getChannel()->endpoint() == NULL)
	{
		PyTuple_SetItem(pyobj, 0, PyLong_FromLong(0));
		PyTuple_SetItem(pyobj, 1, PyLong_FromLong(0));
	}
	else
	{
		const Mercury::Address& addr = getClientMailbox()->getChannel()->endpoint()->addr();
		PyTuple_SetItem(pyobj, 0, PyLong_FromUnsignedLong(addr.ip));
		PyTuple_SetItem(pyobj, 1, PyLong_FromUnsignedLong(addr.port));
	}

	return pyobj;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetEntitiesEnabled()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			getScriptName(), getID());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	if(this->entitiesEnabled())
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::__py_pyStreamFileToClient(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Proxy* pobj = static_cast<Proxy*>(self);

	if(pobj->getClientMailbox() == NULL)
	{
		PyErr_Format(PyExc_AssertionError,
						"Proxy::streamStringToClient: has no client.");
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
						"Proxy::streamStringToClient: has no client.");
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
Proxy::Bundles* Proxy::pBundles()
{
	if(!getClientMailbox())
		return NULL;

	Mercury::Channel* pChannel = getClientMailbox()->getChannel();
	if(!pChannel)
		return NULL;

	return &pChannel->bundles();
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(const Mercury::MessageHandler& msgHandler, Mercury::Bundle* pBundle)
{
	return sendToClient(pBundle);
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(Mercury::Bundle* pBundle)
{
	Bundles* lpBundles = pBundles();

	if(lpBundles)
	{
		lpBundles->push_back(pBundle);
		return true;
	}

	ERROR_MSG(boost::format("Proxy::sendToClient: %1% pBundles is NULL, not found channel.\n") % getID());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return false;
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(bool expectData)
{
	if(!getClientMailbox())
		return false;

	Mercury::Channel* pChannel = getClientMailbox()->getChannel();
	if(!pChannel)
		return false;

	if(expectData)
	{
		if(pChannel->bundles().size() == 0)
		{
			WARNING_MSG("Proxy::sendToClient: no data!\n");
			return false;
		}
	}

	{
		// 如果数据大量阻塞发不出去将会报警
		AUTO_SCOPED_PROFILE("sendToClient");
		pChannel->send();
	}

	return true;
}

//-------------------------------------------------------------------------------------
}
