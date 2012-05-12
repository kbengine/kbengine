#include "cellappmgr.hpp"
#include "cellappmgr_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "network/broadcast_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Cellappmgr);

//-------------------------------------------------------------------------------------
Cellappmgr::Cellappmgr(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
	ServerApp(dispatcher, ninterface, componentType)
{
}

//-------------------------------------------------------------------------------------
Cellappmgr::~Cellappmgr()
{
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::run()
{
	bool ret = true;

	while(!this->getMainDispatcher().isBreakProcessing())
	{
		this->getMainDispatcher().processOnce(false);
		KBEngine::sleep(100);
	};

	return ret;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::handleTimeout(TimerHandle handle, void * arg)
{
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Cellappmgr::initializeEnd()
{
	// 广播自己的地址给网上上的所有kbemachine
	// 无需关心new 对象的释放， 内部会自动释放。
	new Mercury::BroadcastInterface(getNetworkInterface(), 
		componentType(), componentID());

	return true;
}

//-------------------------------------------------------------------------------------
void Cellappmgr::finalise()
{
}

//-------------------------------------------------------------------------------------

}
