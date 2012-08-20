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
Proxy::Proxy(ENTITY_ID id, ScriptModule* scriptModule):
Base(id, scriptModule, getScriptType(), true),
rndUUID_(KBEngine::genUUID64()),
addr_(Mercury::Address::NONE)
{
}

//-------------------------------------------------------------------------------------
Proxy::~Proxy()
{
	ENTITY_DECONSTRUCTION(Proxy);
}

//-------------------------------------------------------------------------------------
void Proxy::initClientPropertys()
{
	Mercury::Bundle bundle;

	// 初始化cellEntity的位置和方向变量
	Vector3 v;
	PyObject* cellData = getCellData();
	KBE_ASSERT(cellData != NULL);

	PyObject* position = PyDict_GetItemString(cellData, "position");
	script::ScriptVector3::convertPyObjectToVector3(v, position);

	Vector3 v1;
	PyObject* direction = PyDict_GetItemString(cellData, "direction");
	script::ScriptVector3::convertPyObjectToVector3(v1, direction);
	
	ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
	ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
	
	Mercury::FixedMessages::MSGInfo* msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::position");
	if(msgInfo != NULL)
	{
		posuid = msgInfo->msgid;
		msgInfo = NULL;
	}

	msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");
	if(msgInfo != NULL)
	{
		posuid = msgInfo->msgid;
	}

	bundle.newMessage(ClientInterface::onUpdatePropertys);
	
	bundle << getID();

#ifdef CLIENT_NO_FLOAT
	int32 x = (int32)v.x;
	int32 y = (int32)v.x;
	int32 z = (int32)v.x;
	
	
	bundle << posuid << x << y << z;

	x = (int32)v1.x;
	y = (int32)v1.x;
	z = (int32)v1.x;

	bundle << diruid << x << y << z;
#else
	bundle << posuid << v.x << v.y << v.z;
	bundle << diruid << v1.x << v1.y << v1.z;
#endif
	
	// celldata获取客户端感兴趣的数据初始化客户端 如:ALL_CLIENTS
	MemoryStream s;
	getCellDataByFlags(ED_FLAG_ALL_CLIENTS|ED_FLAG_CELL_PUBLIC_AND_OWN|ED_FLAG_OWN_CLIENT, &s);
	bundle.append(s);

	MemoryStream s1;
	getClientPropertys(&s1);
	bundle.append(s1);

	getClientMailbox()->postMail(bundle);
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
	DEBUG_MSG("%s::onClientDeath: %d.\n", this->getScriptName(), this->getID());

	if(getClientMailbox() != NULL)
	{
		Py_DECREF(getClientMailbox());
		setClientMailbox(NULL);
		addr(Mercury::Address::NONE);
	}

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
		kbe_snprintf(err, 255, "Proxy[%s]::giveClientTo: no has client.\n", getScriptName());			
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
		//Mercury::Bundle bundle;
		//bundle.newMessage(ClientInterface::onEntityDestroyed);
		//ClientInterface::onEntityLeaveWorldArgs1::staticAddToBundle(bundle, id_);
		//bundle.send(Baseapp::getSingleton().getNetworkInterface(), lpChannel);
		
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
	setClientMailbox(new EntityMailbox(this->scriptModule_, &lpChannel->addr(), 0, id_, MAILBOX_TYPE_CLIENT));
	addr(lpChannel->addr());
	Baseapp::getSingleton().createClientProxies(this);
}

//-------------------------------------------------------------------------------------
}
