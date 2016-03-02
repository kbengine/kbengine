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
#include "proxy.h"
#include "proxy_forwarder.h"
#include "profile.h"
#include "data_download.h"
#include "client_lib/client_interface.h"
#include "network/fixed_messages.h"
#include "network/channel.h"

#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"

#ifndef CODE_INLINE
#include "proxy.inl"
#endif

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(Proxy)
SCRIPT_METHOD_DECLARE("giveClientTo",					pyGiveClientTo,					METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("getClientType",					pyGetClientType,				METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE("getClientDatas",					pyGetClientDatas,				METH_VARARGS,			0)
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
Proxy::Proxy(ENTITY_ID id, const ScriptDefModule* pScriptModule):
Base(id, pScriptModule, getScriptType(), true),
rndUUID_(KBEngine::genUUID64()),
addr_(Network::Address::NONE),
dataDownloads_(),
entitiesEnabled_(false),
bandwidthPerSecond_(0),
encryptionKey(),
pProxyForwarder_(NULL),
clientComponentType_(UNKNOWN_CLIENT_COMPONENT_TYPE),
clientDatas_()
{
	Baseapp::getSingleton().incProxicesCount();

	pProxyForwarder_ = new ProxyForwarder(this);
}

//-------------------------------------------------------------------------------------
Proxy::~Proxy()
{
	Baseapp::getSingleton().decProxicesCount();
	kick();
	SAFE_RELEASE(pProxyForwarder_);
}

//-------------------------------------------------------------------------------------
void Proxy::kick()
{
	// ���������Ƶ����Ȼ�������ر�
	Network::Channel* pChannel = Baseapp::getSingleton().networkInterface().findChannel(addr_);
	if(pChannel && !pChannel->isDestroyed())
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(ClientInterface::onKicked);
		ClientInterface::onKickedArgs1::staticAddToBundle(*pBundle, SERVER_ERR_PROXY_DESTROYED);
		//pBundle->send(Baseapp::getSingleton().networkInterface(), pChannel);
		this->sendToClient(ClientInterface::onKicked, pBundle);
		this->sendToClient();
		pChannel->condemn();
	}
}

//-------------------------------------------------------------------------------------
void Proxy::initClientBasePropertys()
{
	if(clientMailbox() == NULL)
		return;

	MemoryStream* s1 = MemoryStream::ObjPool().createObject();
	addClientDataToStream(s1);
	
	if(s1->wpos() > 0)
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(ClientInterface::onUpdatePropertys);
		(*pBundle) << this->id();
		(*pBundle).append(*s1);
		sendToClient(ClientInterface::onUpdatePropertys, pBundle);
		//clientMailbox()->postMail((*pBundle));
	}

	MemoryStream::ObjPool().reclaimObject(s1);
}

//-------------------------------------------------------------------------------------
void Proxy::initClientCellPropertys()
{
	if(clientMailbox() == NULL)
		return;

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onUpdatePropertys);
	(*pBundle) << this->id();

	ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

	Network::FixedMessages::MSGInfo* msgInfo = 
		Network::FixedMessages::getSingleton().isFixed("Property::spaceID");

	if(msgInfo != NULL)
	{
		spaceuid = msgInfo->msgid;
	}
	
	if(pScriptModule()->usePropertyDescrAlias())
	{
		uint8 aliasID = ENTITY_BASE_PROPERTY_ALIASID_SPACEID;
		(*pBundle) << aliasID << this->spaceID();
	}
	else
	{
		(*pBundle) << spaceuid << this->spaceID();
	}

	MemoryStream* s = MemoryStream::ObjPool().createObject();

	// celldata��ȡ�ͻ��˸���Ȥ�����ݳ�ʼ���ͻ��� ��:ALL_CLIENTS
	addCellDataToStream(ED_FLAG_ALL_CLIENTS|ED_FLAG_CELL_PUBLIC_AND_OWN|ED_FLAG_OWN_CLIENT, s, true);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	//clientMailbox()->postMail((*pBundle));
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
		const_cast<char*>("onLogOnAttempt"), const_cast<char*>("sks"), 
		addr, 
		port,
		password
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
	if(clientMailbox() == NULL)
	{
		ERROR_MSG(fmt::format("{}::onClientDeath: {}, channel is null!\n", 
			this->scriptName(), this->id()));

		return;
	}

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	DEBUG_MSG(fmt::format("{}::onClientDeath: {}.\n", 
		this->scriptName(), this->id()));

	Py_DECREF(clientMailbox());
	clientMailbox(NULL);
	addr(Network::Address::NONE);

	entitiesEnabled_ = false;
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientDeath"));
}

