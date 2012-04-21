#include "bundle.hpp"
#include "network/network_interface.hpp"
#include "network/packet.hpp"
#include "network/channel.hpp"

#ifndef CODE_INLINE
#include "bundle.ipp"
#endif

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
Bundle::Bundle(Channel * pChannel):
	pChannel_(pChannel),
	packets_()
{
}

//-------------------------------------------------------------------------------------
Bundle::Bundle(Packet * p):
	pChannel_(NULL),
	packets_()
{
	packets_.push_back(p);
}

//-------------------------------------------------------------------------------------
Bundle::~Bundle()
{
	clear();
}

//-------------------------------------------------------------------------------------
void Bundle::clear()
{
	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
		delete (*iter);
	
	packets_.clear();
}

//-------------------------------------------------------------------------------------
void Bundle::send(NetworkInterface & networkInterface, Channel * pChannel)
{
	networkInterface.send(*this, pChannel);
}

//-------------------------------------------------------------------------------------
}
}