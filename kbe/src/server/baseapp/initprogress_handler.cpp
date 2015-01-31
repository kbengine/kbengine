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

#include "baseapp.h"
#include "initprogress_handler.h"
#include "entity_autoloader.h"
#include "network/bundle.h"
#include "network/channel.h"

#include "../../server/baseappmgr/baseappmgr_interface.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
InitProgressHandler::InitProgressHandler(Network::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
delayTicks_(0),
pEntityAutoLoader_(NULL),
autoLoadState_(-1)
{
	networkInterface.dispatcher().addTask(this);
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
bool InitProgressHandler::process()
{
	Network::Channel* pChannel = Components::getSingleton().getBaseappmgrChannel();

	if(pChannel == NULL)
		return true;

	if(Baseapp::getSingleton().idClient().getSize() == 0)
		return true;

	if(delayTicks_++ < 1)
		return true;

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
			return true;
		}
	}

	pEntityAutoLoader_ = NULL;

	float v = 0.0f;
	bool completed = false;

	if(PyObject_HasAttrString(Baseapp::getSingleton().getEntryScript().get(), "onReadyForLogin") > 0)
	{
		// 所有脚本都加载完毕
		PyObject* pyResult = PyObject_CallMethod(Baseapp::getSingleton().getEntryScript().get(), 
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

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

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
