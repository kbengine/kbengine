// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	int8 type = 0;
	(*pBundle) << type;
	(*pBundle) << timinglen_;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
PyTickProfileHandler::PyTickProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen,
	std::string name, const Network::Address& addr) :
	ProfileHandler(networkInterface, timinglen, name, addr)
{
	DEBUG_MSG(fmt::format("PyTickProfileHandler::PyTickProfileHandler(), name = {}, timinglen = {}\n", name_, timinglen));
	networkInterface_.dispatcher().addTask(this);
	script::PyProfile::start(name_);
}

//-------------------------------------------------------------------------------------
PyTickProfileHandler::~PyTickProfileHandler()
{
	DEBUG_MSG(fmt::format("PyTickProfileHandler::~PyTickProfileHandler(), name = {}\n", name_));
	networkInterface_.dispatcher().cancelTask(this);
	script::PyProfile::remove(name_);
}

//-------------------------------------------------------------------------------------
void PyTickProfileHandler::timeout()
{
	script::PyProfile::stop(name_);
}

//-------------------------------------------------------------------------------------
bool PyTickProfileHandler::process()
{
	MemoryStream s;
	script::PyProfile::stop(name_);
	script::PyProfile::addToStream(name_, &s);
	script::PyProfile::remove(name_);
	script::PyProfile::start(name_);
	sendStream(&s);
	return true;
}

//-------------------------------------------------------------------------------------
void PyTickProfileHandler::sendStream(MemoryStream* s)
{
	Network::Channel* pChannel = networkInterface_.findChannel(addr_);
	if (pChannel == NULL)
	{
		WARNING_MSG(fmt::format("PyTickProfileHandler::sendStream: not found {} addr({})\n",
			name_, addr_.c_str()));
		timeout();
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	int8 type = 4;
	(*pBundle) << type;
	(*pBundle) << timinglen_;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------

}
