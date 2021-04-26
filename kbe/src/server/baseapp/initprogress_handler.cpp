// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "baseapp.h"
#include "initprogress_handler.h"
#include "entity_autoloader.h"
#include "network/bundle.h"
#include "network/channel.h"

#include "../../server/baseappmgr/baseappmgr_interface.h"
#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
InitProgressHandler::InitProgressHandler(Network::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
delayTicks_(0),
pEntityAutoLoader_(NULL),
autoLoadState_(-1),
error_(false),
baseappReady_(false),
pendingConnectEntityApps_(),
startGlobalOrder_(0),
startGroupOrder_(0),
componentID_(0)
{
}

//-------------------------------------------------------------------------------------
InitProgressHandler::~InitProgressHandler()
{
	// networkInterface_.dispatcher().cancelTask(this);
	DEBUG_MSG("InitProgressHandler::~InitProgressHandler()\n");

	if(pEntityAutoLoader_)
	{
		pEntityAutoLoader_->pInitProgressHandler(NULL);
		pEntityAutoLoader_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
void InitProgressHandler::start()
{
	networkInterface_.dispatcher().addTask(this);
}

//-------------------------------------------------------------------------------------
void InitProgressHandler::setAutoLoadState(int8 state)
{ 
	autoLoadState_ = state; 

	if(state == 1)
		pEntityAutoLoader_ = NULL;
}

//-------------------------------------------------------------------------------------
void InitProgressHandler::onEntityAutoLoadCBFromDBMgr(Network::Channel* pChannel, MemoryStream& s)
{
	pEntityAutoLoader_->onEntityAutoLoadCBFromDBMgr(pChannel, s);
}

//-------------------------------------------------------------------------------------
void InitProgressHandler::setError()
{
	error_ = true;
}

//-------------------------------------------------------------------------------------
bool InitProgressHandler::sendRegisterNewApps()
{
	if (pendingConnectEntityApps_.size() == 0)
		return true;

	PendingConnectEntityApp& appInfos = pendingConnectEntityApps_.front();

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(appInfos.componentType, appInfos.uid, appInfos.componentID);
	if (!cinfos)
	{
		pendingConnectEntityApps_.erase(pendingConnectEntityApps_.begin());
		return true;
	}

	int ret = Components::getSingleton().connectComponent(appInfos.componentType, appInfos.uid, appInfos.componentID);
	if (ret == -1)
	{
		if (++appInfos.count > 10)
		{
			ERROR_MSG(fmt::format("InitProgressHandler::sendRegisterNewApps(): connect to {}({}) error!\n"));
			Baseapp::getSingleton().dispatcher().breakProcessing();
			return false;
		}
		else
		{
			return true;
		}
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	switch (appInfos.componentType)
	{
	case BASEAPP_TYPE:
		(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle),
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			networkInterface_.intTcpAddr().ip, networkInterface_.intTcpAddr().port,
			networkInterface_.extTcpAddr().ip, networkInterface_.extTcpAddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	case CELLAPP_TYPE:
		(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle),
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			networkInterface_.intTcpAddr().ip, networkInterface_.intTcpAddr().port,
			networkInterface_.extTcpAddr().ip, networkInterface_.extTcpAddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
		break;
	};

	cinfos->pChannel->send(pBundle);
	pendingConnectEntityApps_.erase(pendingConnectEntityApps_.begin());
	return true;
}
//-------------------------------------------------------------------------------------
bool InitProgressHandler::process()
{
	if(error_)
	{
		Baseapp::getSingleton().dispatcher().breakProcessing();
		return false;
	}

	Network::Channel* pChannel = Components::getSingleton().getBaseappmgrChannel();

	if(pChannel == NULL)
		return true;

	if(Baseapp::getSingleton().idClient().size() == 0)
		return true;

	if (pendingConnectEntityApps_.size() > 0)
	{
		return sendRegisterNewApps();
	}

	if(delayTicks_++ < 1)
		return true;

	// 只有第一个baseapp上会创建EntityAutoLoader来自动加载数据库实体
	if(g_componentGroupOrder == 1)
	{
		if(autoLoadState_ == -1)
		{
			autoLoadState_ = 0;
			pEntityAutoLoader_ = new EntityAutoLoader(networkInterface_, this);
			return true;
		}
		else if(autoLoadState_ == 0)
		{
			// 必须等待EntityAutoLoader执行完毕
			// EntityAutoLoader执行完毕会设置autoLoadState_ = 1
			if(!pEntityAutoLoader_->process())
				setAutoLoadState(1);
			
			return true;
		}
	}

	pEntityAutoLoader_ = NULL;

	if(!baseappReady_)
	{
		baseappReady_ = true;

		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		// 所有脚本都加载完毕
		PyObject* pyResult = PyObject_CallMethod(Baseapp::getSingleton().getEntryScript().get(), 
											const_cast<char*>("onBaseAppReady"), 
											const_cast<char*>("O"), 
											PyBool_FromLong((g_componentGroupOrder == 1) ? 1 : 0));

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();

		return true;
	}

	float v = 0.0f;
	bool completed = false;

	if(PyObject_HasAttrString(Baseapp::getSingleton().getEntryScript().get(), "onReadyForLogin") > 0)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		// 回调获得是否能够登录
		PyObject* pyResult = PyObject_CallMethod(Baseapp::getSingleton().getEntryScript().get(), 
											const_cast<char*>("onReadyForLogin"), 
											const_cast<char*>("O"), 
											PyBool_FromLong((g_componentGroupOrder == 1) ? 1 : 0));

		if(pyResult != NULL)
		{
			completed = (pyResult == Py_True);
			
			if(!completed)
			{
				v = (float)PyFloat_AsDouble(pyResult);
				if (PyErr_Occurred())
				{
					SCRIPT_ERROR_CHECK();
					Py_DECREF(pyResult);
					return true;
				}
			}
			else
			{
				v = 100.f;
			}

			Py_DECREF(pyResult);
		}
		else
		{
			SCRIPT_ERROR_CHECK();
			return true;
		}
	}
	else
	{
		v = 100.f;
		completed = true;
	}
	
	if(v >= 0.9999f)
	{
		v = 100.f;
		completed = true;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	(*pBundle).newMessage(BaseappmgrInterface::onBaseappInitProgress);
	(*pBundle) << g_componentID << v;
	pChannel->send(pBundle);

	if(completed)
	{
		delete this;
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
