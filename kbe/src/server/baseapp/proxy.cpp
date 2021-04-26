// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
SCRIPT_METHOD_DECLARE("disconnect",						pyDisconnect,					METH_VARARGS,			0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Proxy)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Proxy)
SCRIPT_GET_DECLARE("roundTripTime",						pyGetRoundTripTime,				0,						0)	
SCRIPT_GET_DECLARE("timeSinceHeardFromClient",			pyGetTimeSinceHeardFromClient,	0,						0)	
SCRIPT_GET_DECLARE("clientAddr",						pyClientAddr,					0,						0)	
SCRIPT_GET_DECLARE("hasClient",							pyHasClient,					0,						0)	
SCRIPT_GET_DECLARE("clientEnabled",						pyGetClientEnabled,				0,						0)	
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Proxy, 0, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
Proxy::Proxy(ENTITY_ID id, const ScriptDefModule* pScriptModule):
Entity(id, pScriptModule, getScriptType(), true),
rndUUID_(KBEngine::genUUID64()),
addr_(Network::Address::NONE),
dataDownloads_(),
clientEnabled_(false),
bandwidthPerSecond_(0),
encryptionKey(),
pProxyForwarder_(NULL),
clientComponentType_(UNKNOWN_CLIENT_COMPONENT_TYPE),
loginDatas_(),
createDatas_()
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
PyObject* Proxy::pyDisconnect()
{
	Network::Channel* pChannel = Baseapp::getSingleton().networkInterface().findChannel(addr_);
	if (pChannel && !pChannel->isDestroyed())
	{
		pChannel->condemn("");
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
void Proxy::kick()
{
	// 如果被销毁频道仍然存活则将其关闭
	Network::Channel* pChannel = Baseapp::getSingleton().networkInterface().findChannel(addr_);
	if(pChannel && !pChannel->isDestroyed())
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(ClientInterface::onKicked);
		ClientInterface::onKickedArgs1::staticAddToBundle(*pBundle, SERVER_ERR_PROXY_DESTROYED);
		//pBundle->send(Baseapp::getSingleton().networkInterface(), pChannel);
		this->sendToClient(ClientInterface::onKicked, pBundle);
		this->sendToClient();
		pChannel->condemn("", true);
	}
}

//-------------------------------------------------------------------------------------
void Proxy::initClientBasePropertys()
{
	if(clientEntityCall() == NULL)
		return;

	MemoryStream* s1 = MemoryStream::createPoolObject(OBJECTPOOL_POINT);
	addClientDataToStream(s1);
	
	if(s1->wpos() > 0)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(ClientInterface::onUpdatePropertys);
		(*pBundle) << this->id();
		(*pBundle).append(*s1);
		sendToClient(ClientInterface::onUpdatePropertys, pBundle);
		//clientEntityCall()->sendCall((*pBundle));
	}

	MemoryStream::reclaimPoolObject(s1);
}

//-------------------------------------------------------------------------------------
void Proxy::initClientCellPropertys()
{
	if(clientEntityCall() == NULL)
		return;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
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
		(*pBundle) << (uint8)0 << aliasID << this->spaceID();
	}
	else
	{
		(*pBundle) << (ENTITY_PROPERTY_UID)0 << spaceuid << this->spaceID();
	}

	MemoryStream* s = MemoryStream::createPoolObject(OBJECTPOOL_POINT);

	// celldata获取客户端感兴趣的数据初始化客户端 如:ALL_CLIENTS
	try
	{
		addCellDataToStream(CLIENT_TYPE, ED_FLAG_ALL_CLIENTS|ED_FLAG_CELL_PUBLIC_AND_OWN|ED_FLAG_OWN_CLIENT, s, true);
	}
	catch (MemoryStreamWriteOverflow & err)
	{
		ERROR_MSG(fmt::format("{}::initClientCellPropertys({}): {}\n",
			scriptName(), id(), err.what()));

		MemoryStream::reclaimPoolObject(s);
		Network::Bundle::reclaimPoolObject(pBundle);
		return;
	}

	(*pBundle).append(*s);
	MemoryStream::reclaimPoolObject(s);
	//clientEntityCall()->sendCall((*pBundle));
	sendToClient(ClientInterface::onUpdatePropertys, pBundle);
}

