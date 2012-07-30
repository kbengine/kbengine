#include "baseapp.hpp"
#include "proxy.hpp"

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
Base(id, scriptModule, getScriptType(), true)
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
void Proxy::onClientGetCell(void)
{
	PyObject* pyResult = PyObject_CallMethod(this, 
		const_cast<char*>("onClientGetCell"), const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Proxy::giveClientTo(Proxy* proxy)
{
	/*
	Mercury::Channel* lpChannel = clientMailbox_->getChannel();
	clientMailbox_->setChannel(NULL);
	
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

		proxy->onGiveClientToMe(lpChannel);
	}
	
	// 通知客户端销毁本entity
	SocketPacket* sp = new SocketPacket(OP_DESTROY_CLIENT_ENTITY, 16);
	(*sp) << id_;
	lpChannel->sendPacket(sp);
	*/
}

//-------------------------------------------------------------------------------------
void Proxy::onGiveClientToMe(Mercury::Channel* lpChannel)
{
	//Baseapp::getSingleton().createClientProxyEntity(lpChannel, this);
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
}
