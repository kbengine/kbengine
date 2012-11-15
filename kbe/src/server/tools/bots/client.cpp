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
#include "bots.hpp"
#include "client.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "entitydef/scriptdef_module.hpp"


#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"


namespace KBEngine{

//-------------------------------------------------------------------------------------
Client::Client(const Mercury::Address& addr):
s_(0),
addr_(addr),
endpoint_()
{
}

//-------------------------------------------------------------------------------------
Client::~Client()
{
}

//-------------------------------------------------------------------------------------
bool Client::send(Mercury::Bundle& bundle)
{
	Mercury::Channel* pChannel = Bots::getSingleton().getNetworkInterface().findChannel(addr_);
	
	if(pChannel){
		bundle.send(Bots::getSingleton().getNetworkInterface(), pChannel);
	}
	else{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Client::initNetwork()
{
	endpoint_.socket(SOCK_STREAM);
	if (!endpoint_.good())
	{
		ERROR_MSG("Client::initNetwork: couldn't create a socket\n");
		return false;
	}
	
	ENGINE_COMPONENT_INFO& infos = g_kbeSrvConfig.getBots();
	return true;
}

//-------------------------------------------------------------------------------------
bool Client::process()
{
	return false;
}

//-------------------------------------------------------------------------------------
}