//-------------------------------------------------------------------------------------
void Proxy::onClientEnabled(void)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	clientEnabled_ = true;
	CALL_COMPONENTS_AND_ENTITY_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onClientEnabled"), GETERR));
}

//-------------------------------------------------------------------------------------
int32 Proxy::onLogOnAttempt(const char* addr, uint32 port, const char* password)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	Py_INCREF(this);

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

	CALL_ENTITY_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS3(pyTempObj, const_cast<char*>("onLogOnAttempt"),
		const_cast<char*>("sks"),
		addr,
		port,
		password,
		GETERR));

	Py_DECREF(this);
	return ret;
}

//-------------------------------------------------------------------------------------
void Proxy::onClientDeath(void)
{
	if(clientEntityCall() == NULL)
	{
		ERROR_MSG(fmt::format("{}::onClientDeath: {}, channel is null!\n", 
			this->scriptName(), this->id()));

		return;
	}

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	DEBUG_MSG(fmt::format("{}::onClientDeath: {}.\n", 
		this->scriptName(), this->id()));

	Py_DECREF(clientEntityCall());
	clientEntityCall(NULL);
	addr(Network::Address::NONE);

	clientEnabled_ = false;
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onClientDeath"), GETERR));
}

//-------------------------------------------------------------------------------------
void Proxy::onClientGetCell(Network::Channel* pChannel, COMPONENT_ID componentID)
{	
	// 回调给脚本，获得了cell
	if(cellEntityCall_ == NULL)
		cellEntityCall_ = new EntityCall(pScriptModule_, NULL, componentID, id_, ENTITYCALL_TYPE_CELL);

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onClientGetCell"), GETERR));
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetClientType()
{
	return PyLong_FromLong((long)getClientType());
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetClientDatas()
{
	const std::string& datas1 = this->getLoginDatas();
	PyObject* pyDatas1 = PyBytes_FromStringAndSize(datas1.data(), datas1.size());

	const std::string& datas2 = this->getCreateDatas();
	PyObject* pyDatas2 = PyBytes_FromStringAndSize(datas2.data(), datas2.size());

	PyObject* pyDatas = PyTuple_New(2);
	PyTuple_SetItem(pyDatas, 0, pyDatas1);
	PyTuple_SetItem(pyDatas, 1, pyDatas2);

	return pyDatas;
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

	if (pyOterProxy == NULL || !PyObject_TypeCheck(pyOterProxy, Proxy::getScriptType()))
	{
		PyErr_Format(PyExc_AssertionError, "%s[%d]::giveClientTo: arg1 not is Proxy!\n",
			scriptName(), id());
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
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS0(pyTempObj, const_cast<char*>("onGiveClientToFailure"), GETERR));
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

	if(clientEntityCall_ == NULL || clientEntityCall_->getChannel() == NULL)
	{
		char err[255];
		kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: no has client.", scriptName());
		PyErr_SetString(PyExc_TypeError, err);
		PyErr_PrintEx(0);
		onGiveClientToFailure();
		return;
	}

	Network::Channel* lpChannel = clientEntityCall_->getChannel();

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

		EntityCall* mb = proxy->clientEntityCall();
		if(mb != NULL)
		{
			ERROR_MSG(fmt::format("Proxy::giveClientTo: {}[{}] give client to {}[{}], {} has clientEntityCall.\n", 
					scriptName(),
					id(),
					proxy->scriptName(), 
					proxy->id(),
					proxy->scriptName()));

			onGiveClientToFailure();
			return;
		}

		if(cellEntityCall())
		{
			// 当前这个entity如果有cell，说明已经绑定了witness， 那么既然我们将控制权
			// 交换给了另一个entity， 这个entity需要解绑定witness。
			// 通知cell丢失witness
			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			(*pBundle).newMessage(CellappInterface::onLoseWitness);
			(*pBundle) << this->id();
			sendToCellapp(pBundle);
		}

		// 既然客户端失去对其的控制, 那么通知client销毁这个entity
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(ClientInterface::onEntityDestroyed);
		(*pBundle) << this->id();
		sendToClient(ClientInterface::onEntityDestroyed, pBundle);

		// 将控制权交换
		clientEnabled_ = false;
		clientEntityCall()->addr(Network::Address::NONE);
		Py_DECREF(clientEntityCall());
		proxy->setClientType(this->getClientType());
		proxy->setLoginDatas(this->getLoginDatas());
		this->setClientType(UNKNOWN_CLIENT_COMPONENT_TYPE);
		this->setLoginDatas("");
		clientEntityCall(NULL);
		proxy->onGiveClientTo(lpChannel);
		addr(Network::Address::NONE);
	}
}

//-------------------------------------------------------------------------------------
void Proxy::onGiveClientTo(Network::Channel* lpChannel)
{
	clientEntityCall(new EntityCall(this->pScriptModule_, 
		&lpChannel->addr(), 0, id_, ENTITYCALL_TYPE_CLIENT));

	addr(lpChannel->addr());
	Baseapp::getSingleton().createClientProxies(this);

	// 如果有cell, 需要通知其获得witness， 因为这个客户端刚刚绑定到这个proxy
	// 此时这个entity即使有cell正常情况必须是没有witness的。
	onGetWitness();
}

//-------------------------------------------------------------------------------------
void Proxy::onGetWitness()
{
	if(cellEntityCall())
	{
		// 通知cell获得客户端
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(CellappInterface::onGetWitnessFromBase);
		(*pBundle) << this->id();
		sendToCellapp(pBundle);
	}
}

//-------------------------------------------------------------------------------------
double Proxy::getRoundTripTime() const
{
	if(clientEntityCall() == NULL || clientEntityCall()->getChannel() == NULL)
		return 0.0;

	return double(clientEntityCall()->getChannel()->getRTT()) / 1000000.0;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetRoundTripTime()
{ 
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		

		return 0;																				
	}

	return PyFloat_FromDouble(this->getRoundTripTime()); 
}

//-------------------------------------------------------------------------------------
double Proxy::getTimeSinceHeardFromClient() const
{
	if(clientEntityCall() == NULL || clientEntityCall()->getChannel() == NULL || 
		clientEntityCall()->getChannel()->pEndPoint() == NULL)
		return DBL_MAX;

	return double(timestamp() - clientEntityCall()->getChannel()->lastReceivedTime()) / stampsPerSecondD();
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetTimeSinceHeardFromClient()
{ 
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		

		return 0;																					
	}
	
	return PyFloat_FromDouble(this->getTimeSinceHeardFromClient()); 
}

//-------------------------------------------------------------------------------------
bool Proxy::hasClient() const
{
	if(clientEntityCall() == NULL || clientEntityCall()->getChannel() == NULL || 
		clientEntityCall()->getChannel()->pEndPoint() == NULL)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyHasClient()
{ 
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		

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
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		

		return 0;																				
	}

	PyObject* pyobj = PyTuple_New(2);

	if(clientEntityCall() == NULL || clientEntityCall()->getChannel() == NULL || 
		clientEntityCall()->getChannel()->pEndPoint() == NULL)
	{
		PyTuple_SetItem(pyobj, 0, PyLong_FromLong(0));
		PyTuple_SetItem(pyobj, 1, PyLong_FromLong(0));
	}
	else
	{
		const Network::Address& addr = clientEntityCall()->getChannel()->pEndPoint()->addr();
		PyTuple_SetItem(pyobj, 0, PyLong_FromUnsignedLong(addr.ip));
		PyTuple_SetItem(pyobj, 1, PyLong_FromUnsignedLong(addr.port));
	}

	return pyobj;
}

//-------------------------------------------------------------------------------------
PyObject* Proxy::pyGetClientEnabled()
{ 
	if (!hasFlags(ENTITY_FLAGS_DESTROYING) && isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "%s: %d is destroyed!\n",		
			scriptName(), id());		

		return 0;																				
	}

	if(this->clientEnabled())
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

	if(pobj->clientEntityCall() == NULL)
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
		if(!PyArg_ParseTuple(args, "O", &pyResourceName))
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 2)
	{
		if(!PyArg_ParseTuple(args, "O|O", &pyResourceName, &pyDesc))
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 3)
	{
		if(!PyArg_ParseTuple(args, "O|O|H", &pyResourceName, &pyDesc, &id))
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}

	const char* pDescr = NULL;

	if (pyDesc)
	{
		pDescr = PyUnicode_AsUTF8AndSize(pyDesc, NULL);
	}

	if(pDescr && strlen(pDescr) > 255)
	{
		PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: the descr-size(%d > 255)!", 
			strlen(pDescr));

		PyErr_PrintEx(0);
		return NULL;
	}

	int16 rid = pobj->streamFileToClient(pyResourceName, 
							(pDescr == NULL ? "" : pDescr),  
							id);

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

	if(pobj->clientEntityCall() == NULL)
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
		if(!PyArg_ParseTuple(args, "O", &pyData))
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamStringToClient: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 2)
	{
		if(!PyArg_ParseTuple(args, "O|O", &pyData, &pyDesc))
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamStringToClient: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if(currargsSize == 3)
	{
		if(!PyArg_ParseTuple(args, "O|O|H", &pyData, &pyDesc, &id))
		{
			PyErr_Format(PyExc_TypeError, "Proxy::streamStringToClient: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}

	const char* pDescr = NULL;

	if (pyDesc)
	{
		pDescr = PyUnicode_AsUTF8AndSize(pyDesc, NULL);
	}

	if(pDescr && strlen(pDescr) > 255)
	{
		PyErr_Format(PyExc_TypeError, "Proxy::streamFileToClient: the descr-size(%d > 255)!", 
			strlen(pDescr));

		PyErr_PrintEx(0);
		return NULL;
	}

	int16 rid = pobj->streamStringToClient(pyData, 
						(pDescr == NULL ? "" : pDescr),  
						id);

	if (rid != id)
	{
		WARNING_MSG(fmt::format("Proxy::streamFileToClient: the id({}) has been used, a new id({}) is assigned!\n", id, rid));
	}

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
Network::Channel* Proxy::pChannel()
{
	if(!clientEntityCall())
		return NULL;

	Network::Channel* pChannel = clientEntityCall()->getChannel();
	if(!pChannel)
		return NULL;
	
	return pChannel;
}

//-------------------------------------------------------------------------------------
bool Proxy::pushBundle(Network::Bundle* pBundle)
{
	if(!clientEntityCall())
		return false;

	Network::Channel* pChannel = clientEntityCall()->getChannel();
	if(!pChannel)
		return false;

	pBundle->pChannel(pChannel);
	pBundle->finiMessage(true);
	pChannel->pushBundle(pBundle);

	{
		// 如果数据大量阻塞发不出去将会报警
		//AUTO_SCOPED_PROFILE("pushBundleAndSendToClient");
		//pChannel->send(pBundle);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle)
{
	return sendToClient(pBundle);
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(Network::Bundle* pBundle, bool immediately)
{
	if (pushBundle(pBundle))
	{
		if (immediately)
			return sendToClient(false);
		else
			return true;
	}

	ERROR_MSG(fmt::format("Proxy::sendToClient: {} pBundles is NULL, not found channel.\n", id()));
	Network::Bundle::reclaimPoolObject(pBundle);
	return false;
}

//-------------------------------------------------------------------------------------
bool Proxy::sendToClient(bool expectData)
{
	if(!clientEntityCall())
		return false;

	Network::Channel* pChannel = clientEntityCall()->getChannel();
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
void Proxy::onStreamComplete(int16 id, bool success)
{
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	CALL_ENTITY_AND_COMPONENTS_METHOD(this, SCRIPT_OBJECT_CALL_ARGS2(pyTempObj, const_cast<char*>("onStreamComplete"),
		const_cast<char*>("hO"), id, success ? Py_True : Py_False, GETERR));
}

//-------------------------------------------------------------------------------------
}