//-------------------------------------------------------------------------------------
void Proxy::onClientGetCell(Network::Channel* pChannel, COMPONENT_ID componentID)
{
	// �ص����ű��������cell
	if(cellMailbox_ == NULL)
		cellMailbox_ = new EntityMailbox(pScriptModule_, NULL, componentID, id_, MAILBOX_TYPE_CELL);

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onClientGetCell"));
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetClientType()
{
	return PyLong_FromLong((long)getClientType());
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetClientDatas()
{
	const std::string& datas = this->getClientDatas();
	return PyBytes_FromStringAndSize(datas.data(), datas.size());
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGiveClientTo(PyObject* pyOterProxy)
{
	if(this->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;
	}

	// ���ΪNone ������ΪNULL
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
			scriptName(), id());

		PyErr_SetString(PyExc_TypeError, err);
		PyErr_PrintEx(0);	
		onGiveClientToFailure();
		return;
	}

	if(clientMailbox_ == NULL || clientMailbox_->getChannel() == NULL)
	{
		char err[255];
		kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: no has client.", scriptName());
		PyErr_SetString(PyExc_TypeError, err);
		PyErr_PrintEx(0);
		onGiveClientToFailure();
		return;
	}

	Network::Channel* lpChannel = clientMailbox_->getChannel();

	if(proxy)
	{
		if(proxy->isDestroyed())
		{
			char err[255];
			kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: target(%d) is destroyed.",
				scriptName(), proxy->id());

			PyErr_SetString(PyExc_TypeError, err);
			PyErr_PrintEx(0);	
			onGiveClientToFailure();
			return;
		}

		if(proxy->id() == this->id())
		{
			char err[255];
			kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: target(%d) is self.", 
				scriptName(), proxy->id());	

			PyErr_SetString(PyExc_TypeError, err);
			PyErr_PrintEx(0);	
			onGiveClientToFailure();
			return;
		}

		EntityMailbox* mb = proxy->clientMailbox();
		if(mb != NULL)
		{
			ERROR_MSG(fmt::format("Proxy::giveClientTo: {}[{}] give client to {}[{}], {} has clientMailbox.\n", 
					scriptName(),
					id(),
					proxy->scriptName(), 
					proxy->id(),
					proxy->scriptName()));

			onGiveClientToFailure();
			return;
		}

		if(cellMailbox())
		{
			// ��ǰ���entity�����cell��˵���Ѿ�����witness�� ��ô��Ȼ���ǽ�����Ȩ
			// ����������һ��entity�� ���entity��Ҫ���witness��
			// ֪ͨcell��ʧwitness
			Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
			(*pBundle).newMessage(CellappInterface::onLoseWitness);
			(*pBundle) << this->id();
			sendToCellapp(pBundle);
		}

		// ��Ȼ�ͻ���ʧȥ����Ŀ���, ��ô֪ͨclient�������entity
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(ClientInterface::onEntityDestroyed);
		(*pBundle) << this->id();
		sendToClient(ClientInterface::onEntityDestroyed, pBundle);

		// ������Ȩ����
		entitiesEnabled_ = false;
		clientMailbox()->addr(Network::Address::NONE);
		Py_DECREF(clientMailbox());
		proxy->setClientType(this->getClientType());
		proxy->setClientDatas(this->getClientDatas());
		this->setClientType(UNKNOWN_CLIENT_COMPONENT_TYPE);
		this->setClientDatas("");
		clientMailbox(NULL);
		proxy->onGiveClientTo(lpChannel);
		addr(Network::Address::NONE);
	}
}

//-------------------------------------------------------------------------------------
void Proxy::onGiveClientTo(Network::Channel* lpChannel)
{
	clientMailbox(new EntityMailbox(this->pScriptModule_, 
		&lpChannel->addr(), 0, id_, MAILBOX_TYPE_CLIENT));

	addr(lpChannel->addr());
	Baseapp::getSingleton().createClientProxies(this);

	// �����cell, ��Ҫ֪ͨ����witness�� ��Ϊ����ͻ��˸ոհ󶨵����proxy
	// ��ʱ���entity��ʹ��cell�������������û��witness�ġ�
	onGetWitness();
}

