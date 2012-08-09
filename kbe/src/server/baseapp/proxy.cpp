#include "baseapp.hpp"
#include "proxy.hpp"
#include "client_lib/client_interface.hpp"

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
Proxy::Proxy(ENTITY_ID id, ScriptModule* scriptModule):
Base(id, scriptModule, getScriptType(), true),
rndUUID_(KBEngine::genUUID64()),
addr_(Mercury::Address::NONE)
{
}

//-------------------------------------------------------------------------------------
Proxy::~Proxy()
{
}

//-------------------------------------------------------------------------------------
void Proxy::onEntitiesEnabled(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onEntitiesEnabled"), const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Proxy::onLogOnAttempt(std::string& addr, uint32& port, std::string& password)
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onLogOnAttempt"), const_cast<char*>("sk"), 
		PyBytes_FromString(addr.c_str()), 
		PyLong_FromLong(port),
		PyBytes_FromString(password.c_str())
	);
	
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Proxy::onClientDeath(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onClientDeath"), const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Proxy::onClientGetCell(Mercury::Channel* pChannel)
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onClientGetCell"), const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
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
void Proxy::giveClientTo(Proxy* proxy)
{
	Mercury::Channel* lpChannel = clientMailbox_->getChannel();
	if(lpChannel == NULL)
	{
		char err[255];																				
		sprintf(err, "Proxy[%s]::giveClientTo: no has client.\n", getScriptName());			
		PyErr_SetString(PyExc_TypeError, err);														
		PyErr_PrintEx(0);	
		return;
	}

	if(proxy)
	{
		EntityMailbox* mb = proxy->getClientMailbox();
		if(mb != NULL)
		{
			ERROR_MSG("Proxy::giveClientTo: %s[%ld] give client to %s[%ld], %s have clientMailbox.", 
					getScriptName(), 
					getID(), 
					proxy->getScriptName(), 
					proxy->getID(), 
					proxy->getScriptName());
			return;
		}

		// 通知客户端销毁本entity
		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onEntityDestroyed);
		ClientInterface::onEntityLeaveWorldArgs1::staticAddToBundle(bundle, id_);
		bundle.send(Baseapp::getSingleton().getNetworkInterface(), lpChannel);

		Py_DECREF(getClientMailbox());
		proxy->onGiveClientTo(lpChannel);
		getClientMailbox()->addr(Mercury::Address::NONE);
		setClientMailbox(NULL);
		addr(Mercury::Address::NONE);
	}
}

//-------------------------------------------------------------------------------------
void Proxy::onGiveClientTo(Mercury::Channel* lpChannel)
{
	setClientMailbox(new EntityMailbox(this->scriptModule_, &lpChannel->addr(), 0, id_, MAILBOX_TYPE_CLIENT));
	addr(lpChannel->addr());
	Baseapp::getSingleton().createClientProxies(this);
}

//-------------------------------------------------------------------------------------
}
