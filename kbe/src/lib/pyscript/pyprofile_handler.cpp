/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "pyprofile_handler.h"
#include "network/network_interface.h"
#include "network/event_dispatcher.h"
#include "network/address.h"
#include "network/network_stats.h"
#include "network/bundle.h"
#include "network/message_handler.h"
#include "pyscript/pyprofile.h"
#include "common/memorystream.h"
#include "helper/console_helper.h"
#include "helper/profile.h"
#include "server/serverconfig.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
PyProfileHandler::PyProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Network::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
	script::PyProfile::start(name_);
}

//-------------------------------------------------------------------------------------
PyProfileHandler::~PyProfileHandler()
{
	if(name_ != "kbengine" || !(g_componentType == BASEAPP_TYPE ? g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile : 
		g_kbeSrvConfig.getCellApp().profiles.open_pyprofile))
		script::PyProfile::remove(name_);
}

//-------------------------------------------------------------------------------------
void PyProfileHandler::timeout()
{
	if(name_ != "kbengine" || !(g_componentType == BASEAPP_TYPE ? g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile : 
		g_kbeSrvConfig.getCellApp().profiles.open_pyprofile))
		script::PyProfile::stop(name_);

	MemoryStream s;
	script::PyProfile::addToStream(name_, &s);

	if(name_ == "kbengine" && (g_componentType == BASEAPP_TYPE ? g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile : 
		g_kbeSrvConfig.getCellApp().profiles.open_pyprofile))
		script::PyProfile::start(name_);
	
	sendStream(&s);
}

//-------------------------------------------------------------------------------------
void PyProfileHandler::sendStream(MemoryStream* s)
{
	Network::Channel* pChannel = networkInterface_.findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(fmt::format("PyProfileHandler::sendStream: not found {} addr({})\n",
			name_, addr_.c_str()));
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	int8 type = 0;
	(*pBundle) << type;
	(*pBundle) << timinglen_;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------

}