//-------------------------------------------------------------------------------------
void Proxy::onGetWitness()
{
	if(cellMailbox())
	{
		// ֪ͨcell��ÿͻ���
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(CellappInterface::onGetWitnessFromBase);
		(*pBundle) << this->id();
		sendToCellapp(pBundle);
	}
}

//-------------------------------------------------------------------------------------
double Proxy::getRoundTripTime() const
{
	if(clientMailbox() == NULL || clientMailbox()->getChannel() == NULL || 
		clientMailbox()->getChannel()->pEndPoint() == NULL)
		return 0.0;

	return double(clientMailbox()->getChannel()->pEndPoint()->getRTT()) / 1000000.0;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetRoundTripTime()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	return PyFloat_FromDouble(this->getRoundTripTime()); 
}

//-------------------------------------------------------------------------------------
double Proxy::getTimeSinceHeardFromClient() const
{
	if(clientMailbox() == NULL || clientMailbox()->getChannel() == NULL || 
		clientMailbox()->getChannel()->pEndPoint() == NULL)
		return DBL_MAX;

	return double(timestamp() - clientMailbox()->getChannel()->lastReceivedTime()) / stampsPerSecondD();
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetTimeSinceHeardFromClient()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																					
	}
	
	return PyFloat_FromDouble(this->getTimeSinceHeardFromClient()); 
}

//-------------------------------------------------------------------------------------
bool Proxy::hasClient() const
{
	if(clientMailbox() == NULL || clientMailbox()->getChannel() == NULL || 
		clientMailbox()->getChannel()->pEndPoint() == NULL)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyHasClient()
{ 
	if(isDestroyed())	
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		
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
			scriptName(), id());		
		PyErr_PrintEx(0);
		return 0;																				
	}

	PyObject* pyobj = PyTuple_New(2);

	if(clientMailbox() == NULL || clientMailbox()->getChannel() == NULL || 
		clientMailbox()->getChannel()->pEndPoint() == NULL)
	{
		PyTuple_SetItem(pyobj, 0, PyLong_FromLong(0));
		PyTuple_SetItem(pyobj, 1, PyLong_FromLong(0));
	}
	else
	{
		const Network::Address& addr = clientMailbox()->getChannel()->pEndPoint()->addr();
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
			scriptName(), id());		
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

	if(pobj->clientMailbox() == NULL)
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
			currargsSize, pobj->scriptName());
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

	pDataDownload->entityID(this->id());
	return dataDownloads_.pushDownload(pDataDownload);
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::__py_pyStreamStringToClient(PyObject* self, PyObject* args)
{
	uint16 currargsSize = PyTuple_Size(args);
	Proxy* pobj = static_cast<Proxy*>(self);

	if(pobj->clientMailbox() == NULL)
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
			currargsSize, pobj->scriptName());
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

	pDataDownload->entityID(this->id());
	return dataDownloads_.pushDownload(pDataDownload);
}

//-------------------------------------------------------------------------------------
bool Proxy::pushBundle(Network::Bundle* pBundle)
{
	if(!clientMailbox())
		return false;

	Network::Channel* pChannel = clientMailbox()->getChannel();
	if(!pChannel)
		return false;

	pChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle)
{
	return sendToClient(pBundle);
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(Network::Bundle* pBundle)
{
	if(pushBundle(pBundle))
		return true;

	ERROR_MSG(fmt::format("Proxy::sendToClient: {} pBundles is NULL, not found channel.\n", id()));
	Network::Bundle::ObjPool().reclaimObject(pBundle);
	return false;
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(bool expectData)
{
	if(!clientMailbox())
		return false;

	Network::Channel* pChannel = clientMailbox()->getChannel();
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
		// ������ݴ�������������ȥ���ᱨ��
		AUTO_SCOPED_PROFILE("sendToClient");
		pChannel->send();
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Proxy::onStreamComplete(int16 id, bool success)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	SCRIPT_OBJECT_CALL_ARGS2(this, const_cast<char*>("onStreamComplete"), 
		const_cast<char*>("hO"), id, success ? Py_True : Py_False);
}

//-------------------------------------------------------------------------------------
}
