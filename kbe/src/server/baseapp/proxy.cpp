#include "baseapp.hpp"
#include "proxy.hpp"
#include "client_lib/client_interface.hpp"
#include "network/fixed_messages.hpp"

#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(Proxy)
SCRIPT_METHOD_DECLARE("giveClientTo",					pyGiveClientTo,					METH_VARARGS,			0)
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
addr_(Mercury::Address::NONE)
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
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onEntitiesEnabled"));
}

//-------------------------------------------------------------------------------------
int32 Proxy::onLogOnAttempt(const char* addr, uint32 port, const char* password)
{
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
	SCRIPT_OBJECT_CALL_ARGS0(this, const_cast<char*>("onGiveClientToFailure"));
}

//-------------------------------------------------------------------------------------
void Proxy::giveClientTo(Proxy* proxy)
{
	Mercury::Channel* lpChannel = clientMailbox_->getChannel();
	if(lpChannel == NULL)
	{
		char err[255];																				
		kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: no has client.\n", getScriptName());			
		PyErr_SetString(PyExc_TypeError, err);														
		PyErr_PrintEx(0);	
		onGiveClientToFailure();
		return;
	}

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
}
