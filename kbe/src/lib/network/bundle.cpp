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
Bundle::Bundle(uint8 spareSize, Channel * pChannel)
{
}

Bundle::Bundle(Packet * p)
{
}

Bundle::~Bundle()
{
}


void Bundle::clear(bool newBundle)
{
}

int Bundle::size() const
{
	int	total = 0;
	return total;
}

void Bundle::send(NetworkInterface & networkInterface, Channel * pChannel)
{
	networkInterface.send(*this, pChannel);
}


}
}