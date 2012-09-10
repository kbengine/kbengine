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
}

//-------------------------------------------------------------------------------------
Proxy::~Proxy()
{
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
		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onUpdatePropertys);
		bundle << this->getID();
		bundle.append(*s1);
		getClientMailbox()->postMail(bundle);
	}

	MemoryStream::ObjPool().reclaimObject(s1);
}

//-------------------------------------------------------------------------------------
void Proxy::initClientCellPropertys()
{
	if(getClientMailbox() == NULL)
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onUpdatePropertys);
	bundle << this->getID();

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
	ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

	Mercury::FixedMessages::MSGInfo* msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::position");
	if(msgInfo != NULL)
	{
		posuid = msgInfo->msgid;
		msgInfo = NULL;
	}

	msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");
	if(msgInfo != NULL)
	{
		diruid = msgInfo->msgid;
		msgInfo = NULL;
	}

	msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::spaceID");
	if(msgInfo != NULL)
	{
		spaceuid = msgInfo->msgid;
	}
	
	bundle << spaceuid << this->getSpaceID();
	uint32 posdirLen = 3;

#ifdef CLIENT_NO_FLOAT
	int32 x = (int32)v.x;
	int32 y = (int32)v.y;
	int32 z = (int32)v.z;
	
	
	bundle << posuid << posdirLen << x << y << z;

	x = (int32)v1.x;
	y = (int32)v1.y;
	z = (int32)v1.z;

	bundle << diruid << posdirLen << x << y << z;
#else
	bundle << posuid << posdirLen << v.x << v.y << v.z;
	bundle << diruid << posdirLen << v1.x << v1.y << v1.z;
#endif
	
	// celldata获取客户端感兴趣的数据初始化客户端 如:ALL_CLIENTS
	MemoryStream* s = MemoryStream::ObjPool().createObject();
	addCellDataToStream(ED_FLAG_ALL_CLIENTS|ED_FLAG_CELL_PUBLIC_AND_OWN|ED_FLAG_OWN_CLIENT, s);
	bundle.append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
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
void Proxy::onGiveClientToFailure()
{
	PyObject* pyResult = PyObject_CallMethod(this, const_cast<char*>("onGiveClientToFailure"), 
																		const_cast<char*>(""));
	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		PyErr_Clear();
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
			ERROR_MSG("Proxy::giveClientTo: %s[%ld] give client to %s[%ld], %s have clientMailbox.", 
					getScriptName(), 
					getID(), 
					proxy->getScriptName(), 
					proxy->getID(), 
					proxy->getScriptName());

			onGiveClientToFailure();
			return;
		}

		if(getCellMailbox())
		{
			// 通知cell丢失客户端
			Mercury::Bundle bundle;
			bundle.newMessage(CellappInterface::onLoseWitness);
			getCellMailbox()->postMail(bundle);
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
	setClientMailbox(new EntityMailbox(this->scriptModule_, &lpChannel->addr(), 0, id_, MAILBOX_TYPE_CLIENT));
	addr(lpChannel->addr());
	Baseapp::getSingleton().createClientProxies(this);

	if(getCellMailbox())
	{
		// 通知cell获得客户端
		Mercury::Bundle bundle;
		bundle.newMessage(CellappInterface::onGetWitness);
		getCellMailbox()->postMail(bundle);
	}
}

//-------------------------------------------------------------------------------------
}
