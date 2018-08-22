// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "initprogress_handler.h"
#include "network/bundle.h"
#include "network/channel.h"

#include "../../server/cellappmgr/cellappmgr_interface.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
InitProgressHandler::InitProgressHandler(Network::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
delayTicks_(0)
{
	networkInterface.dispatcher().addTask(this);
}

//-------------------------------------------------------------------------------------
InitProgressHandler::~InitProgressHandler()
{
	// networkInterface_.dispatcher().cancelTask(this);
	DEBUG_MSG("InitProgressHandler::~InitProgressHandler()\n");
}

//-------------------------------------------------------------------------------------
bool InitProgressHandler::process()
{
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Network::Channel* pChannel = NULL;

	if(cts.size() > 0)
	{
		Components::COMPONENTS::iterator ctiter = cts.begin();
		if((*ctiter).pChannel == NULL)
			return true;

		pChannel = (*ctiter).pChannel;
	}

	if(pChannel == NULL)
		return true;

	if(Cellapp::getSingleton().idClient().size() == 0)
		return true;

	if(delayTicks_++ < 1)
		return true;

	float v = 0.0f;
	bool completed = false;

	if(PyObject_HasAttrString(Cellapp::getSingleton().getEntryScript().get(), "onReadyForLogin") > 0)
	{
		// 所有脚本都加载完毕
		PyObject* pyResult = PyObject_CallMethod(Cellapp::getSingleton().getEntryScript().get(), 
											const_cast<char*>("onReadyForLogin"), 
											const_cast<char*>("i"), 
											g_componentGroupOrder);

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

	(*pBundle).newMessage(CellappmgrInterface::onCellappInitProgress);
	(*pBundle) << g_componentID << v << g_componentGlobalOrder << g_componentGroupOrder;
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
